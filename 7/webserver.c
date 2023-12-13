#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 3490 /* default port */
#define QLEN 10 /* request queue size */
#define BUF_SIZE 1024

typedef struct {
    char *key;
    char *value;
} KeyValuePair;

KeyValuePair *params;

int num_params;
char html[1024];

void *handle_clnt(int sockfd);
void send_err(int sockfd);
void send_msg(int sockfd);
void create_html();
KeyValuePair *parse_post_data(char *post_data);

int main(int argc, char *argv[]) {
    int sockfd, new_fd; /* listen on sock_fd, new connection on new_fd */
    struct sockaddr_in server_addr; /* structure to hold server's address */
    struct sockaddr_in client_addr; /* structure to hold client's address */
    int alen; /* length of address */

    fd_set readfds, activefds; /* the set of read descriptors */

    int i, maxfd = 0, numbytes;
    char buf[100];

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket() failed");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind() failed");
        exit(1);
    }

    /* Specify a size of request queue */
    if (listen(sockfd, QLEN) < 0) {
        fprintf(stderr,"listen failed\n");
        exit(1);
    }

    alen = sizeof(client_addr);

    /* Initialize the set of active sockets. */
    FD_ZERO (&activefds);
    FD_SET (sockfd, &activefds);
    maxfd = sockfd;

    /* Main server loop - accept and handle requests */
    fprintf(stderr, "Server up and running.\n");
    while (1) {
        printf("SERVER: Waiting for contact..., %d\n", maxfd);
        /* Block until input arrives on one or more active sockets. */
        readfds = activefds;
        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror ("select");
            exit (EXIT_FAILURE);
        }

        /* Service all the sockets with input pending. */
        for (i = 0; i <= maxfd; i++) {
            if (FD_ISSET(i, &readfds)) {
                if (i == sockfd) {
                    if ( (new_fd=accept(sockfd, (struct sockaddr *)&client_addr, &alen)) < 0) {
                        fprintf(stderr, "accept failed\n");
                        exit (1);
                    }

                    FD_SET(new_fd, &activefds); //add the new socket desc to our active connections set
                    if (new_fd > maxfd)
                    maxfd = new_fd;
                }
                else {
                    printf("handle clnt\n");
                    handle_clnt(i);
                    close(i);
                    FD_CLR(i, &activefds);
                }
            }
        }
    }
    close(sockfd);
}
void *handle_clnt(int client_sock) {
    int i;
    int recv=0, str_len=0;
    int readcnt=0; // read count
    char msg[BUF_SIZE]; // 메시지 변수
    char msg_copy[BUF_SIZE];
    char method[10];

    if ((str_len=read(client_sock, &msg[recv], BUF_SIZE)) == -1) {
        printf("read() error!\n");
        exit(1);
    }

    recv += str_len;
    msg[recv] = '\0';
    printf("%s\n", msg);

    strcpy(msg_copy, msg);

    strcpy(method, strtok(msg, " "));
    if (strcmp(method, "GET") == 0)
        send_msg(client_sock); // 메시지 전송(이름+메시지)
    else if (strcmp(method, "POST") == 0) {
        parse_post_data(msg_copy);
        recv_msg(client_sock);
    }
    else
        send_err(client_sock);

    return NULL;
}

void send_err(int client_sock) {// send to all
    
    char protocol[] = "HTTP/1.1 400 Bad Request\r\n";
    char server[] = "Server:Netscape-Enterprise/6.0\r\n";
    char contenttype[] = "Content-Type:text/html\r\n";
    char html[] = "<html><head>BAD Connection</head><body><H1>Bad Request</H1></body></html>\r\n";
    char end[]="\r\n";
    printf("send err\n");
    write(client_sock, protocol, strlen(protocol));
    write(client_sock, server, strlen(server));
    write(client_sock, contenttype, strlen(contenttype));
    write(client_sock, end, strlen(end));
    write(client_sock, html, strlen(html));
    fflush(fdopen(client_sock, "w"));
}

void send_msg(int client_sock)  {// send to all
    create_html();
    int html_len = strlen(html);

    printf("send msg, len=%d\n", html_len);
    char protocol[] = "HTTP/1.1 200 OK\r\n";
    char server[] = "Server:Netscape-Enterprise/6.0\r\n";
    char contentlength[500];
    snprintf(contentlength, sizeof(contentlength), "Content-Length: %d\r\n", html_len);
    char contenttype[] = "Content-Type:text/html\r\n";
    char end[]="\r\n";


    write(client_sock, protocol, strlen(protocol));
    write(client_sock, server, strlen(server));
    write(client_sock, contentlength, strlen(contentlength));
    write(client_sock, contenttype, strlen(contenttype));
    write(client_sock, end, strlen(end));
    write(client_sock, html, strlen(html));

    fflush(fdopen(client_sock, "w"));
}

void recv_msg(int client_sock) {
    char msg[1024];
    char protocol[] = "HTTP/1.1 201 OK\r\n";
    char contenttype[] = "application/x-www-form-urlencoded\r\n";
    char end[]="\r\n";
    
    write(client_sock, protocol, strlen(protocol));
    write(client_sock, contenttype, strlen(contenttype));
    write(client_sock, end, strlen(end));
}

KeyValuePair *parse_post_data(char *post_data) {
    char *token, *body;

    while(num_params != 0) {
        free(params++);
        num_params--;
    }


    params = NULL;
    int params_capacity = 10;  // 초기 용량
    num_params = 0;

    // 메모리 할당
    params = (KeyValuePair *)malloc(params_capacity * sizeof(KeyValuePair));

    if (params == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        exit(EXIT_FAILURE);
    }


    // key-value 파싱
    if (strstr(post_data, "Content-Type: application/x-www-form-urlencoded") != NULL) {
        char *body_start = strstr(post_data, "\r\n\r\n");
        if (body_start != NULL) {
            body = strdup(body_start + 4);
        }
    } 

    token = strtok(body, "&");
    while (token != NULL) {
        // 메모리 할당
        if (num_params >= params_capacity) {
            params_capacity *= 2;
            params = (KeyValuePair *)realloc(params, params_capacity * sizeof(KeyValuePair));
            if (params == NULL) {
                fprintf(stderr, "Memory allocation error\n");
                exit(EXIT_FAILURE);
            }
        }

        // key와 value 구분
        char *equal_sign = strchr(token, '=');
        if (equal_sign != NULL) {
            *equal_sign = '\0';
            params[num_params].key = strdup(token);
            params[num_params].value = strdup(equal_sign + 1);
            num_params++;
        }

        // 다음 토큰
        token = strtok(NULL, "&");
    }

    return params;
}

void create_html() {
    snprintf(html, sizeof(html), "<html><head>Hello World</head><body>");

    for (int i = 0; i < num_params; i++) {
        char h1[100];
        snprintf(h1, sizeof(h1), "<H1>%s is %s<H1>", params[i].key, params[i].value);

        strncat(html, h1, sizeof(html) - strlen(html) - 1);
    }

    strncat(html, "</body></html>\r\n", sizeof(html) - strlen(html) - 1);
    printf("html : %s\n", html);
}

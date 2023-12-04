#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_CLIENTS 3

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond_server = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_client = PTHREAD_COND_INITIALIZER;

typedef struct {
    int client_id;
    int state; // 방송이 되었는 지의 상태
    char message[100];
} MessageInfo;

MessageInfo messageinfo;

void* server_thread(void* arg) {
    int status, i;

    while (1) {
        status = pthread_mutex_lock(&mutex);
        if (status != 0) {
            perror("mutex lock");
            exit(1);
        }
  
        while (messageinfo.state == 0 && status == 0)
            status = pthread_cond_wait(&cond_server, &mutex);
        if (status != 0) {
            perror("cond wait");
            exit(1);
        }
        // 모든 클라이언트에게 메시지 방송
        for (i = 0; i < MAX_CLIENTS; i++) {
            printf("\nBroadcasting message to client%d : %s", i, messageinfo.message);
        }
        messageinfo.state = 0;

        status = pthread_cond_signal(&cond_client);
        if (status != 0) {
            perror("signal");
            exit(1);
        }

        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

void* client_thread(void* arg) {
    int client_id = *((int*)arg);
    int status;
    char message[100];

    while (1) {
        status = pthread_mutex_lock(&mutex);
        if (status != 0) {
            perror("mutex lock");
            pthread_mutex_unlock(&mutex);
            exit(1);
        }

        while(messageinfo.state != 0 && status == 0) {
            status = pthread_cond_wait(&cond_client, &mutex);
        }
        if (status != 0) {
            pthread_mutex_unlock(&mutex);
            return NULL;
        }

        printf("\nEnter message to broadcast (Client %d): ", client_id);
        fgets(message, sizeof(message), stdin);

        // 클라이언트 정보 업데이트
        messageinfo.client_id = client_id;
        messageinfo.state = 1;
        snprintf(messageinfo.message, sizeof(messageinfo.message), "%s", message);

        // 조건 변수 신호 전송
        status = pthread_cond_signal(&cond_server);
        if (status != 0) {
            perror("signal");
            return NULL;
        }

        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_t server_tid;
    pthread_t client_tid[MAX_CLIENTS];
    int client_id[MAX_CLIENTS];
    int status;

    status = pthread_create(&server_tid, NULL, server_thread, NULL);
    if (status != 0) {
        perror("create server thread");
        pthread_mutex_unlock(&mutex);
        exit(1);
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_id[i] = i;
        status = pthread_create(&client_tid[i], NULL, client_thread, &client_id[i]);
        if (status != 0) {
            fprintf(stderr, "create client thread %d", i);
            exit(1);
        }
    }

    pthread_join(server_tid, NULL);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        pthread_join(client_tid[i], NULL);
    }

    return 0;
}

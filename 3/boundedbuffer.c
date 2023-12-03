#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFFER_SIZE 20
#define NUM_THREADS 3

typedef struct {
    char *data[BUFFER_SIZE];
    int totalitems;
    int in, out;
    pthread_mutex_t mutex;
    pthread_cond_t full;
    pthread_cond_t empty;
} buffer_t;

pthread_mutex_t rmutex = PTHREAD_MUTEX_INITIALIZER;

buffer_t bb = { {0,}, 0, 0, 0,
PTHREAD_MUTEX_INITIALIZER,
PTHREAD_COND_INITIALIZER,
PTHREAD_COND_INITIALIZER};

int produce_item (int tnum) {
    int item = (int) (100.0*rand()/(RAND_MAX+1.0));

    sleep((unsigned long) (1.0*rand()/(RAND_MAX+1.0)));

    printf("%d. produce_item: item=%d\n", tnum, item);

    return item;
}

void *producer(void *rfd) {
    char data[16];
    
    while (1) {
        read_file((int)rfd, &data);
        insert_item(data);
    }
}
void read_file(int rfd, char **data) {
    int nread;

    pthread_mutex_lock(&rmutex);
    if ((nread = read(rfd, *data, 16)) <= 0) {
        pthread_exit((void *)0);
    }
    pthread_mutex_unlock(&rmutex);
}
int insert_item (char *data) {
    int status;

    status = pthread_mutex_lock(&bb.mutex);
    if (status != 0)
        return status;

    while (bb.totalitems >= BUFFER_SIZE && status == 0)
        status = pthread_cond_wait(&bb.empty, &bb.mutex);
    if (status != 0) {
        pthread_mutex_unlock(&bb.mutex);
        return status;
    }
    
    bb.data[bb.in] = data;
    bb.in = (bb.in + 1) % BUFFER_SIZE;
    bb.totalitems++;

    if (status = pthread_cond_signal(&bb.full)) {
        pthread_mutex_unlock (&bb.mutex);
        return status;
    }
    return pthread_mutex_unlock(&bb.mutex);
}

void * consumer(void *arg) {
    int fd;
    char *data;
    char filenum = (char)arg;
    char filename[10] = "file";
    strcat(filename, &filenum);

    if ((fd = open(filename, O_CREAT | O_TRUNC, 0644))) {
        perror("creat");
        exit(1);
    }
    while (1) {
        remove_item(&data);
        consume_item(data, (int)arg);
    }
}

void consume_item (int item, int tnum) {
    //sleep((unsigned long)(1.0*rand()/(RAND_MAX+1.0)));
    printf("\t\t%d. consume_item: item=%d\n", tnum, item);
}

int remove_item (char **data) {
    int status;

    status = pthread_mutex_lock (&bb.mutex);
    if (status != 0)
        return status;

    while (bb.totalitems <= 0 && status == 0)
        status = pthread_cond_wait (&bb.full, &bb.mutex);
    if (status != 0) {
        pthread_mutex_unlock(&bb.mutex);
        return status;

    }
    *data = bb.data[bb.out];
    bb.out = (bb.out + 1) % BUFFER_SIZE;
    bb.totalitems--;

    if (status = pthread_cond_signal(&bb.empty)) {
        pthread_mutex_unlock (&bb.mutex);
        return status;
    }
    return pthread_mutex_unlock(&bb.mutex);
}

int main () {
    int status, i, rfd, wfd;
    void *result;
    pthread_t producer_tid[NUM_THREADS], consumer_tid[NUM_THREADS];

    if ((rfd = open("file.txt", O_RDONLY)) == -1) {
        perror("open");
        exit(1);
    }
    if ((wfd = open("result.txt", O_CREAT | O_TRUNC, 0644)) == -1) {
        peror("creat");
        exit(1);
    }

    for (i = 0; i < NUM_THREADS; i++) {
        status = pthread_create (&producer_tid[i], NULL, producer, (void *)rfd);
        if (status != 0)
            perror ("Create producer thread");
    }

    for (i = 0; i < NUM_THREADS; i++) {
        status = pthread_create (&consumer_tid[i], NULL, consumer, (void *)wfd);
        if (status != 0)
            perror ("Create consumer thread");
    }

    for (i = 0; i < NUM_THREADS; i++) {
        status = pthread_join (producer_tid[i], NULL);
        if (status != 0)
            perror ("Join producer thread");
    }

    for (i = 0; i < NUM_THREADS; i++) {
        status = pthread_join (consumer_tid[i], NULL);
        if (status != 0)
            perror ("Join consumer thread");
    }
}
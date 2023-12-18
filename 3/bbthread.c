#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BUFFER_SIZE 20
#define NUMITEMS 30

typedef struct {
    int item[BUFFER_SIZE];
    int totalitems;
    int in, out;
    pthread_mutex_t mutex;
    pthread_cond_t full;
    pthread_cond_t empty;
} buffer_t;

buffer_t bb = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 0, 0, 0,
               PTHREAD_MUTEX_INITIALIZER,
               PTHREAD_COND_INITIALIZER,
               PTHREAD_COND_INITIALIZER};

int produce_item(void *arg) {
    int item = (int)(100.0 * rand() / (RAND_MAX + 1.0));
    sleep((unsigned long)(5.0 * rand() / (RAND_MAX + 1.0)));
    printf("%d:produce_item: item=%d\n", (int)arg, item);
    return item;
}

int insert_item(int item) {
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

    bb.item[bb.in] = item;
    bb.in = (bb.in + 1) % BUFFER_SIZE;
    bb.totalitems++;

    if (status = pthread_cond_signal(&bb.full)) {
        pthread_mutex_unlock(&bb.mutex);
        return status;
    }
    return pthread_mutex_unlock(&bb.mutex);
}

int consume_item(int item, void *arg) {
    sleep((unsigned long)(5.0 * rand() / (RAND_MAX + 1.0)));
    printf("\t\t%dconsume_item: item=%d\n", (int)arg, item);
    return 0;
}

int remove_item(int *temp) {
    int status;
    status = pthread_mutex_lock(&bb.mutex);

    if (status != 0)
        return status;
    while (bb.totalitems <= 0 && status == 0)
        status = pthread_cond_wait(&bb.full, &bb.mutex);
    if (status != 0) {
        pthread_mutex_unlock(&bb.mutex);
        return status;
    }

    *temp = bb.item[bb.out];
    bb.out = (bb.out + 1) % BUFFER_SIZE;
    bb.totalitems--;
    if (status = pthread_cond_signal(&bb.empty)) {
        pthread_mutex_unlock(&bb.mutex);
        return status;
    }
    return pthread_mutex_unlock(&bb.mutex);
}

void *producer(void *arg) {
    int item;
    while (1) {
        item = produce_item(arg);
        insert_item(item);
    }
}

void *consumer(void *arg) {
    int item;
    while (1) {
        remove_item(&item);
        consume_item(item, arg);
    }
}

int main() {
    int status;
    void *result;
    pthread_t producer_tid[2], consumer_tid[2];

    // 쓰레드 생성 
    for (int i = 0; i < 2; ++i) {
        status = pthread_create(&producer_tid[i], NULL, producer, (void *)i);
        if (status != 0)
            perror("Create producer thread");

        status = pthread_create(&consumer_tid[i], NULL, consumer, (void *)i);
        if (status != 0)
            perror("Create consumer thread");
    }

    for (int i = 0; i < 2; ++i) {
        status = pthread_join(producer_tid[i], NULL);
        if (status != 0)
            perror("Join producer thread");

        status = pthread_join(consumer_tid[i], NULL);
        if (status != 0)
            perror("Join consumer thread");
    }

    return 0;
}

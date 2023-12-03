#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

void *thread(void *arg) {
    int i;

    if (((int)arg % 2) == 0) {
        pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    }
    else
        pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    for (i = (int)arg; i > 0; i--) {
        sleep(1);

        arg = (void *)i;
        pthread_testcancel();
    }
    pthread_exit(arg);
}

int main(int argc, char *argv[]) {
    pthread_t tid;
    pthread_attr_t attr;
    int status, arg;
    void *result;

    if (argc < 2) {
        fprintf(stderr, "Usage: thread <number>\n");
        exit(1);
    }
    arg = atoi(argv[1]);

    pthread_attr_init(&attr);
    status = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    if (status != 0) {
        fprintf(stderr, "setdetachstate\n");
        exit(1);
    }

    status = pthread_create(&tid, &attr, thread, (void *)arg);
    if (status != 0) {
        fprintf(stderr, "create\n");
        exit(1);
    }
    pthread_attr_destroy(&attr);

    pthread_cancel(tid);
    
    status = pthread_join(tid, &result);
    if (status != 0) {
        fprintf(stderr, "join\n");
        exit(1);
    }

    printf("%d\n", (int)result);
}
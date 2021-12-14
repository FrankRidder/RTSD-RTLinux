#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#include <termios.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

// Terminal variables
static struct termios term;

static pthread_t thread;

static void *taskOne();
static void *taskTwo();
static void *taskThree();

static void terminate();
static void initialiseTerminal();
static void createThreads();
static void sighandler(int sig_num);

static void create_timer();

static sigset_t sset;
static int sig;

int main() {
    createThreads(); // Create threads
    terminate(); // Wait for threads to end and terminate program
    return 0;
}

void createThreads() {
    long i = 0;

    pthread_attr_t tattr;

    pthread_attr_init(&tattr); //tattr init met defaultwaarden

    //int status = pthread_create(&thread, &tattr, taskOne, (void *) i);    //Create threads
    //int status = pthread_create(&thread, &tattr, taskTwo, (void *) i);    //Create threads
    int status = pthread_create(&thread, &tattr, taskThree, (void *)i); //Create threads
    if (status != 0) {
        printf("While creating thread 1, pthread_create returned error code %d\n", status);
        exit(-1);
    }

    pthread_exit(NULL);
}

void *taskOne() {
    int iterations = 10;
    struct timespec time[iterations];
    for (int i = 0; i < iterations; i++) {
        struct timespec tim, tim2;
        double sum = 0.0;

        for (int j = 0; j < 1e4; j++) {
            sum += sqrt(j);
        }

        if (clock_gettime(CLOCK_REALTIME, &time[i]) == -1) {
            perror("clock gettime");
            exit(EXIT_FAILURE);
        }

        tim.tv_sec = 0;
        tim.tv_nsec = 1000000L;
        if (nanosleep(&tim, &tim2) < 0) {
            printf("Nano sleep system call failed \n");
        }
    }
    for (int i = 0; i < iterations; i++) {
        printf("%02ld:%03ld \n", time[i].tv_sec % 100, (long) (time[i].tv_nsec / 1e6));
    }
}

void *taskTwo() {
    int iterations = 10;
    struct timespec time[iterations];
    for (int i = 0; i < iterations; i++) {
        struct timespec deadline;
        clock_gettime(CLOCK_MONOTONIC, &deadline);
        // Add the time you want to sleep
        deadline.tv_nsec += 1000000L;

        // Normalize the time to account for the second boundary
        if (deadline.tv_nsec >= 1000000000) {
            deadline.tv_nsec -= 1000000000;
            deadline.tv_sec++;
        }

        double sum = 0.0;
        for (int j = 0; j < 1e2; j++) {
            sum += sqrt(j);
        }

        if (clock_gettime(CLOCK_MONOTONIC, &time[i]) == -1) {
            perror("clock gettime");
            exit(EXIT_FAILURE);
        }

        if (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &deadline, NULL) < 0) {
            printf("BIgly error");
        }
    }
    for (int i = 0; i < iterations; i++) {
        printf("%02ld:%03ld \n", time[i].tv_sec % 100, (long) (time[i].tv_nsec / 1e6));
    }
}


void create_timer() {
    struct sigevent ev;
    timer_t timer;
    struct itimerspec tset;

    /* block SIGUSR1 */
    if (sigemptyset(&sset) < 0) {
        printf("sigemptyset() failed");
    }
    if (sigaddset(&sset, SIGUSR1) < 0) {
        printf("sigaddset() failed");
    }
    if (sigprocmask(SIG_BLOCK, &sset, NULL) < 0) {
        printf("sigprocmask() failed");
    }

    /* create timer that sends SIGUSR1 on expiration */
    ev.sigev_notify = SIGEV_SIGNAL;
    ev.sigev_signo = SIGUSR1;
    ev.sigev_value.sival_ptr = &timer;

    if (timer_create(CLOCK_MONOTONIC, &ev, &timer) < 0) {
        printf("timer_create() failed");
    }
    /* get current time value */
    if (clock_gettime(CLOCK_MONOTONIC, &(tset.it_value)) < 0) {
        printf("clock_gettime() failed");
    }

    /* arm timer to start after 1s and expire every 1ms */
    tset.it_value.tv_sec = tset.it_value.tv_sec + 1;
    tset.it_interval.tv_sec = 0;
    tset.it_interval.tv_nsec = 1000000L;
    if (timer_settime(timer, TIMER_ABSTIME, &tset, NULL) < 0) {
        printf("timer_settime() failed");
    }
}

void *taskThree() {
    int iterations = 10;
    struct timespec time[iterations];
    create_timer();
    //Wait initial time
    if (sigwait(&sset, &sig)) {
        printf("failed sigwait()");
    }
    for (int i = 0; i < iterations; i++) {
        double sum = 0.0;
        for (int j = 0; j < 1e2; j++) {
            sum += sqrt(j);
        }

        if (clock_gettime(CLOCK_MONOTONIC, &time[i]) == -1) {
            perror("clock gettime");
            exit(EXIT_FAILURE);
        }
        /* sleep for another 10s */

        if (sigwait(&sset, &sig)) {
            printf("failed sigwait()");
        }
    }
    for (int i = 0; i < iterations; i++) {
        printf("%02ld:%03ld \n", time[i].tv_sec % 100, (long) (time[i].tv_nsec / 1e6));
    }
}

void terminate() {
    /* Wait for threads to end */
    pthread_join(thread, NULL);
}

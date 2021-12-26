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
#include <sys/mman.h>
#include <sys/syscall.h>

// Exercise 2.1.x
#define EXERCISE    3

// 0 = print outside loop
// 1 = printf
// 2 = __real_printf
#define PRINT_MODE  2

#define ADD_MODE_SWITCHES 1

// Terminal variables
static struct termios term;

static pthread_t thread;

static void init_xenomai();

static void *taskOne();
static void *taskTwo();
static void *taskThree();

static void terminate();
static void initialiseTerminal();
static void createThreads();
static void sighandler(int sig_num);

static void create_timer();
static void load();
static void print_time(struct timespec time[], int iterations);

static sigset_t sset;
static int sig = SIGALRM;

int main() {
    init_xenomai();

    createThreads(); // Create threads
    terminate(); // Wait for threads to end and terminate program
    return 0;
}

void init_xenomai() {
    /* Avoids memory swapping for this program */
    mlockall(MCL_CURRENT|MCL_FUTURE);
}

void createThreads() {
    long i = 0;

    pthread_attr_t tattr;

    pthread_attr_init(&tattr); //tattr init met defaultwaarden
    pthread_attr_setschedpolicy(&tattr, SCHED_FIFO); //sched policy to real time fifo

#if EXERCISE == 1
    int status = pthread_create(&thread, &tattr, taskOne, (void *) i);  //Create threads
#elif EXERCISE == 2
    int status = pthread_create(&thread, &tattr, taskTwo, (void *) i);  //Create threads
#elif EXERCISE == 3
    int status = pthread_create(&thread, &tattr, taskThree, (void *) i); //Create threads
#endif
    if (status != 0) {
        printf("While creating thread 1, pthread_create returned error code %d\n", status);
        exit(-1);
    }

    pthread_exit(NULL);
}

void load()
{
    double sum = 0.0;
    for (int j = 0; j < 1e4; j++) {
        sum += sqrt(j);
    }
}

void print_time(struct timespec time[], int iterations)
{
    for (int i = 0; i < iterations; i++) {
        if (i != 0)
            printf("%02ld:%06ld %04ld\n", 
                time[i].tv_sec % 100, 
                (long) (time[i].tv_nsec / 1e3), 
                (long) (time[i].tv_nsec / 1e3) - (long) (time[i-1].tv_nsec / 1e3)
            );
        else
            printf("%02ld:%06ld\n", time[i].tv_sec % 100, (long) (time[i].tv_nsec / 1e3));
    }
}

void create_timer() {
    struct sigevent ev;
    timer_t timer;
    struct itimerspec tset;

    struct itimerspec new_value, old_value;

    /* create timer that sends SIGUSR1 on expiration */
    ev.sigev_notify = SIGEV_THREAD_ID;
    ev.sigev_notify_thread_id = syscall(__NR_gettid);
    ev.sigev_signo = sig;

    /* block SIGUSR1 */
    if (sigemptyset(&sset) < 0) {
        printf("sigemptyset() failed\n");
    }
    if (sigaddset(&sset, sig) < 0) {
        printf("sigaddset() failed\n");
    }
    if(pthread_sigmask(SIG_BLOCK, &sset, NULL) > 0) {
        printf("sigprocmask() failed\n");
    }

    if (timer_create(CLOCK_MONOTONIC, &ev, &timer) < 0) {
        printf("timer_create() failed\n");
    }
    /* get current time value */
    if (clock_gettime(CLOCK_MONOTONIC, &(tset.it_value)) < 0) {
        printf("clock_gettime() failed\n");
    }

    /* arm timer to start after 1s and expire every 1ms */
    tset.it_value.tv_sec = 0;
    tset.it_interval.tv_sec = 0;
    tset.it_interval.tv_nsec = 1000000L;
    if (timer_settime(timer, TIMER_ABSTIME, &tset, NULL) < 0) {
        printf("timer_settime() failed");
    }
}

void *taskOne() {
    int iterations = 10;
    struct timespec time[iterations];
    for (int i = 0; i < iterations; i++) {
        struct timespec tim, tim2;
        load();

        if (clock_gettime(CLOCK_REALTIME, &time[i]) == -1) {
            perror("clock gettime");
            exit(EXIT_FAILURE);
        }

        tim.tv_sec = 0;
        tim.tv_nsec = 1000000L;
        if (nanosleep(&tim, &tim2) < 0) {
            printf("Nano sleep system call failed \n");
        }
#if PRINT_MODE == 1
        printf("%02ld:%06ld %04ld\n", 
                time[i].tv_sec % 100, 
                (long) (time[i].tv_nsec / 1e3), 
                (long) (time[i].tv_nsec / 1e3) - (long) (time[i-1].tv_nsec / 1e3)
            );
#elif PRINT_MODE == 2
        __real_printf("%02ld:%06ld %04ld\n", 
            time[i].tv_sec % 100, 
            (long) (time[i].tv_nsec / 1e3), 
            (long) (time[i].tv_nsec / 1e3) - (long) (time[i-1].tv_nsec / 1e3)
        );
#endif
    }
#if PRINT_MODE == 0
    print_time(time, iterations);
#endif
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

        load();

        if (clock_gettime(CLOCK_MONOTONIC, &time[i]) == -1) {
            perror("clock gettime");
            exit(EXIT_FAILURE);
        }

        if (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &deadline, NULL) < 0) {
            perror("clock_nanosleep");
            exit(EXIT_FAILURE);
        }
#if PRINT_MODE == 1
        printf("%02ld:%06ld %04ld\n", 
                time[i].tv_sec % 100, 
                (long) (time[i].tv_nsec / 1e3), 
                (long) (time[i].tv_nsec / 1e3) - (long) (time[i-1].tv_nsec / 1e3)
            );
#elif PRINT_MODE == 2
        __real_printf("%02ld:%06ld %04ld\n", 
            time[i].tv_sec % 100, 
            (long) (time[i].tv_nsec / 1e3), 
            (long) (time[i].tv_nsec / 1e3) - (long) (time[i-1].tv_nsec / 1e3)
        );
#endif
    }
#if PRINT_MODE == 0
    print_time(time, iterations);
#endif
}

void *taskThree() {
    int iterations = 10;
    struct timespec time[iterations];


    create_timer();
    printf("Timers created\n");
    //Wait initial time
    if (sigwait(&sset, &sig)) {
        printf("failed sigwait()");
    }
    for (int i = 0; i < iterations; i++) {
#if ADD_MODE_SWITCHES == 1
        FILE *fp;
        fp = __real_fopen("test.txt", "w+");
        __real_fprintf(fp, "This is testing for fprintf...\n");
        __real_fputs("This is testing for fputs...\n", fp);
        __real_fclose(fp);
#endif
        load();

        if (clock_gettime(CLOCK_MONOTONIC, &time[i]) == -1) {
            perror("clock gettime");
            exit(EXIT_FAILURE);
        }

        /* sleep for another 10s */
        if (sigwait(&sset, &sig)) {
            printf("failed sigwait()");
        }
#if PRINT_MODE == 1
        printf("%02ld:%06ld %04ld\n", 
                time[i].tv_sec % 100, 
                (long) (time[i].tv_nsec / 1e3), 
                (long) (time[i].tv_nsec / 1e3) - (long) (time[i-1].tv_nsec / 1e3)
            );
#elif PRINT_MODE == 2
        __real_printf("%02ld:%06ld %04ld\n", 
            time[i].tv_sec % 100, 
            (long) (time[i].tv_nsec / 1e3), 
            (long) (time[i].tv_nsec / 1e3) - (long) (time[i-1].tv_nsec / 1e3)
        );
#endif
    }
#if PRINT_MODE == 0
    print_time(time, iterations);
    while(1){

    }
#endif
}

void terminate() {
    /* Wait for threads to end */
    pthread_join(thread, NULL);
}

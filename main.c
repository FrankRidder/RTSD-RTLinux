#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>


#include <termios.h>
#include <unistd.h>
#include <signal.h>

// Terminal variables
static struct termios term;

#define BILLION  1000000000L;

static pthread_t thread;

static void *taskOne();
static void *taskTwo();
static void terminate();
static void initialiseTerminal();
static void createThreads();
static void sighandler(int sig_num);

int main() {
    createThreads();                    // Create threads

    terminate();                        // Wait for threads to end and terminate program
    return 0;
}

void createThreads() {
    long i = 0;

    pthread_attr_t tattr;

    pthread_attr_init(&tattr);                      //tattr init met defaultwaarden

    //int status = pthread_create(&thread, &tattr, taskOne, (void *) i);    //Create threads
    int status = pthread_create(&thread, &tattr, taskTwo, (void *) i);    //Create threads
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

        for (int i = 0; i < 1e4; i++) {
            sum += sqrt(i);
        }

        if( clock_gettime( CLOCK_REALTIME, &time[i]) == -1 ) {
            perror( "clock gettime" );
            exit( EXIT_FAILURE );
        }      
        
        tim.tv_sec  = 0;
        tim.tv_nsec = 1000000L;
        if(nanosleep(&tim , &tim2) < 0 )
        {
            printf("Nano sleep system call failed \n");
        }
    }
    for (int i = 0; i < iterations; i++) {
        printf("%02ld:%03ld \n", time[i].tv_sec % 100, (long)(time[i].tv_nsec / 1e6));
    }
}

void *taskTwo() {
    int iterations = 10;
    struct timespec time[iterations];
    for (int i = 0; i < iterations; i++) {
        struct timespec deadline;
        clock_gettime(CLOCK_MONOTONIC , &deadline);
        // Add the time you want to sleep
        deadline.tv_nsec += 1000000L;

        // Normalize the time to account for the second boundary
        if(deadline.tv_nsec >= 1000000000) {
            deadline.tv_nsec -= 1000000000;
            deadline.tv_sec++;
        }

        double sum = 0.0;
        for (int i = 0; i < 1e2; i++) {
            sum += sqrt(i);
        }

        if( clock_gettime( CLOCK_MONOTONIC, &time[i]) == -1 ) {
            perror( "clock gettime" );
            exit( EXIT_FAILURE );
        }          

        if (clock_nanosleep(CLOCK_MONOTONIC , TIMER_ABSTIME, &deadline, NULL) < 0) {
            printf("BIgly error");
        }
    }
    for (int i = 0; i < iterations; i++) {
        printf("%02ld:%03ld \n", time[i].tv_sec % 100, (long)(time[i].tv_nsec / 1e6));
    }
}

void wait_next_activation(void) { 
    int dummy; 
    sigwait (&sigset, &dummy); 
} 
int start_periodic_timer(long offs, int period) { 
    struct itimerval t;
    t.it_value.tv_sec = offs / 1000000; t.it_value.tv_usec = offs % 1000000;
    t.it_interval.tv_sec = period / 1000000; 
    t.it_interval.tv_usec = period % 1000000; 
    sigemptyset(&sigset); 
    sigaddset(&sigset, SIGALRM); 
    sigprocmask(SIG_BLOCK, &sigset„L3 NULL); 
    return setitimer(ITIMER_REAL, &t, NULL); 
} 

void *taskTwo() {
    int iterations = 10;
    struct timespec time[iterations];
    for (int i = 0; i < iterations; i++) {

        double sum = 0.0;
        for (int i = 0; i < 1e2; i++) {
            sum += sqrt(i);
        }

        if( clock_gettime( CLOCK_MONOTONIC, &time[i]) == -1 ) {
            perror( "clock gettime" );
            exit( EXIT_FAILURE );
        }    

    for (int i = 0; i < iterations; i++) {
        printf("%02ld:%03ld \n", time[i].tv_sec % 100, (long)(time[i].tv_nsec / 1e6));
    }
}

void terminate() {
    /* Wait for threads to end */
    pthread_join(thread, NULL);
}

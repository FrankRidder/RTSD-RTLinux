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

    int status = pthread_create(&thread, &tattr, taskOne, (void *) i);    //Create threads
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
        p
}

void terminate() {
    /* Wait for threads to end */
    pthread_join(thread, NULL);
}

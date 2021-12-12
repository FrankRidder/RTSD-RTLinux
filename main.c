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
    signal(SIGTSTP, sighandler);        // Disable CTRL+Z
    initialiseTerminal();               // Disable echo
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
    double accum;
    while (1) {
        struct timespec start, stop, tim, tim2;
        double sum = 0.0;
        if( clock_gettime( CLOCK_REALTIME, &start) == -1 ) {
            perror( "clock gettime" );
            exit( EXIT_FAILURE );
        }
        for (int i = 0; i < 10000000; i++) {
            sum += sqrt(i);
        }
        if( clock_gettime( CLOCK_REALTIME, &stop) == -1 ) {
            perror( "clock gettime" );
            exit( EXIT_FAILURE );
        }
        accum = ( stop.tv_nsec - start.tv_nsec )
                + ( stop.tv_nsec - start.tv_nsec )
                  / BILLION;
        printf( "%lf\n", accum );

        tim.tv_sec  = 0;
        tim.tv_nsec = 1000000L;
        if(nanosleep(&tim , &tim2) < 0 )
        {
            printf("Nano sleep system call failed \n");
            return -1;
        }

    }
}

void terminate() {
    /* Wait for threads to end */

    pthread_join(thread, NULL);

    /* remove garbage from stdin */
    int stdin_copy = dup(STDIN_FILENO);
    tcdrain(stdin_copy);
    tcflush(stdin_copy, TCIFLUSH);
    close(stdin_copy);
    term.c_lflag |= ECHO;  /* turn on ECHO */
    tcsetattr(fileno(stdin), 0, &term);
}

void initialiseTerminal() {
    tcgetattr(fileno(stdin), &term);
    term.c_lflag &= ~ECHO;
    tcsetattr(fileno(stdin), 0, &term);
}

// Signal Handler for SIGTSTP, we will get huge memory leaks if not closing properly.
void sighandler(int sig_num) {
    // Reset handler to catch SIGTSTP next time
    signal(SIGTSTP, sighandler);
    printf("Cannot execute Ctrl+Z, use ESC instead\n");

    pthread_cancel(thread);
}
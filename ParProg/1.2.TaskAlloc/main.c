#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <pthread.h>

#define STEPS 1000000

void * integratePI( double * t, double * step_size, double * sum );

// function for pthreads
void * integPI( void * ptr );

// mutex for exclusive access to variable sum & t
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct pthread_args
{
    double * _t; // this points to common variable t which
                 // is incremented by all the threads
    double  _step; // just a variable as the values remains constant
    double * _sum; // points to common variable where the sum is stored
};

int main(int argc, char * argv[] )
{
    double step_size = 1.0/STEPS;
    double t = 0.5 * step_size;
    double sum = 0;
    double duration = 0.00;

    int NumThreads = 1; // default

    struct timeval tv1,tv2;

    if( argc < 2 )
    {
        printf("\n\n \
      USAGE: ComputePi [CPU=0, THREADS=1] [NUMTHREADS,optional,default=1]\n\n");
        return 0;
    }

    // get no of threads
    if( argc > 2)
    {
        NumThreads = atoi(argv[2]);

        // for erroneous input, return to default
        if( NumThreads < 1 )
            NumThreads = 1;
    }

    if( strcmp(argv[1],"1") == 0 )
    {
        printf("\n Computation with pthreads !\n");

        gettimeofday( &tv1, NULL );
        struct pthread_args * thread_arg;
        pthread_t * threads;

        threads = malloc( NumThreads * sizeof(*threads));
        thread_arg = malloc( NumThreads * sizeof(*thread_arg) );

        for( int i = 0; i < NumThreads; i++ )
        {
            thread_arg[i]._t = &t;
            thread_arg[i]._step = step_size;
            thread_arg[i]._sum = &sum;

            pthread_create( threads + i, NULL, &integPI, thread_arg + i );
        }

        for(int i = 0; i < NumThreads; i++ )    
            pthread_join( threads[i], NULL);

        sum *= 4;
        free( threads );
        gettimeofday( &tv2, NULL);
    }
    else if( strcmp(argv[1],"0") == 0 )
    {
        printf("\n Computation without pthreads !\n");
        
        gettimeofday( &tv1, NULL );
        while( t < 1.0 )
        {
            integratePI( &t, &step_size, &sum );
        }

        sum *= 4;
        gettimeofday( &tv2, NULL);
    }
    else
    {
        printf("\nError: Invalid second argument !!\n");
        return 0;
    }
    duration = (tv2.tv_sec - tv1.tv_sec) * 1000000 + \
                tv2.tv_usec - tv1.tv_usec;
    duration /= 1000000;

    printf("\nComputed PI = %.10lf\n", sum);
    printf("Difference to reference is %.10lf\n", M_PI - sum);
    printf("Time for computation = %.5lf sec\n", duration );
    
    return 0;
}

void * integratePI( double * t, double * step_size, double * sum )
{
    *sum += ( sqrt( 1-(*t)*(*t) ) * (*step_size)); 
    *t += *step_size;

    return 0;
}

void * integPI( void * ptr )
{
    struct pthread_args * arg = ptr;

    while( 1 )
    {
        int status = pthread_mutex_lock(&mutex);

        if( status == 0 ) // lock acquired
        {
//            if( *(arg->_t) >= 1.0 )
//                printf("Erroneous Value = %lf\t ", *(arg->_t));

            if( *(arg->_t) <= 1.0 ) // check to avoid nAn
            {
                *(arg->_sum) += sqrt( 1- ( (*(arg->_t)) * (*(arg->_t)) ) ) * \
                                (arg->_step);
                *(arg->_t) += arg->_step;
            }

            status = pthread_mutex_unlock(&mutex);
            if( status != 0 )
            {
                printf("\nError: Unlock Mutex");
                pthread_exit(0);
                return 0;
            }
        }

        if( *(arg->_t) >= 1.0 ) // exit
        {
            pthread_exit(0);
            return 0;
        }
    }
}

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

#define EPSILON 0.00001
#define MAX_ITER 1000000
//#define MAX_ITER 30

//NOTE: program designed to equal # of variables = # threads
#define NUMTHREADS 3
#define DIM 3
#define PTHREADS 1

void init(int n, double (**A)[n], double **b, double **x0, double **x1)
{
	*A = malloc(n * n * sizeof(double));
	*b = malloc(n * sizeof(double));
	*x0 = malloc(n * sizeof(double));
	*x1 = malloc(n * sizeof(double));

	unsigned short xi[3] = {4, 153, 17}; // may b something like seed
	int i,j;

	for (i = 0; i < n; i++) {
	  (*A)[i][i] = 0.0; 
	  (*x0)[i] = 0.0; // initially original vector set to 0
	  (*x1)[i] = 0.0; // initially temp vector set to 0
	  for (j = 0; j < n; j++) {
		 if (i != j)
		 {
			 (*A)[i][j] = erand48(xi);
			 (*A)[i][i] += 2.0 * (*A)[i][j]; // to make diagonally dominant
		 }
	  }
	  (*b)[i] = 0.0; 
	  for (j = 0; j < n; j++) {
		  (*b)[i] += (*A)[i][j] * j; // vector b set to some value
	  }
	}
}

void print_vector(int n, double v[n])
{
	for(int i = 0; i < n; i++)
		printf("%lf ", v[i]);
	printf("\n");
}

void print_matrix(int n, double M[n][n])
{
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
		{
			printf("%lf ", M[i][j]);
		}
		printf("\n");
	}
}

// for pthreads
void * jacobi( void * ptr );

struct pthread_args
{
    int NoIter;             // no of iterations (static allocation)
    int id;                 // thread id
    double b_loc[DIM];      // local copy of array b
    double A_loc[DIM][DIM]; // local copy of array A
    double * stop;          // global pointer to stop
    double * x1;            // global pointer to x1
    pthread_barrier_t * barrier;
};

//mutex for access to x1 & stop
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char **argv)
{
	int n = 3, iter = 0;
	if(argc > 1)
		n = atoi(argv[1]);

    // arrays
	double stop, (*A)[n], *b, *x0, *x1;

    // initialising
	init(n, &A, &b, &x0, &x1);

    // printing
	printf("Matrix A:\n");
	print_matrix(n, A);
	printf("\nVector b:\n");
	print_vector(n, b);
	printf("\nInitial Guess of Vector x:\n");
	print_vector(n, x1);

#if PTHREADS
    struct pthread_args * t_arg;
    pthread_t * threads;
    pthread_barrier_t barrier;

    threads = malloc( NUMTHREADS * sizeof(*threads));
    t_arg = malloc( NUMTHREADS * sizeof(*t_arg));

    pthread_barrier_init( &barrier, NULL, NUMTHREADS);

    for( int i=0; i<NUMTHREADS; i++ )
    {
        t_arg[i].id = i;
        t_arg[i].NoIter = MAX_ITER/NUMTHREADS;
        t_arg[i].barrier = &barrier;

        for( int j=0; j< DIM; j++)
        {
            t_arg[i].b_loc[j] = b[j];
            for(int k=0; k<DIM;k++)
                t_arg[i].A_loc[j][k] = A[j][k];
        }

        t_arg[i].stop = &stop;        
        t_arg[i].x1 = x1;        
        
        pthread_create( threads+i, NULL, &jacobi, t_arg+i);
    }

    for( int i=0; i<NUMTHREADS; i++ )
        pthread_join( threads[i], NULL);

    // destroy barrier constructs
    pthread_barrier_destroy( &barrier );

    free(threads);
    free(t_arg);
#endif

#if !PTHREADS
    double * tmp;
    // jacobi solver
	do
	{
		stop = 0.0;
		for (int i = 0; i < n; i++) //loop1
		{
		  x1[i] = 0.0;
		  for (int j = 0; j < n; j++) //loop2
		  {
			  if(i != j)
				  x1[i] += A[i][j] * x0[j];
		  }
		  x1[i] = (b[i] - x1[i]) / A[i][i];
		  stop += pow(x1[i] - x0[i], 2.0);
		}
		tmp = x1; x1 = x0; x0 = tmp; //pointer swapping (array x0 & x1)
		iter++;
	} while( (sqrt(stop) > EPSILON) && (iter < MAX_ITER));
#endif

	printf("\nSolution Vector x after %d Iterations: \n", iter);
	print_vector(n, x1);
    return 0;
}

void * jacobi( void * ptr )
{
    struct pthread_args * arg = ptr;
    int iter = 0,n=DIM;
    double x = 0;
    int row = arg->id;

	do
	{
        x  = 0;
		*(arg->stop) = 0;
 //       printf("\nThread %d waits..., Stop = %lf", arg->id, *(arg->stop));
        pthread_barrier_wait(arg->barrier); // waits to start with stop =0 for all threads

		for (int j = 0; j < n; j++) //loop2
        {
            if(row != j)
            {
//                pthread_mutex_lock(&mutex);  
				x += arg->A_loc[row][j] * arg->x1[j]; // need mutex access
//                pthread_mutex_unlock(&mutex);
            }
        }
		x  = (arg->b_loc[row] - x) / \
              arg->A_loc[row][row];

        pthread_mutex_lock(&mutex);  
		*(arg->stop) += pow(x - arg->x1[row], 2.0);
        arg->x1[row] = x; // need mutex
        pthread_mutex_unlock(&mutex);

//        printf("\nThread ~%d~ waits..., Stop = %lf", arg->id, *(arg->stop));
        // need barrier
        pthread_barrier_wait(arg->barrier); //waits to let every thread update stop
//        printf("\nThread ~%d~ , Stop after synchro = %lf", arg->id, *(arg->stop));

        if(sqrt(*(arg->stop)) < EPSILON) 
        {
            printf("\nExiting...ThreadID=%d, iterations=%d, stop=%lf", arg->id, iter,sqrt(*(arg->stop)));
            break;
        }
        pthread_barrier_wait(arg->barrier); //waits s.t. every thread checks stop value, to avoid unnecessary exits
		iter++;

	} while(iter < arg->NoIter);

    printf("\nThreadID=%d, iterations=%d, x =%lf", arg->id, iter,x);
    return 0;
}



    // global variables epsilon & MAX_ITER
    // stop can be mutexed
    // static work allocation as per breakdown for iteration statically
    // loop even breaks for stop,epsilon comparison but that cant be--
    // --controlled for static work allocation

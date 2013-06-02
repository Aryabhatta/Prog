#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define NUMTHREADS 5  // Num of threads available
#define ArraySz    11  // Size of array to sort

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct pthread_args
{
    int * numtask; // containing count of #tasks available
    int * work; // containing indices of work to do
    int * a;   // ptr to array
    int id;
};

void swap(int *a, int *b)
{
	int tmp = *a;
	*a = *b;
	*b = tmp;
}

void * doWork( void * ptr )
{
    struct pthread_args * arg = ptr;
    unsigned short int xsubi [3] = {3, 7 , 11}; 
do{
    // check if there's any task
    if( *(arg->numtask) == 0 )
    {
        // Sleep random time
        double sleep_time = 1.0 + erand48 ( xsubi ) ;
        usleep(1000000 * sleep_time ) ;
       printf("\n Thread %d slept for %lf seconds",arg->id, 1000000*sleep_time);

        // Be a producer
        printf("\nThread %d is producer", arg->id);

        pthread_mutex_lock(&mutex);

        int left = 0;
        int right = ArraySz-1;
       
        // do sorting
    	if(left < right)
	    {
	    	int x = left, y = (left+right)/2, z =right;
		    int pivotIdx = (arg->a[x] <= arg->a[y])
        ? ((arg->a[y] <= arg->a[z]) ? y : ((arg->a[x] < arg->a[z]) ? z : x))
        : ((arg->a[x] <= arg->a[z]) ? x : ((arg->a[y] < arg->a[z]) ? z : y));

       	  int pivotVal = arg->a[pivotIdx];
	      swap(arg->a + pivotIdx, arg->a + right);

    	  int swapIdx = left;

    	  for(int i=left; i < right; i++)
	      {
		      if(arg->a[i] <= pivotVal)
		      {
			    swap(arg->a + swapIdx, arg->a + i);
			    swapIdx++;
		      }
	      }
	      swap(arg->a + swapIdx, arg->a + right);

	     // quicksort(a, left, swapIdx - 1);
	     // quicksort(a, swapIdx + 1, right);

//       printf("\nNumtask = %d, left=%d, right=%d",*(arg->numtask), left, right);
        int cntr=*(arg->numtask)*2;

        // create tasks only if necessary
        if( swapIdx-1>left)
        {
            // Insert task in task array
            arg->work[cntr++] = left;
            arg->work[cntr++] = swapIdx-1;
            *(arg->numtask)= *(arg->numtask) + 1;
            printf("\nThread %d created task %d-%d", arg->id, left, swapIdx-1);
        }
        if(right > swapIdx+1)
        {
            // Insert task in task array        
            arg->work[cntr++] = swapIdx+1;
            arg->work[cntr++] =right ;
            *(arg->numtask)= *(arg->numtask) + 1;
            printf("\nThread %d created task %d-%d", arg->id, swapIdx+1, right);
        }
        /*
        // Insert task in task array
        arg->work[cntr++] = left;
        arg->work[cntr++] = swapIdx-1;
        arg->work[cntr++] = swapIdx+1;
        arg->work[cntr++] =right ;
        *(arg->numtask)= *(arg->numtask) + 2;*/

        printf("\n Pivot is %d", pivotVal);
	    }
        pthread_mutex_unlock(&mutex);
    }
    else
    {
        pthread_mutex_lock(&mutex);

        // Be a consumer
        printf("\nThread %d is consumer", arg->id);

        // get the work
        int cntr=(*(arg->numtask)-1)*2;
        int left = arg->work[cntr++];
        int right = arg->work[cntr];
        printf("\nnumtask=%d, Thread %d working on indices from %d to  %d",*(arg->numtask),arg->id,left,right);

        // do the work
        // do sorting
    	if(left < right)
	    {
	    	int x = left, y = (left+right)/2, z =right;
		    int pivotIdx = (arg->a[x] <= arg->a[y])
        ? ((arg->a[y] <= arg->a[z]) ? y : ((arg->a[x] < arg->a[z]) ? z : x))
        : ((arg->a[x] <= arg->a[z]) ? x : ((arg->a[y] < arg->a[z]) ? z : y));

       	  int pivotVal = arg->a[pivotIdx];
	      swap(arg->a + pivotIdx, arg->a + right);

    	  int swapIdx = left;

    	  for(int i=left; i < right; i++)
	      {
		      if(arg->a[i] <= pivotVal)
		      {
			    swap(arg->a + swapIdx, arg->a + i);
			    swapIdx++;
		      }
	      }
	      swap(arg->a + swapIdx, arg->a + right); // swaping done

	     // quicksort(a, left, swapIdx - 1);
	     // quicksort(a, swapIdx + 1, right);

        // change #tasks
        *(arg->numtask)= *(arg->numtask) - 1;

  //     printf("\nNumtask = %d, left=%d, right=%d",*(arg->numtask), left, right);
         cntr=*(arg->numtask)*2;

        // create tasks only if necessary
        if( swapIdx-1>left)
        {
            // Insert task in task array
            arg->work[cntr++] = left;
            arg->work[cntr++] = swapIdx-1;
            *(arg->numtask)= *(arg->numtask) + 1;
            printf("\nThread %d created task %d-%d", arg->id, left, swapIdx-1);
        }
        if(right > swapIdx+1)
        {
            // Insert task in task array        
            arg->work[cntr++] = swapIdx+1;
            arg->work[cntr++] =right ;
            *(arg->numtask)= *(arg->numtask) + 1;
            printf("\nThread %d created task %d-%d", arg->id, swapIdx+1, right);
        }

        printf("\n Pivot is %d", pivotVal);
      }

        pthread_mutex_unlock(&mutex);     
    }
}while( *(arg->numtask) > 0);
    // how to stop ?
    return 0;
}


double time_diff(const struct timespec *first, const struct timespec *second,
		struct timespec *diff)
{
	struct timespec tmp;
	const struct timespec *tmp_ptr;

	if(first->tv_sec >  second->tv_sec ||
      (first->tv_sec == second->tv_sec && first->tv_nsec > second->tv_nsec))
	{
		tmp_ptr = first;
		first = second;
		second = tmp_ptr;
	}

	tmp.tv_sec = second->tv_sec - first->tv_sec;
	tmp.tv_nsec = second->tv_nsec - first->tv_nsec;

	if (tmp.tv_nsec < 0)
	{
		tmp.tv_sec -= 1;
		tmp.tv_nsec += 1000000000;
	}

	if (diff != NULL )
	{
		diff->tv_sec = tmp.tv_sec;
		diff->tv_nsec = tmp.tv_nsec;
	}

	return tmp.tv_sec + tmp.tv_nsec / 1000000000.0;
}

int * random_int_array(long size, int num_swaps, unsigned int seed)
{
	srand(seed);

	int *a = malloc(size * (long)sizeof(*a));

	for(int i = 0; i < size; i++)
		a[i] = i;

	for(long i = 0; i < num_swaps; i++)
	{
		long idx0 = rand() % size;
		long idx1 = rand() % size;

		int tmp = a[idx0];
		a[idx0] = a[idx1];
		a[idx1] = tmp;
	}
	return a;
}


void quicksort(int *a, int left, int right)
{
	if(left < right)
	{
		int x = left, y = (left+right)/2, z =right;
		int pivotIdx = (a[x] <= a[y])
		    ? ((a[y] <= a[z]) ? y : ((a[x] < a[z]) ? z : x))
		    : ((a[x] <= a[z]) ? x : ((a[y] < a[z]) ? z : y));

	  int pivotVal = a[pivotIdx];
	  swap(a + pivotIdx, a + right);

	  int swapIdx = left;

	  for(int i=left; i < right; i++)
	  {
		  if(a[i] <= pivotVal)
		  {
			  swap(a + swapIdx, a + i);
			  swapIdx++;
		  }
	  }
	  swap(a + swapIdx, a + right);

	  quicksort(a, left, swapIdx - 1);
	  quicksort(a, swapIdx + 1, right);

	}
}

void print_array(int *a, int elements)
{
	for(int i=0; i < elements; i++)
		printf("%4d ", a[i]);
	printf("\n");
}

int main(int argc, char **argv)
{
	int elements = ArraySz;
	if(argc > 1) elements = atoi(argv[1]);
	struct timespec start, stop;

    int numtask=0;        // no of tasks
    int work[ArraySz*2]; // max size

    // Created random array
	int *a = random_int_array(elements, elements/2, 13);

    // Print array
	print_array(a, elements);

	clock_gettime(CLOCK_MONOTONIC, &start);

//	quicksort(a, 0, elements-1);

    // create threads
    pthread_t * threads;
    struct pthread_args * t_arg;

    threads = malloc(NUMTHREADS * sizeof(*threads));
    t_arg= malloc(NUMTHREADS * sizeof(*t_arg));

    for( int i=0; i < NUMTHREADS; i++ )
    {
        t_arg[i].numtask = &numtask;
        t_arg[i].work = work;
        t_arg[i].a = a;
        t_arg[i].id = i;
        pthread_create( threads+i, NULL, &doWork, t_arg+i );
        pthread_join( threads[i], NULL );
    } 

    free(threads);
    free(t_arg);

	clock_gettime(CLOCK_MONOTONIC, &stop);

    // print numtask
    printf("\nNumTask = %d", numtask);


    // print workarray
    for( int i=0; i< ArraySz*2; i++)
    {
        printf("\nwork[%d]=%d\t", i,work[i]);
        i++;
        printf("work[%d]=%d", i,work[i]);
    }
    printf("\n\n");

	print_array(a, elements);

	printf("\nTime = %lf\n", time_diff(&start, &stop, NULL));

    return 0;
}

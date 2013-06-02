#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 4

struct pthread_args
{
	int id;
	pthread_barrier_t *barrier;
};

void * run_thread(void *ptr)
{
	struct pthread_args *args = (struct pthread_args*)ptr;

	for(int i = 0; i < 4; i++)
	{
		pthread_barrier_wait(args->barrier);
		printf("Thread = %d I = %d\n", args->id, i);
	}

	return NULL;
}

void main()
{
	pthread_t *thread; struct pthread_args *thread_arg;
	pthread_barrier_t barrier;

	pthread_barrier_init(&barrier, NULL, NUM_THREADS);

	thread = malloc((unsigned long)NUM_THREADS * sizeof(*thread));
	thread_arg = malloc((unsigned long)NUM_THREADS * sizeof(*thread_arg));

	for (int i = 0; i < NUM_THREADS; i++)
	{
		thread_arg[i].id = i;
		thread_arg[i].barrier = &barrier;
		pthread_create(thread + i, NULL, &run_thread, thread_arg + i);
	}

	for (int i = 0; i < NUM_THREADS; i++)
	{
		pthread_join(thread[i], NULL );
	}

	pthread_barrier_destroy(&barrier);
}

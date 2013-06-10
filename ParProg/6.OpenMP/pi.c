#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

#define STEPS 1000000

int main(int argc, char * argv[] )
{
	double step_size = 1.0/STEPS;
	double t = 0.5 * step_size, sum = 0;
    double tEnd, chunk; 
    int numthreads;
    int iam;

    if( argc < 2 )
    {
        printf("\n\nUSAGE: ./pi [NUMTHREADS]\n\n");
        return 0;
    }
    
    numthreads= atoi(argv[1]);
    printf("\nNumThreads = %d\n", numthreads);
    omp_set_num_threads( numthreads );
    chunk = (1.0)/numthreads;

    #pragma omp parallel private(t,tEnd,iam) reduction(+:sum)
    {
        iam = omp_get_thread_num();
        t = 0.5*step_size + (chunk*iam);
        tEnd = chunk * (iam+1);

	    while(t < tEnd)
	    {
		    sum += sqrt(1-t*t) * step_size;
		    t += step_size;
	    }
    }
	sum *= 4;

    printf("\n");
	printf("Reference PI = %.10lf Computed PI = %.10lf\n", M_PI, sum);
	printf("Difference to Reference is %.10lf\n", M_PI - sum);
    return 0;
}

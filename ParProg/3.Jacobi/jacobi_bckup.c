#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define EPSILON 0.00001
#define MAX_ITER 1000000

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

void main(int argc, char **argv)
{
	int n = 3, iter = 0;
	if(argc > 1)
		n = atoi(argv[1]);

	double stop, *tmp, (*A)[n], *b, *x0, *x1;

	init(n, &A, &b, &x0, &x1);

	printf("Matrix A:\n");
	print_matrix(n, A);
	printf("\nVector b:\n");
	print_vector(n, b);
	printf("\nInitial Guess of Vector x:\n");
	print_vector(n, x1);

	do
	{
		stop = 0.0;
		for (int i = 0; i < n; i++)
		{
		  x1[i] = 0.0;
		  for (int j = 0; j < n; j++)
		  {
			  if(i != j)
				  x1[i] += A[i][j] * x0[j];
		  }
		  x1[i] = (b[i] - x1[i]) / A[i][i];
		  stop += pow(x1[i] - x0[i], 2.0);
		}
		tmp = x1; x1 = x0; x0 = tmp;
		iter++;
	} while((sqrt(stop) > EPSILON) && (iter < MAX_ITER));

	printf("\nSolution Vector x after %d Iterations:\n", iter);
	print_vector(n, x1);
}

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <omp.h>

char *colored_digit[] =
{
	"\e[1;30;40m0",
	"\e[1;31;40m1",
	"\e[1;32;40m2",
	"\e[1;33;40m3",
	"\e[1;34;40m4",
	"\e[1;35;40m5",
	"\e[1;36;40m6",
	"\e[1;37;40m7"
};

int main(int argc, char** argv)
{
	unsigned int x_size = 80;
	unsigned int y_size = 40;
	int collapse = 0;

	int c;
	while ((c = getopt(argc, argv, "t:cr:")) != -1)
	{
		switch (c)
		{
		case 'c':
			collapse = 1;
			break;
		case 'r':
			if (sscanf(optarg, "%ux%u", &x_size, &y_size) != 2)
				goto error;
			break;
		case '?':
			error: printf(
							"Usage:\n"
							"-c \t to use collapse(2) in nested for loop \n"
							"-r \t number of elements in x and y direction\n"
							"\n"
							"Example:\n"
							"OMP_NUM_THREADS=4 OMP_SCHEDULE=\"STATIC,1\" %s -c -r 80x40\n",
					argv[0]);
			exit(EXIT_FAILURE);
			break;
		}
	}

	if(omp_get_num_threads() > 8)
	{
		fprintf(stderr, "This application supports only up to 8 OpenMP threads!");
		exit(EXIT_FAILURE);
	}

	unsigned long str_len = strlen(colored_digit[0]);
	char *string_2D = malloc(x_size * y_size * str_len + y_size);

	if (collapse == 0)
	{
		#pragma omp parallel for schedule(runtime)
		for (unsigned long i = 0; i < y_size; i++)
		{
			for (unsigned int j = 0; j < x_size; j++)
			{
				memcpy(string_2D + (i * x_size * str_len + i) + (j * str_len), colored_digit[omp_get_thread_num()], str_len);
			}
		}
	}
	else
	{
		#pragma omp parallel for schedule(runtime) collapse(2)
		for (unsigned long i = 0; i < y_size; i++)
		{
			for (unsigned int j = 0; j < x_size; j++)
			{
				memcpy(string_2D + (i * x_size * str_len + i) + (j * str_len), colored_digit[omp_get_thread_num()], str_len);
			}
		}
	}

	for (unsigned int i = 0; i < y_size; i++)
	{
		unsigned long row = i * x_size * str_len + i;
		printf("%s\n", string_2D + row);
	}

	printf("\033[0m");

	return 0;
}

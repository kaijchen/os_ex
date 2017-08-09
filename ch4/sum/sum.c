#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_THREADS 16

struct node {
	long lower;
	long upper;
	long result;
};

void *runner(void *param);

int main(int argc, char **argv)
{
	pthread_t tid[MAX_THREADS];
	pthread_attr_t attr;
	struct node data[MAX_THREADS];
	long n, m, sum = 0;
	int num_threads;

	if (argc != 3) {
		fprintf(stderr, "usage: %s <value> <threads>\n", argv[0]);
		return -1;
	}
	if ((n = atol(argv[1])) < 0) {
		fprintf(stderr, "value: %ld must be >= 0\n", n);
		return -1;
	}
	if ((num_threads = atoi(argv[2])) <= 0 || num_threads > MAX_THREADS) {
		fprintf(stderr, "threads: %d must be > 0 and <= %d\n",
				num_threads, MAX_THREADS);
		return -1;
	}

	m = n / num_threads;
	for (long i = 0; i < num_threads; i++) {
		data[i].lower = m * i + 1;
		data[i].upper = m * (i + 1);
	}
	data[num_threads - 1].upper = n;

	pthread_attr_init(&attr);
	for (long i = 0; i < num_threads; i++) {
		pthread_create(&tid[i], &attr, runner, &data[i]);
	}

	for (long i = 0; i < num_threads; i++) {
		pthread_join(tid[i], NULL);
		sum += data[i].result;
	}

	printf("sum = %ld\n", sum);

	return 0;
}

void *runner(void *param)
{
	struct node *data = (struct node *)param;
	long sum = 0;

	for (long i = data->lower; i <= data->upper; i++)
		sum += i;

	data->result = sum;

	pthread_exit(0);
}

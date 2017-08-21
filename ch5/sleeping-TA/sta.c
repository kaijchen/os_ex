#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define STU_MAX 64
#define QUEUE_MAX 64

struct queue {
	int pool[QUEUE_MAX];
	int size;
	int head;
	int tail;
	sem_t empty;
	sem_t full;
	sem_t mutex;
};

void *stu_runner(void *param);
void *ta_runner(void *param);

int main(int argc, char *argv[])
{
	pthread_t stu_tid[STU_MAX];
	pthread_t ta_tid;
	pthread_attr_t attr;
	int stu_num;
	struct queue q;

	if (argc != 3) {
		fprintf(stderr, "usage: %s <stu_num> <queue_size>\n", argv[0]);
		return -1;
	}

	if ((stu_num = atoi(argv[1])) <= 0 || stu_num > STU_MAX) {
		fprintf(stderr, "%d must be >0 and <=%d\n", stu_num, STU_MAX);
		return -1;
	}

	if ((q.size = atoi(argv[2])) <= 0 || q.size > QUEUE_MAX) {
		fprintf(stderr, "%d must be >0 and <=%d\n", q.size, QUEUE_MAX);
		return -1;
	}

	sem_init(&q.empty, 0, 0);
	sem_init(&q.full, 0, q.size);
	sem_init(&q.mutex, 0, 1);
	q.head = q.tail = 0;

	srand(time(0));
	pthread_attr_init(&attr);
	pthread_create(&ta_tid, &attr, ta_runner, &q);
	for (int i = 0; i < stu_num; i++)
		pthread_create(&stu_tid[i], &attr, stu_runner, &q);
	for (int i = 0; i < stu_num; i++)
		pthread_join(stu_tid[i], NULL);
//	pthread_join(ta_tid, NULL);
	return 0;
}

void *stu_runner(void *param)
{
	struct queue *p = (struct queue *)param;

	sem_wait(&p->full);
	sem_wait(&p->mutex);
	p->tail = (p->tail + 1) % p->size;
	p->pool[p->tail] = rand() % 100;
	printf("question %d was added into the queue #%d\n",
			p->pool[p->tail],
			p->tail);
	sem_post(&p->mutex);
	sem_post(&p->empty);

	pthread_exit(0);
}

void *ta_runner(void *param)
{
	struct queue *p = (struct queue *)param;

	for (;;) {
		sem_wait(&p->empty);
		sem_wait(&p->mutex);
		p->head = (p->head + 1) % p->size;
		printf("#%d question %d was solved by the TA\n",
				p->head,
				p->pool[p->head]);
		sem_post(&p->mutex);
		sem_post(&p->full);
	}

	pthread_exit(0);
}

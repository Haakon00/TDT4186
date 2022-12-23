#include <stdlib.h>
#include <pthread.h>

#include "sem.h"

typedef struct SEM
{
	int counter;
	int wakeups;
	pthread_mutex_t mutex;
	pthread_cond_t condition;
} SEM;

SEM *sem_init(int initVal)
{
	SEM *semaphorePointer = malloc(sizeof(SEM));
	if (semaphorePointer == NULL)
	{
		return NULL;
	}

	semaphorePointer->counter = initVal;

	if (pthread_mutex_init(&semaphorePointer->mutex, (pthread_mutexattr_t *)NULL))
	{
		free(semaphorePointer);
		return NULL;
	}

	if (pthread_cond_init(&semaphorePointer->condition, NULL))
	{
		pthread_mutex_destroy(&semaphorePointer->mutex);
		free(semaphorePointer);
		return NULL;
	}

	return semaphorePointer;
}

int sem_del(SEM *sem)
{
	int code = 0;
	if (pthread_mutex_destroy(&sem->mutex))
	{
		code = -1;
	}
	if (pthread_cond_destroy(&sem->condition))
	{
		code = -1;
	}

	free(sem);
	return code;
}

void P(SEM *sem)
{
	pthread_mutex_lock(&sem->mutex);

	sem->counter--;
	if (sem->counter < 0)
	{
		do
		{
			pthread_cond_wait(&sem->condition, &sem->mutex);
		} while (sem->wakeups < 1);
		sem->wakeups--;
	}

	pthread_mutex_unlock(&sem->mutex);
}

void V(SEM *sem)

{
	pthread_mutex_lock(&sem->mutex);

	sem->counter++;

	if (sem->counter <= 0)
	{
		sem->wakeups++;
		pthread_cond_signal(&sem->condition);
	}

	pthread_mutex_unlock(&sem->mutex);
}

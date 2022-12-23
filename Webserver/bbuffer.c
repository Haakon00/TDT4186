#include "bbuffer.h"
#include "sem.c"

typedef struct BNDBUF
{
	SEM *fullSemaphore;
	SEM *emptySemaphore;
	SEM *oneAtATimeSemaphore;
	unsigned int bufferSize;
	unsigned int headPosition;
	unsigned int tailPosition;
	int fileDescriptors[];
} BNDBUF;

BNDBUF *bb_init(unsigned int size)
{
	BNDBUF *bufferPointer = malloc(sizeof(BNDBUF) + sizeof(int) * size);
	if (bufferPointer == NULL)
	{
		return NULL;
	}

	bufferPointer->bufferSize = size;
	bufferPointer->headPosition = 0;
	bufferPointer->tailPosition = 0;

	bufferPointer->fullSemaphore = sem_init(size);
	if (bufferPointer->fullSemaphore == NULL)
	{
		free(bufferPointer);
		return NULL;
	}

	bufferPointer->emptySemaphore = sem_init(0);
	if (bufferPointer->emptySemaphore == NULL)
	{
		sem_del(bufferPointer->fullSemaphore);
		free(bufferPointer);
		return NULL;
	}

	bufferPointer->oneAtATimeSemaphore = sem_init(1);
	if (bufferPointer->oneAtATimeSemaphore == NULL)
	{
		sem_del(bufferPointer->fullSemaphore);
		sem_del(bufferPointer->emptySemaphore);
		free(bufferPointer);
		return NULL;
	}

	return bufferPointer;
}

void bb_del(BNDBUF *bb)
{
	sem_del(bb->fullSemaphore);
	sem_del(bb->emptySemaphore);
	sem_del(bb->oneAtATimeSemaphore);
	free(bb);
}

int bb_get(BNDBUF *bb)
{

	P(bb->oneAtATimeSemaphore);
	P(bb->emptySemaphore);

	int returnValue = bb->fileDescriptors[bb->headPosition];
	bb->headPosition = (bb->headPosition + 1) % bb->bufferSize;

	V(bb->fullSemaphore);
	V(bb->oneAtATimeSemaphore);
	return returnValue;
}

void bb_add(BNDBUF *bb, int fd)
{

	P(bb->fullSemaphore);
	bb->fileDescriptors[bb->tailPosition] = fd;
	bb->tailPosition = (bb->tailPosition + 1) % bb->bufferSize;
	V(bb->emptySemaphore);
}

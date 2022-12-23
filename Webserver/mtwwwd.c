#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "bbuffer.c"

#define MAXREQ (4096 * 1024)

char pathBuffer[MAXREQ], body[MAXREQ], msg[MAXREQ];

typedef struct ThreadParams
{
	char *ROOT;
	BNDBUF *buffer;
} ThreadParams;

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

void *threadWork(void *inputParams)
{
	ThreadParams *params = (ThreadParams *)inputParams;
	char *ROOT = params->ROOT;
	BNDBUF *buffer = params->buffer;

	int n;
	int newsockfd;
	while (1)
	{
		newsockfd = bb_get(buffer);
		if (newsockfd < 0)
			error("ERROR on accept");

		// Parse request start
		bzero(pathBuffer, sizeof(pathBuffer));
		n = read(newsockfd, pathBuffer, sizeof(pathBuffer) - 1);
		if (n < 0)
			error("ERROR reading from socket");
		/* Find out where everything is */
		const char *start_of_path = strchr(pathBuffer, ' ') + 1;
		const char *end_of_path = strchr(start_of_path, ' ');
		/* Get the right amount of memory */
		char path[end_of_path - start_of_path];
		/* Copy the strings into our memory */
		strncpy(path, start_of_path, end_of_path - start_of_path);
		/* Null terminators (because strncpy does not provide them) */
		path[sizeof(path)] = 0;
		// Parse request end

		char rootcopy[50];
		strcpy(rootcopy, ROOT);
		char *fullpath = strcat(rootcopy, path);

		n = write(newsockfd, msg, strlen(msg));
		if (n < 0)
			error("ERROR writing to socket");

		FILE *f = fopen(fullpath, "r");
		char response[30000] = {0};

		if (f)
		{
			fseek(f, 0, SEEK_END);
			long fsize = ftell(f);
			fseek(f, 0, SEEK_SET);

			char *filebuffer = malloc(fsize + 1);
			fread(filebuffer, 1, fsize, f);
			fclose(f);

			strcpy(response, "HTTP/1.1 200 OK\nContents-Type: text/html\n\n");
			strcat(response, filebuffer);
			free(filebuffer);
		}
		else
		{
			strcpy(response, "HTTP/1.1 404 Not Found\nContents-Type: text/html\n\n<html><body><h1>404 Page Not Found</h1></body></html>\n");
		}

		write(newsockfd, response, strlen(response));
		close(newsockfd);
	}
}

int main(int argc, char *argv[])
{
	// Args
	char *ROOT = argv[1];
	unsigned int PORT = atoi(argv[2]);
	unsigned int THREADS = atoi(argv[3]);
	unsigned int BUFFERSLOTS = atoi(argv[4]);

	// Socket Setup Start
	int sockfd = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);
	if (sockfd < 0)
		error("ERROR opening socket");

	struct sockaddr_in6 server_address;
	bzero((char *)&server_address, sizeof(server_address));
	server_address.sin6_family = AF_INET6;
	server_address.sin6_addr = in6addr_any;
	server_address.sin6_port = htons(PORT);

	if (bind(sockfd, (struct sockaddr *)&server_address,
			 sizeof(server_address)) < 0)
	{
		error("ERROR on binding");
	}
	listen(sockfd, 5);
	// Socket Setup End

	// Buffer Setup
	BNDBUF *buffer = bb_init(BUFFERSLOTS);

	// Thread creation
	pthread_t threadID[THREADS];
	ThreadParams params = {ROOT, buffer};

	for (int i = 0; i < THREADS; i++)
	{
		pthread_create(&threadID[i], NULL, threadWork, (void *)&params);
	}

	// New Socket
	struct sockaddr_in cli_addr;
	unsigned int clilen;
	int newsockfd;
	while (1)
	{
		clilen = sizeof(cli_addr);
		newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr,
						   &clilen);

		bb_add(buffer, newsockfd);
	}

	return 0;
}
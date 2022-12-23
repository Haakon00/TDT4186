#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_SIZE 1000

/* Data type for list elements */
typedef struct element
{
	int PID;
	char commandline[MAX_SIZE];
	struct element *next; /* pointer to next list element */
} element;
/* Data type for list administration */
typedef struct linked_list
{
	element *head;	/* first element */
	element **tail; /* 'next' pointer in last element */
} linked_list;
/* Function to add a new element to the end of the list */
void enqueue(linked_list *list, element *item)
{
	item->next = NULL;
	*list->tail = item;
	list->tail = &item->next;
}

void zombieCatcher(linked_list *list)
{
	if (list->head == NULL)
	{
		return;
	}
	int status;
	int zombie_PID = waitpid(0, &status, WNOHANG);
	while (zombie_PID > 0)
	{
		element *tmp = list->head;
		element *prev;
		while (tmp != NULL && tmp->PID != zombie_PID)
		{
			prev = tmp;
			tmp = tmp->next;
		}
		if (tmp != NULL)
		{
			if (WIFEXITED(status))
			{
				printf("Exit status [%s] = %i\n", tmp->commandline, WEXITSTATUS(status));
			}
			else
			{
				printf("[%s] did not terminate normally.\n", tmp->commandline);
			}

			if (tmp == list->head)
			{
				list->head = tmp->next;
				if (list->head == NULL)
				{
					list->tail = &list->head;
					free(tmp);
					return;
				}
			}
			else
			{
				prev->next = tmp->next;
				if (prev->next == NULL)
				{
					list->tail = &prev->next;
				}
			}

			free(tmp);
		}

		zombie_PID = waitpid(0, &status, WNOHANG);
	}
	if (zombie_PID == -1)
	{
		printf("Error waiting for background processes. Status = %i", errno);
	}
}

int main()
{
	linked_list list = {.head = NULL, .tail = &list.head};
	while (1)
	{
		zombieCatcher(&list);
		char current_path[MAX_SIZE];
		if (getcwd(current_path, sizeof(current_path)) != NULL)
		{
			printf("%s: ", current_path);
			fflush(stdout);
		}
		else
		{
			printf("Error getting current directory. Status = %i", errno);
			return errno;
		}

		char input[MAX_SIZE];

		int i = 0;
		int r;
		while ((r = read(STDIN_FILENO, &input[i], sizeof(char))) > 0)
		{
			if (input[i] == *"\n")
			{
				goto execute;
			}
			i++;
		}
		if (r == -1)
		{
			printf("Error reading input. Status = %i", errno);
		}
		goto finished;

	execute:
		input[i] = '\0';
		if (input[0] == '\0')
		{
			continue;
		}

		char input_copy[MAX_SIZE];
		strcpy(input_copy, input);

		char *token;
		token = strtok(input, " \t");
		char *arguments[MAX_SIZE];
		arguments[0] = token;
		int argument_count = 1;
		while (token != NULL)
		{
			token = strtok(NULL, " 	\t");
			arguments[argument_count] = token;
			argument_count++;
		}

		if (!strcmp(arguments[0], "cd"))
		{

			if (arguments[1] == NULL)
			{
				arguments[1] = "/home";
				arguments[2] = NULL;
			}

			if (chdir(arguments[1]) == -1)
			{
				printf("Exit status [%s] = %i\n", input_copy, errno);
			}
			else
			{
				printf("Exit status [%s] = 0\n", input_copy);
			}

			continue;
		}

		if (!strcmp(arguments[0], "jobs"))
		{
			element *tmp = list.head;
			while (tmp != NULL)
			{
				printf("PID: %i [%s]\n", tmp->PID, tmp->commandline);
				tmp = tmp->next;
			}
			continue;
		}

		char *lastChar = &arguments[argument_count - 2][(int)strlen(arguments[argument_count - 2]) - 1];
		int background = 0;
		element *background_process;
		if (*lastChar == *"&")
		{
			background = 1;
			*lastChar = '\0';
			if (arguments[argument_count - 2][0] == '\0')
			{
				arguments[argument_count - 2] = NULL;
			}
			background_process = malloc(sizeof(element));
			if (background_process == NULL)
			{
				perror("Error creating background process.\n");
				printf("Exit status [%s] = %i\n", input_copy, errno);
				continue;
			}
		}

		int e = 1;
		int new_read = -2;
		int new_write = -2;
		int earliest = -1;
		char input_redirect[MAX_SIZE];
		char output_redirect[MAX_SIZE];
		while (arguments[e] != NULL)
		{
			if (!strcmp(arguments[e], "<"))
			{
				new_read = -3;
				strcpy(input_redirect, arguments[e + 1]);
				if (earliest == -1)
				{
					earliest = e;
				}
			}
			else if (!strcmp(arguments[e], ">"))
			{
				new_write = -3;
				strcpy(output_redirect, arguments[e + 1]);
				if (earliest == -1)
				{
					earliest = e;
				}
			}
			e++;
		}

		if (earliest >= 1)
		{
			arguments[earliest] = NULL;
		}

		int pid = fork();
		if (pid == 0)
		{ // Child
			if (new_read == -3)
			{
				int fd = open(input_redirect, O_RDONLY);
				if (fd == -1)
				{
					exit(errno);
				}
				new_read = dup2(fd, STDIN_FILENO);
				if (new_read == -1)
				{
					exit(errno);
				}

				if (close(fd) == -1)
				{
					exit(errno);
				}
			}
			if (new_write == -3)
			{
				int fd = open(output_redirect, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
				if (fd == -1)
				{
					exit(errno);
				}
				new_write = dup2(fd, STDOUT_FILENO);
				if (new_write == -1)
				{
					exit(errno);
				}

				if (close(fd) == -1)
				{
					exit(errno);
				}
			}
			execv(arguments[0], arguments);
			exit(errno);
		}
		else if (pid == -1)
		{
			perror("Error! child process was not created");
			printf("Exit status [%s] = %i\n", input_copy, errno);
		}
		else
		{ // Parent
			int status;

			if (background)
			{
				background_process->PID = pid;
				strcpy(background_process->commandline, input_copy);
				enqueue(&list, background_process);
			}

			else
			{
				if (waitpid((pid_t)pid, &status, 0) == -1)
				{
					printf("Exit status [%s] = %i\n", input_copy, errno);
				}
				else if (WIFEXITED(status))
				{
					printf("Exit status [%s] = %i\n", input_copy, WEXITSTATUS(status));
				}
				else
				{
					printf("[%s] did not terminate normally.\n", input_copy);
				}
			}
		}
	}

finished:
	zombieCatcher(&list);
	element *tmp = list.head;
	while (tmp != NULL)
	{
		kill(tmp->PID, SIGKILL);
		element *next = tmp->next;
		free(tmp);
		tmp = next;
	}
	printf("\n");
	return 0;
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

typedef struct Alarm
{
    int id;
    int alarm_pid;
    struct tm date;
} Alarm;

Alarm alarms[20];
int length = sizeof(alarms) / sizeof(Alarm);

void zombieCatcher()
{
    int zombiePID = waitpid(0, NULL, WNOHANG);
    if (zombiePID > 0)
    {
        for (int i = 0; i < length; i++)
        {
            if (alarms[i].alarm_pid == zombiePID)
            {
                alarms[i].id = 0;
            }
        }
    }
}

void flushBuffer()
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
        ;
}

int availableIndex()
{
    for (int i = 0; i < length; i++)
    {
        if (alarms[i].id == 0)
        {
            return i;
        }
    }
    return -1;
}

int main()
{
    char command;
    time_t t;
    struct tm tm;
    int diff;
    int alarmToCancel;
    int pid;
    struct tm inputTm;
    int newAlarmIndex = 0;
    while (1)
    {
        t = time(NULL);
        tm = *localtime(&t);
        tm.tm_year += 1900;
        tm.tm_mon += 1;
        printf("\nWelcome to the alarm clock! It is currently %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
        printf("Please enter s (schedule), l (list), c (cancel), x (exit)\n");

        command = getchar();
        flushBuffer();
        zombieCatcher();
        switch (command)
        {
        case 's':

            newAlarmIndex = availableIndex();
            if (newAlarmIndex == -1)
            {
                printf("There is not space for more alarms!\n\n");
                break;
            }

            printf("Schedule alarm at which date and time?\n");

            scanf("%d-%02d-%02d %02d:%02d:%02d", &inputTm.tm_year, &inputTm.tm_mon, &inputTm.tm_mday, &inputTm.tm_hour, &inputTm.tm_min, &inputTm.tm_sec);
            flushBuffer();
            zombieCatcher();

            diff = difftime(mktime(&inputTm), mktime(&tm));
            printf("Scheduling alarm in %d seconds\n", diff);
            pid = fork();
            if (pid == 0)
            { // Child
                sleep(diff);
                printf("ALARM from pid %d! \a\n", getpid());
                char *bin_path = "afplay";
                char *arg1 = "alarm.wav";
                execlp(bin_path, bin_path, arg1, (char *)NULL);
                exit(0);
            }
            else
            { // Parent
                printf("New child: %d\n", pid);
                Alarm newAlarm;
                newAlarm.alarm_pid = pid;
                newAlarm.id = newAlarmIndex + 1;
                newAlarm.date = inputTm;
                alarms[newAlarmIndex] = newAlarm;
            }

            break;

        case 'l':
            for (int i = 0; i < length; i++)
            {
                if (alarms[i].id != 0)
                {
                    printf("Alarm %d: %d-%02d-%02d %02d:%02d:%02d\n\n", alarms[i].id, alarms[i].date.tm_year, alarms[i].date.tm_mon, alarms[i].date.tm_mday, alarms[i].date.tm_hour, alarms[i].date.tm_min, alarms[i].date.tm_sec);
                }
            }
            break;

        case 'c':
            printf("Cancel which alarm?\n");

            scanf("%d", &alarmToCancel);
            flushBuffer();
            zombieCatcher();

            alarms[alarmToCancel - 1].id = 0;
            kill(alarms[alarmToCancel - 1].alarm_pid, SIGKILL);
            break;

        case 'x':
            printf("Goodbye!\n");
            for (int i = 0; i < length; i++)
            {
                if (alarms[i].id > 0)
                {
                    kill(alarms[i].alarm_pid, SIGKILL);
                }
            }
            exit(0);
            break;

        default:
            printf("Invalid command\n");
        }
    }
    return 1;
}

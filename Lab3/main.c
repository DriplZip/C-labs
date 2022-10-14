#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

struct maxTimeRequest
{
    int requestCount;

    long long startTime;
    long long endTime;
};

typedef struct timeNode
{
    long long time;
    int index;

    struct timeNode *next;
    struct timeNode *prev;
} timeNode;

typedef struct timeWindow
{
    struct timeNode *head;
    struct timeNode *tail;
} timeWindow;

void push(timeWindow *timeWindow, long long time, int index)
{
    timeNode *temp = malloc(sizeof (timeNode));

    temp->time = time;
    temp->index = index;
    temp->next = NULL;
    temp->prev = NULL;

    if (timeWindow->head == NULL)
    {
        timeWindow->head = temp;
        timeWindow->tail = temp;
    }
    else
    {
        temp->prev = timeWindow->head;

        timeWindow->head->next = temp;

        timeWindow->head = temp;
    }
}

void pop(timeWindow *timeWindow)
{
    timeNode *temp = timeWindow->tail;

    if (temp != NULL)
    {
        if (temp->next == NULL)
        {
            timeWindow->head = NULL;
            timeWindow->tail = NULL;
        }
        else
        {
            timeWindow->tail = temp->next;
            timeWindow->tail->prev = NULL;
        }

        free(temp);
    }
}

long long parseDate(char *line, int point)
{
    int date = (line[point] - '0') * 10 + (line[point + 1] - '0');
    point += 3;

    char month[4];
    month[0] = line[point];
    month[1] = line[point + 1];
    month[2] = line[point + 2];
    month[3] = '\0';
    point += 4;

    int *yearNumber = malloc(4);
    int yearPoint = 0;

    while (line[point] != ':')
    {
        yearNumber[yearPoint++] = (line[point] - '0');

        yearNumber = realloc(yearNumber, (yearPoint + 1) * sizeof(int));

        point++;
    }

    int year = 0, ten = 1;

    for (int i = sizeof(yearNumber) - 1; i >= 0; i--)
    {
        year += yearNumber[i] * ten;

        ten *= 10;
    }

    free(yearNumber);

    point++;

    int hour = (line[point] - '0') * 10 + (line[point + 1] - '0');
    point += 3;

    int minute = (line[point] - '0') * 10 + (line[point + 1] - '0');
    point += 3;

    int second = (line[point] - '0') * 10 + (line[point + 1] - '0');

    struct tm m_time;

    m_time.tm_mday = date;

    if (strcmp(month, "Jan") == 0) m_time.tm_mon = 0;
    else if (strcmp(month, "Feb") == 0) m_time.tm_mon = 1;
    else if (strcmp(month, "Mar") == 0) m_time.tm_mon = 2;
    else if (strcmp(month, "Apr") == 0) m_time.tm_mon = 3;
    else if (strcmp(month, "May") == 0) m_time.tm_mon = 4;
    else if (strcmp(month, "Jun") == 0) m_time.tm_mon = 5;
    else if (strcmp(month, "Jul") == 0) m_time.tm_mon = 6;
    else if (strcmp(month, "Aug") == 0) m_time.tm_mon = 7;
    else if (strcmp(month, "Sep") == 0) m_time.tm_mon = 8;
    else if (strcmp(month, "Oct") == 0) m_time.tm_mon = 9;
    else if (strcmp(month, "Nov") == 0) m_time.tm_mon = 10;
    else if (strcmp(month, "Dec") == 0) m_time.tm_mon = 11;

    m_time.tm_year = year - 1900;
    m_time.tm_hour = hour;
    m_time.tm_min = minute;
    m_time.tm_sec = second;

    return mktime(&m_time);
}

long long timeParam()
{
    char line[12];
    int i = 0;
    char sym = ' ';

    printf("In the file \"param.txt\" enter the parameter in the format: 00:00:00:00 day:hour:minute:second\n");

    FILE *param;
    param = fopen("param.txt", "r");

    fgets(line, 12, param);

    fclose(param);

    long long day = (line[0] - '0') * 10 + (line[1] - '0');
    long long hour = (line[3] - '0') * 10 + (line[4] - '0');
    long long minute = (line[6] - '0') * 10 + (line[7] - '0');
    long long second = (line[9] - '0') * 10 + (line[10] - '0');

    long long outTime = day * 24 * 60 * 60 + hour * 60 * 60 + minute * 60 + second;

    return outTime;
}

int main()
{
    struct maxTimeRequest maxTimeRequest;
    maxTimeRequest.requestCount = 0;
    maxTimeRequest.startTime = 0;
    maxTimeRequest.endTime = 0;

    long long timeWindowParam = timeParam();

    struct timeWindow timeWindow;
    timeWindow.head = NULL;
    timeWindow.tail = NULL;

    FILE *access_log;
    FILE *log_out;

    log_out = fopen("log_out.txt", "w");

    if ((access_log = fopen("access_log_Jul95", "r")) == NULL)
    {
        printf("ERROR_OPEN_FILE");
        exit(1);
    }

    int lineIndex = 0, sizeLine = 150;
    char *line = malloc(150 * sizeof(char));
    memset(line, 0, sizeof(line));

    int errorCount = 0, indexLog = 1;

    int timeFlagStart = 0, errorFlag = 0, requestHooks = 0, requestStart = 0, requestEnd = 0;

    long long tempTime = 0;

    int symbol = 0;

    while (1)
    {
        symbol = fgetc(access_log);

        if (lineIndex >= sizeLine)
        {
            line = realloc(line, sizeLine * 2 * sizeof(char));

            sizeLine *= 2;
        }

        line[lineIndex++] = symbol;

        if (symbol == '[') timeFlagStart = lineIndex;

        if (symbol == ']')
        {
            tempTime = parseDate(line, timeFlagStart);
        }

        if (symbol == '"')
        {
            requestHooks++;

            if (requestHooks == 1) requestStart = lineIndex;
            if (requestHooks == 2) requestEnd = lineIndex - 1;
        }

        if (requestHooks == 2 && symbol >= '0' && symbol <= '9')
        {
            if (symbol == '5')
            {
                errorFlag = 1;
            } else
            {
                errorFlag = 0;
            }

            requestHooks = 0;
        }

        if (symbol == '\n' || symbol == EOF)
        {
            if (symbol == EOF)
            {
                line[lineIndex - 1] = '\n';
            }

            if (timeWindow.head == NULL || timeWindow.tail->time + timeWindowParam >= tempTime)
            {
                push(&timeWindow, tempTime, indexLog);
            }
            else
            {
                if (timeWindow.head->index - timeWindow.tail->index + 1 > maxTimeRequest.requestCount)
                {
                    maxTimeRequest.requestCount = timeWindow.head->index - timeWindow.tail->index + 1;

                    maxTimeRequest.startTime = timeWindow.tail->time;
                    maxTimeRequest.endTime = timeWindow.head->time;
                }

                int lastTime = timeWindow.tail->time;

                while (timeWindow.tail != NULL && timeWindow.tail->time == lastTime)
                {
                    pop(&timeWindow);
                }

                push(&timeWindow, tempTime, indexLog);
            }

            if (errorFlag)
            {
                for (int i = requestStart; i < requestEnd; i++)
                {
                    fputc(line[i], log_out);
                }

                fprintf(log_out, "%c", '\n');

                errorCount++;
            }

            free(line);

            line = malloc(150 * sizeof(char));
            memset(line, 0, 150);

            sizeLine = 150;
            lineIndex = 0;

            requestHooks = 0;
            timeFlagStart = 0;

            indexLog++;
        }

        if (symbol == EOF)
        {
            if (timeWindow.head->index - timeWindow.tail->index + 1 > maxTimeRequest.requestCount)
            {
                maxTimeRequest.requestCount = timeWindow.head->index - timeWindow.tail->index + 2;

                maxTimeRequest.startTime = timeWindow.tail->time;
                maxTimeRequest.endTime = timeWindow.head->time;
            }

            break;
        }

    }

    fprintf(log_out, "%d\n", errorCount);

    printf("Max request is %d\n", maxTimeRequest.requestCount);

    struct tm *outTime;
    time_t temp = maxTimeRequest.startTime;
    outTime = localtime(&temp);

    printf("Max request begin with: %s", asctime(outTime));

    temp = maxTimeRequest.endTime;
    outTime = localtime(&temp);

    printf("Max request end in: %s", asctime(outTime));

    fclose(access_log);
    fclose(log_out);

    return 0;
}
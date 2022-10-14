#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    unsigned char marker[4];
    int version[2];
    int flags;
    int size;
} Header;

Header header;

typedef struct
{
    unsigned char frameID[5];
    int size;
    unsigned char flags[2];
    int unicode;
    unsigned char unicodeFlags[2];

    char *value;
} Frame;

typedef struct Node
{
    int value;

    struct Node *next;
    struct Node *prev;
} FileForSet;

FileForSet *setValueHead = NULL;
FileForSet *setValueTail = NULL;

int mode = 0;

void push(int b)
{
    FileForSet *byte = (FileForSet *) malloc(sizeof(FileForSet));
    byte->value = b;
    byte->next = NULL;
    byte->prev = NULL;

    if (setValueHead == NULL)
    {

        setValueHead = byte;
        setValueTail = byte;
    }
    else
    {

        byte->prev = setValueTail;
        setValueTail->next = byte;
        setValueTail = byte;
    }
}

void pop()
{
    FileForSet *set = setValueTail;
    if (setValueTail != NULL)
    {
        setValueTail = setValueTail->prev;
        setValueTail->next = NULL;
        free(set);
    }
}

int getByte(FILE *mp3File)
{
    int b = getc(mp3File);

    if (mode)
    {
        push(b);
    }

    return b;
}

int readSize(FILE *mp3File)
{
    int size[4];
    for (int i = 0; i < 4; i++)
    {
        size[i] = getByte(mp3File);
    }

    int mask = 127;

    size[3] = (size[3] & mask);
    size[2] = (size[2] & mask);
    size[1] = (size[1] & mask);
    size[0] = (size[0] & mask);

    int result = 0;
    result += (size[3]);
    result += (size[2] << 7);
    result += (size[1] << 14);
    result += (size[0] << 21);

    return result;
}

void readValueUtf16(FILE *mp3File, Frame *frame)
{
    frame->value = malloc(frame->size / 2);
    frame->size -= 3;

    frame->unicodeFlags[0] = getByte(mp3File);
    frame->unicodeFlags[1] = getByte(mp3File);

    if (frame->unicodeFlags[0] == 0xFF)
    {
        for (int i = 0; i < frame->size / 2; i++)
        {
            char currentSymbol = getByte(mp3File);

            if (currentSymbol == 0)
            {
                frame->value[i] = ' ';
            }
            else
            {
                frame->value[i] = currentSymbol;
            }

            currentSymbol = getByte(mp3File);
        }
    }
    else
    {
        for (int i = 0; i < frame->size / 2; i++)
        {
            char currentSymbol = getByte(mp3File);
            currentSymbol = getByte(mp3File);

            if (currentSymbol == 0)
            {
                frame->value[i] = ' ';
            }
            else
            {
                frame->value[i] = currentSymbol;
            }
        }
    }

    frame->value[frame->size / 2] = '\0';
    frame->size += 3;
}

void readValueUtf8(FILE *mp3File, Frame *frame)
{
    frame->value = malloc(frame->size);
    frame->size--;

    for (int i = 0; i < frame->size; i++)
    {
        char currentSymbol = getByte(mp3File);

        if (currentSymbol == 0)
        {
            frame->value[i] = ' ';
        }
        else
        {
            frame->value[i] = currentSymbol;
        }
    }

    frame->value[frame->size] = '\0';
    frame->size++;
}

Header readHeader(FILE *mp3File)
{
    for (int i = 0; i < 3; i++) header.marker[i] = getByte(mp3File);
    header.marker[3] = '\0';

    if (strcmp(header.marker, "ID3") != 0)
    {
        printf("ERROR: no ID3 in mp3File");
        exit(1);
    }

    for (int i = 0; i < 2; i++) header.version[i] = getByte(mp3File);

    header.flags = getByte(mp3File);
    header.size = readSize(mp3File);

    return header;
}

void outFrameValue(Frame *frame)
{
    if (frame->unicode)
    {
        for (int i = 0; i < frame->size / 2; i++)
        {
            printf("%c", frame->value[i]);
        }
    }
    else
    {
        for (int i = 0; i < frame->size; i++)
        {
            printf("%c", frame->value[i]);
        }
    }
}

void rewriteHeaderSize(Frame *frame, char *propValue)
{
    int sizePropValue = strlen(propValue);
    int newHeaderSize = header.size - frame->size + sizePropValue;

    int size[4] = {0, 0, 0, 0};

    for (int i = 3; i >= 0; i--)
    {
        size[i] = newHeaderSize % 256;
        newHeaderSize /= 256;
    }

    FileForSet *setSize = setValueHead;

    for (int i = 0; i < 6; i++)
    {
        setSize = setSize->next;
    }

    for (int i = 0; i < 4; i++)
    {
        setSize->value = size[i];
        setSize = setSize->next;
    }
}

void rewriteFrame(Frame *frame, char *propValue)
{
    for (int i = 0; i < frame->size; i++)
    {
        pop();
    }

    int sizePropValue = strlen(propValue) + 1;
    int size[4] = {0, 0, 0, 0};

    for (int i = 3; i >= 0; i--)
    {
        size[i] = sizePropValue % 256;
        sizePropValue /= 256;
    }

    FileForSet *setFrame = setValueTail;

    for (int i = 0; i < 2; i++) setFrame = setFrame->prev;

    for (int i = 3; i >= 0; i--)
    {
        setFrame->value = size[i];
        setFrame = setFrame->prev;
    }
    // utf8
    push(0);
    sizePropValue = strlen(propValue);

    setFrame = setValueTail;

    for (int i = 0; i < sizePropValue; i++)
    {
        push(propValue[i]);
    }
}

Frame readFrame(FILE *mp3File, char *propValue, char *propName)
{
    Frame frame;
    frame.frameID[4] = '\0';

    for (int i = 0; i < 4; i++)
    {
        unsigned char currentID = getByte(mp3File);

        if (currentID < '0' || (currentID > '9' && currentID < 'A') || currentID > 'Z')
        {
            frame.value = NULL;

            return frame;
        }

        frame.frameID[i] = currentID;
    }

    frame.size = readSize(mp3File);

    for (int i = 0; i < 2; i++) frame.flags[i] = getByte(mp3File);

    frame.unicode = getByte(mp3File);
    //frame.size -= 1;

    if (frame.unicode)
    {
        readValueUtf16(mp3File, &frame);
    }
    else
    {
        readValueUtf8(mp3File, &frame);
    }

    //frame.size++;

    if (mode && !strcmp(frame.frameID, propName))
    {
        rewriteHeaderSize(&frame, propValue);
        rewriteFrame(&frame, propValue);
    }

    return frame;
}

int parseArgument(char *argv[], char **propName, char **propValue)
{
    int argumentValue = -1;

    char *str = argv[2];
    char *arg = strtok(str, "=");

    if (strcmp(arg, "--show") == 0)
    {
        return 0;
    }

    *propName = strtok(NULL, "");

    if (strcmp(arg, "--get") == 0)
    {
        return 1;
    }

    if (strcmp(arg, "--set") == 0)
    {
        char *valueStr = argv[3];
        *propValue = strtok(valueStr, "=");
        *propValue = strtok(NULL, "");

        mode = 1;

        return 2;
    }

    printf("ERROR: incorrect argument\n");
    exit(1);
}

void showMetaInfo(FILE *mp3File, char *propValue, char *propName)
{
    printf("ID3v2.%d.%d\n", header.version[0], header.version[1]);

    int currentByte = 0;

    while (currentByte < header.size)
    {
        Frame currentFrame = readFrame(mp3File, propValue, propName);
        if (currentFrame.value == NULL) break;
        currentByte += currentFrame.size + 10;

        //if (!strcmp(currentFrame.frameID, "APIC")) continue;

        printf("%s = ", currentFrame.frameID);
        outFrameValue(&currentFrame);
        printf("\n");
    }
}

void getMetaInfo(FILE *mp3File, char *propValue, char *propName)
{
    int currentByte = 0, findPropName = 0;

    while (currentByte < header.size)
    {
        Frame currentFrame = readFrame(mp3File, propValue, propName);
        if (currentFrame.value == NULL) break;
        currentByte += currentFrame.size + 10;

        if (!strcmp(currentFrame.frameID, propName))
        {
            printf("%s = ", currentFrame.frameID);
            outFrameValue(&currentFrame);
            printf("\n");

            findPropName++;
        }
    }

    if (!findPropName)
    {
        printf("ERROR: Incorrect propName\n");
        exit(1);
    }
}

void setMetaInfo(FILE *mp3File, char* filepath, char *propName, char *propValue)
{
    printf("\npropName is: %s - propValue is: %s\n", propName, propValue);

    int currentByte = 0, findPropName = 0;

    while (currentByte < header.size)
    {
        Frame currentFrame = readFrame(mp3File, propValue, propName);

        currentByte += currentFrame.size + 10;

        if (!strcmp(currentFrame.frameID, propName))
        {
            findPropName++;

            int b = 0;
            while (b != EOF)
            {
                b = getByte(mp3File);
            }

            break;
        }
    }

    if (!findPropName)
    {
        printf("ERROR: Incorrect propName\n");
        exit(1);
    }

    fclose(mp3File);
    FILE *outMp3 = fopen(filepath, "wb");

    FileForSet *set = setValueHead;

    while (set != NULL)
    {
        fputc(set->value, outMp3);
        set = set->next;
    }

    printf("set propValue - completed\n");
}

int main(int argc, char *argv[])
{
    char *propValue;
    char *propName;

    if (argc <= 1)
    {
        printf("ERROR: no arguments\n");
        exit(1);
    }

    char *filepath = strtok(argv[1], "=");
    filepath = strtok(NULL, "");

    FILE *mp3File;

    if ((mp3File = fopen(filepath, "rb")) == NULL)
    {
        printf("ERROR: open file\n");
        exit(1);
    }

    int command = parseArgument(argv, &propName, &propValue);

    header = readHeader(mp3File);

    switch (command)
    {
        case 0:
            showMetaInfo(mp3File, propValue, propName);
            break;
        case 1:
            if (propName == NULL)
            {
                printf("ERROR: propName not set");
                exit(1);
            }
            getMetaInfo(mp3File, propValue, propName);
            break;
        case 2:
            if (propName == NULL)
            {
                printf("ERROR: propName not set");
                exit(1);
            }
            setMetaInfo(mp3File, filepath, propName, propValue);
            break;
    }

    return 0;
}

#include <stdio.h>
#include <string.h>
#include <malloc.h>

// --create a.txt b.txt c.txt
void createArchive(char *archiveName, int argc, char *argv[])
{
    FILE *archive = fopen(archiveName, "wb");

    for (int i = 4; i < argc; i++)
    {
        char *fileName = argv[i];
        FILE *fileToAdd;
        if ((fileToAdd = fopen(fileName, "rb")) != NULL)
        {
            int fileSize = 0;

            fseek(fileToAdd, 0, SEEK_END);
            fileSize = ftell(fileToAdd);
            rewind(fileToAdd);

            fprintf(archive, "%d %s %d ", strlen(fileName), fileName, fileSize);
            fseek(archive, 0, SEEK_END);
            int tempSymbol = 0;
            while ((tempSymbol = getc(fileToAdd)) != EOF) putc(tempSymbol, archive);

            putc(' ', archive);
            continue;
        }

        printf("Error open file: %s\nFile not added to archive\n", fileName);
    }

    printf("Archiving was successful\n");
}

void extractArchive(char *archiveName)
{
    FILE *archive = fopen(archiveName, "rb");
    while (1)
    {
        int fileNameSize = 0, fileDataSize = 0;
        fscanf(archive, "%d", &fileNameSize);
        getc(archive);

        if (fileNameSize == 0) break;

        char *fileName = (char *) calloc(fileNameSize + 1, 1);
        for (int i = 0; i < fileNameSize; i++) fileName[i] = getc(archive);
        getc(archive);

        fscanf(archive, "%d", &fileDataSize);
        getc(archive);

        char *fileData = (char *) calloc(fileDataSize + 1, 1);
        for (int i = 0; i < fileDataSize; i++) fileData[i] = getc(archive);
        getc(archive);

        FILE *outFIle = fopen(fileName, "wb");
        fwrite(fileData, 1, fileDataSize, outFIle);
    }

    printf("Extracting was successful\n");
}

void listArchive(char *archiveName)
{
    FILE *archive = fopen(archiveName, "rb");
    while (1)
    {
        int fileNameSize = 0, fileDataSize = 0;
        fscanf(archive, "%d", &fileNameSize);

        if (fileNameSize == 0) break;

        char *fileName = (char *) calloc(fileNameSize + 1, 1);
        for (int i = 0; i < fileNameSize; i++) fileName[i] = getc(archive);

        fscanf(archive, "%d", &fileDataSize);
        for (int i = 0; i < fileDataSize; i++) getc(archive);

        printf("File Name: %s\n", fileName);
    }
}

int main(int argc, char *argv[])
{
    char *archiveName;

    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "--file"))
        {
            archiveName = argv[++i];
        }

        if (!strcmp(argv[i], "--create"))
        {
            createArchive(archiveName, argc, argv);
        }

        if (!strcmp(argv[i], "--extract"))
        {
            extractArchive(archiveName);
        }

        if (!strcmp(argv[i], "--list"))
        {
            listArchive(archiveName);
        }
    }
    return 0;
}

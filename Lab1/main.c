#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        perror("ERROR_NUMBER_OF_ARGUMENTS");
        exit(1);
    }

    FILE *fp;

    if ((fp = fopen(argv[argc - 1], "rb")) == NULL)
    {
        perror("ERROR_OPEN_FILE");
        exit(1);
    }

    int linesCount = 1, wordsCount = 0, bytesCount = 0;
    int symbol;
    int lastChar = ' ', symbolContains = 0;

    while ((symbol = getc(fp)) != EOF)
    {
        bytesCount += 1;

        if (isspace(symbol) != 0 && isspace(lastChar) == 0)
        {
            wordsCount++;
        }

        if (symbol == '\n')
        {
            linesCount++;
            symbolContains = 0;
        }
        else
        {
            symbolContains++;
        }

        lastChar = symbol;
    }

    if (lastChar != ' ' && lastChar != '\n')
    {
        wordsCount++;
    }

    if (symbolContains == 0)
    {
        linesCount--;
    }

    for (int i = 1; i < argc - 1; i++)
    {
        if (strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--lines") == 0)
        {
            printf("%d\n", linesCount);
        }
        else if (strcmp(argv[i], "-c") == 0 || strcmp(argv[i], "--bytes") == 0)
        {
            printf("%d\n", bytesCount);
        }
        else if (strcmp(argv[i], "-w") == 0 || strcmp(argv[i], "--words") == 0)
        {
            printf("%d\n", wordsCount);
        }
        else
        {
            perror("ERROR_OPTION");

            printf("%s", "OPTION \"");
            printf("%s", argv[i]);
            printf("%s\n", "\" NOT FOUND");
        }
    }

    fclose(fp);

    return 0;
}
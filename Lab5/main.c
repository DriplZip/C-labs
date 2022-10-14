#define _CRT_SECURE_NO_WARNINGS
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

char *fileName = "";
char *output = "";

#pragma pack(push, 1)

typedef struct
{
    char type[2]; // тип 2
    int size; // размер рисунка 4
    short reserved1; // зарезервированные байты 2
    short reserved2; // зарезервированные байты 2
    int offBits; // смещение расположения изображения 4
} bmpFileHeader;

typedef struct
{
    int size; // размер блока 4
    int width; // ширина изображения (в пикселях) 4
    int height; // высота изображения 4
    short planes; // количество цветовых плоскостей 2
    short bitCount; // глубина цвета 2
    int compression; // тип сжатия 4
    int sizeImage; // размер изображения 4
    int xPixPerMeter; // горизонтальное разрешение 4
    int yPixPerMeter; // вертикальное разрешение 4
    int clrUsed; // количество цветов 4
    int clrImportant; // количество цветов (во время обработки) 4
} bmpImageHeader;

#pragma pack(pop)

typedef struct
{
    unsigned char rgbBlue;
    unsigned char rgbGreen;
    unsigned char rgbRed;
    unsigned char rgbReserved;
} RGBPixel;

int lifeCheck(RGBPixel pixel)
{
    if (pixel.rgbBlue != 255 && pixel.rgbGreen != 255 && pixel.rgbRed != 255)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

RGBPixel **fillPixel(FILE *file, bmpImageHeader infoHeader)
{
    RGBPixel **field = (RGBPixel **) calloc(1 ,infoHeader.height * sizeof(RGBPixel *));

    for (int i = 0; i < infoHeader.height; i++)
    {
        field[i] = (RGBPixel *) calloc(1, infoHeader.width * sizeof(RGBPixel));
    }

    for (int i = infoHeader.height - 1; i >= 0; i--)
    {
        for (int j = 0; j < infoHeader.width; j++)
        {
            field[i][j].rgbBlue = getc(file);
            field[i][j].rgbGreen = getc(file);
            field[i][j].rgbRed = getc(file);
        }

        if (infoHeader.width % 4 != 0) getc(file);
    }

    return field;

}

void createBmpFile(FILE *file, bmpFileHeader header, bmpImageHeader infoHeader, RGBPixel **pixels)
{
    fwrite(&header, sizeof(bmpFileHeader), 1, file);
    fwrite(&infoHeader, sizeof(bmpImageHeader), 1, file);

    for (int i = infoHeader.height - 1; i >= 0; i--)
    {
        for (int j = 0; j < infoHeader.width; j++)
        {
            putc(pixels[i][j].rgbBlue, file);
            putc(pixels[i][j].rgbGreen, file);
            putc(pixels[i][j].rgbRed, file);
        }

        if (infoHeader.width % 4 != 0) putc(0, file);
    }
}

RGBPixel **createImage(int **in, bmpImageHeader infoHeader)
{
    RGBPixel **image = (RGBPixel **) calloc(1, infoHeader.height * sizeof(RGBPixel *));

    for (int i = 0; i < infoHeader.height; ++i)
    {
        image[i] = (RGBPixel *) calloc(1, infoHeader.width * sizeof(RGBPixel));
    }

    for (int i = infoHeader.height - 1; i >= 0; i--)
    {
        for (int j = 0; j < infoHeader.width; j++)
        {
            if (in[i][j])
            {
                image[i][j].rgbBlue = 0;
                image[i][j].rgbGreen = 0;
                image[i][j].rgbRed = 0;
            }
            else
            {
                image[i][j].rgbBlue = 255;
                image[i][j].rgbGreen = 255;
                image[i][j].rgbRed = 255;
            }
        }
    }

    return image;
}

int main(int argc, char *argv[])
{
    int maxIterations = INT_MAX, dumpFrequency = 1;

    for (int i = 1; i < argc; i += 2)
    {
        if (!strcmp(argv[i], "--input")) fileName = argv[i + 1];

        if (!strcmp(argv[i], "--output")) output = argv[i + 1];

        if (!strcmp(argv[i], "--max_iter")) maxIterations = atoi(argv[i + 1]);

        if (!strcmp(argv[i], "--dump_freq")) dumpFrequency = atoi(argv[i + 1]);
    }

    FILE *bmpFile;

    if ((bmpFile = fopen(fileName, "rb")) == NULL)
    {
        printf("ERROR: open file\n");
        exit(1);
    }

    bmpFileHeader fileHeader;
    fread(&fileHeader, sizeof(bmpFileHeader), 1, bmpFile);

    bmpImageHeader infoHeader;
    fread(&infoHeader, sizeof(bmpImageHeader), 1, bmpFile);

    RGBPixel **rgb = fillPixel(bmpFile, infoHeader);

    int **lastAge = (int **) malloc(infoHeader.height * sizeof(int*));

    for (int i = 0; i < infoHeader.height; i++)
    {
        lastAge[i] = (int *) malloc(infoHeader.width * sizeof(int));
    }

    for (int i = 0; i < infoHeader.height; i++)
    {
        for (int j = 0; j < infoHeader.width; j++)
        {
            lastAge[i][j] = (lifeCheck(rgb[i][j]) ? 1 : 0);
        }
    }

    int **newAge = (int **) malloc(infoHeader.height * sizeof(int *));

    for (int i = 0; i < infoHeader.height; i++)
    {
        newAge[i] = (int *) malloc(infoHeader.width * sizeof(int));
    }

    for (int i = 0; i < infoHeader.height; i++)
    {
        for (int j = 0; j < infoHeader.width; j++)
        {
            newAge[i][j] = (lifeCheck(rgb[i][j]) ? 1 : 0);
        }
    }

    fclose(bmpFile);

    char pathOutFile[265];
    int up = 0, down = 0, left = 0, right = 0, neighborhoods = 0;

    for (int k = 0; k < maxIterations; k++)
    {
        for (int i = 0; i < infoHeader.height; i++)
        {
            up = i - 1;
            down = i + 1;

            if (i == 0) up = infoHeader.height - 1;
            if (i == infoHeader.height - 1) down = 0;

            for (int j = 0; j < infoHeader.width; j++)
            {
                left = j - 1;
                right = j + 1;

                if (j == 0) left = infoHeader.width - 1;
                if (j == infoHeader.width - 1) right = 0;

                neighborhoods =
                        lastAge[up][left] + lastAge[up][j] + lastAge[up][right] + lastAge[i][left] + lastAge[i][right] +
                        lastAge[down][left] + lastAge[down][j] + lastAge[down][right];

                if (lastAge[i][j])
                {
                    if (neighborhoods < 2 || neighborhoods > 3)
                    {
                        newAge[i][j] = 0;
                    }
                    else
                    {
                        newAge[i][j] = 1;
                    }
                }
                else
                {
                    if (neighborhoods == 3)
                    {
                        newAge[i][j] = 1;
                    }
                    else
                    {
                        newAge[i][j] = 0;
                    }
                }
            }
        }

        for (int i = 0; i < infoHeader.height; i++)
        {
            for (int j = 0; j < infoHeader.width; j++)
            {
                lastAge[i][j] = newAge[i][j];
            }
        }

        if (!(k % dumpFrequency))
        {
            memset(pathOutFile, 0, 265);
            strcpy(pathOutFile, output);
            sprintf(pathOutFile, "%s%d.bmp", pathOutFile, k);

            bmpFile = fopen(pathOutFile, "w");

            createBmpFile(bmpFile, fileHeader, infoHeader, createImage(lastAge, infoHeader));

            fclose(bmpFile);
        }

        if (k == INT_MAX - 1) k = -1;
    }
}
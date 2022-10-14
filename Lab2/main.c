#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define MODULE 1000000000
#define SEGMENTSIZE 9
#define UINT1024SIZE 35 // 1120 бит ~ на 9,5% больше


typedef struct uint1024
{
    uint32_t arr[UINT1024SIZE];
} uint1024_t;

uint1024_t from_uint(unsigned int generatedNumber)
{
    uint1024_t value;
    memset(value.arr, 0, sizeof (value.arr));

    value.arr[UINT1024SIZE - 1] = generatedNumber;

    return value;
}

uint1024_t add_op(uint1024_t x, uint1024_t y)
{
    uint1024_t value;
    memset(value.arr, 0, sizeof(value.arr));

    uint8_t carry = 0;

    for (int i = UINT1024SIZE - 1; i >= 0; i--)
    {
        uint64_t temp = carry + x.arr[i] + y.arr[i];

        value.arr[i] = temp % MODULE;

        carry = temp / MODULE;
    }

    return value;
}

uint1024_t subtr_op(uint1024_t x, uint1024_t y)
{
    uint1024_t value;
    memset(value.arr, 0, sizeof(value.arr));

    uint8_t carry = 0;

    for (int i = UINT1024SIZE - 1; i >= 0; i--)
    {
        uint32_t temp = (MODULE + x.arr[i]) - (carry + y.arr[i]);

        value.arr[i] = temp % MODULE;

        carry = (temp >= MODULE) ? 0 : 1;
    }

    return value;
}

uint1024_t mult_op(uint1024_t x, uint1024_t y)
{
    uint1024_t value;
    memset(value.arr, 0, sizeof(value.arr));

    for (unsigned int i = UINT1024SIZE - 1; i != -1; i--)
    {
        uint64_t carry = 0;

        for (unsigned int j = UINT1024SIZE - 1; j != -1; j--) //< ((unsigned int)(0 - 1))
        {
<<<<<<< HEAD
            if ((i + j - UINT1024SIZE + 1) > 34) break; //(i + j - UINT1024SIZE + 1) < 0 ||
=======
           if ((i + j - UINT1024SIZE + 1) > 34 || (i + j - UINT1024SIZE + 1) < 0) break;
>>>>>>> f89f079ec761230bf293167a8a7accd6555c8964

            uint64_t temp = 1ull * value.arr[i + j - UINT1024SIZE + 1] + 1ull * x.arr[j] * y.arr[i] + 1ull *carry;

            value.arr[i + j - UINT1024SIZE + 1] = temp % MODULE;

            carry = temp / MODULE;
        }
    }

    return value;
}

void printf_value(uint1024_t outValue)
{
    int i = 0, before = UINT1024SIZE - 1;

    while (outValue.arr[i] == 0) i++;

    if (i == UINT1024SIZE)
    {
        printf("%u", 0);
    }

    printf("%u", outValue.arr[i++]);

    for (; i < UINT1024SIZE; i++)
    {
            printf("%.9u", outValue.arr[i]);
    }

    printf("%c", '\n');
}

unsigned int unsignedParse(uint8_t *temp)
{
    unsigned int out = 0, step = 100000000;

    for (int i = 0; i < 9; i++)
    {
        unsigned int add = temp[i] - '0';

        unsigned int val = step * add;
        step/= 10;
        out += val;
    }

    return out;
}

void scanf_value(uint1024_t *value)
{
    uint8_t inValue[315];
    memset(inValue, 0, sizeof(inValue));

    int pointFromInValue = 0, symbol = 0, countInValue = 0;
    while ((symbol = getchar()) != EOF)
    {
        if (symbol == '\n')
        {
            break;
        }

        if (pointFromInValue > 315)
        {
            perror("UINT1024_OVERFLOW");

            exit(1);
        }

        countInValue++;

        inValue[pointFromInValue++] = symbol;
    }

    memset(value->arr, 0, sizeof(value->arr));

    int valueIndex = UINT1024SIZE - 1;

    uint8_t temp[SEGMENTSIZE];
    memset(temp, '0', sizeof(temp));

    int index = SEGMENTSIZE - 1;

    for (int i = countInValue - 1; i >= 0; i--)
    {
        temp[index] = inValue[i];

        if (index == 0)
        {
            index = SEGMENTSIZE - 1;

            value->arr[valueIndex--] = unsignedParse(temp);

            memset(temp, '0', sizeof(temp));

            continue;
        }
        index--;
    }

    if (index != 8 && valueIndex >= 0)
    {
        value->arr[valueIndex] = unsignedParse(temp);
    }
}



int main()
{
    uint1024_t a;
    scanf_value(&a);

    uint1024_t b;
    scanf_value(&b);

    a = mult_op(a, b);

    printf_value(a);

    return 0;
}
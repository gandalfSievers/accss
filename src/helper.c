/*
 * ACCSS C Code Copyright (C) 2014 by Gandalf Sievers
 * Created by Gandalf Sievers on 23.08.14.
 *
 * MIT-License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "helper.h"

void memoryFailure()
{
    fprintf(stderr, "Out of memory.\n");
}

/*
 * Reimplement of non ansii function strcasecmp for portability
 *
 */
size_t casecmp(const char* str1, const char* str2)
{
    size_t i = 0,
    k = 0;

    if(strlen(str1) != strlen(str2))
    {
        return 1;
    }

    while(str1[i] != '\0' && str2[i] != '\0')
    {
        char a = str1[i];
        char b = str2[i];

        if(a >= 97 && a <= 122)
        {
            a -= 32;
        }

        if(b >= 97 && b <= 122)
        {
            b -= 32;
        }

        if(a != b)
        {
            if(a > b)
            {
                k = 1;
            }
            else
            {
                k = -1;
            }
            break;
        }
        i++;
    }
    return k;
}

char* resizeValue(char* old, const char* new)
{
    size_t len = strlen(new);

    char* tmp = realloc(old, sizeof(char)*(len+1));
    if(tmp == NULL)
    {
        memoryFailure();
        exit(EXIT_FAILURE);
    }

    old = tmp;
    strcpy(old, new);
    return old;
}

char* copyValue(const char* value)
{
    if(value == NULL)
    {
        return NULL;
    }
    else
    {
        size_t len = strlen(value);
        char* newValue = malloc(sizeof(char) * (len+1));
        strcpy(newValue, value);

        return newValue;
    }
}

char** copyCharList(char** charList)
{
    if(charList == NULL)
    {
        return NULL;
    }
    else
    {
        char** tmp = charList;
        char** newlist = NULL;
        size_t len = 0, i = 0;


        while(*tmp != NULL)
        {
            len++;
            tmp++;
        }

        newlist = malloc(sizeof(char*)*(len+1));
        for(;i < len;i++)
        {
            newlist[i] = copyValue(charList[i]);
        }

        newlist[len] = NULL;
        return newlist;
    }
}

void freeCharList(char** charList)
{
    char** tmp = charList;
    if(tmp != NULL)
    {
        while(*tmp != NULL)
        {
            free(*tmp);
            tmp++;
        }

        free(charList);
    }
}

size_t charListLength(char** charList)
{
    size_t len = 0;

    if(charList != NULL && *charList != NULL)
    {
        while(*charList != NULL)
        {
            len++;
            charList++;
        }
    }
    return len;
}

char* lowerCase(char* string)
{
    if(string == NULL)
    {
        return NULL;
    }
    else
    {
        char* start = string;
        while(*string != '\0')
        {
            if(*string >= 65 && *string <= 90)
            {
                *string += 32;
            }
            string++;
        }

        return start;
    }
}

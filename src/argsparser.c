/*
 * ACCSS C Code Copyright (C) 2014 by Gandalf Sievers
 * Created by Gandalf Sievers on 31.12.14.
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

#include "argsparser.h"

struct args* findSingleOption(struct args *args, const char* optlong, char opt)
{
    struct args* tmp = args;

    while ((*tmp).letter != 0)
    {
        if ((optlong != NULL && strcmp((*tmp).longName, optlong) == 0 ) || (opt != 0 && (*tmp).letter == opt))
        {
            (*tmp).found = (*tmp).found != 0 ? (*tmp).found+1 : !(*tmp).needsValue;
            return tmp;
        }
        tmp++;
    }

    return NULL;
}

char** argsparser(struct args *args, int* ulen, int* error, int argc, const char* argv[])
{
    int i = 1;
    char wasArg = 0;
    char** unordered = NULL;
    struct args* last = NULL;
    *error = 0;
    *ulen = 0;

    for (; i < argc; i++)
    {
        if (!wasArg && argv[i][0] == '-')
        {
            if (argv[i][1] == '-' && strlen(argv[i]) > 2)
            {
                last = findSingleOption(args, &argv[i][2], 0);
                if(last != NULL)
                {
                    wasArg = last->needsValue;
                }
                else
                {
                    (*error)++;
                }
            }
            else if(strlen(argv[i]) == 2)
            {
                last = findSingleOption(args, NULL, argv[i][1]);
                if(last != NULL)
                {
                    wasArg = last->needsValue;
                }
                else
                {
                    (*error)++;
                }
            }
            else
            {
                (*error)++;
            }
        }
        else if (wasArg)
        {
            size_t len = strlen(argv[i]);

            wasArg = 0;
            if(last == NULL)
            {
                (*error)++;
            }
            else
            {
                if (last->value != NULL)
                {
                    free(last->value);
                }

                last->value = malloc(sizeof(char)*(len+1));
                if(last->value == NULL)
                {
                    exit(EXIT_FAILURE);
                }

                strcpy(last->value, argv[i]);
                last->found++;

                last = NULL;
            }
        }
        else
        {
            size_t len = strlen(argv[i]);
            char** utmp = realloc(unordered, sizeof(char*)*(*ulen + 2));
            if(utmp == NULL)
            {
                exit(EXIT_FAILURE);
            }

            unordered = utmp;
            unordered[*ulen] = malloc(sizeof(char)*(len+1));
            if(unordered[*ulen] == NULL)
            {
                exit(EXIT_FAILURE);
            }

            strcpy(unordered[*ulen], argv[i]);
            (*ulen)++;
            unordered[*ulen] = NULL;
        }
    }

    return unordered;
}

void freeArgValues(struct args* args)
{
    struct args* tmp = args;

    while ((*tmp).letter != 0)
    {
        if((*tmp).value!=NULL)
        {
            free((*tmp).value);
            (*tmp).value = NULL;
        }
        tmp++;
    }
}

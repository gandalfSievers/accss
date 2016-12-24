/*
 * ACCSS C Code Copyright (C) 2014 by Gandalf Sievers
 * Created by Gandalf Sievers on 13.07.14.
 *
 * CSS parsing rules, ASTree creation, compression and translation based on CSSO
 * Original NodeJS CSSO code - Copyright (C) 2011 by Sergey Kryzhanovsky
 *
 * see CSSO https://github.com/css/csso
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "argsparser.h"
#include "ast.h"
#include "helper.h"
#include "gettokens.h"
#include "getpast.h"
#include "compressor.h"
#include "translator.h"
#include "debug.h"
#include "iohelper.h"

#ifdef _M_X64
#define VERSION "1.1.4 x86_64 Win"
#elif __x86_64__
#define VERSION "1.1.4 x86_64"
#else
#define VERSION "1.1.4"
#endif

void usage(const char* name)
{
    printf("%s [options] [input...]\n", name);
    printf("-h\t--help\t\t\tprint this help\n");
    printf("-o\t--output\t\t\twrite output to file (instead of stdout)\n");
    printf("-s\t--stats\t\t\tprint stats (if output present)\n");
    printf("-r\t--restructure-off\t\tcompression without restructure\n");
    printf("-p\t--preserve-splitted\t\tdont merge splitted shorthands\n");
    printf("-c,\t--compat\t\t\tcompatibility to browser versions\n");
    printf("\t\t\t\t\t\tall, ie7, ie8, ie9, ie10, ie11, chrome, future, none\n");
    printf("\t\t\t\t\t\t(option includes options right of it)\n");
}

void checkCompat(unsigned char* compat, const char* opt, int* error)
{
    if(strcmp(opt, "all") == 0)
    {
        *compat = ACCSSOPTION_ALL;
    }
    else if(strcmp(opt, "ie7") == 0)
    {
        *compat = ACCSSOPTION_ALL;
    }
    else if(strcmp(opt, "ie8") == 0)
    {
        *compat = ACCSSOPTION_GE_IE8;
    }
    else if(strcmp(opt, "ie9") == 0)
    {
        *compat = ACCSSOPTION_GE_IE9;
    }
    else if(strcmp(opt, "ie10") == 0)
    {
        *compat = ACCSSOPTION_GE_IE10;
    }
    else if(strcmp(opt, "ie11") == 0)
    {
        *compat = ACCSSOPTION_GE_IE11;
    }
    else if(strcmp(opt, "chrome") == 0)
    {
        *compat = ACCSSOPTION_GE_CHROME;
    }
    else if(strcmp(opt, "future") == 0)
    {
        *compat = ACCSSOPTION_FUTURE;
    }
    else if(strcmp(opt, "none") == 0)
    {
        *compat = ACCSSOPTION_NONE;
    }
    else
    {
        (*error)++;
    }
}

int main(int argc, const char **argv)
{
    int ulen = 0, error = 0;
    unsigned char compat = ACCSSOPTION_ALL;
    char input = 0;
    char* output = NULL;
    char* string = NULL;

    struct args options[] =
    {
        {'v', "version", 0, 0, NULL},
        {'h', "help", 0, 0, NULL },
        {'r', "restructure-off", 0, 0, NULL},
        {'p', "preserve-splitted", 0, 0, NULL},
        {'c', "compat", 1, 0, NULL},
        {'s', "stats", 0, 0, NULL},
        {'o', "output", 1, 0, NULL },
        {0, NULL, 0, 0, NULL}
    };

    char** unordered = argsparser(options, &ulen, &error, argc, argv);

    if(options[0].found)
    {
        printf("%s\n", VERSION);
        exit(EXIT_SUCCESS);
    }

    if(options[1].found)
    {
        usage(argv[0]);
        exit(EXIT_SUCCESS);
    }

    if(error)
    {
        fprintf(stderr, "Unknown options");
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    if(options[4].found)
    {
        checkCompat(&compat, options[4].value, &error);
        free(options[4].value);
        options[4].value = NULL;
    }

    /* Unordered Options */
    if (ulen > 0)
    {
        int i = 0;
        size_t len = 0;
        for(; i < ulen; i++)
        {
            if(!getStringFromFile(unordered[i], &string, &len))
            {
                exit(EXIT_FAILURE);
            }
        }
        input = 1;
    }

    freeCharList(unordered);

    if(input == 0)
    {
        size_t len = 0;
        getStringFromStdin(&string, &len);
    }

    if(string != NULL)
    {
        clock_t start = clock();

        size_t inlen = strlen(string);
        size_t outlen = 0;
        char*  outstr = NULL;
        char o = 0;
        int msec = 0;
        clock_t clockdiff = 0;

        if(inlen > 0)
        {
            char error = 0;
            struct astnode* stylesheet = NULL;
            struct token_info tokens = getTokens(string, &error);

            free(string);

            if(error)
            {
                printf("Error Tokenizing CSS\n");
                deleteTokens(&tokens);
                exit(EXIT_FAILURE);
            }

    #ifdef DEBUG
            printTokens(&tokens);
            fflush(stdout);
    #endif
            stylesheet = getAST(&tokens, 0, &error);

            deleteTokens(&tokens);
            if(error)
            {
                printf ("Error creating AST tree\n");
                exit(EXIT_FAILURE);
            }

    #ifdef DEBUG
            printASTNodeJSON(stylesheet, 0);
            fflush(stdout);
    #endif

            stylesheet = compress(stylesheet, !options[2].found, !options[3].found, compat);

    #ifdef DEBUG
            printf("\n\n=========================================================\n\n");
            printASTNodeJSON(stylesheet, 0);
            printf("\n\n=========================================================\n");
            fflush(stdout);
    #endif

            outstr = translate(stylesheet);

            deleteASTTree(stylesheet);

            outlen = strlen(outstr);

        }
        else
        {
            outlen = inlen;
            outstr = string;
        }
        o = writeOutput(options[6].value, outstr);

        free(outstr);

        if(!o)
        {
            fprintf(stderr, "Could write output file %s!\n", options[6].value);
            exit(EXIT_FAILURE);
        }

        clockdiff = clock()-start;
        msec = (int)(clockdiff * 1000 / CLOCKS_PER_SEC);

        if(output != NULL && options[5].found)
        {
            float ratio = outlen < inlen ? ((float)outlen / (float)inlen) : 0;
            float insize = (float)inlen / 1024;
            float outsize = (float)outlen / 1024;
            printf("Total time: %d seconds %d milliseconds, ratio: %.2f%% insize: %.2fKB outsize %.2fKB\n", msec/1000, msec%1000, ratio, insize, outsize);
        }

    }

    freeArgValues(options);

    if(output!=NULL)
    {
        free(output);
    }

    exit(0);
}

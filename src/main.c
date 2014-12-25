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
#include <getopt.h>
#include <string.h>
#include <time.h>
#include "ast.h"
#include "helper.h"
#include "gettokens.h"
#include "getpast.h"
#include "compressor.h"
#include "translator.h"
#include "debug.h"

#define VERSION "1.0"

#define MAX_FILEBUFFER_SIZE 400000000
#define STDBUFFERSIZE 2048

#define OPTION_VERSION 1
#define OPTION_HELP 2
#define OPTION_PARSER 4
#define OPTION_RESTRUCTURE_OFF 8
#define OPTION_STATS 16
#define OPTION_PRESERVESPLITTED 32

char writeOutput(const char* filename, const char* string)
{
    if(filename == NULL)
    {
        printf("%s\n", string);
    }
    else
    {
        FILE* f = fopen(filename, "w");
        if(f)
        {
            fputs(string, f);
            fclose(f);
        }
        else
        {
            return 0;
        }
    }
    return 1;
}

char getStringFromFile(const char* filename, char** string)
{
    FILE* file = fopen(filename, "rb");
    if(file)
    {
        fseek(file, 0, SEEK_END);
        size_t bufferlen = ftell(file);
        rewind(file);

        if(bufferlen<MAX_FILEBUFFER_SIZE)
        {
            char* filebuffer = (char*)malloc(sizeof(char)*bufferlen);

            if(filebuffer==NULL)
            {
                fprintf(stderr, "Out of memory!\n");
                fclose(file);
                return 0;
            }
            else
            {
                size_t res = fread(filebuffer, sizeof(char), bufferlen, file);
                if(bufferlen!=res)
                {
                    fprintf(stderr, "Error reading file!\n");
                    fclose(file);
                    free(filebuffer);
                    return 0;
                }
                else
                {
                    *string = filebuffer;
                }
            }
        }
        else
        {
            fprintf(stderr, "File too big!\n");
            fclose(file);
            return 0;
        }

        fclose(file);
    }
    else
    {
        fprintf(stderr, "Could not open Input file!\n");
        return 0;
    }
    return 1;
}

void usage(const char* name)
{
    printf("%s [options] [input] [output]\n", name);
    printf("-h\t--help\t\t\tprint this help\n");
    printf("-s\t--stats\t\t\tprint stats (if output present)\n");
    printf("-r\t--restructure-off\tcompression without restructure\n");
    printf("-p\t--preserve-splitted\tdont merge splitted shorthands\n");
    printf("-c,\t--compat\t\tcompatibility to browser versions\n");
    printf("\t\t\t\tall, ie7, ie8, ie9, ie10, ie11, chrome, future, none\n");
    printf("\t\t\t\t(option includes options right of it)\n");
}

void checkCompat(unsigned char* compat, const char* opt, char* error)
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

int main(int argc, char **argv)
{
    int c;
    char error = 0;
    unsigned char flags = 0;
    unsigned char compat = ACCSSOPTION_ALL;
    char* input = NULL;
    char* output = NULL;

    while(1)
    {
        static struct option long_options[] =
        {
            /* These options don't set a flag.
             We distinguish them by their indices. */
            {"version",     no_argument,       0, 'v'},
            {"help",  no_argument,       0, 'h'},
            {"restructure-off",  no_argument, 0, 'r'},
            {"preserve-splitted",    no_argument, 0, 'p'},
            {"compat",    required_argument, 0, 'c'},
            {"stats",    no_argument, 0, 's'},
            {0, 0, 0, 0}
        };
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "pvshrc:", long_options, &option_index);

        /* Detect the end of the options. */
        if(c == -1)
        {
            break;
        }

        switch(c)
        {
            case 'v':
            {
                flags |= OPTION_VERSION;
            }
            break;

            case 'h':
            {
                flags |= OPTION_HELP;
            }
            break;

            case 'r':
            {
                flags |= OPTION_RESTRUCTURE_OFF;
            }
            break;

            case 'p':
            {
                flags |= OPTION_PRESERVESPLITTED;
            }
                break;

            case 's':
            {
                flags |= OPTION_STATS;
            }
                break;

            case 'c':
            {
                char* comstr = copyValue(optarg);
                checkCompat(&compat, comstr, &error);
                free(comstr);
            }
                break;

            default:
            {
                error++;
            }
        }
    }

    if(flags & OPTION_PRESERVESPLITTED)
    {
        printf("p\n");
        exit(EXIT_SUCCESS);
    }

    if(flags & OPTION_VERSION)
    {
        printf("%s\n", VERSION);
        exit(EXIT_SUCCESS);
    }

    if(flags & OPTION_HELP)
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

    /* Unknown options. */
    if(optind < argc)
    {
        if(argc - optind > 2)
        {
            fprintf(stderr, "Unknown options");
            usage(argv[0]);
            exit(EXIT_FAILURE);
        }

        input=copyValue(argv[optind++]);
        if(optind < argc)
        {
            output=copyValue(argv[optind]);
        }
    }

    char* string = NULL;

    if(input!=NULL)
    {
        if(!getStringFromFile(input, &string))
        {
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        char buffer[STDBUFFERSIZE];
        size_t bufflen = 0;
        size_t readlen = 0;
        size_t oldlen;
        while((readlen = fread(buffer, sizeof(char), STDBUFFERSIZE, stdin)) > 0)
        {
            oldlen = bufflen;
            bufflen += readlen;
            char* tmp = realloc(string, sizeof(char)*(bufflen+1));
            if(tmp == NULL)
            {
                memoryFailure();
                exit(EXIT_FAILURE);
            }

            string = tmp;
            char* start = string + oldlen;

            memcpy(start, buffer, readlen);

        }
        if(bufflen > 0)
        {
            string[bufflen] = '\0';
        }
    }

    if(string != NULL)
    {
        clock_t start = clock();

        size_t inlen = strlen(string);
        size_t outlen = 0;
        char*  outstr = NULL;

        if(inlen > 0)
        {
            char error = 0;
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
            struct astnode* stylesheet = getAST(&tokens, 0, &error);

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

            stylesheet = compress(stylesheet, !(flags & OPTION_RESTRUCTURE_OFF), !(flags & OPTION_PRESERVESPLITTED), compat);

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
        char o = writeOutput(output, outstr);

        free(outstr);

        if(!o)
        {
            fprintf(stderr, "Could write output file %s!\n", output);
            exit(EXIT_FAILURE);
        }

        clock_t clockdiff = clock()-start;
        int msec = (int)(clockdiff * 1000 / CLOCKS_PER_SEC);

        if(output != NULL && flags & OPTION_STATS)
        {
            float ratio = outlen < inlen ? ((float)outlen / (float)inlen) : 0;
            float insize = (float)inlen / 1024;
            float outsize = (float)outlen / 1024;
            printf("Total time: %d seconds %d milliseconds, ratio: %.2f%% insize: %.2fKB outsize %.2fKB\n", msec/1000, msec%1000, ratio, insize, outsize);
        }

    }

    if(output!=NULL)
    {
        free(output);
    }

    if(input!=NULL)
    {
        free(input);
    }

    exit(0);
}
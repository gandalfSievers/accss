/*
 * ACCSS C Code Copyright (C) 2014 by Gandalf Sievers
 * Created by Gandalf Sievers on 07.02.15.
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

#include "iohelper.h"


#define MAX_FILEBUFFER_SIZE 400000000
#define STDBUFFERSIZE 2048

char writeOutput(const char* filename, const char* string)
{
    if(filename == NULL)
    {
        printf("%s", string);
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

char getStringFromFile(const char* filename, char** string, size_t *len)
{
    FILE* file = fopen(filename, "rb");
    if(file)
    {
        size_t bufferlen = 0;
        
        fseek(file, 0, SEEK_END);
        bufferlen = ftell(file);
        rewind(file);
        
        if(bufferlen<MAX_FILEBUFFER_SIZE)
        {
            char* filebuffer = (char*)malloc(sizeof(char)*(bufferlen+1));
            
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
                    char* tmp = NULL;
                    filebuffer[bufferlen] = '\0';
                    
                    tmp = realloc(*string, *len+bufferlen+1);
                    if(tmp == NULL)
                    {
                        memoryFailure();
                        exit(1);
                    }
                    *string = tmp;
                    
                    memcpy(&(*string)[*len], filebuffer, bufferlen+1);
                    free(filebuffer);
                    
                    *len += bufferlen;
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

char getStringFromStdin(char** string, size_t* len)
{
    char buffer[STDBUFFERSIZE];
    *len = 0;
    size_t readlen = 0;
    size_t oldlen = 0;
    
    while((readlen = fread(buffer, sizeof(char), STDBUFFERSIZE, stdin)) > 0)
    {
        char* tmp = NULL,
        *start = NULL;
        
        oldlen = *len;
        *len += readlen;
        tmp = realloc(*string, sizeof(char)*(*len+1));
        if(tmp == NULL)
        {
            memoryFailure();
            exit(EXIT_FAILURE);
        }
        
        *string = tmp;
        start = *string + oldlen;
        
        memcpy(start, buffer, readlen);
        
    }
    if(*len > 0)
    {
        (*string)[*len] = '\0';
        return 1;
    }
    return 0;
}
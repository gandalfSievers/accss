/*
 * ACCSS C Code Copyright (C) 2014 by Gandalf Sievers
 * Created by Gandalf Sievers on 18.07.14.
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
#include <string.h>
#include <stdlib.h>
#include "helper.h"
#include "gettokens.h"
#include "tokentypes.h"

char isDecimalDigit(const char c)
{
    if(c >= 48 && c<= 57)
    {
        return 1;
    }
    return 0;
}

char isPunctuation(struct char_char* punctuation, char c)
{
    struct char_char* p = punctuation;
    while(p->c != 0)
    {
        if(p->c == c)
        {
            return p->type;
        }

        p++;
    }
    return 0;
}

struct token* pushToken(struct token_info* info, int *tn, int ln, char type, char* value)
{
    struct token* newtoken = malloc(sizeof(struct token));
    if(newtoken == NULL)
    {
        memoryFailure();
        exit(EXIT_FAILURE);
    }

    memset(newtoken, 0, sizeof(struct token));

    (*tn)++;
    newtoken->tn = *tn;
    newtoken->ln = ln;
    newtoken->type = type;

    size_t len = strlen(value);
    newtoken->value = malloc(sizeof(char)*(len+1));
    if(newtoken->value == NULL)
    {
        memoryFailure();
        exit(EXIT_FAILURE);
    }

    strcpy(newtoken->value, value);
    info->len++;
    struct token** tmpList  = realloc(info->list, (sizeof(struct token*))*info->len);
    if(tmpList == NULL)
    {
        memoryFailure();
        exit(EXIT_FAILURE);
    }

    info->list = tmpList;
    info->list[info->len-1] = newtoken;

    return newtoken;
}

void deleteTokens(struct token_info* info)
{
    size_t i = 0;
    for(; i < info->len; i++)
    {
        free(info->list[i]->value);
        free(info->list[i]);
    }

    free(info->list);
}

struct token* getTokenByIndex(struct token_info* info, size_t index)
{
    if(index >= info->len)
    {
        return NULL;
    }

    return info->list[index];
}

void pushSubString(struct token_info* info, int* tn, int ln, char type, const char* s, size_t start, size_t end)
{
    size_t len = end-start;

    char* substring = malloc(sizeof(char)*(len+1));
    if(substring == NULL)
    {
        memoryFailure();
        exit(EXIT_FAILURE);
    }

    memcpy(substring, &s[start], len);
    substring[len]='\0';
    pushToken(info, tn, ln, type, substring);
    free(substring);
}

void pushChar(struct token_info* info, int* tn, int ln, char type, const char c)
{
    char* substring = malloc(sizeof(char)*(2));
    if(substring == NULL)
    {
        memoryFailure();
        exit(EXIT_FAILURE);
    }

    substring[0] = c;
    substring[1] = '\0';

    pushToken(info, tn, ln, type, substring);
    free(substring);

}

void parseSpaces(struct token_info* info, int* tn, int ln, const char* s, size_t slen, size_t* pos)
{
    size_t start = *pos;

    for(; *pos < slen; (*pos)++)
    {
        if(s[*pos] != ' ')
        {
            break;
        }
    }

    pushSubString(info, tn, ln, TOKENTYPE_SPACE, s, start, *pos);

    (*pos)--;
}

void parseMLComment(struct token_info* info, int* tn, int ln, const char* s, size_t slen, size_t* pos)
{
    size_t start = *pos;

    for(*pos = *pos + 2; *pos < slen; (*pos)++)
    {
        if(s[*pos] == '*')
        {
            if(s[(*pos + 1)] == '/')
            {
                (*pos)++;
                break;
            }
        }
    }

    pushSubString(info,  tn, ln, TOKENTYPE_COMMENTML, s, start, *pos+1);
}

void parseSLComment(struct token_info* info, int* tn, int ln, const char* s, size_t slen, size_t* pos)
{
    size_t start = *pos;
    *pos = *pos + 2;
    for(; *pos < slen; (*pos)++)
    {
        if(s[*pos] == '\n' || s[*pos] == '\r')
        {
            (*pos)++;
            break;
        }
    }

    pushSubString(info,  tn, ln, TOKENTYPE_COMMENTSL, s, start, *pos);

    (*pos)--;
}

void parseString(struct token_info* info, int* tn, int ln, const char* s, size_t slen, size_t* pos, char q)
{
    size_t start = *pos;

    for(*pos = *pos + 1; *pos < slen; (*pos)++)
    {
        if(s[*pos] == '\\')
        {
            (*pos)++;
        }
        else if(s[*pos] == q)
        {
            break;
        }
    }

    pushSubString(info,  tn, ln, q == '"' ? TOKENTYPE_STRINGDQ : TOKENTYPE_STRINGSQ, s, start, *pos +1);
}

void parseDecimalNumber(struct token_info* info, int* tn, int ln, const char* s, size_t slen, size_t* pos)
{
    size_t start = *pos;

    for(; *pos < slen; (*pos)++)
    {
        if(!isDecimalDigit(s[*pos]))
        {
            break;
        }
    }

    pushSubString(info,  tn, ln, TOKENTYPE_DECIMALNUMBER, s, start, *pos);

    (*pos)--;
}

void parseIdentifier(struct token_info* info, int* tn, int ln, const char* s, size_t slen, size_t* pos, char* urlMode, struct char_char* p)
{
    size_t start = *pos;

    while(s[*pos] == '/')
    {
        (*pos)++;
    }

    for(; *pos < slen; (*pos)++)
    {
        if(s[*pos] == '\\')
        {
            (*pos)++;
        }
        else if(isPunctuation(p, s[*pos]))
        {
            break;
        }
    }

    size_t len = *pos-start;
    char* ident = malloc(sizeof(char)*(len+1));
    if(ident == NULL)
    {
        memoryFailure();
        exit(EXIT_FAILURE);
    }

    memcpy(ident, &s[start], len);
    ident[len]='\0';

    if(casecmp(ident, "url") == 0)
    {
        *urlMode = 1;
    }

    pushToken(info, tn, ln, TOKENTYPE_IDENTIFIER, ident);
    free(ident);

    (*pos)--;
}

void _getTokens(const char* s, struct char_char* p, struct token_info* info)
{
    char urlMode = 0, blockMode = 0;
    int tn = 0, ln = 0;
    size_t pos = 0, slen= strlen(s);

    for(; pos < slen; pos++)
    {
        char c = s[pos];
        char cn = s[pos+1];

        if(c == '/' && cn == '*')
        {
            parseMLComment(info, &tn, ln, s, slen, &pos);
        }
        else if(!urlMode && c == '/' && cn == '/')
        {
            if(blockMode > 0)
            {
                parseIdentifier(info, &tn, ln, s, slen, &pos, &urlMode, p);
            }
            else
            {
                parseSLComment(info, &tn, ln, s, slen, &pos);
            }
        }
        else if(c == '"' || c == '\'')
        {
            parseString(info, &tn, ln, s, slen, &pos, c);
        }
        else if(c == ' ')
        {
            parseSpaces(info, &tn, ln, s, slen, &pos);
        }
        else if(isPunctuation(p, c))
        {
            pushChar(info, &tn, ln, isPunctuation(p, c), c);
            if(c == '\n' || c == '\r')
            {
                ln++;
            }

            if(c == ')')
            {
                urlMode = 0;
            }

            if(c == '{')
            {
                blockMode++;
            }

            if(c == '}')
            {
                blockMode--;
            }
        }
        else if(isDecimalDigit(c))
        {
            parseDecimalNumber(info, &tn, ln, s, slen, &pos);
        }
        else
        {
            parseIdentifier(info, &tn, ln, s, slen, &pos, &urlMode, p);
        }
    }
}

struct int_prev* pushInt(struct int_prev* front, int value)
{
    struct int_prev* newfirst = malloc(sizeof(struct int_prev));
    if(newfirst == NULL)
    {
        memoryFailure();
        exit(EXIT_FAILURE);
    }

    memset(newfirst, 0, sizeof(struct int_prev));

    if(front != NULL)
    {
        newfirst->prev = front;
    }

    newfirst->pos = value;

    return newfirst;
}

int popInt(struct int_prev** front)
{
    struct int_prev* oldfront = NULL;
    int value = 0;

    if(*front != NULL)
    {
        value = (*front)->pos;
        oldfront = *front;
        if((*front)->prev != NULL)
        {
            *front = (*front)->prev;
        }

        free(oldfront);
    }
    return value;
}

// ====================================
// second run
// ====================================

void mark(struct token_info* info, char* error)
{
    struct int_prev* ps = NULL; // Parenthesis
    struct int_prev* sbs = NULL; // SquareBracket
    struct int_prev* cbs = NULL; // CurlyBracket

    struct token* t = NULL;
    int i = 0;
    for(; i < info->len; i++)
    {
        t = getTokenByIndex(info, i);

        switch(t->type)
        {
            case TOKENTYPE_LEFTPARENTHESIS:
            {
                ps = pushInt(ps, i);
            }
            break;

            case TOKENTYPE_RIGHTPARENTHESIS:
            {
                if(ps!=NULL)
                {
                    t->left = popInt(&ps);
                    struct token* tmp = getTokenByIndex(info, t->left);
                    tmp->right = i;
                }
            }
            break;

            case TOKENTYPE_LEFTSQUAREBRACKET:
            {
                sbs = pushInt(sbs, i);
            }
            break;

            case TOKENTYPE_RIGHTSQUAREBRACKET:
            {
                if(sbs!=NULL)
                {
                    t->left = popInt(&sbs);
                    struct token* tmp = getTokenByIndex(info, t->left);
                    tmp->right = i;
                }
            }
            break;

            case TOKENTYPE_LEFTCURLYBRACKET:
            {
                cbs = pushInt(cbs, i);
            }
            break;

            case TOKENTYPE_RIGHTCURLYBRACKET:
            {
                if(cbs!=NULL)
                {
                    t->left = popInt(&cbs);
                    struct token* tmp = getTokenByIndex(info, t->left);
                    tmp->right = i;
                }
            }
            break;
        }
    }

    if(ps!=NULL || sbs!=NULL || cbs!=NULL )
    {
        *error = 1;
    }

    *error = 0;
}

struct token_info getTokens(const char* string, char* error)
{
    struct char_char punctuation[] =
    {
        { ' ', TOKENTYPE_SPACE },
        { '\n', TOKENTYPE_NEWLINE },
        { '\r', TOKENTYPE_NEWLINE },
        { '\t', TOKENTYPE_TAB },
        { '!', TOKENTYPE_EXCLAMATIONMARK },
        { '"', TOKENTYPE_QUOTATIONMARK },
        { '#', TOKENTYPE_NUMBERSIGN },
        { '$', TOKENTYPE_DOLLARSIGN },
        { '%', TOKENTYPE_PERCENTSIGN },
        { '&', TOKENTYPE_AMPERSAND },
        { '\'', TOKENTYPE_APOSTROPHE },
        { '(', TOKENTYPE_LEFTPARENTHESIS },
        { ')', TOKENTYPE_RIGHTPARENTHESIS },
        { '*', TOKENTYPE_ASTERISK },
        { '+', TOKENTYPE_PLUSSIGN },
        { ',', TOKENTYPE_COMMA },
        { '-', TOKENTYPE_HYPHENMINUS },
        { '.', TOKENTYPE_FULLSTOP },
        { '/', TOKENTYPE_SOLIDUS },
        { ':', TOKENTYPE_COLON },
        { ';', TOKENTYPE_SEMICOLON },
        { '<', TOKENTYPE_LESSTHANSIGN },
        { '=', TOKENTYPE_EQUALSSIGN },
        { '>', TOKENTYPE_GREATERTHANSIGN },
        { '?', TOKENTYPE_QUESTIONMARK },
        { '@', TOKENTYPE_COMMERCIALAT },
        { '[', TOKENTYPE_LEFTSQUAREBRACKET },
        // '\\', TOKENTYPE_REVERSESOLIDUS },
        { ']', TOKENTYPE_RIGHTSQUAREBRACKET },
        { '^', TOKENTYPE_CIRCUMFLEXACCENT },
        { '_', TOKENTYPE_LOWLINE },
        { '{', TOKENTYPE_LEFTCURLYBRACKET },
        { '|', TOKENTYPE_VERTICALLINE },
        { '}', TOKENTYPE_RIGHTCURLYBRACKET },
        { '~', TOKENTYPE_TILDE },
        { 0, 0 }
    };

    struct token_info info = { NULL, 0 };

    _getTokens(string, punctuation, &info);

    mark(&info, error);

    return info;
}
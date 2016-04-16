/*
 * ACCSS C Code Copyright (C) 2014 by Gandalf Sievers
 * Created by Gandalf Sievers on 25.07.14.
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
#include "helper.h"
#include "getpast.h"
#include "tokentypes.h"
#include "gettokens.h"
#include "debug.h"

void markSC(struct token_info* info);
void throwError(const char* s, int currentBlockLN);

char* joinValues(struct token_info* info, size_t start, size_t finish);

size_t checkSC(struct token_info* info, size_t pos);
size_t checkS(struct token_info* info, size_t pos);

size_t checkComment(struct token_info* info, size_t pos);

size_t checkSimpleselector(struct token_info* info, size_t pos);
size_t _checkSimpleSelector(struct token_info* info, size_t pos);
size_t checkSelector(struct token_info* info, size_t pos);
size_t checkRuleset(struct token_info* info, size_t pos);

size_t checkNthselector(struct token_info* info, size_t pos);
size_t checkNthf(struct token_info* info, size_t pos);
size_t checkNth(struct token_info* info, size_t pos);
size_t checkNth1(struct token_info* info, size_t pos);
size_t checkNth2(struct token_info* info, size_t pos);

size_t checkUnary(struct token_info* info, size_t pos);
size_t checkCombinator(struct token_info* info, size_t pos);
size_t checkAttrib(struct token_info* info, size_t pos);
size_t checkAttrib1(struct token_info* info, size_t pos);
size_t checkAttrib2(struct token_info* info, size_t pos);
size_t checkAttrselector(struct token_info* info, size_t pos);

size_t checkPseudo(struct token_info* info, size_t pos);
size_t checkPseudoe(struct token_info* info, size_t pos);
size_t checkPseudoc(struct token_info* info, size_t pos);

size_t checkDelim(struct token_info* info, size_t pos);

size_t checkIdent(struct token_info* info, size_t pos);
size_t checkIdentLowLine(struct token_info* info, size_t pos);

size_t checkClazz(struct token_info* info, size_t pos);
size_t checkShash(struct token_info* info, size_t pos);

size_t checkNmName(struct token_info* info, size_t pos);
size_t checkNamespace(struct token_info* info, size_t pos);

size_t checkBlock(struct token_info* info, size_t pos);

size_t checkAtrule(struct token_info* info, size_t pos);
size_t checkAtruleb(struct token_info* info, size_t pos);
size_t checkAtruler(struct token_info* info, size_t pos);

size_t checkAtkeyword(struct token_info* info, size_t pos);
size_t checkAtrulerq(struct token_info* info, size_t pos);
size_t checkAtrulers(struct token_info* info, size_t pos);
size_t checkAtrules(struct token_info* info, size_t pos);

size_t checkTset(struct token_info* info, size_t pos);
size_t checkTsets(struct token_info* info, size_t pos);

size_t checkVhash(struct token_info* info, size_t pos);
size_t checkNmName2(struct token_info* info, size_t pos);

size_t checkAny(struct token_info* info, size_t pos);

size_t checkBraces(struct token_info* info, size_t pos);
size_t checkString(struct token_info* info, size_t pos);
size_t checkPercentage(struct token_info* info, size_t pos);

size_t checkDimension(struct token_info* info, size_t pos);
size_t checkNumber(struct token_info* info, size_t pos);
size_t checkUri(struct token_info* info, size_t pos);

size_t checkFunctionExpression(struct token_info* info, size_t pos);
size_t checkFunction(struct token_info* info, size_t pos);

size_t checkOperator(struct token_info* info, size_t pos);

size_t checkUri1(struct token_info* info, size_t pos);
size_t checkExcluding(struct token_info* info, size_t pos);

size_t checkUnknown(struct token_info* info, size_t pos);

size_t checkBlockdecl(struct token_info* info, size_t pos);
size_t _checkBlockdecl0(struct token_info* info, size_t pos);
size_t _checkBlockdecl1(struct token_info* info, size_t pos);
size_t _checkBlockdecl2(struct token_info* info, size_t pos);
size_t _checkBlockdecl3(struct token_info* info, size_t pos);

size_t checkFilter(struct token_info* info, size_t pos);
size_t checkFilterp(struct token_info* info, size_t pos);
size_t checkFilterv(struct token_info* info, size_t pos);

size_t checkProgid(struct token_info* info, size_t pos);
size_t checkImportant(struct token_info* info, size_t pos);

size_t checkDeclaration(struct token_info* info, size_t pos);
size_t checkDecldelim(struct token_info* info, size_t pos);
size_t checkProperty(struct token_info* info, size_t pos);

size_t checkValue(struct token_info* info, size_t pos);
size_t _checkValue(struct token_info* info, size_t pos);

size_t checkStyleSheet(struct token_info* info, size_t pos);

struct astnode* getS(struct token_info* info, size_t* pos);
struct astnode* getComment(struct token_info* info, size_t* pos);
struct astnode** getSC(struct token_info* info, size_t* pos);

struct astnode* getRuleset(struct token_info* info, size_t* pos);
struct astnode* getSelector(struct token_info* info, size_t* pos);
struct astnode* getSimpleSelector(struct token_info* info, size_t* pos);
struct astnode** _getSimpleSelector(struct token_info* info, size_t* pos);

struct astnode* getNthf(struct token_info* info, size_t* pos);
struct astnode* getNthselector(struct token_info* info, size_t* pos);

struct astnode* getUnary(struct token_info* info, size_t* pos);
struct astnode* getNth(struct token_info* info, size_t* pos);

struct astnode* getDelim(struct token_info* info, size_t* pos);
struct astnode* getCombinator(struct token_info* info, size_t* pos);

struct astnode* getAttrib(struct token_info* info, size_t* pos);
struct astnode* getAttrib1(struct token_info* info, size_t* pos);
struct astnode* getAttrib2(struct token_info* info, size_t* pos);

struct astnode* getIdent(struct token_info* info, size_t* pos);
struct astnode* getAttrselector(struct token_info* info, size_t* pos);

struct astnode* getString(struct token_info* info, size_t* pos);

struct astnode* getPseudo(struct token_info* info, size_t* pos);
struct astnode* getPseudoe(struct token_info* info, size_t* pos);
struct astnode* getPseudoc(struct token_info* info, size_t* pos);

struct astnode* getFunction(struct token_info* info, size_t* pos);
struct astnode* getFunctionBody(struct token_info* info, size_t* pos);
struct astnode* getNotFunctionBody(struct token_info* info, size_t* pos);

struct astnode** getTset(struct token_info* info, size_t* pos);
struct astnode* getVhash(struct token_info* info, size_t* pos);

char* getNmName2(struct token_info* info, size_t* pos);

struct astnode* getAny(struct token_info* info, size_t* pos);
struct astnode* getBraces(struct token_info* info, size_t* pos);

struct astnode* getPercentage(struct token_info* info, size_t* pos);
struct astnode* getNumber(struct token_info* info, size_t* pos);

struct astnode** getTsets(struct token_info* info, size_t* pos);
struct astnode* getDimension(struct token_info* info, size_t* pos);
struct astnode* getUri(struct token_info* info, size_t* pos);

struct astnode* getFunctionExpression(struct token_info* info, size_t* pos);
struct astnode* getOperator(struct token_info* info, size_t* pos);

struct astnode* getClazz(struct token_info* info, size_t* pos);

struct astnode* getShash(struct token_info* info, size_t* pos);
char* getNmName(struct token_info* info, size_t* pos);
struct astnode* getNamespace(struct token_info* info, size_t* pos);
struct astnode* getBlock(struct token_info* info, size_t* pos);

struct astnode** getBlockdecl(struct token_info* info, size_t* pos);
struct astnode** _getBlockdecl0(struct token_info* info, size_t* pos);
struct astnode** _getBlockdecl1(struct token_info* info, size_t* pos);
struct astnode** _getBlockdecl2(struct token_info* info, size_t* pos);
struct astnode** _getBlockdecl3(struct token_info* info, size_t* pos);

struct astnode* getFilter(struct token_info* info, size_t* pos);
struct astnode* getFilterv(struct token_info* info, size_t* pos);
struct astnode* getFilterp(struct token_info* info, size_t* pos);

struct astnode* getDeclaration(struct token_info* info, size_t* pos);
struct astnode* getDecldelim(struct token_info* info, size_t* pos);

struct astnode* getImportant(struct token_info* info, size_t* pos);

struct astnode* getProgid(struct token_info* info, size_t* pos);
struct astnode* _getProgid(struct token_info* info, size_t* pos, size_t progid_end);

struct astnode* getProperty(struct token_info* info, size_t* pos);

struct astnode* getValue(struct token_info* info, size_t* pos);
struct astnode** _getValue(struct token_info* info, size_t* pos);

struct astnode* getAtkeyword(struct token_info* info, size_t* pos);

struct astnode* getAtrule(struct token_info* info, size_t* pos);
struct astnode* getAtruleb(struct token_info* info, size_t* pos);
struct astnode* getAtruler(struct token_info* info, size_t* pos);
struct astnode* getAtrulerq(struct token_info* info, size_t* pos);
struct astnode* getAtrulers(struct token_info* info, size_t* pos);

struct astnode* getAtrules(struct token_info* info, size_t* pos);

struct astnode* getStylesheet(struct token_info* info, size_t* pos, char* error);

void markSC(struct token_info* info)
{
    size_t ws = 0,
    sc = 0;
    char isws = 0,
    issc = 0;

    struct token* t;
    size_t i = 0;
    for(; i < info->len; i++)
    {
        t = getTokenByIndex(info, i);
        switch(t->type)
        {
            case TOKENTYPE_SPACE:
            case TOKENTYPE_TAB:
            case TOKENTYPE_NEWLINE:
            {
                t->ws = 1;
                t->sc = 1;

                if(!isws)
                {
                    ws = i;
                    isws=1;
                }
                if(!issc)
                {
                    sc = i;
                    issc= 1;
                }
            }
            break;
            case TOKENTYPE_COMMENTML:
            {
                if(isws)
                {
                    struct token* tmp = getTokenByIndex(info, ws);
                    tmp->ws_last = i - 1;
                    ws = 0;
                    isws=0;
                }

                t->sc = 1;
            }
            break;
            default:
            {
                if(isws)
                {
                    struct token* tmp = getTokenByIndex(info, ws);
                    tmp->ws_last = i - 1;
                    ws=0;
                    isws = 0;
                }

                if(issc)
                {
                    struct token* tmp = getTokenByIndex(info, sc);
                    tmp->sc_last = i - 1;
                    sc=0;
                    issc=0;
                }
            }
        }
    }

    if(isws)
    {
        struct token* tmp = getTokenByIndex(info, ws);
        tmp->ws_last = i - 1;
    }

    if(issc)
    {
        struct token* tmp = getTokenByIndex(info, sc);
        tmp->sc_last = i - 1;
    }
}

void throwError(const char* s, int currentBlockLN)
{
    fprintf(stderr, "%s: Please check the validity of the CSS block starting from the line #%i\n", s, currentBlockLN);
}

char* joinValues(struct token_info* info, size_t start, size_t finish)
{
    size_t len = 0, i = start;
    char* s = NULL;

    if(finish >= info->len)
    {
        return NULL;
    }

    for(; i < finish + 1; i++)
    {
        struct token* tmp = getTokenByIndex(info, i);

        size_t slen = strlen(tmp->value);
        if(slen > 0)
        {
            char* stmp = realloc(s, sizeof(char)*(slen+len+1));
            if(stmp == NULL)
            {
                memoryFailure();
                exit(EXIT_FAILURE);
            }

            s = stmp;
            memcpy(&s[len], tmp->value, slen);
            len += slen;
            s[len] = '\0';
        }
    }

    return s;
}

size_t checkSC(struct token_info* info, size_t pos)
{
    size_t l = 0,
    lsc = 0;

    while(pos < info->len)
    {
        if((l = checkS(info, pos)) != 0)
        {
            pos += l;
            lsc += l;
        }
        else if((l = checkComment(info, pos)) != 0)
        {
            pos += l;
            lsc += l;
        }
        else
        {
            break;
        }
    }

    if(lsc)
    {
        return lsc;
    }

    if(pos >= info->len)
    {
        return 0;
    }

    return 0;
}

size_t checkS(struct token_info* info, size_t pos)
{
    struct token* tmp = getTokenByIndex(info, pos);
    if(tmp->ws)
    {
        return tmp->ws_last - pos + 1;
    }

    return 0;
}

size_t checkComment(struct token_info* info, size_t pos)
{
    struct token* tmp = getTokenByIndex(info, pos);
    if(tmp->type == TOKENTYPE_COMMENTML)
    {
        return 1;
    }

    return 0;;
}

size_t checkSimpleselector(struct token_info* info, size_t pos)
{
    size_t start = pos,
    l = 0;

    while(pos < info->len)
    {
        if((l = _checkSimpleSelector(info, pos)) != 0)
        {
            pos += l;
        }
        else
        {
            break;
        }
    }

    if(pos - start)
    {
        return pos - start;
    }

    if(pos >= info->len)
    {
        return 0;
    }

    return 0;
}

size_t _checkSimpleSelector(struct token_info* info, size_t pos)
{
    size_t l = 0;
    if((l = checkNthselector(info, pos)) != 0)
    {
        return l;
    }
    else if((l = checkCombinator(info, pos)) != 0)
    {
        return l;
    }
    else if((l = checkAttrib(info, pos)) != 0)
    {
        return l;
    }
    else if((l = checkPseudo(info, pos)) != 0)
    {
        return l;
    }
    else if((l = checkClazz(info, pos)) != 0)
    {
        return l;
    }
    else if((l = checkShash(info, pos)) != 0)
    {
        return l;
    }
    else if((l = checkAny(info, pos)) != 0)
    {
        return l;
    }
    else if((l = checkSC(info, pos)) != 0)
    {
        return l;
    }
    else if((l = checkNamespace(info, pos)) != 0)
    {
        return l;
    }
    return 0;
}

size_t checkSelector(struct token_info* info, size_t pos)
{
    size_t start = pos,
    l = 0;

    if(pos < info->len)
    {
        size_t i = start;
        do
        {
            l = checkSimpleselector(info, pos);
            if(l == 0)
            {
                l = checkDelim(info, pos);
                if(l == 0)
                {
                    break;
                }
            }

            pos += l;
        }while(1);

        for(; i < pos; i++)
        {
            struct token* tmp = getTokenByIndex(info, i);
            tmp->selector_end = pos -1;
        }

        return pos - start;
    }
    return 0;
}

size_t checkNthselector(struct token_info* info, size_t pos)
{
    size_t start = pos,
    rp = 0,
    l = 0;
    struct token* tmp = NULL;

    if((l = checkNthf(info, pos)) != 0 )
    {
        pos += l;
    }
    else
    {
        return 0;
    }

    tmp = getTokenByIndex(info, pos);
    if(tmp->type != TOKENTYPE_LEFTPARENTHESIS || !tmp->right)
    {
        return 0;
    }

    l++;

    tmp = getTokenByIndex(info, pos++);
    rp = tmp->right;

    while(pos < rp)
    {
        if((l = checkSC(info, pos)) != 0)
        {
            pos += l;
        }
        else if((l = checkUnary(info, pos)) != 0)
        {
            pos += l;
        }
        else if((l = checkNth(info, pos)) != 0)
        {
            pos += l;
        }
        else
        {
            return 0;
        }
    }

    return rp - start + 1;
}

size_t checkNthf(struct token_info* info, size_t pos)
{
    size_t start = pos,
    l = 0;
    char found = 0;
    struct token* tmp = getTokenByIndex(info, pos++),
    *next = NULL;

    if(tmp->type != TOKENTYPE_COLON)
    {
        return 0;
    }

    l++;

    tmp = getTokenByIndex(info, pos++);
    next = getTokenByIndex(info, pos++);
    if((strcmp(tmp->value, "nth") != 0) || (strcmp(next->value, "-") != 0))
    {
        return 0;
    }

    l += 2;
    tmp = getTokenByIndex(info, pos);

    if(strcmp(tmp->value, "child") == 0)
    {
        found=1;
        l += 1;
    }
    else
    {
        char* value = joinValues(info, pos, pos+2);
        if(strcmp(value, "last-child") == 0)
        {
            found = 1;
            l += 3;
        }
        else if(strcmp(value, "of-type") == 0)
        {
            found = 1;
            l += 3;
        }
        else
        {
            free(value);
            value = joinValues(info, pos, pos+4);
            if(strcmp(value, "last-of-type") == 0)
            {
                found = 1;
                l += 5;
            }
        }
        free(value);
    }

    if(!found)
    {
        return 0;
    }

    tmp = getTokenByIndex(info, start+1);
    tmp->nthf_last = start + l - 1;

    return l;
}

size_t checkNth(struct token_info* info, size_t pos)
{
    size_t l = checkNth1(info, pos);
    if(l != 0)
    {
        return l;
    }

    return checkNth2(info, pos);
}

size_t checkNth1(struct token_info* info, size_t pos)
{
    size_t start = pos;

    for(; pos < info->len; pos++)
    {
        struct token* tmp = getTokenByIndex(info, pos);
        if(tmp->type != TOKENTYPE_DECIMALNUMBER && (strcmp(tmp->value, "n") != 0 ))
        {
            break;
        }
    }

    if(pos != start)
    {
        struct token* tmp = getTokenByIndex(info, pos);

        tmp->nth_last = pos - 1;
        return pos - start;
    }

    return 0;
}

size_t checkNth2(struct token_info* info, size_t pos)
{
    struct token* tmp = getTokenByIndex(info, pos);
    if((strcmp(tmp->value, "even") == 0)  || (strcmp(tmp->value, "odd") == 0) )
    {
        return 1;
    }

    return 0;
}

size_t checkUnary(struct token_info* info, size_t pos)
{
    struct token* tmp = getTokenByIndex(info, pos);

    if(pos < info->len
      && (tmp->type == TOKENTYPE_HYPHENMINUS || tmp->type == TOKENTYPE_PLUSSIGN)
      )
    {
        return 1;
    }

    return 0;
}

size_t checkCombinator(struct token_info* info, size_t pos)
{
    struct token* tmp = getTokenByIndex(info, pos);

    if(tmp->type == TOKENTYPE_PLUSSIGN
      || tmp->type ==  TOKENTYPE_GREATERTHANSIGN
      || tmp->type == TOKENTYPE_TILDE
      || tmp->type == TOKENTYPE_DEEP
      )
    {
        return 1;
    }

    return 0;
}

size_t checkAttrib(struct token_info* info, size_t pos)
{
    struct token* tmp = getTokenByIndex(info, pos);
    if(tmp->type != TOKENTYPE_LEFTSQUAREBRACKET)
    {
        return 0;
    }

    if(!tmp->right)
    {
        return 0;
    }

    return tmp->right - pos + 1;
}

size_t checkAttrib1(struct token_info* info, size_t pos)
{
    size_t start = pos,
    l = 0;
    struct token* tmp = NULL;

    pos++;
    l = checkSC(info, pos);

    if(l != 0)
    {
        pos += l;
    }

    if((l = checkIdent(info, pos)) != 0)
    {
        pos += l;
    }
    else
    {
        return 0;
    }

    if((l = checkSC(info, pos)) != 0)
    {
        pos += l;
    }

    if((l = checkAttrselector(info, pos)) != 0)
    {
        pos += l;
    }
    else
    {
        return 0;
    }

    if((l = checkSC(info, pos)) != 0)
    {
        pos += l;
    }

    if((l = checkIdent(info, pos)) != 0 || (l = checkString(info, pos)) != 0)
    {
        pos += l;
    }
    else
    {
        return 0;
    }

    if((l = checkSC(info, pos)) != 0)
    {
        pos += l;
    }

    tmp = getTokenByIndex(info, pos);
    if(tmp->type == TOKENTYPE_RIGHTSQUAREBRACKET)
    {
        return pos - start;
    }

    return 0;
}

size_t checkAttrib2(struct token_info* info, size_t pos)
{
    size_t start = pos,
    l = 0;
    struct token* tmp = NULL;

    pos++;
    l = checkSC(info, pos);

    if(l != 0)
    {
        pos += l;
    }

    if((l = checkIdent(info, pos)) != 0)
    {
        pos += l;
    }

    if((l = checkSC(info, pos)) != 0)
    {
        pos += l;
    }

    tmp = getTokenByIndex(info, pos);
    if(tmp->type == TOKENTYPE_RIGHTSQUAREBRACKET)
    {
        return pos - start;
    }

    return 0;
}

size_t checkAttrselector(struct token_info* info, size_t pos)
{
    struct token* tmp = getTokenByIndex(info, pos),
    *tmp1 = NULL;
    if(tmp->type == TOKENTYPE_EQUALSSIGN)
    {
        return 1;
    }

    tmp1 = getTokenByIndex(info, pos+1);
    if(tmp->type == TOKENTYPE_VERTICALLINE && (!tmp1 || tmp1->type != TOKENTYPE_EQUALSSIGN))
    {
        return 1;
    }

    if(!tmp1 || tmp1->type != TOKENTYPE_EQUALSSIGN)
    {
        return 0;
    }

    switch(tmp->type)
    {
        case TOKENTYPE_TILDE:
        case TOKENTYPE_CIRCUMFLEXACCENT:
        case TOKENTYPE_DOLLARSIGN:
        case TOKENTYPE_ASTERISK:
        case TOKENTYPE_VERTICALLINE:
        {
            return 2;
        }
    }

    return 0;
}

size_t checkPseudo(struct token_info* info, size_t pos)
{
    size_t l = checkPseudoe(info, pos);
    if(l == 0)
    {
        l = checkPseudoc(info, pos);
    }

    return l;
}

size_t checkPseudoe(struct token_info* info, size_t pos)
{
    size_t l = 0;

    struct token* tmp = getTokenByIndex(info, pos++);
    if(tmp->type != TOKENTYPE_COLON)
    {
        return 0;
    }

    tmp = getTokenByIndex(info, pos++);
    if(tmp->type != TOKENTYPE_COLON)
    {
        return 0;
    }

    if((l = checkIdent(info, pos)) != 0)
    {
        return l + 2;
    }

    return 0;
}

size_t checkPseudoc(struct token_info* info, size_t pos)
{
    size_t l = 0;

    struct token* tmp = getTokenByIndex(info, pos++);
    if(tmp->type != TOKENTYPE_COLON)
    {
        return 0;
    }

    if(((l = checkFunction(info, pos)) != 0) || ((l = checkIdent(info, pos)) != 0))
    {
        return l + 1;
    }

    return 0;
}

size_t checkIdent(struct token_info* info, size_t pos)
{
    size_t start = pos;
    char wasIdent = 0;
    struct token* tmp = NULL;

    if(pos >= info->len)
    {
        return 0;
    }

    tmp = getTokenByIndex(info, pos);
    if(tmp->type == TOKENTYPE_LOWLINE)
    {
        return checkIdentLowLine(info, pos);
    }

    if(tmp->type == TOKENTYPE_HYPHENMINUS
      || tmp->type == TOKENTYPE_IDENTIFIER
      || tmp->type == TOKENTYPE_DOLLARSIGN
      || tmp->type == TOKENTYPE_ASTERISK
      )
    {
        pos++;
    }
    else
    {
        return 0;
    }

    tmp = getTokenByIndex(info, pos-1);
    wasIdent = tmp->type == TOKENTYPE_IDENTIFIER;

    for(; pos < info->len; pos++)
    {
        tmp = getTokenByIndex(info, pos);
        if(tmp->type != TOKENTYPE_HYPHENMINUS
          && tmp->type != TOKENTYPE_LOWLINE
          )
        {
            if(tmp->type != TOKENTYPE_IDENTIFIER
              && (tmp->type != TOKENTYPE_DECIMALNUMBER || !wasIdent)
              )
            {
                break;
            }
            else
            {
                wasIdent = 1;
            }
        }
    }

    tmp = getTokenByIndex(info, start);
    if(!wasIdent && tmp->type != TOKENTYPE_ASTERISK)
    {
        return 0;
    }

    tmp->ident_last = pos - 1;

    return pos - start;
}

size_t checkIdentLowLine(struct token_info* info, size_t pos)
{
    size_t start = pos;
    struct token* tmp = NULL;

    pos++;

    for(; pos < info->len; pos++)
    {
        struct token* tmp = getTokenByIndex(info, pos);
        if(tmp->type != TOKENTYPE_HYPHENMINUS
          && tmp->type != TOKENTYPE_DECIMALNUMBER
          && tmp->type != TOKENTYPE_LOWLINE
          && tmp->type != TOKENTYPE_IDENTIFIER
          )
        {
            break;
        }
    }

    tmp = getTokenByIndex(info, start);
    tmp->ident_last = pos - 1;

    return pos - start;
}

size_t checkDelim(struct token_info* info, size_t pos)
{
    struct token* tmp = getTokenByIndex(info, pos);
    if(pos < info->len && tmp->type == TOKENTYPE_COMMA)
    {
        return 1;
    }

    if(pos >= info->len)
    {
        return 0;
    }

    return 0;
}

size_t checkClazz(struct token_info* info, size_t pos)
{
    size_t l = 0;

    struct token* tmp = getTokenByIndex(info, pos);
    if(tmp->clazz_l)
    {
        return tmp->clazz_l;
    }

    if(tmp->type == TOKENTYPE_FULLSTOP)
    {
        if((l = checkIdent(info, pos + 1)) != 0)
        {
            tmp->clazz_l = l + 1;
            return l + 1;
        }
    }

    return 0;
}

size_t checkShash(struct token_info* info, size_t pos)
{
    struct token* tmp = getTokenByIndex(info, pos);
    size_t l = 0;
    if(tmp->type != TOKENTYPE_NUMBERSIGN)
    {
        return 0;
    }

    l = checkNmName(info, pos + 1);

    if(l)
    {
        return l + 1;
    }

    return 0;
}

size_t checkNmName(struct token_info* info, size_t pos)
{
    size_t start = pos;
    struct token* tmp = getTokenByIndex(info, pos);

    if(tmp->type == TOKENTYPE_HYPHENMINUS
      || tmp->type == TOKENTYPE_LOWLINE
      || tmp->type == TOKENTYPE_IDENTIFIER
      || tmp->type == TOKENTYPE_DECIMALNUMBER
      )
    {
        pos++;
    }
    else
    {
        return 0;
    }

    for(; pos < info->len; pos++)
    {
        tmp = getTokenByIndex(info, pos);
        if(tmp->type != TOKENTYPE_HYPHENMINUS
          || tmp->type != TOKENTYPE_LOWLINE
          || tmp->type != TOKENTYPE_IDENTIFIER
          || tmp->type != TOKENTYPE_DECIMALNUMBER
          )
        {
            break;
        }
    }

    tmp = getTokenByIndex(info, start);
    tmp->nm_name_last = pos - 1;

    return pos - start;
}

size_t checkNamespace(struct token_info* info, size_t pos)
{
    struct token* tmp = getTokenByIndex(info, pos);

    if(tmp->type == TOKENTYPE_VERTICALLINE)
    {
        return 1;
    }

    return 0;
}

size_t checkBlock(struct token_info* info, size_t pos)
{
    struct token* tmp = getTokenByIndex(info, pos);
    if(pos < info->len && tmp->type == TOKENTYPE_LEFTCURLYBRACKET)
    {
        return tmp->right - pos + 1;
    }

    return 0;
}

size_t checkAtrule(struct token_info* info, size_t pos)
{
    size_t l = 0;
    struct token* tmp = getTokenByIndex(info, pos);
    if(tmp->atrule_type != 0)
    {
        return tmp->atrule_l;
    }

    if((l = checkAtruler(info, pos)) != 0 )
    {
        tmp->atrule_type = 1;
    }
    else if((l = checkAtruleb(info, pos)) != 0)
    {
        tmp->atrule_type = 2;
    }
    else if((l = checkAtrules(info, pos)) != 0 )
    {
        tmp->atrule_type = 3;
    }
    else
    {
        return 0;
    }

    tmp->atrule_l = l;

    return l;
}

size_t checkAtruleb(struct token_info* info, size_t pos)
{
    size_t start = pos,
    l = 0;

    if((l = checkAtkeyword(info,pos)) != 0)
    {
        pos += l;
    }
    else
    {
        return 0;
    }

    if((l = checkTsets(info, pos)) != 0)
    {
        pos += l;
    }

    if((l = checkBlock(info, pos)) != 0)
    {
        pos+= l;
    }
    else
    {
        return 0;
    }

    return pos - start;
}

size_t checkAtrules(struct token_info* info, size_t pos)
{
    size_t start = pos,
    l = 0;
    struct token* tmp = NULL;

    if((l = checkAtkeyword(info,pos)) != 0)
    {
        pos += l;
    }
    else
    {
        return 0;
    }

    if((l = checkTsets(info, pos)) != 0)
    {
        pos += l;
    }

    if(pos >= info->len)
    {
        return pos - start;
    }

    tmp = getTokenByIndex(info, pos);
    if(tmp->type == TOKENTYPE_SEMICOLON)
    {
        pos++;
    }
    else
    {
        return 0;
    }

    return pos - start;
}

size_t checkAtruler(struct token_info* info, size_t pos)
{
    size_t start = pos,
    l = 0;
    struct token* tmp = NULL;

    if((l = checkAtkeyword(info, pos)) != 0)
    {
        pos += l;
    }
    else
    {
        return 0;
    }

    if((l = checkAtrulerq(info, pos)) != 0)
    {
        pos += l;
    }

    tmp = getTokenByIndex(info, pos);
    if(pos < info->len && tmp->type == TOKENTYPE_LEFTCURLYBRACKET)
    {
        pos++;
    }
    else
    {
        return 0;
    }

    if((l = checkAtrulers(info, pos)) != 0)
    {
        pos += l;
    }

    tmp = getTokenByIndex(info, pos);
    if(pos < info->len && tmp->type == TOKENTYPE_RIGHTCURLYBRACKET)
    {
        pos++;
    }
    else
    {
        return 0;
    }

    return pos - start;
}

size_t checkAtkeyword(struct token_info* info, size_t pos)
{
    size_t l = 0;

    struct token* tmp = getTokenByIndex(info, pos++);
    if(tmp->type != TOKENTYPE_COMMERCIALAT)
    {
        return 0;
    }

    if((l = checkIdent(info, pos)) != 0)
    {
        return l + 1;
    }

    return 0;
}

size_t checkAtrulerq(struct token_info* info, size_t pos)
{
    return checkTsets(info, pos);
}

size_t checkAtrulers(struct token_info* info, size_t pos)
{
    size_t start = pos,
    l = 0;
    struct token* tmp = NULL;

    if((l = checkSC(info, pos)) != 0)
    {
        pos += l;
    }

    while((l = checkRuleset(info, pos)) != 0 || (l = checkAtrule(info, pos)) != 0 || (l = checkSC(info, pos)) != 0)
    {
        pos += l;
    }

    tmp = getTokenByIndex(info, pos);
    tmp->atrulers_end = 1;

    if((l = checkSC(info, pos)) != 0)
    {
        pos += l;
    }

    return pos - start;
}

size_t checkTset(struct token_info* info, size_t pos)
{
    size_t l = 0;
    if((l = checkVhash(info, pos)) != 0)
    {
        return l;
    }
    else if((l = checkAny(info, pos)) != 0)
    {
        return l;
    }
    else if((l = checkSC(info, pos)) != 0)
    {
        return l;
    }
    else
    {
        return checkOperator(info, pos);
    }
}

size_t checkTsets(struct token_info* info, size_t pos)
{
    size_t start = pos,
    l = 0;

    while((l = checkTset(info, pos)) != 0)
    {
        pos += l;
    }

    return pos - start;
}

size_t checkVhash(struct token_info* info, size_t pos)
{
    size_t l = 0;
    struct token* tmp = getTokenByIndex(info, pos);
    if(pos >= info->len || tmp->type != TOKENTYPE_NUMBERSIGN)
    {
        return 0;
    }

    l = checkNmName2(info, pos + 1);

    if(l != 0)
    {
        return l + 1;
    }

    return 0;
}

size_t checkNmName2(struct token_info* info, size_t pos)
{
    struct token* tmp = getTokenByIndex(info, pos);
    if(tmp->type == TOKENTYPE_IDENTIFIER)
    {
        return 1;
    }
    else if(tmp->type != TOKENTYPE_DECIMALNUMBER)
    {
        return 0;
    }

    pos++;

    tmp = getTokenByIndex(info, pos);
    if(tmp != NULL)
    {
        if(tmp->type != TOKENTYPE_IDENTIFIER)
        {
            return 1;
        }
    }

    return 2;
}

size_t checkAny(struct token_info* info, size_t pos)
{
    size_t l = 0;
    if((l = checkBraces(info, pos)) != 0)
    {
        return l;
    }
    else if((l = checkString(info, pos)) != 0)
    {
        return l;
    }
    else if((l = checkPercentage(info, pos)) != 0)
    {
        return l;
    }
    else if((l = checkDimension(info, pos)) != 0)
    {
        return l;
    }
    else if((l = checkNumber(info, pos)) != 0)
    {
        return l;
    }
    else if((l = checkUri(info, pos)) != 0)
    {
        return l;
    }
    else if((l = checkFunctionExpression(info, pos)) != 0)
    {
        return l;
    }
    else if((l = checkFunction(info, pos)) != 0)
    {
        return l;
    }
    else if((l = checkIdent(info, pos)) != 0)
    {
        return l;
    }
    else
    {
        return checkUnary(info, pos);
    }
    return 0;
}

size_t checkBraces(struct token_info* info, size_t pos)
{
    struct token* tmp = getTokenByIndex(info, pos);
    if(pos >= info->len
      || (tmp->type != TOKENTYPE_LEFTPARENTHESIS && tmp->type != TOKENTYPE_LEFTSQUAREBRACKET)
      )
    {
        return 0;
    }

    return tmp->right - pos + 1;
}

size_t checkString(struct token_info* info, size_t pos)
{
    struct token* tmp = getTokenByIndex(info, pos);
    if(pos < info->len
      && (tmp->type == TOKENTYPE_STRINGSQ || tmp->type == TOKENTYPE_STRINGDQ)
      )
    {
        return 1;
    }

    return 0;
}

size_t checkPercentage(struct token_info* info, size_t pos)
{
    size_t x = checkNumber(info, pos);
    struct token* tmp = NULL;

    if(x == 0 || (x && pos + x >= info->len))
    {
        return 0;
    }

    tmp = getTokenByIndex(info, pos + x);
    if(tmp->type == TOKENTYPE_PERCENTSIGN)
    {
        return x + 1;
    }

    return 0;
}

size_t checkDimension(struct token_info* info, size_t pos)
{
    size_t ln = checkNumber(info, pos),
    li;

    if(!ln || (ln && pos + ln >= info->len))
    {
        return 0;
    }

    if((li = checkNmName2(info, pos + ln)))
    {
        return ln + li;
    }

    return 0;
}

size_t checkNumber(struct token_info* info, size_t pos)
{
    struct token* tmp = getTokenByIndex(info, pos),
    *tmp1 = NULL,
    *tmp2 = NULL;

    if(pos < info->len && tmp->number_l)
    {
        return tmp->number_l;
    }

    tmp1 = getTokenByIndex(info, pos + 1);
    tmp2 = getTokenByIndex(info, pos + 2);
    if(pos < info->len && tmp->type == TOKENTYPE_DECIMALNUMBER
      && (!tmp1 || (tmp1 && tmp1->type != TOKENTYPE_FULLSTOP))
      )
    {
        return (tmp->number_l = 1);
    }

    if(pos < info->len
      && tmp->type == TOKENTYPE_DECIMALNUMBER
      && tmp1 && tmp1->type == TOKENTYPE_FULLSTOP
      && (!tmp2 || (tmp2->type != TOKENTYPE_DECIMALNUMBER))
      )
    {
        return (tmp->number_l = 2);
    }

    if(pos < info->len
      && tmp->type == TOKENTYPE_FULLSTOP
      && tmp1 && tmp1->type == TOKENTYPE_DECIMALNUMBER
      )
    {
        return (tmp->number_l = 2);
    }

    if(pos < info->len
      && tmp->type == TOKENTYPE_DECIMALNUMBER
      && tmp1 && tmp1->type == TOKENTYPE_FULLSTOP
      && tmp2 && tmp2->type == TOKENTYPE_DECIMALNUMBER
      )
    {
        return (tmp->number_l = 3);
    }

        return 0;
}

size_t checkUri(struct token_info* info, size_t pos)
{
    size_t start = pos;

    struct token* tmp = getTokenByIndex(info, pos++);

    if(pos < info->len && strcmp(tmp->value, "url") != 0)
    {
        return 0;
    }

    tmp = getTokenByIndex(info, pos);
    if(!tmp || tmp->type != TOKENTYPE_LEFTPARENTHESIS)
    {
        return 0;
    }

    return tmp->right - start + 1;
}

size_t checkFunctionExpression(struct token_info* info, size_t pos)
{
    size_t start = pos;
    struct token* tmp = getTokenByIndex(info, pos++);
    if(!tmp || strcmp(tmp->value, "expression") != 0)
    {
        return 0;
    }

    tmp = getTokenByIndex(info, pos);
    if(!tmp || tmp->type != TOKENTYPE_LEFTPARENTHESIS)
    {
        return 0;
    }

    return tmp->right - start + 1;
}

size_t checkFunction(struct token_info* info, size_t pos)
{
    size_t start = pos,
    l = checkIdent(info, pos);
    struct token* tmp = NULL;

    if(l == 0)
    {
        return 0;
    }

    pos += l;
    tmp = getTokenByIndex(info, pos);

    if(pos >= info->len || tmp->type != TOKENTYPE_LEFTPARENTHESIS)
    {
        return 0;
    }

    return tmp->right - start + 1;
}

size_t checkOperator(struct token_info* info, size_t pos)
{
    struct token* tmp = getTokenByIndex(info, pos);
    if(pos < info->len
      && (tmp->type == TOKENTYPE_SOLIDUS
         || tmp->type == TOKENTYPE_COMMA
         || tmp->type == TOKENTYPE_COLON
         || tmp->type == TOKENTYPE_EQUALSSIGN
         )
      )
    {
        return 1;
    }

    return 0;
}

size_t checkUri1(struct token_info* info, size_t pos)
{
    size_t start = pos,
    l = checkSC(info, pos);
    struct token* tmp = NULL;

    if(l != 0)
    {
        pos += l;
    }

    tmp = getTokenByIndex(info, pos);
    if(tmp->type != TOKENTYPE_STRINGDQ && tmp->type != TOKENTYPE_STRINGSQ)
    {
        return 0;
    }

    pos++;

    if((l = checkSC(info, pos)) != 0)
    {
        pos += l;
    }

    return pos - start;
}

size_t checkExcluding(struct token_info* info, size_t pos)
{
    size_t start = pos;

    while(pos < info->len)
    {
        struct token* tmp = getTokenByIndex(info, pos);
        pos++;
        if(tmp->type == TOKENTYPE_SPACE
          || tmp->type == TOKENTYPE_TAB
          || tmp->type == TOKENTYPE_NEWLINE
          || tmp->type == TOKENTYPE_LEFTPARENTHESIS
          || tmp->type == TOKENTYPE_RIGHTPARENTHESIS
          )
        {
            break;
        }
    }

    return pos - start - 2;
}

size_t checkRuleset(struct token_info* info, size_t pos)
{
    size_t start = pos,
    l = 0;

    struct token* tmp = getTokenByIndex(info, start);

    if(tmp->ruleset_l != 0)
    {
        return tmp->ruleset_l;
    }

    while((l = checkSelector(info, pos)) != 0)
    {
        pos += l;
    }

    if((l = checkBlock(info, pos)) != 0)
    {
        pos += l;
    }
    else
    {
        return 0;
    }

    if(start > pos)
    {
        return 0;
    }

    tmp->ruleset_l = pos - start;

    return pos - start;
}

size_t checkUnknown(struct token_info* info, size_t pos)
{
    struct token* tmp = getTokenByIndex(info, pos);
    if(pos < info->len && tmp->type == TOKENTYPE_COMMENTSL)
    {
        return 1;
    }

    return 0;
}

size_t checkBlockdecl(struct token_info* info, size_t pos)
{
    size_t l = 0;
    struct token* tmp = getTokenByIndex(info, pos);
    if((l = _checkBlockdecl0(info, pos)) != 0)
    {
        tmp->bd_type = 1;

    }
    else if((l = _checkBlockdecl1(info, pos)) != 0)
    {
        tmp->bd_type = 2;
    }
    else if((l = _checkBlockdecl2(info, pos)) != 0)
    {
        tmp->bd_type = 3;
    }
    else if((l = _checkBlockdecl3(info, pos)) != 0)
    {
        tmp->bd_type = 4;
    }
    else
    {
        return 0;
    }

    return l;
}

size_t _checkBlockdecl0(struct token_info* info, size_t pos)
{
    size_t start = pos,
    l = 0;
    struct token* tmp = NULL;

    if((l = checkSC(info, pos)) != 0)
    {
        pos += l;
    }

    tmp = getTokenByIndex(info, pos);
    if((l = checkFilter(info, pos)) != 0)
    {
        tmp->bd_filter = 1;
        pos += l;
    }
    else if((l = checkDeclaration(info, pos)) != 0)
    {
        tmp->bd_decl = 1;
        pos += l;
    }
    else
    {
        return 0;
    }

    if(pos < info->len && (l = checkDecldelim(info, pos)) != 0)
    {
        pos += l;
    }
    else
    {
        return 0;
    }

    if((l = checkSC(info, pos)) != 0)
    {
        pos += l;
    }

    return pos - start;
}

size_t _checkBlockdecl1(struct token_info* info, size_t pos)
{
    size_t start = pos,
    l = 0;
    struct token* tmp = NULL;

    if((l = checkSC(info, pos)) != 0)
    {
        pos += l;
    }

    tmp = getTokenByIndex(info, pos);
    if((l = checkFilter(info, pos)) != 0)
    {
        tmp->bd_filter = 1;
        pos += l;
    }
    else if((l = checkDeclaration(info, pos)) != 0)
    {
        tmp->bd_decl = 1;
        pos += l;
    }
    else
    {
        return 0;
    }

    if((l = checkSC(info, pos)) != 0)
    {
        pos += l;
    }

    return pos - start;
}

size_t _checkBlockdecl2(struct token_info* info, size_t pos)
{
    size_t start = pos,
    l = 0;

    if((l = checkSC(info, pos)) != 0)
    {
        pos += l;
    }

    if((l = checkDecldelim(info, pos)) != 0)
    {
        pos += l;
    }
    else
    {
        return 0;
    }

    if((l = checkSC(info, pos)) != 0)
    {
        pos += l;
    }

    return pos - start;
}

size_t _checkBlockdecl3(struct token_info* info, size_t pos)
{
    return checkSC(info, pos);
}

size_t checkFilter(struct token_info* info, size_t pos)
{
    size_t start = pos,
    l = 0;
    struct token* tmp = NULL;

    if((l = checkFilterp(info, pos)) != 0)
    {
        pos += l;
    }
    else
    {
        return 0;
    }

    tmp = getTokenByIndex(info, pos);
    if(tmp->type == TOKENTYPE_COLON)
    {
        pos++;
    }
    else
    {
        return 0;
    }

    if((l = checkFilterv(info, pos)) != 0)
    {
        pos += l;
    }
    else
    {
        return 0;
    }

    return pos - start;
}

size_t checkFilterp(struct token_info* info, size_t pos)
{
    size_t start = pos,
    l = 0;

    if(pos < info->len)
    {
        struct token* tmp = getTokenByIndex(info, pos);
        if(strcmp(tmp->value, "filter") == 0)
        {
            l = 1;
        }
        else
        {
            char* x = joinValues(info, pos, pos+1);

            if(strcmp(x, "-filter") == 0 || strcmp(x, "_filter") == 0 || strcmp(x, "*filter") == 0)
            {
                l = 2;
            }
            else
            {
                free(x);
                x = joinValues(info, pos, pos+3);

                if(strcmp(x, "-ms-filter") == 0)
                {
                    l = 4;
                }
                else
                {
                    free(x);
                    return 0;
                }
            }
            free(x);
        }

        tmp = getTokenByIndex(info, start);
        tmp->filterp_l = l;

        pos += l;

        if((checkSC(info, pos)) != 0)
        {
            pos += l;
        }
        return pos - start;
    }

    return 0;
}

size_t checkFilterv(struct token_info* info, size_t pos)
{
    size_t start = pos,
    l = 0;
    struct token* tmp = NULL;

    if((l = checkProgid(info, pos)) != 0)
    {
        pos += l;
    }
    else
    {
        return 0;
    }

    while((l = checkProgid(info, pos)) != 0)
    {
        pos += l;
    }

    tmp = getTokenByIndex(info, start);
    tmp->last_progid = pos;

    if(pos < info->len && (l = checkSC(info, pos)) != 0)
    {
        pos += l;
    }

    if(pos < info->len && (l = checkImportant(info, pos)) != 0)
    {
        pos += l;
    }

    return pos - start;
}

size_t checkProgid(struct token_info* info, size_t pos)
{
    size_t start = pos,
    l = 0;
    char* x = NULL;
    struct token* tmp = NULL;

    if((l = checkSC(info, pos)) != 0)
    {
        pos += l;
    }
    x = joinValues(info, pos, pos+5);
    if(x != NULL && strcmp(x, "progid:DXImageTransform.Microsoft.") == 0)
    {
        free(x);
        pos += 6;
    }
    else
    {
        free(x);
        return 0;
    }

    if((l = checkIdent(info, pos)) != 0)
    {
        pos += l;
    }
    else
    {
        return 0;
    }

    if((l = checkSC(info, pos)) != 0)
    {
        pos += l;
    }

    tmp = getTokenByIndex(info, pos);
    if(tmp->type == TOKENTYPE_LEFTPARENTHESIS)
    {
        struct token* tmpStart = getTokenByIndex(info, start);
        tmpStart->progid_end = tmp->right;
        pos = tmp->right + 1;
    }
    else
    {
        return 0;
    }

    if((l = checkSC(info, pos)) != 0)
    {
        pos += l;
    }

    return pos - start;
}

size_t checkImportant(struct token_info* info, size_t pos)
{
    size_t start = pos,
    l = 0;
    struct token* tmp = getTokenByIndex(info, pos);

    pos++;
    if(tmp->type != TOKENTYPE_EXCLAMATIONMARK)
    {
        return 0;
    }

    if((l = checkSC(info, pos)) != 0)
    {
        pos += l;
    }

    tmp = getTokenByIndex(info, pos);
    if(casecmp(tmp->value, "important") != 0)
    {
        return 0;
    }

    return pos - start + 1;
}

size_t checkDeclaration(struct token_info* info, size_t pos)
{
    size_t start = pos,
    l = 0;
    struct token* tmp = NULL;

    if((l = checkProperty(info, pos)) != 0)
    {
        pos += l;
    }
    else
    {
        return 0;
    }

    tmp = getTokenByIndex(info, pos);
    if(pos < info->len && tmp->type == TOKENTYPE_COLON)
    {
        pos++;
    }
    else
    {
        return 0;
    }

    if((l = checkValue(info, pos)) != 0)
    {
        pos += l;
    }
    else
    {
        return 0;
    }

    return pos - start;
}

size_t checkDecldelim(struct token_info* info, size_t pos)
{
    struct token* tmp = getTokenByIndex(info, pos);
    if(pos < info->len && tmp->type == TOKENTYPE_SEMICOLON)
    {
        return 1;
    }

    return 0;
}

size_t checkProperty(struct token_info* info, size_t pos)
{
    size_t start = pos,
    l = 0;

    if((l = checkIdent(info, pos)) != 0)
    {
        pos += l;
    }
    else
    {
        return 0;
    }
    if((l = checkSC(info, pos)) != 0 )
    {
        pos += l;
    }

    return pos - start;
}

size_t checkValue(struct token_info* info, size_t pos)
{
    size_t start = pos,
    l = 0;

    while(pos < info->len)
    {
        if((l = _checkValue(info, pos)) != 0)
        {
            pos += l;
        }
        else
        {
            break;
        }
    }

    if(pos - start)
    {
        return pos - start;
    }

    return 0;
}

size_t _checkValue(struct token_info* info, size_t pos)
{
    size_t l = 0;
    if((l = checkSC(info, pos)) != 0)
    {
        return l;
    }
    else if((l = checkVhash(info, pos)) != 0)
    {
        return l;
    }
    else if((l = checkAny(info, pos)) != 0)
    {
        return l;
    }
    else if((l = checkBlock(info, pos)) != 0)
    {
        return l;
    }
    else if((l = checkAtkeyword(info, pos)) != 0)
    {
        return l;
    }
    else if((l = checkOperator(info, pos)) != 0)
    {
        return l;
    }
    else
    {
        return checkImportant(info, pos);
    }
}

size_t checkStyleSheet(struct token_info* info, size_t pos)
{
    size_t start = pos;
    size_t l = 0;

    while(pos < info->len)
    {
        if((l = checkSC(info, pos)))
        {
            pos += l;
        }
        else
        {
            struct token* tmp = getTokenByIndex(info, pos);
            if((l = checkAtrule(info, pos)))
            {
                pos += l;
            }
            else if((l = checkRuleset(info, pos)))
            {
                pos += l;
            }
            else if((l = checkUnknown(info, pos)))
            {
                pos += l;
            }
            else
            {
                throwError("checkStyleSheet", tmp->ln);
                return 0;
            }
        }
    }

    return pos - start;
}
struct astnode* getS(struct token_info* info, size_t* pos)
{
    struct astnode* newnode = createASTNodeWithType(ACCSSNODETYPE_S);

    struct token* tmp = getTokenByIndex(info, *pos);
    char* s = joinValues(info, *pos, tmp->ws_last);

    newnode->content = s;

    *pos = tmp->ws_last + 1;

    return newnode;
}

struct astnode* getComment(struct token_info* info, size_t* pos)
{
    struct astnode* newnode = createASTNodeWithType(ACCSSNODETYPE_COMMENT);
    struct token* tmp = getTokenByIndex(info, *pos);
    size_t len = strlen(tmp->value);
    char* s = NULL;

    if(tmp->value[len - 2] == '*' && tmp->value[len - 1] == '/')
    {
        len -= 4;
    }

    s = malloc(sizeof(char)*(len+1));
    if(s == NULL)
    {
        memoryFailure();
        exit(EXIT_FAILURE);
    }

    memcpy(s, &tmp->value[2], len);
    s[len] = '\0';
    newnode->content = s;

    (*pos)++;

    return newnode;
}

struct astnode** getSC(struct token_info* info, size_t* pos)
{
    struct astnode** sc = NULL;

    while(*pos < info->len)
    {
        if(checkS(info, *pos) != 0)
        {
            sc = pushASTNode(sc, getS(info, pos));
        }
        else if(checkComment(info, *pos) != 0)
        {
            sc = pushASTNode(sc, getComment(info, pos));
        }
        else
        {
            break;
        }
    }

    return sc;
}

struct astnode* getRuleset(struct token_info* info, size_t* pos)
{
    struct astnode* ruleset = createASTNodeWithType(ACCSSNODETYPE_RULESET);

    while(*pos < info->len && !(checkBlock(info, *pos)) && checkSelector(info, *pos))
    {
        ruleset->children = pushASTNode(ruleset->children, getSelector(info, pos));
    }

    ruleset->children = pushASTNode(ruleset->children, getBlock(info, pos));

    return ruleset;
}

struct astnode* getSelector(struct token_info* info, size_t* pos)
{
    struct astnode* selector = createASTNodeWithType(ACCSSNODETYPE_SELECTOR);

    struct token* tmp = getTokenByIndex(info, *pos);

    checkSelector(info, *pos);

    while(*pos <= tmp->selector_end)
    {
        selector->children = pushASTNode(selector->children, (checkDelim(info, *pos) != 0 ? getDelim(info, pos) : getSimpleSelector(info, pos)));
    }

    return selector;
}

struct astnode* getSimpleSelector(struct token_info* info, size_t* pos)
{
    struct astnode* simpleselector = createASTNodeWithType(ACCSSNODETYPE_SIMPLESELECTOR);

    while(*pos < info->len && _checkSimpleSelector(info, *pos))
    {
        simpleselector->children = mergeList(simpleselector->children, _getSimpleSelector(info, pos));
    }

    return simpleselector;
}

struct astnode** _getSimpleSelector(struct token_info* info, size_t* pos)
{
    if((checkNthselector(info, *pos)) != 0)
    {
        return pushASTNode(NULL, getNthselector(info, pos));
    }
    else if((checkCombinator(info, *pos)) != 0)
    {
        return pushASTNode(NULL, getCombinator(info, pos));
    }
    else if((checkAttrib(info, *pos)) != 0)
    {
        return pushASTNode(NULL, getAttrib(info, pos));
    }
    else if((checkPseudo(info, *pos)) != 0)
    {
        return pushASTNode(NULL, getPseudo(info, pos));
    }
    else if((checkClazz(info, *pos)) != 0)
    {
        return pushASTNode(NULL, getClazz(info, pos));
    }
    else if((checkShash(info, *pos)) != 0)
    {
        return pushASTNode(NULL,getShash(info, pos));
    }
    else if((checkAny(info, *pos)) != 0)
    {
        return pushASTNode(NULL, getAny(info, pos));
    }
    else if((checkSC(info, *pos)) != 0)
    {
        return getSC(info, pos);
    }
    else if((checkNamespace(info, *pos)) != 0)
    {
        return pushASTNode(NULL, getNamespace(info, pos));
    }

    return NULL;
}

struct astnode* getNthf(struct token_info* info, size_t* pos)
{
    struct token* tmp = NULL;
    struct astnode* nthf = createASTNodeWithType(ACCSSNODETYPE_IDENT);

    (*pos)++;

    tmp = getTokenByIndex(info, *pos);
    nthf->content = joinValues(info, *pos, tmp->nthf_last);

    *pos = tmp->nthf_last + 1;

    return nthf;
}

struct astnode* getNthselector(struct token_info* info, size_t* pos)
{
    struct token* tmp = NULL;
    struct astnode* ns = createASTNodeWithType(ACCSSNODETYPE_NTHSELECTOR);

    ns->children = pushASTNode(ns->children, getNthf(info, pos));

    (*pos)++;

    while(((tmp = getTokenByIndex(info, *pos)) != NULL) && tmp->type != TOKENTYPE_RIGHTPARENTHESIS)
    {
        if((checkSC(info, *pos)) != 0)
        {
            ns->children = mergeList(ns->children, getSC(info, pos));
        }
        else if((checkUnary(info, *pos)) != 0)
        {
            ns->children = pushASTNode(ns->children, getUnary(info, pos));
        }
        else if((checkNth(info, *pos)) != 0)
        {
            ns->children = pushASTNode(ns->children, getNth(info, pos));
        }
    }

    (*pos)++;

    return ns;
}


struct astnode* getUnary(struct token_info* info, size_t* pos)
{
    struct astnode* unary = createASTNodeWithType(ACCSSNODETYPE_UNARY);

    struct token* tmp = getTokenByIndex(info, *pos);

    unary->content = copyValue(tmp->value);

    (*pos)++;

    return unary;
}

struct astnode* getNth(struct token_info* info, size_t* pos)
{
    struct astnode* n = createASTNodeWithType(ACCSSNODETYPE_NTH);

    struct token* tmp = getTokenByIndex(info, *pos);
    if(tmp->nth_last)
    {
        n->content = joinValues(info, *pos, tmp->nth_last);
        *pos = tmp->nth_last + 1;

        return n;
    }

    n->content = copyValue(tmp->value);
    (*pos)++;

    return n;
}

struct astnode* getDelim(struct token_info* info, size_t* pos)
{
    struct astnode* delim = createASTNodeWithType(ACCSSNODETYPE_DELIM);

    (*pos)++;

    return delim;
}

struct astnode* getCombinator(struct token_info* info, size_t* pos)
{
    struct astnode* combinator =createASTNodeWithType(ACCSSNODETYPE_COMBINATOR);

    struct token* tmp = getTokenByIndex(info, *pos);

    combinator->content = copyValue(tmp->value);

    (*pos)++;

    return combinator;
}
struct astnode* getAttrib(struct token_info* info, size_t* pos)
{
    if((checkAttrib1(info, *pos)) != 0)
    {
        return getAttrib1(info, pos);
    }

    if((checkAttrib2(info, *pos)) != 0 )
    {
        return getAttrib2(info, pos);
    }

    return NULL;
}

struct astnode* getAttrib1(struct token_info* info, size_t* pos)
{
    struct astnode* a = createASTNodeWithType(ACCSSNODETYPE_ATTRIB);

    (*pos)++;

    a->children = mergeList(a->children, getSC(info, pos));
    a->children = pushASTNode(a->children, getIdent(info, pos));
    a->children = mergeList(a->children, getSC(info, pos));
    a->children = pushASTNode(a->children, getAttrselector(info, pos));
    a->children = mergeList(a->children, getSC(info, pos));
    a->children = pushASTNode(a->children, checkString(info, *pos) ? getString(info, pos) : getIdent(info, pos));
    a->children = mergeList(a->children, getSC(info, pos));

   (*pos)++;

    return a;
}

struct astnode* getAttrib2(struct token_info* info, size_t* pos)
{
    struct astnode* a = createASTNodeWithType(ACCSSNODETYPE_ATTRIB);

    (*pos)++;

    a->children = mergeList(a->children, getSC(info, pos));
    a->children = pushASTNode(a->children, getIdent(info, pos));
    a->children = mergeList(a->children, getSC(info, pos));

    (*pos)++;

    return a;
}

struct astnode* getIdent(struct token_info* info, size_t* pos)
{
    struct astnode* ident = createASTNodeWithType(ACCSSNODETYPE_IDENT);

    struct token* tmp = getTokenByIndex(info, *pos);
    ident->content = joinValues(info, *pos, tmp->ident_last);

    *pos = tmp->ident_last + 1;

    return ident;
}

struct astnode* getAttrselector(struct token_info* info, size_t* pos)
{
    struct astnode* attrselector = createASTNodeWithType(ACCSSNODETYPE_ATTRSELECTOR);

    struct token* tmp = getTokenByIndex(info, *pos +1);
    if(tmp && tmp->type == TOKENTYPE_EQUALSSIGN)
    {
        attrselector->content = joinValues(info, *pos, *pos+1);
        (*pos)++;
    }
    else
    {
        tmp = getTokenByIndex(info, *pos);
        attrselector->content = copyValue(tmp->value);
    }

    (*pos)++;

    return attrselector;
}

struct astnode* getString(struct token_info* info, size_t* pos)
{
    struct astnode* string = createASTNodeWithType(ACCSSNODETYPE_STRING);

    struct token* tmp = getTokenByIndex(info, *pos);
    string->content = copyValue(tmp->value);

    (*pos)++;

    return string;
}

struct astnode* getPseudo(struct token_info* info, size_t* pos)
{
    if((checkPseudoe(info, *pos))!= 0)
    {
        return getPseudoe(info,pos);
    }

    if((checkPseudoc(info, *pos)) != 0)
    {
        return getPseudoc(info, pos);
    }

    return NULL;
}

struct astnode* getPseudoe(struct token_info* info, size_t* pos)
{
    struct astnode* pseudoe = createASTNodeWithType(ACCSSNODETYPE_PSEUDOE);

    (*pos) += 2;

    pseudoe->children = pushASTNode(pseudoe->children, getIdent(info, pos));

    return pseudoe;
}

struct astnode* getPseudoc(struct token_info* info, size_t* pos)
{
    struct astnode* pseudoc = createASTNodeWithType(ACCSSNODETYPE_PSEUDOC);

    (*pos)++;

    pseudoc->children = pushASTNode(pseudoc->children, checkFunction(info, *pos) ? getFunction(info, pos) : getIdent(info, pos));

    return pseudoc;
}

struct astnode* getFunction(struct token_info* info, size_t* pos)
{
    struct astnode* ident = getIdent(info, pos);
    struct astnode* body = NULL;
    struct astnode* function = createASTNodeWithType(ACCSSNODETYPE_FUNCTION);

    (*pos)++;

    body = strcmp(ident->content, "not") != 0 ? getFunctionBody(info, pos) : getNotFunctionBody(info, pos);
    function->children = pushASTNode(function->children, ident);
    function->children = pushASTNode(function->children, body);

    return function;
}

struct astnode* getFunctionBody(struct token_info* info, size_t* pos)
{
    struct astnode* body = createASTNodeWithType(ACCSSNODETYPE_FUNCTIONBODY);
    struct token* tmp;

    while((tmp = getTokenByIndex(info, *pos)) != NULL && tmp->type != TOKENTYPE_RIGHTPARENTHESIS)
    {
        if((checkTset(info, *pos)) != 0)
        {
            body->children = mergeList(body->children, getTset(info, pos));
        }
        else if((checkClazz(info, *pos)) != 0)
        {
            body->children = pushASTNode(body->children, getClazz(info, pos));
        }
        else
        {
            throwError("getFunctionBody", tmp->ln);
        }
    }

    (*pos)++;

    return body;
}
struct astnode* getNotFunctionBody(struct token_info* info, size_t* pos)
{
    struct astnode* body = createASTNodeWithType(ACCSSNODETYPE_FUNCTIONBODY);
    struct token* tmp;

    while((tmp = getTokenByIndex(info, *pos)) != NULL && tmp->type != TOKENTYPE_RIGHTPARENTHESIS)
    {
        if((checkSimpleselector(info, *pos)) != 0)
        {
            body->children = pushASTNode(body->children, getSimpleSelector(info, pos));
        }
        else
        {
            if(checkOperator(info, *pos))
            {
                 body->children = pushASTNode(body->children, getOperator(info, pos));
            }
            else
            {
                 throwError("getNotFunctionBofy", tmp->ln);
            }
        }
    }

    (*pos)++;

    return body;
}

struct astnode** getTset(struct token_info* info, size_t* pos)
{
    if((checkVhash(info, *pos)) != 0)
    {
        return pushASTNode(NULL, getVhash(info, pos));
    }

    else if((checkAny(info, *pos)) != 0)
    {
        return pushASTNode(NULL, getAny(info, pos));
    }
    else if((checkSC(info, *pos)) != 0)
    {
        return getSC(info, pos);
    }
    else if((checkOperator(info, *pos)) != 0)
    {
        return pushASTNode(NULL, getOperator(info, pos));
    }
    return NULL;
}

struct astnode* getVhash(struct token_info* info, size_t* pos)
{
    struct astnode* vhash = createASTNodeWithType(ACCSSNODETYPE_VHASH);

    (*pos)++;
    vhash->content = getNmName2(info, pos);

    return vhash;
}

char* getNmName2(struct token_info* info, size_t* pos)
{
    struct token* tmp = getTokenByIndex(info, *pos),
    *tmp1 = NULL;
    char* s = NULL;
    size_t l = strlen(tmp->value);

    (*pos)++;
    tmp1 = getTokenByIndex(info, *pos);

    if(tmp->type == TOKENTYPE_DECIMALNUMBER &&
        *pos < info->len &&
        tmp1->type == TOKENTYPE_IDENTIFIER
        )
    {
        size_t l2 = strlen(tmp1->value);

        s = malloc(sizeof(char)*(l + l2 +1));
        if(s == NULL)
        {
            memoryFailure();
            exit(EXIT_FAILURE);
        }

        memcpy(s, tmp->value, l);
        memcpy(&s[l], tmp1->value, l2);
        s[l+l2] = '\0';

        (*pos)++;
    }
    else
    {
        s = malloc(sizeof(char)*(l +1));
        if(s == NULL)
        {
            memoryFailure();
            exit(EXIT_FAILURE);
        }

        strcpy(s, tmp->value);
    }

    return s;
}

struct astnode* getAny(struct token_info* info, size_t* pos)
{
    if((checkBraces(info, *pos)) != 0)
    {
        return getBraces(info, pos);
    }
    else if((checkString(info, *pos)) != 0)
    {
        return getString(info, pos);
    }
    else if((checkPercentage(info, *pos)) != 0)
    {
        return getPercentage(info, pos);
    }
    else if((checkDimension(info, *pos)) != 0)
    {
        return getDimension(info, pos);
    }
    else if((checkNumber(info, *pos)) != 0)
    {
        return getNumber(info, pos);
    }
    else if((checkUri(info, *pos)) != 0)
    {
        return getUri(info, pos);
    }
    else if((checkFunctionExpression(info, *pos)) != 0)
    {
        return getFunctionExpression(info, pos);
    }
    else if((checkFunction(info, *pos)) != 0)
    {
        return getFunction(info, pos);
    }
    else if((checkIdent(info, *pos)) != 0)
    {
        return getIdent(info, pos);
    }
    else if((checkUnary(info, *pos)) != 0)
    {
        return getUnary(info, pos);
    }
    return NULL;
}

struct astnode* getBraces(struct token_info* info, size_t* pos)
{
    struct token* tmp = getTokenByIndex(info, *pos);
    struct astnode* braces = createASTNodeWithType(ACCSSNODETYPE_BRACES);

    (*pos)++;

    braces->children = getTsets(info, pos);

    braces->content = malloc(sizeof(char)*3);
    if(braces->content == NULL)
    {
        memoryFailure();
        exit(EXIT_FAILURE);
    }

    if(tmp->value[0] == '[')
    {
         braces->content[0] = '[';
         braces->content[1] = ']';
         braces->content[2] = '\0';
    }
    else
    {
        braces->content[0] = '(';
        braces->content[1] = ')';
        braces->content[2] = '\0';
    }

    (*pos)++;

    return braces;
}

struct astnode* getPercentage(struct token_info* info, size_t* pos)
{
    struct astnode* n = getNumber(info, pos);
    struct astnode* percentage = createASTNodeWithType(ACCSSNODETYPE_PERCENTAGE);

    (*pos)++;

    percentage->children = pushASTNode(percentage->children, n);

    return percentage;
}

struct astnode* getNumber(struct token_info* info, size_t* pos)
{
    struct token* tmp = getTokenByIndex(info, *pos);
    struct astnode* number = createASTNodeWithType(ACCSSNODETYPE_NUMBER);

    size_t l = tmp->number_l;

    number->content = joinValues(info, *pos, *pos+l-1);

    *pos += l;

    return number;
}

struct astnode** getTsets(struct token_info* info, size_t* pos)
{
    struct astnode** tsets = NULL;
    struct astnode** x = NULL;

    while((x = getTset(info, pos)) != NULL)
    {
        tsets = mergeList(tsets, x);
    }

    return tsets;
}

struct astnode* getDimension(struct token_info* info, size_t* pos)
{
    struct astnode* n = getNumber(info, pos);
    struct astnode* ident = createASTNodeWithType(ACCSSNODETYPE_IDENT);
    struct astnode* dimension = createASTNodeWithType(ACCSSNODETYPE_DIMENSION);

    ident->content = getNmName2(info, pos);

    dimension->children = pushASTNode(dimension->children, n);
    dimension->children = pushASTNode(dimension->children, ident);

    return dimension;
}

struct astnode* getUri(struct token_info* info, size_t* pos)
{
   *pos += 2;

    if((checkUri1(info, *pos)) != 0)
    {
        struct astnode* uri = createASTNodeWithType(ACCSSNODETYPE_URI);
        uri->children = mergeList(uri->children, getSC(info, pos));
        uri->children = pushASTNode(uri->children, getString(info, pos));
        uri->children = mergeList(uri->children, getSC(info, pos));

        (*pos)++;

        return uri;
    }
    else
    {
        struct astnode* raw = createASTNodeWithType(ACCSSNODETYPE_RAW);
        struct astnode* uri = createASTNodeWithType(ACCSSNODETYPE_URI);
        size_t l = 0;

        uri->children = mergeList(uri->children, getSC(info, pos));
        l = checkExcluding(info, *pos);
        raw->content = joinValues(info, *pos, *pos + l);
        uri->children = pushASTNode(uri->children, raw);
        *pos += l + 1;
        uri->children = mergeList(uri->children, getSC(info,pos));

        (*pos)++;

        return uri;
    }
}

struct astnode* getFunctionExpression(struct token_info* info, size_t* pos)
{
    struct token* tmp = NULL;
    char* e = NULL;
    struct astnode* functionexpression = NULL;

    (*pos)++;

    tmp = getTokenByIndex(info, *pos);
    e = joinValues(info, *pos + 1, tmp->right - 1);
    *pos = tmp->right + 1;
    functionexpression = createASTNodeWithType(ACCSSNODETYPE_FUNCTIONEXPRESSION);
    functionexpression->content = e;

    return functionexpression;
}

struct astnode* getOperator(struct token_info* info, size_t* pos)
{
    struct astnode* operator = createASTNodeWithType(ACCSSNODETYPE_OPERATOR);
    struct token* tmp = getTokenByIndex(info, *pos);

    operator->content = copyValue(tmp->value);
    (*pos)++;

    return operator;
}

struct astnode* getClazz(struct token_info* info, size_t* pos)
{
    struct astnode* clazz = createASTNodeWithType(ACCSSNODETYPE_CLAZZ);

    (*pos)++;

    clazz->children = pushASTNode(clazz->children, getIdent(info, pos));

    return clazz;
}

struct astnode* getShash(struct token_info* info, size_t* pos)
{
    struct astnode* shash = createASTNodeWithType(ACCSSNODETYPE_SHASH);

    (*pos)++;

    shash->content = getNmName(info, pos);

    return shash;
}

char* getNmName(struct token_info* info, size_t* pos)
{
    struct token* tmp = getTokenByIndex(info, *pos);

    char* s = joinValues(info, *pos, tmp->nm_name_last);

    *pos = tmp->nm_name_last + 1;

    return s;
}

struct astnode* getNamespace(struct token_info* info, size_t* pos)
{
    struct astnode* namespace = createASTNodeWithType(ACCSSNODETYPE_NAMESPACE);

    (*pos)++;

    return namespace;
}

struct astnode* getBlock(struct token_info* info, size_t* pos)
{
    struct astnode* block = createASTNodeWithType(ACCSSNODETYPE_BLOCK);

    struct token* tmp = getTokenByIndex(info, *pos);
    size_t end = tmp->right;

    (*pos)++;

    while(*pos < end)
    {
        if((checkBlockdecl(info, *pos)) != 0)
        {
            block->children = mergeList(block->children, getBlockdecl(info, pos));
        }
        else
        {
            throwError("getBlock", tmp->ln);
            return block;
        }
    }

    *pos = end + 1;

    return block;
}

struct astnode** getBlockdecl(struct token_info* info, size_t* pos)
{
    struct token* tmp = getTokenByIndex(info, *pos);
    switch(tmp->bd_type)
    {
        case 1:
        {
            return _getBlockdecl0(info, pos);
        }
        case 2:
        {
            return _getBlockdecl1(info, pos);
        }
        case 3:
        {
            return _getBlockdecl2(info, pos);
        }
        case 4:
        {
            return _getBlockdecl3(info, pos);
        }
        default:
        {
        }
    }
    return NULL;
}

struct astnode** _getBlockdecl0(struct token_info* info, size_t* pos)
{
    struct astnode** list = NULL;
    struct token* tmp = NULL;

    list = mergeList(list, getSC(info, pos));

    tmp = getTokenByIndex(info, *pos);
    list = pushASTNode(list, tmp->bd_filter ? getFilter(info, pos) : getDeclaration(info, pos));
    list = pushASTNode(list, getDecldelim(info, pos));
    list = mergeList(list, getSC(info, pos));

    return list;
}

struct astnode** _getBlockdecl1(struct token_info* info, size_t* pos)
{
    struct astnode** list = NULL;
    struct token* tmp = NULL;

    list = mergeList(list, getSC(info, pos));

    tmp = getTokenByIndex(info, *pos);
    list = pushASTNode(list, tmp->bd_filter ? getFilter(info, pos) : getDeclaration(info, pos));
    list = mergeList(list, getSC(info, pos));

    return list;
}

struct astnode** _getBlockdecl2(struct token_info* info, size_t* pos)
{
    struct astnode** list = NULL;

    list = mergeList(list, getSC(info, pos));

    list = pushASTNode(list, getDecldelim(info, pos));
    list = mergeList(list, getSC(info, pos));

    return list;
}

struct astnode** _getBlockdecl3(struct token_info* info, size_t* pos)
{
    return getSC(info, pos);
}

struct astnode* getFilter(struct token_info* info, size_t* pos)
{
    struct astnode* filter = createASTNodeWithType(ACCSSNODETYPE_FILTER);

    filter->children = pushASTNode(filter->children, getFilterp(info, pos));
    (*pos)++;
    filter->children = pushASTNode(filter->children, getFilterv(info, pos));

    return filter;
}

struct astnode* getFilterv(struct token_info* info, size_t* pos)
{
    struct astnode* filterv = createASTNodeWithType(ACCSSNODETYPE_FILTERV);

    struct token* tmp = getTokenByIndex(info, *pos);

    while(*pos < tmp->last_progid)
    {
        filterv->children = pushASTNode(filterv->children, getProgid(info, pos));
    }

    if(checkSC(info, *pos) != 0)
    {
        filterv->children = mergeList(filterv->children, getSC(info, pos));
    }

    if(*pos < info->len && checkImportant(info, *pos) != 0)
    {
        filterv->children = pushASTNode(filterv->children, getImportant(info, pos));
    }

    return filterv;
}
struct astnode* getFilterp(struct token_info* info, size_t* pos)
{
    struct token* tmp = getTokenByIndex(info, *pos);
    struct astnode* ident = createASTNodeWithType(ACCSSNODETYPE_IDENT);
    struct astnode* filterp = createASTNodeWithType(ACCSSNODETYPE_PROPERTY);

    ident->content = copyValue(tmp->value);

    *pos += tmp->filterp_l;

    filterp->children = pushASTNode(filterp->children, ident);
    filterp->children = mergeList(filterp->children, getSC(info, pos));
    return filterp;
}

struct astnode* getDeclaration(struct token_info* info, size_t* pos)
{
    struct astnode* declaration = createASTNodeWithType(ACCSSNODETYPE_DECLARATION);

    declaration->children = pushASTNode(declaration->children, getProperty(info, pos));

    (*pos)++;

    declaration->children = pushASTNode(declaration->children, getValue(info, pos));

    return declaration;
}

struct astnode* getDecldelim(struct token_info* info, size_t* pos)
{
    struct astnode* decldelim = createASTNodeWithType(ACCSSNODETYPE_DECLDELIM);

    (*pos)++;

    return decldelim;
}

struct astnode* getImportant(struct token_info* info, size_t* pos)
{
    struct astnode** sc = NULL;
    struct astnode* important = createASTNodeWithType(ACCSSNODETYPE_IMPORTANT);

    (*pos)++;

    sc = getSC(info, pos);

    (*pos)++;

    important->children = mergeList(important->children, sc);

    return important;
}

struct astnode* getProgid(struct token_info* info, size_t* pos)
{
    struct astnode* progid = createASTNodeWithType(ACCSSNODETYPE_PROGID);
    struct token* tmp = getTokenByIndex(info, *pos);

    size_t progid_end = tmp->progid_end;

    progid->children = mergeList(progid->children, getSC(info, pos));
    progid->children = pushASTNode(progid->children, _getProgid(info, pos, progid_end));
    progid->children = mergeList(progid->children, getSC(info, pos));

    return progid;
}
struct astnode* _getProgid(struct token_info* info, size_t* pos, size_t progid_end)
{
    char* x = joinValues(info, *pos, progid_end);
    struct astnode* _progid = createASTNodeWithType(ACCSSNODETYPE_RAW);

    *pos = progid_end + 1;

    _progid->content = x;

    return _progid;
}

struct astnode* getProperty(struct token_info* info, size_t* pos)
{
    struct astnode* property = createASTNodeWithType(ACCSSNODETYPE_PROPERTY);

    property->children = pushASTNode(property->children, getIdent(info, pos));
    property->children = mergeList(property->children, getSC(info, pos));

    return property;
}

struct astnode* getValue(struct token_info* info, size_t* pos)
{
    struct astnode* property = createASTNodeWithType(ACCSSNODETYPE_VALUE);

    while(*pos < info->len && _checkValue(info, *pos))
    {
        property->children = mergeList(property->children, _getValue(info, pos));
    }

    return property;
}
struct astnode** _getValue(struct token_info* info, size_t* pos)
{
    if(checkSC(info, *pos) != 0)
    {
        return getSC(info, pos);
    }
    else if(checkVhash(info, *pos) != 0)
    {
        return pushASTNode(NULL, getVhash(info, pos));
    }
    else if(checkAny(info, *pos) != 0)
    {
        return pushASTNode(NULL, getAny(info, pos));
    }
    else if(checkBlock(info, *pos) != 0)
    {
        return pushASTNode(NULL, getBlock(info, pos));
    }
    else if(checkAtkeyword(info, *pos) != 0)
    {
        return pushASTNode(NULL, getAtkeyword(info, pos));
    }
    else if(checkOperator(info, *pos) != 0)
    {
        return pushASTNode(NULL, getOperator(info, pos));
    }
    else if(checkImportant(info, *pos) != 0)
    {
        return pushASTNode(NULL, getImportant(info, pos));
    }

    return NULL;
}

struct astnode* getAtkeyword(struct token_info* info, size_t* pos)
{
    struct astnode* atkeyword = createASTNodeWithType(ACCSSNODETYPE_ATKEYWORD);

    (*pos)++;

    atkeyword->children = pushASTNode(atkeyword->children, getIdent(info, pos));

    return atkeyword;
}

struct astnode* getAtrule(struct token_info* info, size_t* pos)
{
    struct token* tmp = getTokenByIndex(info, *pos);

    switch(tmp->atrule_type)
    {
        case 1: return getAtruler(info, pos);
        case 2: return getAtruleb(info, pos);
        case 3: return getAtrules(info, pos);
    }
    return NULL;
}

struct astnode* getAtruleb(struct token_info* info, size_t* pos)
{
    struct astnode* atruleb = createASTNodeWithType(ACCSSNODETYPE_ATRULEB);

    atruleb->children = pushASTNode(atruleb->children, getAtkeyword(info, pos));
    atruleb->children = mergeList(atruleb->children, getTsets(info, pos));
    atruleb->children = pushASTNode(atruleb->children, getBlock(info, pos));

    return atruleb;
}

struct astnode* getAtruler(struct token_info* info, size_t* pos)
{
    struct astnode* atruler = createASTNodeWithType(ACCSSNODETYPE_ATRULER);

    atruler->children = pushASTNode(atruler->children, getAtkeyword(info, pos));
    atruler->children = pushASTNode(atruler->children, getAtrulerq(info, pos));

    (*pos)++;

    atruler->children = pushASTNode(atruler->children, getAtrulers(info, pos));

    (*pos)++;

    return atruler;
}

struct astnode* getAtrulerq(struct token_info* info, size_t* pos)
{
    struct astnode* atrulerq = createASTNodeWithType(ACCSSNODETYPE_ATRULERQ);

    atrulerq->children = mergeList(atrulerq->children, getTsets(info, pos));

    return atrulerq;
}
struct astnode* getAtrulers(struct token_info* info, size_t* pos)
{
    struct astnode* atrulers = createASTNodeWithType(ACCSSNODETYPE_ATRULERS);
    struct token* tmp = NULL;

    atrulers->children = mergeList(atrulers->children, getSC(info, pos));

    while((tmp = getTokenByIndex(info, *pos)) != NULL && !tmp->atrulers_end)
    {
        if(checkSC(info, *pos) != 0)
        {
            atrulers->children = mergeList(atrulers->children, getSC(info, pos));
        }
        else if(checkRuleset(info, *pos) != 0)
        {
            atrulers->children = pushASTNode(atrulers->children, getRuleset(info, pos));
        }
        else
        {
            atrulers->children = pushASTNode(atrulers->children, getAtrule(info, pos));

        }
    }

    atrulers->children = mergeList(atrulers->children, getSC(info, pos));

    return atrulers;
}

struct astnode* getAtrules(struct token_info* info, size_t* pos)
{
    struct astnode* atrules = createASTNodeWithType(ACCSSNODETYPE_ATRULES);
    atrules->children = pushASTNode(atrules->children, getAtkeyword(info, pos));
    atrules->children = mergeList(atrules->children, getTsets(info, pos));

    (*pos)++;

    return atrules;
}

struct astnode*  getUnknown(struct token_info* info, size_t* pos)
{
    struct astnode* unknown = createASTNodeWithType(ACCSSNODETYPE_UNKNOWN);
    struct token* tmp = getTokenByIndex(info, *pos);

    unknown->content = copyValue(tmp->value);

    (*pos)++;

   return unknown;
}

struct astnode* getStylesheet(struct token_info* info, size_t* pos, char* error)
{
    struct astnode* stylesheet = createASTNodeWithType(ACCSSNODETYPE_STYLESHEET);

    while(*pos < info->len)
    {
        if(checkSC(info, *pos) != 0)
        {
            stylesheet->children = mergeList(stylesheet->children, getSC(info, pos));
        }
        else
        {
            struct token* tmp = getTokenByIndex(info, *pos);
            int currentBlockLN = tmp->ln;
            if(checkRuleset(info, *pos) != 0)
            {
                stylesheet->children = pushASTNode(stylesheet->children, getRuleset(info, pos));
            }
            else if((checkAtrule(info, *pos)) != 0)
            {
                stylesheet->children = pushASTNode(stylesheet->children, getAtrule(info, pos));
            }
            else if((checkUnknown(info, *pos)) != 0)
            {
                stylesheet->children = pushASTNode(stylesheet->children, getUnknown(info, pos));
            }
            else
            {
                throwError("getStylesheet", currentBlockLN);
                (*error)++;
                break;
            }
        }
    }

    return stylesheet;
}

struct astnode* getAST(struct token_info* info, char _needInfo,  char* error)
{
    size_t pos = 0;

    *error = 0;

    markSC(info);

    if(checkStyleSheet(info, 0))
    {
        return getStylesheet(info, &pos, error);
    }
    else
    {
        *error = 1;
        return NULL;
    }

}

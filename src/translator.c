/*
 * ACCSS C Code Copyright (C) 2014 by Gandalf Sievers
 * Created by Gandalf Sievers on 18.08.14.
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
#include <stdarg.h>
#include <string.h>
#include "helper.h"
#include "translator.h"
#include "ast.h"



char* concat(const char* format, int count, ...);

char* percentage(struct astnode* node);
char* comment(struct astnode* node);
char* clazz(struct astnode* node);
char* atkeyword(struct astnode* node);
char* shash(struct astnode* node);
char* vhash(struct astnode* node);
char* attrib(struct astnode* node);
char* important(struct astnode* node);
char* nthselector(struct astnode* node);
char* function(struct astnode* node);
char* declaration(struct astnode* node);
char* msfilter(struct astnode* node);
char* block(struct astnode* node);
char* braces(struct astnode* node);
char* atrules(struct astnode* node);
char* atruler(struct astnode* node);
char* pseudoe(struct astnode* node);
char* pseudoc(struct astnode* node);
char* uri(struct astnode* node);
char* functionExpression(struct astnode* node);

char* simple(struct astnode* node);
char* primitive(struct astnode* node);
char* _t(struct astnode* node);
char* composite(struct astnode* node, int i);

char* concat(const char* format, int count, ...)
{
    va_list ap;
    char* arg ;
    char* buffer = NULL;
    size_t len = 0;
    if(count)
    {
        int x = count;
        va_start(ap, count);

        while(x--)
        {
            arg = va_arg(ap, char*);
            if(arg != NULL && arg[0] != '\0')
            {
                len += strlen(arg);
            }

        }

        buffer = malloc(sizeof(char)*(len+1));
        if(buffer == NULL)
        {
            memoryFailure();
            exit(EXIT_FAILURE);
        }

        va_start(ap, count);
        vsprintf(buffer, format, ap);
        va_end(ap);
    }
    return buffer;
}

char* percentage(struct astnode* node)
{
    char* tmp = _t(node->children[0]);
    char* tmp2 = concat("%s%s", 2, tmp, "%");
    free(tmp);
    return tmp2;
}

char* comment(struct astnode* node)
{
    return concat("%s%s%s", 3, "/*", node->content, "*/");
}

char* clazz(struct astnode* node)
{
    char* tmp = _t(node->children[0]);
    char* tmp2 = concat("%s%s", 2, ".", tmp);
    free(tmp);
    return tmp2;
}

char* atkeyword(struct astnode* node)
{
    char* tmp = _t(node->children[0]);
    char* tmp2 = concat("%s%s", 2, "@", tmp);
    free(tmp);
    return tmp2;
}

char* shash(struct astnode* node)
{
    return concat("%s%s", 2, "#", node->content);
}

char* vhash(struct astnode* node)
{
    return concat("%s%s", 2, "#", node->content);
}

char* attrib(struct astnode* node)
{
    char* tmp = composite(node, 0);
    char* tmp2 = concat("%s%s%s", 3, "[", tmp, "]");
    free(tmp);
    return tmp2;
}

char* important(struct astnode* node)
{
    char* tmp = composite(node, 0);
    char* tmp2 = concat("%s%s%s", 3, "!", tmp, "important");
    free(tmp);
    return tmp2;
}

char* nthselector(struct astnode* node)
{
    char* tmp = simple(node->children[0]);
    char* tmp1 = composite(node, 1);
    char* tmp2 = concat("%s%s%s%s%s", 5, ":", tmp, "(", tmp1, ")");
    free(tmp);
    free(tmp1);
    return tmp2;
}

char* function(struct astnode* node)
{
    char* tmp = simple(node->children[0]);
    char* tmp1 = composite(node->children[1], 0);
    char* tmp2 = concat("%s%s%s%s", 4, tmp, "(", tmp1, ")");
    free(tmp);
    free(tmp1);
    return tmp2;
}

char* declaration(struct astnode* node)
{
    char* tmp = _t(node->children[0]);
    char* tmp1 = _t(node->children[1]);
    char* tmp2 = concat("%s%s%s", 3,  tmp, ":", tmp1);
    free(tmp);
    free(tmp1);
    return tmp2;
}

char* msfilter(struct astnode* node)
{
    char* tmp = _t(node->children[0]);
    char* tmp1 = _t(node->children[1]);
    char* tmp2 = concat("%s%s%s", 3,  tmp, ":", tmp1);
    free(tmp);
    free(tmp1);
    return tmp2;
}

char* block(struct astnode* node)
{
    char* tmp = composite(node, 0);
    char* tmp2 = concat("%s%s%s", 3, "{", tmp, "}");
    free(tmp);
    return tmp2;
}

char* braces(struct astnode* node)
{
    char* tmp = composite(node, 0);
    char* tmp2 = NULL;
    if(node->content[0] == '(')
    {
        tmp2 = concat("%s%s%s", 3, "(", tmp, ")");
    }
    else
    {
        tmp2 = concat("%s%s%s", 3, "[", tmp, "]");
    }

    free(tmp);
    return tmp2;
}

char* atrules(struct astnode* node)
{
    char* tmp = composite(node, 0);
    char* tmp2 = concat("%s%s", 2, tmp, ";");
    free(tmp);
    return tmp2;
}

char* atruler(struct astnode* node)
{
    char* tmp = _t(node->children[0]);
    char* tmp1 = _t(node->children[1]);
    char* tmp2 =  _t(node->children[2]);
    char* tmp3 = concat("%s%s%s%s%s", 5, tmp, tmp1, "{", tmp2, "}");
    free(tmp);
    free(tmp1);
    free(tmp2);
    return tmp3;
}

char* pseudoe(struct astnode* node)
{
    char* tmp =  _t(node->children[0]);
    char* tmp2 = concat("%s%s", 2, "::", tmp);
    free(tmp);
    return tmp2;
}

char* pseudoc(struct astnode* node)
{
    char* tmp =  _t(node->children[0]);
    char* tmp2 = concat("%s%s", 2, ":", tmp);
    free(tmp);
    return tmp2;
}

char* uri(struct astnode* node)
{
    char* tmp = composite(node, 0);
    char* tmp2 = concat("%s%s%s", 3, "url(", tmp, ")");
    free(tmp);
    return tmp2;
}

char* functionExpression(struct astnode* node)
{
    return concat("%s%s%s", 3, "expression(", node->content, ")");
}

char* composite(struct astnode* node, int i)
{
    char* s = malloc(sizeof(char));
    if(s == NULL)
    {
        memoryFailure();
        exit(EXIT_FAILURE);
    }

    s[0] = '\0';
    size_t llen = listLength(node->children);
    for(; i < llen; i++)
    {
        char* tmp = _t(node->children[i]);
        size_t slen = strlen(s);
        size_t tlen = strlen(tmp);
        char* stmp = realloc(s, sizeof(char)*(slen+tlen+1));
        if(stmp == NULL)
        {
            memoryFailure();
            exit(EXIT_FAILURE);
        }

        s=stmp;
        memcpy(&s[slen], tmp, tlen);
        free(tmp);
        slen+=tlen;
        s[slen] = '\0';
    }
    return s;
}

char* primitive(struct astnode* node)
{
    switch(node->type)
    {
        case ACCSSNODETYPE_CDO:
            return copyValue("cdo");
            break;
        case ACCSSNODETYPE_CDC:
            return copyValue("cdc");
            break;
        case ACCSSNODETYPE_DECLDELIM:
            return copyValue(";");
            break;
        case ACCSSNODETYPE_NAMESPACE:
            return copyValue("|");
            break;
        case ACCSSNODETYPE_DELIM:
            return copyValue(",");
            break;
        default:
            return NULL;
            break;
    }
}

char* simple(struct astnode* node)
{
    return copyValue(node->content);
}

char* _t(struct astnode* node)
{
    switch(node->type)
    {
        case ACCSSNODETYPE_UNARY:
        case ACCSSNODETYPE_NTH:
        case ACCSSNODETYPE_COMBINATOR:
        case ACCSSNODETYPE_IDENT:
        case ACCSSNODETYPE_NUMBER:
        case ACCSSNODETYPE_S:
        case ACCSSNODETYPE_STRING:
        case ACCSSNODETYPE_ATTRSELECTOR:
        case ACCSSNODETYPE_OPERATOR:
        case ACCSSNODETYPE_RAW:
        case ACCSSNODETYPE_UNKNOWN:
        {
            return simple(node);
        }
            break;

        case ACCSSNODETYPE_SIMPLESELECTOR:
        case ACCSSNODETYPE_DIMENSION:
        case ACCSSNODETYPE_SELECTOR:
        case ACCSSNODETYPE_PROPERTY:
        case ACCSSNODETYPE_VALUE:
        case ACCSSNODETYPE_FILTERV:
        case ACCSSNODETYPE_PROGID:
        case ACCSSNODETYPE_RULESET:
        case ACCSSNODETYPE_ATRULEB:
        case ACCSSNODETYPE_ATRULERQ:
        case ACCSSNODETYPE_ATRULERS:
        case ACCSSNODETYPE_STYLESHEET:
        {
            return composite(node, 0);
        }
            break;

        case ACCSSNODETYPE_CDO:
        case ACCSSNODETYPE_CDC:
        case ACCSSNODETYPE_DECLDELIM:
        case ACCSSNODETYPE_NAMESPACE:
        case ACCSSNODETYPE_DELIM:
        {
            return primitive(node);
        }
            break;

        case ACCSSNODETYPE_PERCENTAGE:
        {
            return percentage(node);
        }
            break;

        case ACCSSNODETYPE_COMMENT:
        {
            return comment(node);
        }
            break;

        case ACCSSNODETYPE_CLAZZ:
        {
            return clazz(node);
        }
            break;

        case ACCSSNODETYPE_ATKEYWORD:
        {
            return atkeyword(node);
        }
            break;

        case ACCSSNODETYPE_SHASH:
        {
            return shash(node);
        }
            break;

        case ACCSSNODETYPE_VHASH:
        {
            return vhash(node);
        }
            break;

        case ACCSSNODETYPE_ATTRIB:
        {
            return attrib(node);
        }
            break;

        case ACCSSNODETYPE_IMPORTANT:
        {
            return important(node);
        }
            break;

        case ACCSSNODETYPE_NTHSELECTOR:
        {
            return nthselector(node);
        }
            break;

        case  ACCSSNODETYPE_FUNCTION:
        {
            return function(node);
        }
            break;

        case ACCSSNODETYPE_DECLARATION:
        {
            return declaration(node);
        }
            break;

        case ACCSSNODETYPE_FILTER:
        {
            return msfilter(node);
        }
            break;

        case ACCSSNODETYPE_BLOCK:
        {
            return block(node);
        }
            break;

        case ACCSSNODETYPE_BRACES:
        {
            return braces(node);
        }
            break;

        case ACCSSNODETYPE_ATRULES:
        {
            return atrules(node);
        }
            break;

        case ACCSSNODETYPE_ATRULER:
        {
            return atruler(node);
        }
            break;

        case ACCSSNODETYPE_PSEUDOE:
        {
            return pseudoe(node);
        }
            break;

        case ACCSSNODETYPE_PSEUDOC:
        {
            return pseudoc(node);
        }
            break;

        case ACCSSNODETYPE_URI:
        {
            return uri(node);
        }
            break;

        case ACCSSNODETYPE_FUNCTIONEXPRESSION:
        {
            return functionExpression(node);
        }
            break;

        default:
        {
            return NULL;
        }
    }
}

char* translate(struct astnode* node)
{
    return _t(node);
}

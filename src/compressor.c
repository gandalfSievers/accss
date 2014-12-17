/*
 * ACCSS C Code Copyright (C) 2014 by Gandalf Sievers
 * Created by Gandalf Sievers on 08.08.14.
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
#include "debug.h"
#include "compressor.h"
#include "translator.h"
#include "ast.h"

//SIZE_MAX
#define LDBUFFERSIZE 32

char* getVendorIDFromNode(struct astnode* node);

char* extractMain(const char* name)
{
    char* f = strchr(name, '-');
    if( f != NULL)
    {
        size_t len = f-name;
        char* first = malloc(sizeof(char)*len+1);
        memcpy(first, name, len);
        first[len] = '\0';
        return first;
    }
    return copyValue(name);
}

size_t shortsLength(struct shortHand** list)
{
    if(list == NULL)
    {
        return 0;
    }
    if(*list == NULL)
    {
        return 0;
    }
    size_t listLength = 0;
    struct shortHand** tmp = list;
    while(*tmp != NULL)
    {
        listLength++;
        tmp++;
    }

    return listLength;
}

struct shortHand** getByKey(struct pchar_shorts** list, const char* key)
{
    if(list == NULL)
    {
        return NULL;
    }
    struct pchar_shorts** tmp = list;

    while (*tmp != NULL)
    {
        if( casecmp((*tmp)->key, key) == 0 )
        {
            return (*tmp)->list;
        }
        tmp++;
    }

    return NULL;
}

size_t pchar_sLength(struct pchar_shorts** list)
{
    size_t len = 0;
    struct pchar_shorts** tmp = list;
    while (*tmp != NULL)
    {
        len++;
        tmp++;
    }
    return len;
}

void freeSide(struct shortSide* side)
{
    if(side != NULL)
    {
        if(side->s != NULL)
        {
            free(side->s);
        }
        if(side->t != NULL)
        {
            deleteASTList(side->t);
        }
        free(side);
    }
}

void freeShortList(struct shortHand** sList)
{
    while (*sList != NULL)
    {
        struct shortHand* sh = *sList;
        sList++;

        free(sh->name);
        struct shortSide* a[] = {sh->sides.top, sh->sides.right, sh->sides.bottom, sh->sides.left};
        char alen = 4;
		char i = 0;
        for(; i < alen; i++)
        {
            if (a[i] != NULL)
            {
                freeSide(a[i]);
            }
        }
        free(sh);
    }
}

void freeShortHands(struct pchar_shorts** list)
{
    if(list != NULL)
    {
        struct pchar_shorts** psTmp = list;

        while (*psTmp != NULL)
        {
            struct pchar_shorts* psCurrent = *psTmp;
            psTmp++;

            free((psCurrent)->key);
            struct shortHand** sList = psCurrent->list;
            if(sList != NULL)
            {
                freeShortList(sList);
                free(sList);
            }
            free(psCurrent);
        }
        free(list);
    }
}

struct pchar_shorts** addByKey(struct pchar_shorts** list, const char* key, struct shortHand** shorts)
{
    struct pchar_shorts** tmp = list;
    struct pchar_shorts* bucket = NULL;

    if( list == NULL)
    {
        bucket = malloc(sizeof(struct pchar_shorts));
        bucket->list = shorts;
        bucket->key = copyValue(key);

        struct pchar_shorts** list = malloc(sizeof(struct pchar_shorts*)*2);
        if(list == NULL)
        {
            memoryFailure();
            exit(EXIT_FAILURE);
        }

        list[0] = bucket;
        list[1] = NULL;

        return list;
    }

    while (*tmp != NULL)
    {
        if( casecmp((*tmp)->key, key) == 0 )
        {
            bucket = (*tmp);
            break;
        }
        tmp++;
    }

    if(bucket != NULL)
    {
        bucket->list = shorts;
    }
    else
    {
        bucket = malloc(sizeof(struct pchar_shorts));
        bucket->list = shorts;
        bucket->key = copyValue(key);

        size_t size = pchar_sLength(list);
        struct pchar_shorts** newlist = realloc(list, (sizeof(struct pchar_shorts*)*(size+2)));
        if(newlist == NULL)
        {
            memoryFailure();
            exit(EXIT_FAILURE);
        }
        list = newlist;

        list[size+1] = list[size];
        list[size] = bucket;
    }

    return list;
}
struct shortHand** pushShorts(struct shortHand** shorts, struct shortHand* sh)
{
    if(sh == NULL)
    {
        return shorts;
    }
    if(shorts == NULL)
    {

        struct shortHand** list = malloc(sizeof(struct shortHand*)*2);
        if(list == NULL)
        {
            memoryFailure();
            exit(EXIT_FAILURE);
        }
        list[0] = sh;
        list[1] = NULL;
        return list;
    }

    size_t size = shortsLength(shorts);
    struct shortHand** newlist = realloc(shorts, (sizeof(struct shortHand*)*(size+2)));
    if(newlist == NULL)
    {
        memoryFailure();
        exit(EXIT_FAILURE);
    }
    shorts = newlist;

    shorts[size+1] = shorts[size];
    shorts[size] = sh;

    return shorts;
}

char isOkToMinimize(struct shortHand* s)
{
    if (s->sides.top != NULL && s->sides.right != NULL && s->sides.bottom != NULL && s->sides.left != NULL)
    {
        char imp = s->sides.top->imp + s->sides.right->imp + s->sides.bottom->imp + s->sides.left->imp;
        return (imp == 0 || imp == 4 || imp == s->imp);
    }
    return 0;
}

void initShort(struct shortHand* sh, char* p, char imp)
{
    sh->name = extractMain(p);
    sh->imp = imp = imp ? 4 : 0;
}

struct shortSide* initSide()
{
    struct shortSide* side = malloc(sizeof(struct shortSide));
    memset(side, 0, sizeof(struct shortSide));
    return side;
}

void setSide(struct shortSide* side, char* s, struct astnode**t, size_t imp)
{
    side->imp = imp;

    if(side->s != NULL)
    {
        free(side->s);
    }
    if(side->t != NULL)
    {
        free(side->t);
    }

    side->s = s;

    side->t = t;
}



struct shortSide* copySide(struct shortSide* side)
{
    struct shortSide* newSide = initSide();
    if(side != NULL)
    {
        newSide->imp = side->imp;
        if (side->s != NULL)
        {
            newSide->s = copyValue(side->s);
        }
        if (side->t != NULL)
        {
            newSide->t = copyList(side->t);
        }
    }
    return newSide;
}

char addToShort(struct shortHand* sh, char* name, char* sValue, struct astnode** tValue, size_t imp)
{
    int alen=0;
    struct shortSide* a[] = {NULL, NULL, NULL, NULL, NULL};

    imp = imp ? 1 : 0;

    char wasUnary = 0;
    char* f = strchr(name, '-');

    if (f != NULL )
    {
        char* sideStr = copyValue(++f);
        struct shortSide* side = NULL;
        char newSide = 0;

        if(casecmp("top", sideStr) == 0)
        {
            if(sh->sides.top == NULL)
            {
                sh->sides.top = initSide();
                newSide = 1;
            }
            side = sh->sides.top;
        }
        else if(casecmp("left", sideStr) == 0)
        {
            if(sh->sides.left == NULL)
            {
                sh->sides.left = initSide();
                newSide = 1;
            }
            side = sh->sides.left;
        }
        else if(casecmp("bottom", sideStr) == 0)
        {
            if(sh->sides.bottom == NULL)
            {
                sh->sides.bottom = initSide();
                newSide = 1;
            }
            side = sh->sides.bottom;
        }
        else if(casecmp("right", sideStr) == 0)
        {
            if(sh->sides.right == NULL)
            {
                sh->sides.right = initSide();
                newSide = 1;
            }
            side = sh->sides.right;
        }

        free(sideStr);

        if (side != NULL)
        {
            if (newSide || (imp && !side->imp))
            {
                side->imp = imp;

                if(side->s != NULL)
                {
                    free(side->s);
                }
                if(side->t != NULL)
                {
                    deleteASTList(side->t);
                }

                if (imp)
                {
                    size_t slen = strlen(sValue);
                    if (slen > 10)
                    {
                        slen -= 10;
                    }
                    side->s = malloc(sizeof(char)*(slen+1));
                    memcpy(side->s, sValue, slen);
                    side->s[slen] = '\0';
                }
                else
                {
                    side->s = copyValue(sValue);
                }
                side->t = pushASTNode(NULL, copyTree(tValue[0]));
                if (tValue[0]->type == ACCSSNODETYPE_UNARY)
                {
                    side->t = pushASTNode(side->t, copyTree(tValue[1]));
                }
            }

            return 1;
        }
    }
    else if (casecmp(name, sh->name) == 0)
    {
		size_t i = 0, ilen = listLength(tValue);
        for(; i < ilen; i++)
        {
            struct astnode* x = tValue[i];
            int last = alen-1;

            switch(x->type)
            {
                case ACCSSNODETYPE_UNARY:
                {
                    a[alen] = initSide();
                    setSide(a[alen], copyValue(x->content), pushASTNode(NULL, copyTree(x)), imp);
                    wasUnary = 1;
                    alen++;
                    break;
                }
                case ACCSSNODETYPE_NUMBER:
                case ACCSSNODETYPE_IDENT:
                {
                    if (wasUnary)
                    {
                        a[last]->t = pushASTNode(a[last]->t, copyTree(x));
                        char* prev = a[last]->s;
                        a[last]->s = concat("%s%s", 2, a[last]->s, x->content);
                        free(prev);
                    }
                    else
                    {
                        a[alen] = initSide();
                        setSide(a[alen], copyValue(x->content), pushASTNode(NULL, copyTree(x)), imp);
                        alen++;
                    }
                    wasUnary = 0;
                    break;
                }
                case ACCSSNODETYPE_PERCENTAGE:
                {
                    if (wasUnary)
                    {
                        a[last]->t = pushASTNode(a[last]->t, copyTree(x));
                        char* prev = a[last]->s;
                        a[last]->s = concat("%s%s", 3, a[last]->s, x->children[0]->content, "%");
                        free(prev);
                    }
                    else
                    {
                        a[alen] = initSide();
                        setSide(a[alen], concat("%s%s", 2, x->children[0]->content, "%"), pushASTNode(NULL, copyTree(x)), imp);
                        alen++;
                    }
                    wasUnary = 0;
                    break;
                }
                case ACCSSNODETYPE_DIMENSION:
                {
                    if (wasUnary)
                    {
                        a[last]->t = pushASTNode(a[last]->t, copyTree(x));
                        char* prev = a[last]->s;
                        a[last]->s = concat("%s%s", 3, a[last]->s, x->children[0]->content, x->children[1]->content);
                        free(prev);
                    }
                    else
                    {
                        a[alen] = initSide();
                        setSide(a[alen], concat("%s%s", 2, x->children[0]->content, x->children[1]->content), pushASTNode(NULL, copyTree(x)), imp);
                        alen++;
                    }
                    wasUnary = 0;
                    break;
                }
                case ACCSSNODETYPE_S:
                case ACCSSNODETYPE_COMMENT:
                case ACCSSNODETYPE_IMPORTANT:
                    break;
                default:
                    return 0;
            }

            if (alen == 5)
            {
                break;
            }
        }

        if (alen > 4)
        {
			int k = 0;
            for(; k<=alen; k++)
            {
                freeSide(a[alen]);
            }
            return 0;
        }

        if (a[1] == NULL)
        {
            a[1] = copySide(a[0]);
        }
        if (a[2] == NULL)
        {
            a[2] = copySide(a[0]);
        }
        if (a[3] == NULL)
        {
            a[3] = copySide(a[1]);
        }

        if(sh->sides.top == NULL)
        {
            sh->sides.top = a[0];
        }
        else
        {
            freeSide(a[0]);
        }
        if(sh->sides.right == NULL)
        {
            sh->sides.right = a[1];
        }
        else
        {
            freeSide(a[1]);
        }
        if(sh->sides.bottom == NULL)
        {
            sh->sides.bottom = a[2];
        }
        else
        {
            freeSide(a[2]);
        }
        if(sh->sides.left == NULL)
        {
            sh->sides.left = a[3];
        }
        else
        {
            freeSide(a[3]);
        }

        return 1;
    }
    return 0;
}


char previousNodeType(struct astnode* container, size_t i)
{
    if(i == 0 && container->type == ACCSSNODETYPE_BRACES)
    {
        return container->type;
    }
    if (i > 0)
    {
            return container->children[i-1]->type;
    }
	return 0;
}

char nextNodeType(struct astnode* container, size_t i)
{
	return i == listLength(container->children) - 1 ? 0 : container->children[i+1]->type;
}

char _cleanComment(char r)
{
    switch(r)
    {
        case ACCSSNODETYPE_S:
        case ACCSSNODETYPE_OPERATOR:
        case ACCSSNODETYPE_ATTRSELECTOR:
        case ACCSSNODETYPE_BLOCK:
        case ACCSSNODETYPE_DECLDELIM:
        case ACCSSNODETYPE_RULESET:
        case ACCSSNODETYPE_DECLARATION:
        case ACCSSNODETYPE_ATRULEB:
        case ACCSSNODETYPE_ATRULES:
        case ACCSSNODETYPE_ATRULER:
        case ACCSSNODETYPE_IMPORTANT:
        case ACCSSNODETYPE_NTH:
        case ACCSSNODETYPE_COMBINATOR:
    	{
            return 1;
        }
    }
    return 0;
};

struct astnode* cleanComment(struct compdeps* deps, struct astnode* node, char rule, struct astnode* container, size_t index, const char* path)
{

    char pr = previousNodeType(container, index);

    char nr = nextNodeType(container, index);

    if (nr != 0 && pr != 0)
    {
        if (_cleanComment(nr) || _cleanComment(pr))
        {
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
    return node;
}
struct astnode* cleanCharset(struct compdeps* deps, struct astnode* node, char rule, struct astnode* container, size_t index, const char* path)
{

    if(node->children != NULL)
    {
        if(node->children[0]->children != NULL)
        {
    		if (strcmp(node->children[0]->children[0]->content, "charset") == 0)
    		{
				size_t i = index;
            	for(; i > 1; i--)
     	   		{
     	       		if (container->children[i]->type != ACCSSNODETYPE_S && container->children[i]->type != ACCSSNODETYPE_COMMENT)
  		      		{
                    	return NULL;
    	            }
    			}
        	}
        }
    }
    return node;
}

struct astnode* cleanImport(struct compdeps* deps, struct astnode* node, char rule, struct astnode* container, size_t index, const char* path)
{
    char  x;
    if(index > 0)
    {
		size_t i = index - 1;
		for(; i > 1; i--)
		{
			x = container->children[i]->type;
			if (x != ACCSSNODETYPE_S && x != ACCSSNODETYPE_COMMENT)
			{
				if (x == ACCSSNODETYPE_ATRULES)
				{
					char* content = container->children[i]->children[0]->children[0]->content;
					if (casecmp(content, "import") != 0 && casecmp(content, "charset"))
					{
						return NULL;
					}
				} else
				{
					return NULL;
				}
			}
		}
    }
    return node;
}

char previousIsURI(struct astnode* container, size_t i)
{
    return (i != 0 && i != listLength(container->children) - 1 && container->children[i - 1]->type == ACCSSNODETYPE_URI);
};

char braceFollowedByIdent (struct astnode* container, char pr, char nr)
{
    return container->type == ACCSSNODETYPE_ATRULERQ && pr == ACCSSNODETYPE_BRACES && nr == ACCSSNODETYPE_IDENT;
};

char _cleanWhitespace(char r, char left)
{
    switch(r)
    {
        case ACCSSNODETYPE_S:
        case ACCSSNODETYPE_OPERATOR:
        case ACCSSNODETYPE_ATTRSELECTOR:
        case ACCSSNODETYPE_BLOCK:
        case ACCSSNODETYPE_DECLDELIM:
        case ACCSSNODETYPE_RULESET:
        case ACCSSNODETYPE_DECLARATION:
        case ACCSSNODETYPE_ATRULEB:
        case ACCSSNODETYPE_ATRULES:
        case ACCSSNODETYPE_ATRULER:
        case ACCSSNODETYPE_IMPORTANT:
        case ACCSSNODETYPE_NTH:
        case ACCSSNODETYPE_COMBINATOR:
        {
            return 1;
        }
    }
    if (left)
    {
        switch(r) {
            case ACCSSNODETYPE_FUNCTION:
            case ACCSSNODETYPE_BRACES:
            case ACCSSNODETYPE_URI:
            {
                return 1;
            }
        }
    }
    return 0;
};

struct astnode* cleanWhitespace(struct compdeps* deps, struct astnode* node, char rule, struct astnode* container, size_t index, const char* path)
{

    char pr = previousNodeType(container, index);

    char nr = nextNodeType(container, index);

    if (nr == ACCSSNODETYPE_UNKNOWN)
    {
        if(node->content != NULL)
        {
            free(node->content);
        }
        node->content = malloc(sizeof(char)*2);
        if (node->content == NULL)
        {
            memoryFailure();
            exit(EXIT_FAILURE);
        }
        node->content[0] = '\n';
        node->content[1] = '\0';
    }
    else
    {
        if (!(container->type == ACCSSNODETYPE_ATRULERQ && pr == 0) && !previousIsURI(container, index) && !braceFollowedByIdent(container, pr, nr))
        {
            if (nr != 0 && pr != 0)
            {
                if (_cleanWhitespace(nr, 0) || _cleanWhitespace(pr, 1))
                {
                    return NULL;
                }
            }
            else
            {
                return NULL;
            }
        }

        if(node->content != NULL)
        {
            free(node->content);
        }
        node->content = malloc(sizeof(char)*2);
        if (node->content == NULL)
        {
            memoryFailure();
            exit(EXIT_FAILURE);
        }
        node->content[0] = ' ';
        node->content[1] = '\0';
    }

    return node;
};

struct astnode* compressNumber(struct compdeps* deps, struct astnode* node, char rule, struct astnode* container, size_t index, const char* path)
{
    struct astnode* prev = index > 0 ? container->children[index - 1] : NULL;
    struct astnode* pprev = index > 1 ? container->children[index - 2] : NULL;

    //TODO: Dectect Unicode Range on AST creation and introduce new type ACCSSNODETYPE_UNICODERANGE
    //Exclude Unicode Range
    if  ((prev && prev->type == ACCSSNODETYPE_UNARY)
        && (pprev && pprev->type == ACCSSNODETYPE_IDENT && strcmp(pprev->content, "U") == 0))
    {
        return node;
    }
    else if(container->type == ACCSSNODETYPE_DIMENSION && container->parent->type == ACCSSNODETYPE_VALUE )
    {
        struct astnode* first = (container->parent->children[0]->type == ACCSSNODETYPE_S) ? container->parent->children[1] : container->parent->children[0];
        if (first->type == ACCSSNODETYPE_IDENT && strcmp(first->content, "U") == 0) {
            return node;
        }
    }

    char* x = node->content;

    size_t len = strlen(x);

    if (x[0] == '0')
    {
        size_t p = 0;
        while (x[p] != '\0' && x[p] == '0')
        {
            p++;
        }
        memcpy(x, &x[p], len-p);
        x[len-p] = '\0';
        char* xtmp = realloc(x, sizeof(char)*(strlen(x)+1));
        if (xtmp == NULL)
        {
            memoryFailure();
            exit(EXIT_FAILURE);
        }
        x = xtmp;
    }

    size_t p = 0;
    len = strlen(x);
    while (x[p] != '\0' && x[p] != '.')
    {
        p++;
    }
    if(p < len)
    {
        size_t k = p+1;
        while (x[k] != '\0' && x[k] == '0')
        {
            k++;
        }
        if(k == len)
        {
            x[p] = '\0';
            len = strlen(x);
            char* xtmp = realloc(x, sizeof(char)*(len+1));
            if (xtmp == NULL)
            {
                memoryFailure();
                exit(EXIT_FAILURE);
            }
            x = xtmp;
        }
        else
        {
            if(x[len-1] == '0')
            {
                while(x[len-1] == '0')
                {
                    len--;
                }
                char* xtmp = realloc(x, sizeof(char)*(len+1));
                if (xtmp == NULL)
                {
                    memoryFailure();
                    exit(EXIT_FAILURE);
                }
                x = xtmp;
                x[len] = '\0';
            }
        }
    }

    if(len == 0 || (len == 1 && x[0] == '.'))
    {
        char* xtmp = realloc(x, sizeof(char)*2);
        if (xtmp == NULL)
        {
            memoryFailure();
            exit(EXIT_FAILURE);
        }
        x = xtmp;

        x[0]='0';
        x[1]='\0';
    }

    node->content = x;

    return node;
}

struct astnode* cleanUnary(struct compdeps* deps, struct astnode* node, char rule, struct astnode* container, size_t index, const char* path)
{
    struct astnode* next = listLength(container->children) > index+1 ? container->children[index + 1] : NULL;
    struct astnode* prev = index > 0 ? container->children[index - 1] : NULL;

    //TODO: Dectect Unicode Range on AST creation and introduce new type ACCSSNODETYPE_UNICODERANGE
    //Exclude Unicode Range
    if (prev && prev->type == ACCSSNODETYPE_IDENT && strcmp(prev->content, "U") == 0)
    {
        return node;
    }

    if (next && next->type == ACCSSNODETYPE_NUMBER && strcmp(next->content, "0") == 0)
    {
        return NULL;
    }
    return node;
}



struct astnode* compressIdentColor(struct compdeps* deps, struct astnode* node, char rule, struct astnode* container, size_t index, const char* path)
{

    if(container->type == ACCSSNODETYPE_VALUE || container->type == ACCSSNODETYPE_FUNCTIONBODY)
    {
        char rep = 0;
    	if(casecmp(node->content, "yellow") == 0)
        {
            node->content = resizeValue(node->content, "ff0");
            rep++;
        }
        else if(casecmp(node->content, "fuchsia") == 0)
        {
            node->content = resizeValue(node->content, "f0f");
            rep++;
        }
        else if(casecmp(node->content, "white") == 0)
        {
            node->content = resizeValue(node->content, "fff");
            rep++;
        }
        else if(casecmp(node->content, "black") == 0)
        {
            node->content = resizeValue(node->content, "000");
            rep++;
        }
        else if(casecmp(node->content, "blue") == 0)
        {
            node->content = resizeValue(node->content, "00f");
            rep++;
        }
        else if(casecmp(node->content, "aqua") == 0)
        {
            node->content = resizeValue(node->content, "0ff");
            rep++;
        }
        if(rep)
        {
            node->type = ACCSSNODETYPE_VHASH;
        }
    }

    return node;
};

char* _compressHashColor(char* value, char* type)
{
    size_t len = strlen(value);

    if (len == 6 &&
        value[0] == value[1] &&
        value[2] == value[3] &&
        value[4] == value[5])
    {
        char* newValue = malloc(sizeof(char)*4);
        if (newValue == NULL)
        {
            memoryFailure();
            exit(EXIT_FAILURE);
        }
        newValue[0] = value[0];
        newValue[1] = value[2];
        newValue[2] = value[4];
        newValue[3] = '\0';

        free(value);
        value = newValue;
    }

    char rep = 0;
    if(casecmp(value, "f00") == 0)
    {
        value = resizeValue(value, "red");
        rep++;
    }
    else if(casecmp(value, "c0c0c0") == 0)
    {
        value = resizeValue(value, "silver");
        rep++;
    }
    else if(casecmp(value, "808080") == 0)
    {
        value = resizeValue(value, "gray");
        rep++;
    }
    else if(casecmp(value, "800000") == 0)
    {
        value = resizeValue(value, "maroon");
        rep++;
    }
    else if(casecmp(value, "800080") == 0)
    {
        value = resizeValue(value, "purple");
        rep++;
    }
    else if(casecmp(value, "008000") == 0)
    {
        value = resizeValue(value, "green");
        rep++;
    }
    else if(casecmp(value, "808000") == 0)
    {
        value = resizeValue(value, "olive");
        rep++;
    }
    else if(casecmp(value, "000080") == 0)
    {
        value = resizeValue(value, "navy");
        rep++;
    }
    else if(casecmp(value, "008080") == 0)
    {
        value = resizeValue(value, "teal");
        rep++;
    }

    if(rep)
    {
        *type = ACCSSNODETYPE_IDENT;
    }
    else
    {
        *type = ACCSSNODETYPE_VHASH;
    }

    return value;
};

struct astnode* compressHashColor(struct compdeps* deps, struct astnode* node, char rule, struct astnode* container, size_t index, const char* path)
{
    node->content = lowerCase(node->content);
    node->content = _compressHashColor(node->content, &node->type);
    return node;
};

struct astnode* compressFunctionColor(struct compdeps* deps, struct astnode* node, char rule, struct astnode* container, size_t index, const char* path)
{
	char* v[3] = { NULL, NULL, NULL };

    if (strcmp(node->children[0]->content, "rgb") == 0)
    {
        struct astnode** body = node->children[1]->children;
        size_t len = listLength(body);

        char vvalid = 1;
		int i = 0, k = 0;
        for(; i < len; i++)
        {

            if (body[i]->type == ACCSSNODETYPE_NUMBER )
            {
                if (k < 3)
                {
                    v[k] = body[i]->content;
                }
                else
                {
                    vvalid = 0;
                    break;
                }
                k++;
            }
            else if (body[i]->type != ACCSSNODETYPE_OPERATOR)
            {
                vvalid = 0;
                break;
            }
        }

        if (vvalid)
        {
            char* h = malloc(sizeof(char)*7);
            if (h == NULL)
            {
                memoryFailure();
                exit(EXIT_FAILURE);
            }
            int vi1 = atoi(v[0]);
            int vi2 = atoi(v[1]);
            int vi3 = atoi(v[2]);

            if(vi1 < 256 && vi2 < 256 && vi3 < 256)
            {
            	sprintf(h, "%02x%02x%02x", vi1, vi2, vi3);

                struct astnode* vhash = createASTNodeWithType(ACCSSNODETYPE_VHASH);

                vhash->content = h;

                vhash->content = _compressHashColor(vhash->content, &vhash->type);

                deleteASTTree(node);

                return vhash;
            }
            else
            {
                free(h);
            }
        }
    }
    return node;
}

struct astnode* compressColor(struct compdeps* deps, struct astnode* node, char rule, struct astnode* container, size_t index, const char* path)
{
    switch(rule) {
        case ACCSSNODETYPE_VHASH:
            return compressHashColor(deps, node, rule, container, index, path);
        case ACCSSNODETYPE_FUNCTION:
            return compressFunctionColor(deps, node, rule, container, index, path);
        case ACCSSNODETYPE_IDENT:
            return compressIdentColor(deps, node, rule, container, index, path);
    }
    return node;
}

struct astnode* findDeclaration(struct astnode* node)
{
    while(node != NULL)
    {
        if(node->type == ACCSSNODETYPE_DECLARATION)
        {
            break;
        }
        node = node->parent;
    }
    return node;
}

struct astnode* compressDimension(struct compdeps* deps, struct astnode* node, char rule, struct astnode* container, size_t index, const char* path)
{
    struct astnode* declaration;

    if (strcmp(node->children[0]->content, "0") == 0)
    {
        if ((strcmp(node->children[1]->content, "s") == 0 || strcmp(node->children[1]->content, "ms") == 0) && (declaration = findDeclaration(node)))
        {
            char* declName = declaration->children[0]->children[0]->content;

            if  ( strcmp(declName, "-moz-transition") == 0 || strcmp(declName, "transition") == 0 )
            {
                return node;
            }
            if  ( strcmp(declName, "-moz-animation") == 0 || strcmp(declName, "animation") == 0 )
            {
                return node;
            }
        }
        deleteASTList(node->children);
        node->children = NULL;
        node->type = ACCSSNODETYPE_NUMBER;
        if(node->content != NULL)
        {
            free(node->content);
        }
        node->content = copyValue("0");
    }
    return node;
}

struct astnode* compressString(struct compdeps* deps, struct astnode* node, char rule, struct astnode* container, size_t index, const char* path)
{
    char* s = node->content;
    size_t slen = strlen(s);
    char* r = malloc(sizeof(char)*(slen+1));
    if (r == NULL)
    {
        memoryFailure();
        exit(EXIT_FAILURE);
    }
    size_t k=0, p = 0;
    for( ; p < slen; p++)
    {
        char c = s[p];
        if (c == '\\' && s[p + 1] == '\n')
        {
            p++;
        }
        else
        {
            r[k]=c;
            k++;
        }
    }
    r[k]='\0';

    free(s);
    node->content = r;

    return node;
}

struct astnode* compressFontWeight(struct compdeps* deps, struct astnode* node, char rule, struct astnode* container, size_t index, const char* path)
{
    struct astnode* decl = node->children[0];
    struct astnode* value = node->children[1];

    if(listLength(value->children) && value->children[0]->type == ACCSSNODETYPE_IDENT && listLength(decl->children) && decl->children[0]->content != NULL && strcmp(decl->children[0]->content, "font-weight") == 0  )

    {
        if(strcmp(value->children[0]->content, "normal") == 0)
        {
            value->children[0]->content = resizeValue(value->children[0]->content, "400");
            value->children[0]->type = ACCSSNODETYPE_NUMBER;
        }
        else if(strcmp(value->children[0]->content, "bold") == 0)
        {
            value->children[0]->content = resizeValue(value->children[0]->content, "700");
            value->children[0]->type = ACCSSNODETYPE_NUMBER;
        }
    }
    return node;
}

struct astnode* compressFont(struct compdeps* deps, struct astnode* node, char rule, struct astnode* container, size_t index, const char* path)
{
    struct astnode* decl = node->children[0];
    struct astnode* value = node->children[1];

    if (strcmp(decl->children[0]->content, "font") == 0 && listLength(value->children))
    {
        size_t vlen = listLength(value->children);
		size_t i = vlen - 1;

        for(; i+1 > 0; i--)
        {
            struct astnode* x = value->children[i];
            if (x->type == ACCSSNODETYPE_IDENT )
            {
                if (strcmp(x->content, "bold") == 0)
                {
                    x->type = ACCSSNODETYPE_NUMBER;
                    x->content = resizeValue(x->content, "700");
                }
                else if (strcmp(x->content, "normal") == 0)
                {
                    if(i > 0)
                    {
                    	struct astnode* t = value->children[i - 1];
                    	if (t->type == ACCSSNODETYPE_OPERATOR && strcmp(t->content, "/") == 0)
                    	{
                        	value->children = removeItem(value->children, --i, 2);
                    	}
                    	else
                    	{
                        	value->children = removeItem(value->children, i, 1);
                    	}
               	     	if(i  > 1 && listLength(value->children) > 0)
               	     	{
                	    	struct astnode* t2 = value->children[i - 1];

                  	 	 	if (t2->type == ACCSSNODETYPE_S)
                  	 	 	{
                    		    value->children = removeItem(value->children, --i, 1);
                    		}
                    	}
                    }
                    else
                    {
                        if(listLength(value->children) > 1)
                        {
                            value->children = removeItem(value->children, 0, 1);
                        }
                    }
                }
                else if (strcmp(x->content, "medium") == 0 && i+1 < listLength(value->children) && strcmp(value->children[i +1]->content, "/") != 0)
                {
                    value->children = removeItem(value->children, i, 1);

                    if(i - 1 > 0 && listLength(value->children) > 0)
                    {
                    	struct astnode* t2 = value->children[i - 1];
                   	 	if (t2->type == ACCSSNODETYPE_S)
                    	{
                    	    value->children = removeItem(value->children, --i, 1);
                    	}
                    }
                }
            }
        }

        if(listLength(value->children) > 0 && value->children[0]->type == ACCSSNODETYPE_S)
        {
            value->children = removeItem(value->children, 0, 1);
        }
        if (listLength(value->children) == 0)
        {
            struct astnode* node = createASTNodeWithType(ACCSSNODETYPE_IDENT);
            node->content = copyValue("normal");

            value->children = pushASTNode(value->children, node);
        }

    }
    return node;
}

struct astnode* compressBackground(struct compdeps* deps, struct astnode* node, char rule, struct astnode* container, size_t index, const char* path)
{
    //TODO: Compression Ruleset insufficient!
    struct astnode* decl = node->children[0];
    struct astnode* value = node->children[1];

    size_t vlen = listLength(value->children);

    if (strcmp(decl->children[0]->content, "background") == 0 && vlen)
    {
        struct astnode* imp = value->children[vlen -1];
        if (imp->type == ACCSSNODETYPE_IMPORTANT)
        {
            return node;
        }

		size_t i = vlen - 1;
        for(; i+1 > 0; i--)
        {
            struct astnode* x = value->children[i];
            if (x->type == ACCSSNODETYPE_IDENT)
            {

                if (strcmp(x->content, "transparent") == 0 /* || strcmp(x->content, "none") == 0 */ || strcmp(x->content, "repeat") == 0 ||strcmp(x->content, "scroll") == 0 )
                {
                    value->children = removeItem(value->children, i, 1);
                    if(i > 1 && listLength(value->children) > 0)
                    {
                    	struct astnode* t2 = value->children[i - 1];
                   	 	if (t2->type == ACCSSNODETYPE_S)
                    	{
                    	    value->children = removeItem(value->children, --i, 1);
                    	}
                    }
                }
            }
        }
        if(listLength(value->children) > 0 && value->children[0]->type == ACCSSNODETYPE_S)
        {
            value->children = removeItem(value->children, 0, 1);
        }
        if (listLength(value->children) == 0)
        {
            //TODO is this really the desired fallback?
            struct astnode* node = createASTNodeWithType(ACCSSNODETYPE_IDENT);
            node->content = copyValue("0 0");

            value->children = pushASTNode(value->children, node);
        }
    }
    return node;
}

struct astnode* cleanEmpty(struct compdeps* deps, struct astnode* node, char rule, struct astnode* container, size_t index, const char* path)
{
    if (node->children == NULL) {
        return NULL;
    }
    switch(rule)
    {
        case ACCSSNODETYPE_RULESET:
    	{
            if(listLength(node->children) < 2)
            {
                return NULL;
            }
            if (!listLength(node->children[1]->children))
            {
                return NULL;
            }
        }
            break;

        case ACCSSNODETYPE_ATRULEB:
        case ACCSSNODETYPE_ATRULER:
        {
            if (!listLength(node->children[listLength(node->children) - 1]->children))
        	{
                return NULL;
            }
        }
            break;
    }
    return node;
}

struct astnode* reAddSpaces(struct compdeps* deps, struct astnode* node, char rule, struct astnode* container, size_t index, const char* path)
{
    size_t llen = listLength(node->children);

    if(llen > 1)
    {
        for(llen--; llen > 0; llen--)
        {
            if((node->children[llen]->type == ACCSSNODETYPE_VHASH
                || node->children[llen]->type == ACCSSNODETYPE_PERCENTAGE
                || node->children[llen]->type == ACCSSNODETYPE_NUMBER
                || node->children[llen]->type == ACCSSNODETYPE_IDENT)
            && (node->children[llen-1]->type == ACCSSNODETYPE_VHASH
                || node->children[llen-1]->type == ACCSSNODETYPE_PERCENTAGE
                || node->children[llen-1]->type == ACCSSNODETYPE_NUMBER))
            {
                struct astnode* s = createASTNodeWithType(ACCSSNODETYPE_S);
                s->content = copyValue(" ");
                node->children = insertItem(node->children, llen, s);
            }
        }
    }
    return node;
}

struct astnode* destroyDelims(struct compdeps* deps, struct astnode* node, char rule, struct astnode* container, size_t index, const char* path)
{
    return NULL;
}


struct astnode* preTranslate(struct compdeps* deps, struct astnode* node, char rule, struct astnode* container, size_t index, const char* path)
{
    node->s = translate(node);
    return node;
}

char notFPClasses(const char* content)
{
    if(strcmp(content, "link") == 0)
    {
        return 1;
    }
    else if(strcmp(content, "visited") == 0)
    {
        return 1;
    }
    else if(strcmp(content, "hover") == 0)
    {
        return 1;
    }
    else if(strcmp(content, "active") == 0)
    {
        return 1;
    }
    else if(strcmp(content, "first-letter") == 0)
    {
        return 1;
    }
    else if(strcmp(content, "first-line") == 0)
    {
        return 1;
    }
    return 0;
}

char notFPElements(const char* content)
{
    if(strcmp(content, "first-letter") == 0)
    {
        return 1;
    }
    else if(strcmp(content, "first-line") == 0)
    {
        return 1;
    }
    return 0;
}

char freezeNeeded (struct astnode* selector)
{
    struct astnode* ss;
	size_t i = 0, ilen=listLength(selector->children);

    for(; i < ilen; i++)
    {
        ss = selector->children[i];
		size_t j = 0, jlen = listLength(ss->children);
        for(; j < jlen; j++)
        {
            switch (ss->children[j]->type)
            {
                case ACCSSNODETYPE_PSEUDOC:
                {
                    char* content = ss->children[j]->children[0]->type == ACCSSNODETYPE_IDENT ? ss->children[j]->children[0]->content : ss->children[j]->children[0]->children[0]->content;
                    if (!(notFPClasses(content)))
                    {
                        return 1;
                    }
                }
                    break;
                case ACCSSNODETYPE_PSEUDOE:
            	{
                    char* content = ss->children[j]->children[0]->type == ACCSSNODETYPE_IDENT ? ss->children[j]->children[0]->content : ss->children[j]->children[0]->children[0]->content;
                    if (!(notFPElements(content)))
                    {
                    	return 1;
                    }
                }
                    break;
                case ACCSSNODETYPE_NTHSELECTOR:
                {
                    return 1;
                }
                    break;
            }
        }
    }
    return 0;
}

char* joinAndFreeCharArray(char** array, size_t num, const char* delim, char freeValue)
{
    char* s = malloc(sizeof(char));
    if (s == NULL)
    {
        memoryFailure();
        exit(EXIT_FAILURE);
    }

    s[0] = '\0';

    size_t dlen = strlen(delim), slen = 0, i = 0;

    for( ; i < num; i++)
    {
        char* tmp = array[i];

        size_t tlen = strlen(tmp);
        size_t newlen = 0;

        if (i < num-1)
        {
            newlen = slen+tlen+dlen;
        }
        else
        {
            newlen = slen+tlen;
        }

        char* stmp = realloc(s, sizeof(char)*(newlen+1));
        if(stmp == NULL)
        {
            memoryFailure();
            exit(EXIT_FAILURE);
        }
        s = stmp;
        memcpy(&s[slen], tmp, tlen);

        if (i < num-1)
        {
            memcpy(&s[slen+tlen], delim, dlen);
        }

        slen = newlen;
        s[slen] = '\0';

        if (freeValue)
        {
            free(tmp);
        }
    }

    free(array);

    return s;
}

int comp(const void* a, const void* b)
{
    return strcmp(*(char**)a, *(char**)b);
}

char* selectorSignature(struct astnode* selector)
{
 	size_t ilen = ilen=listLength(selector->children);
    char** a = malloc(sizeof(char*)*ilen);
    if (a == NULL)
    {
        memoryFailure();
        exit(EXIT_FAILURE);
    }
    memset(a, 0, sizeof(char)*ilen);
	size_t i = 0;

    for(; i < ilen; i++)
    {
        a[i] = translate(selector->children[i]);
    }

    qsort(a, ilen, sizeof(char*), &comp);

    return joinAndFreeCharArray(a, ilen, ",", 1);
}

char containsPseudo(struct astnode* sselector)
{
	size_t j = 0, jlen=listLength(sselector->children);
    for(; j < jlen; j++)
    {
        switch (sselector->children[j]->type)
        {
            case ACCSSNODETYPE_PSEUDOC:
            case ACCSSNODETYPE_PSEUDOE:
            case ACCSSNODETYPE_NTHSELECTOR:
            {
                char* content = sselector->children[j]->children[0]->type == ACCSSNODETYPE_IDENT ? sselector->children[j]->children[0]->content : sselector->children[j]->children[0]->children[0]->content;
                if (!notFPClasses(content))
                {
                    return 1;
                }
            }
                break;
        }
    }
    return 0;
}

char* composePseudoID(struct astnode* selector)
{
 	size_t ilen = ilen=listLength(selector->children);
    char** a = malloc(sizeof(char*)*ilen);
    if (a == NULL)
    {
        memoryFailure();
        exit(EXIT_FAILURE);
    }

    size_t k = 0, i = 0;

    for( ; i < ilen; i++)
    {
        struct astnode* ss = selector->children[i];
        if (containsPseudo(ss))
        {
            a[k++] = ss->s;
        }
    }

    if(k > 0)
    {
        if(k != ilen)
        {
    		char** atmp = realloc(a, k*sizeof(char*));
            if(atmp == NULL)
            {
                memoryFailure();
                exit(EXIT_FAILURE);
            }
            a = atmp;
    	}

        qsort(a, k, sizeof(char*), &comp);
    	return joinAndFreeCharArray(a, k, ",", 0);
    }
    else
    {
        free(a);
        char* b = malloc(sizeof(char));
        if(b == NULL)
        {
            memoryFailure();
            exit(EXIT_FAILURE);
        }
        *b = '\0';
        return b;
    }
}

char* pseudoSelectorSignature(struct astnode* selector, char dontAppendExcludeMark)
{
    struct astnode* ss = NULL;
    char wasExclude = 0;

    size_t alen = 10, k=0;
    char** a = malloc(sizeof(char*)*alen);
    if (a == NULL)
    {
        memoryFailure();
        exit(EXIT_FAILURE);
    }

    size_t ilen = listLength(selector->children), i = 0;
    for( ; i < ilen; i++)
    {
        ss = selector->children[i];
		size_t j = 0, jlen = listLength(ss->children);

        for( ; j < jlen; j++)
        {
            switch (ss->children[j]->type)
            {
                case ACCSSNODETYPE_PSEUDOC:
                case ACCSSNODETYPE_PSEUDOE:
                case ACCSSNODETYPE_NTHSELECTOR:
                {
                    char* content = ss->children[j]->children[0]->type == ACCSSNODETYPE_IDENT ? ss->children[j]->children[0]->content : ss->children[j]->children[0]->children[0]->content;
                    if (!(strcmp(content, "after") == 0 || strcmp(content, "before") == 0))
                    {
                        a[k++] = content;
                    }
                    else
                    {
                        wasExclude = 1;
                    }
                    if(k >= alen)
                    {
                        alen+=10;
                        char** b = realloc(a, sizeof(char*)*alen);
                        if(b== NULL)
                        {
                            memoryFailure();
                            exit(EXIT_FAILURE);
                        }
                        a = b;
                    }
                }
                    break;
            }
        }
    }

    if(k != 0)
    {
    	if(alen != k)
    	{
    		char** b = realloc(a, k*sizeof(char*));
            if (b == NULL)
            {
                memoryFailure();
                exit(EXIT_FAILURE);
            }
            a = b;
    	}

    	qsort(a, k, sizeof(char*), &comp);
    	char* s = joinAndFreeCharArray(a, k, ",", 0);

    	if (!dontAppendExcludeMark && wasExclude)
    	{
        	char buffer[2] = {'1', '\0'};
        	char* stmp = concat("%s%s", 2, s, buffer);
        	free(s);
        	s = stmp;
    	}
    	return s;
    }
    else
    {
        free(a);
        return NULL;
    }
}

void markSimplePseudo(struct astnode* selector)
{
    struct astnode*  ss;

    size_t ilen = listLength(selector->children);

    if (ilen > 0 )
    {
    	char** sg = malloc(sizeof(char*)*(ilen+1));
        if (sg == NULL)
        {
            memoryFailure();
            exit(EXIT_FAILURE);
        }
        sg[ilen] = NULL;
		size_t i = 0;

    	for( ; i < ilen; i++)
    	{
            ss = selector->children[i];
            sg[i] = copyValue(ss->s);
        }
		size_t x = 0;
        for( ; x < ilen; x++)
        {
        	ss = selector->children[x];
        	if(ss->info == NULL)
        	{
                ss->info = createASTInfo();
        	}
        	ss->info->pseudo = containsPseudo(ss);
        	ss->info->sg = copyCharList(sg);
    	}
        freeCharList(sg);
    }
}

struct astnode* freezeRulesets(struct compdeps* deps, struct astnode* node, char rule, struct astnode* container, size_t index, const char* path)
{
    struct astinfo* info = createASTInfo();
    node->info = info;

    struct astnode* selector = node->children[0];

    info->freeze = freezeNeeded(selector);
    info->freezeID = selectorSignature(selector);
    info->pseudoID = composePseudoID(selector);
    info->pseudoSignature = pseudoSelectorSignature(selector, 1);

    markSimplePseudo(selector);

    return node;
}

char equalHash(char** h0, char** h1)
{
    char k=1;
    char** h01 = h0;
    char** h11 = h1;
    while(*h01 != NULL)
    {
        char j = 0;
        while(*h11 != NULL)
        {
            if(strcmp(*h01, *h11) == 0)
            {
                j=1;
                break;
            }
            h11++;
        }
        if (!j)
        {
            k=0;
            break;
        }

        h11=h1;
        h01++;
    }
    if(!k)
    {
        return 0;
    }

    k=1;
    h01 = h0;
    h11 = h1;
    while(*h11 != NULL)
    {
        char j = 0;
        while(*h01 != NULL)
        {
            if(strcmp(*h11, *h01) == 0)
            {
                j=1;
                break;
            }
            h01++;
        }
        if (!j)
        {
            k=0;
            break;
        }

        h01=h0;
        h11++;
    }
    if(!k)
    {
        return 0;
    }

    return 1;
};

char** getHash(struct astnode** nodes)
{
    size_t ilen = listLength(nodes);

    if(ilen == 0)
    {
        return NULL;
    }

    char** r = malloc(sizeof(char*)*(ilen+1));
    if(r == NULL)
	{
        memoryFailure();
        exit(EXIT_FAILURE);
    }
    r[ilen] = NULL;
    size_t i = 0;

    for( ; i < ilen; i++)
    {
        r[i] = copyValue(nodes[i]->s);
    }
    return r;
}

char hashInHash(char** h0, char** h1)
{
    char k=1;
    char** h01 = h0;
    char** h11 = h1;
    while(*h01 != NULL)
    {
        char j = 0;
        while(*h11 != NULL)
        {
            if(strcmp(*h01, *h11) == 0)
            {
                j=1;
                break;
            }
            h11++;
        }
        if (!j)
        {
            k=0;
            break;
        }

        h11=h1;
        h01++;
    }
    if(!k)
    {
        return 0;
    }

    return 1;
}



char okToJoinByProperties(struct astnode* r0, struct astnode*  r1)
{
    if (r0->info == NULL)
    {
        r0->info =createASTInfo();
    }
    if (r1->info == NULL)
    {
        r1->info =createASTInfo();
    }

    struct astinfo* i0 = r0->info, *i1 = r1->info;

    if((i0->freezeID == NULL && i1->freezeID == NULL)
    	||
    	(
     		(i0->freezeID != NULL && i1->freezeID != NULL)
         	&&
         	strcmp(i0->freezeID, i1->freezeID)
     	)
    )
    {
        return 1;
    }

    if((i0->pseudoID == NULL && i1->pseudoID == NULL)
       ||
    	(
        	(i0->pseudoID != NULL && i1->pseudoID != NULL)
         	&&
         	strcmp(i0->pseudoID, i1->pseudoID)
		)
    )
    {
        return 1;
    }

    if (i0->freeze && i1->freeze)
    {
        char* ps0 = pseudoSelectorSignature(r0, 0);
        char* ps1 = pseudoSelectorSignature(r1, 0);
        if (ps0 == NULL && ps1 == NULL)
        {
            return 1;
        }
        else if (ps0 == NULL)
        {
            free(ps1);
            return 0;
        }
        else if (ps1 == NULL)
        {
            free(ps0);
            return 0;
        }
        char eq = (strcmp(ps0, ps1) == 0);
        free(ps1);
        free(ps1);
        return eq;
    }

    return !(i0->freeze || i1->freeze);
}


char stringInHash(char** h0, char* s)
{
    char k=0;

    if(h0 == NULL)
    {
        return 0;
    }
    char** h1 = h0;
    while(*h1 != NULL)
    {

        if(strcmp(*h1, s) == 0)
        {
            k=1;
            break;
        }

        h1++;
    }
    if(k)
    {
        return 1;
    }

    return 0;
}

char testDimensions(char options, struct astnode* value)
{
    if( (options & ACCSSOPTION_FUTURE)
       && value->type == ACCSSNODETYPE_FUNCTION
       &&   (  strcmp(value->children[0]->content, "min") == 0
             || strcmp(value->children[0]->content, "max") == 0
             )
       )
    {
        return 1;
    }
    if( (options & ACCSSOPTION_IE9)
       && value->type == ACCSSNODETYPE_FUNCTION
       &&   (  strcmp(value->children[0]->content, "calc") == 0
             )
       )
    {
        return 1;
    }
    if( (options & ACCSSOPTION_IE8)
       && value->type == ACCSSNODETYPE_FUNCTION
       &&   (  strcmp(value->children[0]->content, "hsl") == 0
             ||  strcmp(value->children[0]->content, "hsla") == 0
             || strcmp(value->children[0]->content, "rgba") == 0
             || strcmp(value->children[0]->content, "rgb") == 0
             )
       )
    {
        return 1;
    }

    if( (options & ACCSSOPTION_IE8)
       && value->type == ACCSSNODETYPE_DIMENSION
       &&   (  strcmp(value->children[1]->content, "vw") == 0
             || strcmp(value->children[1]->content, "vh") == 0
             || strcmp(value->children[1]->content, "rem") == 0
             )
       )
    {
        return 1;
    }
    if( (options & ACCSSOPTION_IE9)
       && value->type == ACCSSNODETYPE_DIMENSION
       &&   (  strcmp(value->children[1]->content, "vm") == 0
             || strcmp(value->children[1]->content, "vmin") == 0
             )
       )
    {
        return 1;
    }
    if( (options & ACCSSOPTION_IE10)
       && value->type == ACCSSNODETYPE_DIMENSION
       &&   (  strcmp(value->children[1]->content, "vmin") == 0
             || strcmp(value->children[1]->content, "vmax") == 0
             )
       )
    {
        return 1;
    }
    return 0;
}

char dontRestructure(unsigned char options, struct astnode* decl)
{
    if (decl->type == ACCSSNODETYPE_DECLARATION && listLength(decl->children) > 1 && listLength(decl->children[0]->children) > 0)
    {
        struct astnode* prop = decl->children[0]->children[0];
        struct astnode* value = decl->children[1];

        char* str = prop->content;
        if (str == NULL)
        {
            return 0;
        }
        else if(strcmp(str, "src") == 0)
        {
            return 1;
        }
        else if((options & ACCSSOPTION_IE7) && strcmp(str, "clip") == 0)
        {
            return 1;
        }
        else if((options & ACCSSOPTION_IE8) && strcmp(str, "cursor") == 0)
        {
            return 1;
        }
        else if((options & ACCSSOPTION_IE11) && strcmp(str, "cursor") == 0)
        {
            struct astnode** c = value->children;

            while(*c != NULL)
            {
                if((*c)->type == ACCSSNODETYPE_IDENT && (strcmp((*c)->content, "zoom-in") == 0 || strcmp((*c)->content, "zoom-out") == 0))
                {
                    return 1;
                }
                c++;
            }

        }
        //http://www.w3.org/TR/css3-background/#background-position
        else if((options & ACCSSOPTION_IE8) && strcmp(str, "background-position") == 0 && listLength(value->children) > 2)
        {
            struct astnode** c = value->children;
            char n = 0;
            while(*c != NULL)
            {
                if((*c)->type == ACCSSNODETYPE_PERCENTAGE)
                {
                    return 1;
                }
                if((*c)->type == ACCSSNODETYPE_IDENT || (*c)->type == ACCSSNODETYPE_NUMBER || (*c)->type == ACCSSNODETYPE_DIMENSION)
                {
                    n++;
                }
                c++;
            }
            if(n > 2)
            {
                return 1;
            }
        }
        //http://www.w3.org/TR/css-text-decor-3/#text-decoration
        else if((options & ACCSSOPTION_CHROME) && strcmp(str, "text-decoration") == 0 && listLength(value->children) > 2)
        {
            return 1;
        }
        else if((options & ACCSSOPTION_IE8) && strcmp(str, "background-image") == 0 && listLength(value->children) > 1)
        {
            return 1;
        }
        //http://dev.w3.org/csswg/css-images-3/
        else if((options & ACCSSOPTION_IE9) && (strcmp(str, "background") == 0 || strcmp(str, "background-image") == 0 ) )
        {

            int src = 0;
            struct astnode** c = value->children;
            while(*c != NULL)
            {
                if ((options & ACCSSOPTION_IE8) && (*c)->type == ACCSSNODETYPE_IDENT && strcmp((*c)->content, "none") == 0 )
                {
                    src++;
                }
                if ((options & ACCSSOPTION_IE8) && (*c)->type == ACCSSNODETYPE_URI )
                {
                    src++;
                }
                if((*c)->type == ACCSSNODETYPE_FUNCTION)
                {
                    if(    ((options & ACCSSOPTION_IE9) && strcmp((*c)->children[0]->content, "linear-gradient") == 0)
                       ||  ((options & ACCSSOPTION_IE9) && strcmp((*c)->children[0]->content, "radial-gradient") == 0)
                       ||  ((options & ACCSSOPTION_IE9) && strcmp((*c)->children[0]->content, "repeating-linear-gradient") == 0)
                       ||  ((options & ACCSSOPTION_IE9) && strcmp((*c)->children[0]->content, "repeating-radial-gradient") == 0)
                       )
                    {
                        return 1;
                    }
                    src++;
                }

                c++;
            }
            if ((options & ACCSSOPTION_IE8) && src > 1)
            {
                return 1;
            }

        }
        else if((options & ACCSSOPTION_IE7) && casecmp(str, "display") == 0 && listLength(value->children) > 0 && value->children[0]->type == ACCSSNODETYPE_IDENT && strcmp(value->children[0]->content, "inline-block") == 0)
        {
            return 1;
        }
        else if((options & ACCSSOPTION_FUTURE) && listLength(value->children) > 0)
        {
            struct astnode** c = value->children;
            while(*c != NULL)
            {
                char* ven =NULL;
                if ( ( ven = getVendorIDFromNode(*c)) != NULL)
                {
                    free(ven);
                    return 1;
                }

                if( (options & ACCSSOPTION_FUTURE)
                   && (*c)->type == ACCSSNODETYPE_FUNCTION
                   &&   (  strcmp((*c)->children[0]->content, "image") == 0
                         || strcmp((*c)->children[0]->content, "image-set") == 0
                         )
                   )
                {
                    return 1;
                }

                if(testDimensions(options, (*c)))
                {
                    return 1;
                }
                if ((*c)->type == ACCSSNODETYPE_FUNCTION)
                {
                    struct astnode** d = (*c)->children;
                    while(*d != NULL)
                    {
                        if(testDimensions(options, (*d)))
                        {
                            return 1;
                        }
                        d++;
                    }
                }
                c++;
            }

        }
    }
    return 0;
}

struct analyze* analyze(unsigned char compat, struct astnode* r1, struct astnode* r2, char checkProps)
{
    struct analyze* r = malloc(sizeof(struct analyze));
    if(r == NULL)
    {
        memoryFailure();
        exit(EXIT_FAILURE);
    }
    memset(r, 0, sizeof(struct analyze));


    if (r1->type != r2->type)
    {
    	return r;
    }

    if(compat == ACCSSOPTION_NONE)
    {
        checkProps = 0;
    }


    struct astnode* b1 = r1->children[1];
    struct astnode* b2 = r2->children[1];

    struct astnode** d1 = b1->children;
    struct astnode** d2 = b2->children;


    char** h1 = getHash(d1);
    char** h2 = getHash(d2);

    size_t i=0, d1len=listLength(d1), d2len=listLength(d2);
    struct astnode* x = NULL;

    char** comp1 = NULL;
    char** comp2 = NULL;
    if (checkProps)
    {
        size_t c1 = 0;

        for(i=0; i < d1len; i++)
        {
            x = d1[i];
            if(dontRestructure(compat, x))
            {
                char** tmp = realloc(comp1, (c1+2) * sizeof(char*));
                if(tmp == NULL)
                {
                    memoryFailure();
                    exit(EXIT_FAILURE);
                }
                comp1 = tmp;
                comp1[c1] =  copyValue(x->children[0]->s);
                comp1[++c1] = NULL;
            }
        }
        size_t c2 = 0;

        for(i=0; i < d2len; i++)
        {
            x = d2[i];
            if(dontRestructure(compat, x))
            {
                char** tmp = realloc(comp2, (c2+2) * sizeof(char*));
                if(tmp == NULL)
                {
                    memoryFailure();
                    exit(EXIT_FAILURE);
                }
                comp2 = tmp;
                comp2[c2] =  copyValue(x->children[0]->s);
                comp2[++c2] = NULL;
            }
        }
    }

    for(i=0; i < d1len; i++)
    {
        x = d1[i];

        if (checkProps && stringInHash(comp1, x->children[0]->s))
        {
            r->ne1 = pushASTNode(r->ne1, x);
        }
        else if(stringInHash(h2, x->s))
        {
            r->eq = pushASTNode(r->eq, x);
        }
        else
        {
            r->ne1 = pushASTNode(r->ne1, x);
        }
    }

    for(i = 0; i < d2len; i++)
    {
        x = d2[i];
        if (checkProps && stringInHash(comp2, x->children[0]->s))
        {
            r->ne2 = pushASTNode(r->ne2, x);
        }
        else if(!stringInHash(h1, x->s))
        {
            r->ne2 = pushASTNode(r->ne2, x);
        }
    }

    freeCharList(h1);
    freeCharList(h2);
    freeCharList(comp1);
    freeCharList(comp2);

    return r;
}

void freeAnalyze(struct analyze* a)
{
    if(a->eq != NULL)
    {
        free(a->eq);
    }
    if(a->ne1 != NULL)
    {
        free(a->ne1);
    }
    if(a->ne2 != NULL)
    {
        free(a->ne2);
    }
    free(a);
}

void cleanSelector(struct astnode* node)
{
    size_t ilen = listLength(node->children);
    if (!ilen)
    {
        return;
    }

    char** h = NULL;
    size_t hlen = 0, i = 0;

    for( ; i < ilen; i++)
    {
        char* s = node->children[i]->s;
        if(stringInHash(h,  s))
        {
			node->children = removeItem(node->children, i, 1);
            i--;
            ilen = listLength(node->children);
        }
        else
        {
            hlen++;
            char** b = realloc(h, sizeof(char*)*(hlen+1));
            if(b == NULL)
            {
                memoryFailure();
                exit(EXIT_FAILURE);
            }
            h = b;

            h[hlen-1]=copyValue(s);
            h[hlen] = NULL;

        }
    }
    freeCharList(h);
}

struct astnode* rejoinRuleset(struct compdeps* deps, struct astnode* node, char rule, struct astnode* container, size_t index, const char* path)
{
    struct astnode* p = (index == 0 || container->children[index - 1]->type != ACCSSNODETYPE_RULESET) ? NULL : container->children[index - 1];
    if(p == NULL)
    {
        return node;
    }

    struct astnode** ps = p ? p->children[0]->children : NULL;
    struct astnode** pb = p ? p->children[1]->children : NULL;
    struct astnode** ts = node->children[0]->children;
    struct astnode** tb = node->children[1]->children;

    if (!listLength(tb))
    {
    	return NULL;
    }

    char* pSig = p->info != NULL ? p->info->pseudoSignature : NULL;
    char* tSig = node->info != NULL ? node->info->pseudoSignature : NULL;

    char equalSig = ((pSig == NULL && tSig == NULL) || ( (tSig != NULL && pSig != NULL) && strcmp(tSig, pSig) == 0));

    if (listLength(ps) && listLength(pb) && equalSig)
    {
        if (node->type != p->type)
        {
            return node;
        }

        char** ph = getHash(ps);
        char** th = getHash(ts);
        char eq = equalHash(th, ph);
        freeCharList(ph);
        freeCharList(th);

        if (eq)
        {
            p->children[1]->children = mergeList(p->children[1]->children, node->children[1]->children);
            node->children[1]->children = NULL;

            return NULL;
        }

        if (okToJoinByProperties(node, p))
        {
            struct analyze* r = analyze(deps->compat, node, p, 0);
            char eq = !listLength(r->ne1) && !listLength(r->ne2);
            freeAnalyze(r);

            if (eq)
            {
                p->children[0]->children = mergeList(p->children[0]->children, node->children[0]->children);
                node->children[0]->children = NULL;

                cleanSelector(p->children[0]);
                if(p->children[0]->s != NULL)
                {
                    free(p->children[0]->s);
                }
                p->children[0]->s = translate(p->children[0]);

                return NULL;
            }
        }
    }

    return node;
}

struct astnode* superRejoinRuleset(struct compdeps* deps, struct astnode* node, char rule, struct astnode* container, size_t index, const char* path)
{
    struct astnode* x;
    while (( x = rejoinRuleset(deps, node, rule, container, index, path)) != NULL && index-- > 0)
    {
        ;
    }
    return x;
}

char isTBLProp(char* s)
{
    if(s == NULL)
    {
        return 0;
    }

    if (strcmp("margin", s) == 0)
    {
        return 1;
    }
    else if (strcmp("margin-top", s) == 0)
    {
        return 1;
    }
    else if (strcmp("margin-right", s) == 0)
    {
        return 1;
    }
    else if (strcmp("margin-bottom", s) == 0)
    {
        return 1;
    }
    else if (strcmp("margin-left", s) == 0)
    {
        return 1;
    }
    else if (strcmp("padding", s) == 0)
    {
        return 1;
    }
    else if (strcmp("padding-top", s) == 0)
    {
        return 1;
    }
    else if (strcmp("padding-right", s) == 0)
    {
        return 1;
    }
    else if (strcmp("padding-bottom", s) == 0)
    {
        return 1;
    }
    else if (strcmp("padding-left", s) == 0)
    {
        return 1;
    }

    return 0;
}

char* pathUp(const char* path)
{
    if(path == NULL)
    {
        return NULL;
    }

    size_t plen = strlen(path);

    while (plen > 0 && path[--plen] != '/');

    char* parent = malloc(sizeof(char)*plen+1);
    memcpy(parent, path, plen);
    parent[plen] = '\0';
    return parent;
}

struct astnode* markShorthands(struct compdeps* deps, struct astnode* node, char rule, struct astnode* container, size_t index, const char* path)
{
    static char* lastShortSelector = NULL;
    static size_t lastShortGroupID = 0;

    char* selector = NULL;
    char freeze = 0;
    char* freezeID = NULL;
    size_t shortGroupID = 0;

    if (container->type == ACCSSNODETYPE_RULESET)
    {
        selector = copyValue(container->children[0]->children[0]->s);
        freeze = container->info->freeze;
		freezeID = container->info->freezeID;
        shortGroupID = container->info->shortGroupID;
    }
    else
    {
        selector = copyValue("");
    }

    struct shortHand* sh = NULL;

    char* root = pathUp(path);
    char* fr = freeze ? concat("%s%s%s", 3, "&", freezeID, "&") : NULL;

    char* pre = concat("%s%s%s%s%s", 5, root, "/", (fr ? fr : ""), (selector ? selector : ""), "/");

    if(fr != NULL)
    {
        free(fr);
    }
    free(root);

    size_t ilen = listLength(node->children), i = ilen - 1;

    for( ; i+1 > 0; i--)
    {
        char createNew =  1;
        struct astnode* x = node->children[i];
        if (x->type == ACCSSNODETYPE_DECLARATION)
        {
            struct astnode* v = x->children[1];
            size_t vlen = listLength(v->children);
            if(!vlen)
            {
                continue;
            }


            char imp = v->children[vlen - 1]->type == ACCSSNODETYPE_IMPORTANT;
            char* p = x->children[0]->s;
            if (x->info == NULL)
            {
                x->info = createASTInfo();
            }
            if (x->info->id != NULL)
            {
                free(x->info->id);
                x->info->id = NULL;
            }
            char buffer[LDBUFFERSIZE];
            sprintf(buffer, "%ld", i);
            x->info->id = concat("%s%s%s", 3, path, "/", buffer);
            if (isTBLProp(p))
            {
                char* main = extractMain(p);
                char* key = concat("%s%s", 2, pre, main);
                free(main);

                struct shortHand** shorts = getByKey(deps->shorts2, key);

                size_t sL = shorts != NULL ? shortsLength(shorts) : 0;
                size_t shortsI = sL == 0 ? 0 : sL - 1;


                if ((deps->mergesplitted) || (!lastShortSelector || (selector != NULL && strcmp(selector, lastShortSelector) == 0) || shortGroupID == lastShortGroupID))
                {
                    if (sL)
                    {
                        sh = shorts[shortsI];
                        createNew = 0;
                    }
                }


                if (createNew)
                {
                    x->info->replaceByShort = 1;
                    x->info->shortHandKey.i = shortsI;
                    if (x->info->shortHandKey.key != NULL)
                    {
                        free(x->info->shortHandKey.key);
                    }
                    x->info->shortHandKey.key = copyValue(key);

                    sh = malloc(sizeof(struct shortHand));
                    memset(sh, 0, sizeof(struct shortHand));

                    initShort(sh, p, imp);
                    shorts = pushShorts(shorts, sh);
                }

                if (!sh->invalid)
                {
                    sL = shorts != NULL ? shortsLength(shorts) : 0;
                    size_t shortsI = sL == 0 ? 0 : sL - 1;
                    x->info->removeByShort = 1;

                    x->info->shortHandKey.i = shortsI;
                    if (x->info->shortHandKey.key != NULL)
                    {
                        free(x->info->shortHandKey.key);
                    }
                    x->info->shortHandKey.key = copyValue(key);
                    addToShort(sh, p, v->s, v->children, imp);
                }

                deps->shorts2 = addByKey(deps->shorts2, key, shorts);

                free(key);

                lastShortSelector = resizeValue(lastShortSelector, selector);
                lastShortGroupID = shortGroupID;
            }
        }
    }
    free(selector);
    free(pre);

    return node;
}

void disjoin (struct astnode* container)
{

    if(container != NULL)
    {
        struct astnode** sr = NULL;

        static size_t shortGroupID = 0;
        size_t i = listLength(container->children) - 1;
        for(; i+1 > 0; i--)
        {
            struct astnode* t = container->children[i];
            if (t != NULL)
            {
                if (t->type == ACCSSNODETYPE_RULESET)
                {
                    t->info->shortGroupID = shortGroupID++;
                    struct astnode* s = t->children[0];
                    if (listLength(s->children) > 1)
                    {
                        sr = s->children;
                        size_t k = listLength(sr) - 1;
                        for(; k +1 > 0; k--)
                        {
                            struct astnode* r = copyTree(t);
                            r->children[0]->children = extractAST(r->children[0]->children, k);
                            if(r->children[0]->s != NULL)
                            {
                                free(r->children[0]->s);
                            }
                            r->children[0]->s = copyValue(s->children[k]->s);

                            container->children = insertItem(container->children, i+1, r);

                        }
                        container->children = removeItem(container->children, i, 1);
                    }
                }
                else if (t->type == ACCSSNODETYPE_ATRULER || t->type == ACCSSNODETYPE_STYLESHEET)
                {
                    disjoin(t);
                }
            }
        }
    }
}

char impSum (struct shortHand* sh)
{
    char imp = 0, n = 0;

    if(sh->sides.top != NULL)
    {
        n++;
        if (sh->sides.top->imp)
        {
            imp++;
        }
    }

    if(sh->sides.left != NULL)
    {
        n++;
        if (sh->sides.left->imp)
        {
            imp++;
        }
    }

    if(sh->sides.bottom != NULL)
    {
        n++;
        if (sh->sides.bottom->imp)
        {
            imp++;
        }
    }

    if(sh->sides.right != NULL)
    {
        n++;
        if (sh->sides.right->imp)
        {
            imp++;
        }
    }
    return imp == n ? imp : 0;
}

struct astnode* getShValue(struct shortHand* sh)
{
    struct shortSide* a[] = {sh->sides.top, sh->sides.right, sh->sides.bottom, sh->sides.left};
    char alen = 4;

    struct astnode* r = createASTNodeWithType(ACCSSNODETYPE_VALUE);

    if (casecmp(sh->sides.left->s, sh->sides.right->s) == 0)
    {
        alen--;
        if (casecmp(sh->sides.bottom->s, sh->sides.top->s) == 0)
        {
            alen--;
            if (casecmp(sh->sides.right->s, sh->sides.top->s) == 0)
            {
                alen--;
            }
        }
    }
    char i = 0, ilen = alen-1;
    for( ; i < ilen; i++)
    {
        r->children = mergeList(r->children, copyList(a[i]->t));
        struct astnode* s = createASTNodeWithType(ACCSSNODETYPE_S);
        s->content = copyValue(" ");
        r->children = pushASTNode(r->children, s);
    }
    r->children = mergeList(r->children, copyList(a[i]->t));

    if (impSum(sh))
    {
        r->children = pushASTNode(r->children, createASTNodeWithType(ACCSSNODETYPE_IMPORTANT));
    }

    r->s = translate(r);

    return r;
}
struct astnode* getShProperty(struct shortHand* sh)
{
    struct astnode* i = createASTNodeWithType(ACCSSNODETYPE_IDENT);

    i->content = copyValue(sh->name);
    i->s = copyValue(sh->name);

    struct astnode* p = createASTNodeWithType(ACCSSNODETYPE_PROPERTY);
    p->s = copyValue(sh->name);

    p->children = pushASTNode(NULL, i);

    return p;
}
struct astnode* cleanShorthands(struct compdeps* deps, struct astnode* node, char rule, struct astnode* container, size_t index, const char* path)
{
    if(node->info != NULL && (node->info->removeByShort || node->info->replaceByShort))
    {
        struct shortHand** sl = getByKey(deps->shorts2, node->info->shortHandKey.key);
        struct shortHand* s = sl[node->info->shortHandKey.i];


        if (!s->invalid && isOkToMinimize(s))
        {
            if (node->info->replaceByShort)
            {
                struct astnode* t = createASTNodeWithType(ACCSSNODETYPE_DECLARATION);

                t->children = malloc(sizeof(struct astnode*)*3);
                t->children[0] = getShProperty(s);
                t->children[1] = getShValue(s);
                t->children[2] = NULL;
                t->s = translate(t);

                deleteASTTree(node);

                return t;
            }
            else
            {
                return NULL;
            }
        }
    }
    return node;
}


char* getVendorFromString(char* string)
{
    if (string[0] == '-')
    {
        char* i = strchr(&string[1], '-');

        if (i != NULL)
        {
            size_t slen = i-string+1;

            char* vend = malloc(sizeof(char)*(slen+1));
            memcpy(vend, string, slen);
            vend[slen]='\0';
            return vend;
        }
    }
    return NULL;
}

char* vendorID(char* string)
{
    if (casecmp(string, "-o-") == 0)
    {
        return copyValue(ACCSSVENDOR_OPERA);
    }
    else if (casecmp(string, "-moz-") == 0)
    {
        return copyValue(ACCSSVENDOR_MOZILLA);
    }
    else if (casecmp(string, "-webkit-") == 0)
    {
        return copyValue(ACCSSVENDOR_WEBKIT);
    }
    else if (casecmp(string, "-ms-") == 0)
    {
        return copyValue(ACCSSVENDOR_MICROSOFT);
    }
    else if (casecmp(string, "-epub-") == 0)
    {
        return copyValue(ACCSSVENDOR_EPUB);
    }
    else if (casecmp(string, "-apple-") == 0)
    {
        return copyValue(ACCSSVENDOR_APPLE);
    }
    else if (casecmp(string, "-xv-") == 0)
    {
        return copyValue(ACCSSVENDOR_XV);
    }
    else if (casecmp(string, "-wap-") == 0)
    {
        return copyValue(ACCSSVENDOR_WAP);
    }
    return NULL;
}

char* getVendorIDFromNode(struct astnode* node)
{
    char* vID = NULL;
    char* vString = NULL;

    switch(node->type)
    {
        case ACCSSNODETYPE_IDENT:
        {
            if ((vString = getVendorFromString(node->content)) != NULL)
            {
                vID = vendorID(vString);
                free(vString);
            }
            break;
        }
        case ACCSSNODETYPE_FUNCTION:
        {
            if ((vString = getVendorFromString(node->children[0]->content)) != NULL)
            {
                vID = vendorID(vString);
                free(vString);
            }
            break;
        }
    }
    return vID;
}

char* buildPPre(char* pre, char* p, struct astnode** v, struct astnode* d, char freeze)
{
    char* fp = freeze ? copyValue("ft:") : copyValue("ff:");
    char* vID = NULL;

    size_t vlen = listLength(v), i = 0;
    for( ; i < vlen; i++)
    {
        if (vID == NULL)
        {
            vID = getVendorIDFromNode(v[i]);
        }
    }
    char* ret = concat("%s%s%s%s", 5, fp, pre, p, (vID !=NULL ? vID : "")  );
    if(vID != NULL)
    {
        free(vID);
    }
    free(fp);
    return ret;
}

size_t propsLength(struct pchar_prop** list)
{
    if(list == NULL)
    {
        return 0;
    }
    if(*list == NULL)
    {
        return 0;
    }
    size_t listLength = 0;
    struct pchar_prop** tmp = list;
    while(*tmp != NULL)
    {
        listLength++;
        tmp++;
    }

    return listLength;
}

void freeProp(struct prop* prop)
{
    free(prop->id);
    if(prop->sg != NULL)
    {
        freeCharList(prop->sg);
    }
    free(prop->path);
    free(prop->freezeID);
    free(prop->pseudoID);

    free(prop);
}

void freeProps(struct pchar_prop** props)
{
    if (props != NULL)
    {
        struct pchar_prop** psTmp = props;

        while (*psTmp != NULL)
        {
            struct pchar_prop* ppCurrent = *psTmp;
            psTmp++;

            free((ppCurrent)->key);
            freeProp(ppCurrent->prop);
            free(ppCurrent);
        }
        free(props);
    }
}

struct prop* getProp(struct pchar_prop** list, char* key)
{
    if(list == NULL)
    {
        return NULL;
    }
    struct pchar_prop** tmp = list;

    while (*tmp != NULL)
    {
        if( casecmp((*tmp)->key, key) == 0 )
        {
            return (*tmp)->prop;
        }
        tmp++;
    }

    return NULL;
}
struct pchar_prop** addProp(struct pchar_prop** list, struct prop* prop, char* ppre)
{
    struct pchar_prop** tmp = list;
    struct pchar_prop* bucket = NULL;

    if( list == NULL)
    {
        bucket = malloc(sizeof(struct pchar_prop));
        bucket->prop = prop;
        bucket->key = copyValue(ppre);

        struct pchar_prop** list = malloc(sizeof(struct pchar_prop*)*2);
        if(list == NULL)
        {
            memoryFailure();
            exit(EXIT_FAILURE);
        }

        list[0] = bucket;
        list[1] = NULL;

        return list;
    }

    while (*tmp != NULL)
    {
        if( casecmp((*tmp)->key, ppre) == 0 )
        {
            bucket = (*tmp);
            break;
        }
        tmp++;
    }

    if(bucket != NULL)
    {
        if (bucket->prop != NULL)
        {
            freeProp(bucket->prop);
        }
        bucket->prop = prop;
    }
    else
    {
        bucket = malloc(sizeof(struct pchar_prop));
        bucket->prop = prop;
        bucket->key = copyValue(ppre);

        size_t size = propsLength(list);
        struct pchar_prop** newlist = realloc(list, (sizeof(struct pchar_prop*)*(size+2)));
        if(newlist == NULL)
        {
            memoryFailure();
            exit(EXIT_FAILURE);
        }
        list = newlist;

        list[size+1] = list[size];
        list[size] = bucket;
    }

    return list;
}

struct prop* createProp(struct astnode* node, char imp, char* id, char** sg, char freeze, const char* path, char* freezeID, char* pseudoID)
{
    struct prop* prop = malloc(sizeof(struct prop));
    memset(prop, 0, sizeof(struct prop));

    prop->block = node;
    prop->imp = imp;
    prop->id = copyValue(id);
    prop->sg = sg != NULL ? copyCharList(sg) : NULL;
    prop->freeze = freeze;
    prop->path = copyValue(path);
    prop->freezeID = copyValue(freezeID);
    prop->pseudoID = copyValue(pseudoID);

    return prop;
}

void deleteProperty(struct astnode* block, char* id)
{
    size_t blen = listLength(block->children), i = blen - 1;
    for( ; i+1 > 0; i--)
    {
        struct astnode* d = block->children[i];
        if (d->type == ACCSSNODETYPE_DECLARATION && strcmp(d->info->id, id) == 0)
        {
            block->children = removeItem(block->children, i, 1);
            break;
        }
    }
}

char** nlTable(char* prop)
{
    char** hash = NULL;
    if(casecmp(prop, "border-width") == 0)
    {
        char* list[] = {"border", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "border-style") == 0)
    {
        char* list[] = {"border", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "border-color") == 0)
    {
        char* list[] = {"border", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "border-top") == 0)
    {
        char* list[] = {"border", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "border-right") == 0)
    {
        char* list[] = {"border", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "border-bottom") == 0)
    {
        char* list[] = {"border", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "border-left") == 0)
    {
        char* list[] = {"border", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "border-top-width") == 0)
    {
        char* list[] = {"border-top", "border-width", "border", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "border-right-width") == 0)
    {
        char* list[] = {"border-right", "border-width", "border", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "border-bottom-width") == 0)
    {
        char* list[] = {"border-bottom", "border-width", "border", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "border-left-width") == 0)
    {
        char* list[] = {"border-left", "border-width", "border", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "border-top-style") == 0)
    {
        char* list[] = {"border-top", "border-style", "border", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "border-right-style") == 0)
    {
        char* list[] = {"border-right", "border-style", "border", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "border-bottom-style") == 0)
    {
        char* list[] = {"border-bottom", "border-style", "border", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "border-left-style") == 0)
    {
        char* list[] = {"border-left", "border-style", "border", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "border-top-color") == 0)
    {
        char* list[] = {"border-top", "border-color", "border", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "border-right-color") == 0)
    {
        char* list[] = {"border-right", "border-color", "border", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "border-bottom-color") == 0)
    {
        char* list[] = {"border-bottom", "border-color", "border", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "border-left-color") == 0)
    {
        char* list[] = {"border-left", "border-color", "border", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "margin-top") == 0)
    {
        char* list[] = {"margin", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "margin-right") == 0)
    {
        char* list[] = {"margin", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "margin-bottom") == 0)
    {
        char* list[] = {"margin", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "margin-left") == 0)
    {
        char* list[] = {"margin", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "padding-top") == 0)
    {
        char* list[] = {"padding", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "padding-right") == 0)
    {
        char* list[] = {"padding", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "padding-bottom") == 0)
    {
        char* list[] = {"padding", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "padding-left") == 0)
    {
        char* list[] = {"padding", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "font-style") == 0)
    {
        char* list[] = {"font", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "font-variant") == 0)
    {
        char* list[] = {"font", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "font-weight") == 0)
    {
        char* list[] = {"font", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "font-size") == 0)
    {
        char* list[] = {"font", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "font-family") == 0)
    {
        char* list[] = {"font", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "list-style-type") == 0)
    {
        char* list[] = {"list-style", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "list-style-position") == 0)
    {
        char* list[] = {"list-style", NULL};
        hash = copyCharList(list);
    }
    else if(casecmp(prop, "list-style-image") == 0)
    {
        char* list[] = {"list-style", NULL};
        hash = copyCharList(list);
    }
    return hash;
}

char needless(char* name, struct pchar_prop** props, char* pre, char imp,struct astnode** v, struct astnode* d, char freeze)
{
    char* n = copyValue(name);
    char hack = n[0];
    char hackstr[] = {'\0', '\0', '\0'};

    if (hack == '*' || hack == '_' || hack == '$')
    {
        hackstr[0] = hack;
        char* old = n;
        n = copyValue(++n);
        free(old);
    }
    else if (hack == '/' && name[1] == '/')
    {
        hackstr[0] = hackstr[1] = '/';
        char* old = n;
        n = copyValue(n+sizeof(char)*2);
        free(old);
    }

    char* vendor = getVendorFromString(n);
    char* prop = copyValue(n+ (vendor == NULL ? 0 : (sizeof(char)*(strlen(vendor)))));
    char** x = NULL;
    free(n);
    if ((x = nlTable(prop)) != NULL)
    {
		size_t i = 0, xlen = charListLength(x);
        for( ; i < xlen; i++)
        {
            char* hvx = concat("%s%s%s", 3, hackstr, vendor != NULL ? vendor : "", x[i]);
            char* ppre = buildPPre(pre, hvx, v, d, freeze);
            free(hvx);
            struct prop* t = getProp(props, ppre);
            free(ppre);
            if (t !=NULL)
            {
                free(prop);
                free(vendor);
                freeCharList(x);
                return (!imp || t->imp);
            }
        }
        freeCharList(x);
    }

    free(prop);
    free(vendor);

    return 0;
}

struct astnode* restructureBlock(struct compdeps* deps, struct astnode* node, char rule, struct astnode* container, size_t index, const char* path)
{
    struct pchar_prop** props = NULL;
    char isPseudo = 0;
    char* selector = NULL;
    char freeze = 0;
    char* freezeID = NULL;
    char* pseudoID = NULL;
    char** sg = NULL;

    char conttype = container->type;

    if (conttype == ACCSSNODETYPE_RULESET)
    {
        props = deps->props2;
        isPseudo = container->children[0]->children[0]->info->pseudo;
        selector = copyValue(container->children[0]->children[0]->s);
        freeze = container->info->freeze;
        freezeID = copyValue(container->info->freezeID);
        pseudoID = copyValue(container->info->pseudoID);
        sg = container->children[0]->children[0]->info->sg;
    }
    else
    {
        selector = copyValue("");
        freezeID = copyValue("fake");
        pseudoID = copyValue("fake");
    }

    char imp = 0;
    char* pathup = pathUp(path);
    char* pre = concat("%s%s%s%s", 4, pathup , "/", selector, "/" );
    free(pathup);
    size_t tlen = listLength(node->children), i = tlen - 1;

    for( ; i+1 > 0; i--)
    {
        struct astnode* x = node->children[i];

        if (x->type == ACCSSNODETYPE_DECLARATION)
        {
            struct astnode** v = x->children[1]->children;
            size_t vlen = listLength(v);
            if(!vlen)
            {
                node->children = removeItem(node->children, i, 1);
                continue;
            }

            imp = v[vlen - 1]->type == ACCSSNODETYPE_IMPORTANT;
            char* p = x->children[0]->s;
            char* ppre = buildPPre(pre, p, v, x, freeze);

            if(x->info == NULL)
            {
                x->info = createASTInfo();
            }

            if(x->info->id != NULL)
            {
                free(x->info->id);
                x->info->id = NULL;
            }

            char iBuff[LDBUFFERSIZE];
            sprintf(iBuff, "%ld", i);
            x->info->id = concat("%s%s%s", 3, path, "/", iBuff);

            struct prop* t = NULL;
            if (!dontRestructure(deps->compat, x) && (t = getProp(props, ppre)) != NULL)
            {
                if ((isPseudo && strcmp(freezeID,t->freezeID) == 0) || // pseudo from equal selectors group
                    (!isPseudo && strcmp(pseudoID, t->pseudoID) == 0) || // not pseudo from equal pseudo signature group
                    (isPseudo && strcmp(pseudoID, t->pseudoID) == 0 && hashInHash(sg, t->sg))) // pseudo from covered selectors group
                {

                    if (imp && !t->imp)
                    {
                        deleteProperty(t->block, t->id);
                        props = addProp(props, createProp(node, imp, x->info->id, sg, freeze, path, freezeID, pseudoID), ppre);
                        if (conttype == ACCSSNODETYPE_RULESET)
                        {
                            deps->props2 = props;
                        }
                    }
                    else
                    {
                        node->children = removeItem(node->children, i, 1);
                    }
                }
            }
            else if (needless(p, props, pre, imp, v, x, freeze))
            {
                node->children = removeItem(node->children, i, 1);
            }
            else if(!dontRestructure(deps->compat, x))
            {
                props = addProp(props, createProp(node, imp, x->info->id, sg, freeze, path, freezeID, pseudoID), ppre);
                if (conttype == ACCSSNODETYPE_RULESET)
                {
                    deps->props2 = props;
                }
            }

            free(ppre);
        }
    }

    if (conttype != ACCSSNODETYPE_RULESET)
    {
        freeProps(props);
    }

    free(pre);
    free(selector);
    free(freezeID);
    free(pseudoID);
    return node;
}

size_t calcLength(struct astnode** nodes)
{
    size_t r = 0;
    while (*nodes != NULL)
    {
        r += strlen((*nodes)->s);
        nodes++;
    }
    return r;
}


struct astnode* restructureRuleset(struct compdeps* deps, struct astnode* node, char rule, struct astnode* container, size_t index, const char* path)
{

    struct astnode* p = (index == 0 || container->children[index - 1]->type == ACCSSNODETYPE_UNKNOWN) ? NULL : container->children[index - 1];
    if(p == NULL)
    {
        return node;
    }
    if (listLength(p->children) < 2)
    {
        return node;
    }

    struct astnode** ps = p->children[0]->children;
    struct astnode** pb = p->children[1]->children;
    struct astnode** tb = node->children[1]->children;

    if(listLength(tb) == 0)
    {
        return NULL;
    }

    char* pSig = p->info != NULL ? p->info->pseudoSignature : NULL;
    char* tSig = node->info != NULL ? node->info->pseudoSignature : NULL;

    char equalSig = ((pSig == NULL && tSig == NULL) || ( (tSig != NULL && pSig != NULL) && strcmp(tSig, pSig) == 0));

    if (listLength(ps) && listLength(pb) && equalSig)
    {
        if (node->type != p->type)
        {
            return node;
        }

        struct analyze* r = analyze(deps->compat, node, p, 1);

        size_t rEqLen = listLength(r->eq);
        size_t rNe1Len = listLength(r->ne1);
        size_t rNe2Len = listLength(r->ne2);

        if (rEqLen && (rNe1Len || rNe2Len))
        {

            if (rNe1Len && !rNe2Len)
            {
                struct astnode** ns = node->children[0]->children;
                char* nss = translate(node->children[0]);

                // selector length + delims length
                size_t sl = strlen(nss) + listLength(ns) - 1;
                // declarations length + decldelims length
                size_t bl = calcLength(r->eq) + rEqLen - 1;
                free(nss);

                if (sl < bl)
                {
                    p->children[0]->children = mergeList(p->children[0]->children, copyList(node->children[0]->children));
                    cleanSelector( p->children[0] );

                    struct astnode** newlist = copyList(r->ne1);
                    deleteASTList(node->children[1]->children);
                    node->children[1]->children = newlist;
                }

            }
            else if (!rNe1Len && rNe2Len)
            {
                struct astnode** ns = p->children[0]->children;
                char* nss = translate(p->children[0]);

                // selector length + delims length
                size_t sl = strlen(nss) + listLength(ns) - 1;
                // declarations length
                size_t bl = calcLength(r->eq) + rEqLen - 1;
                free(nss);

                if (sl < bl)
                {
                    node->children[0]->children = mergeList(node->children[0]->children, copyList(p->children[0]->children));
                    cleanSelector( node->children[0] );

                    struct astnode** newlist = copyList(r->ne2);
                    deleteASTList(p->children[1]->children);
                    p->children[1]->children = newlist;
                }

            }
            else
            {
                struct astnode* ns = copyTree(p->children[0]);
                ns->children = mergeList(ns->children, copyList(node->children[0]->children));
                cleanSelector(ns);

                char* nss = translate(ns);
                if (ns->s != NULL)
                {
                    free(ns->s);
                }
                ns->s = nss;
                // selector length
                size_t rl = strlen(nss) + listLength(ns->children) - 1 + 2; // braces length
                size_t bl = calcLength(r->eq) + rEqLen - 1;

                if (bl >= rl)
                {

                    struct astnode* b = createASTNodeWithType(ACCSSNODETYPE_BLOCK);
                    b->children = copyList(r->eq);

                    b->info = createASTInfo();

                    struct astnode* nr = createASTNodeWithType(ACCSSNODETYPE_RULESET);
                    nr->info = createASTInfo();

                    nr->children = pushASTNode(NULL, ns); //firstnode = selector
                    nr->children = pushASTNode(nr->children, b); //block definitions

                    struct astnode** newlistT = copyList(r->ne1);
                    struct astnode** newlistP = copyList(r->ne2);

                    deleteASTList(node->children[1]->children);
                    node->children[1]->children = newlistT;

                    deleteASTList(p->children[1]->children);
                    p->children[1]->children = newlistP;

                    container->children = insertItem(container->children, index+1, nr);
                }
                else
                {
                    deleteASTTree(ns);
                }

            }
        }
        freeAnalyze(r);
    }
    return node;
}

struct astnode* delimSelectors(struct compdeps* deps, struct astnode* node, char rule, struct astnode* container, size_t index, const char* path)
{
	size_t j = listLength(node->children) - 1;
    for( ; j > 0; j--)
    {
        node->children = insertItem(node->children, j, createASTNodeWithType(ACCSSNODETYPE_DELIM));
    }
    return node;
}
struct astnode* delimBlocks(struct compdeps* deps, struct astnode* node, char rule, struct astnode* container, size_t index, const char* path)
{
	size_t j = listLength(node->children) - 1;
    for( ; j > 0; j--)
    {
        node->children = insertItem(node->children, j, createASTNodeWithType(ACCSSNODETYPE_DECLDELIM));
    }
    return node;
}

struct astnode* ccrules(struct compdeps* deps, struct astnode* x1, char rule, struct astnode* container, size_t index, const char* path)
{
    if(rule == ACCSSNODETYPE_COMMENT)
    {
        x1 = cleanComment(deps, x1, rule, container, index, path);
    }
    return x1;
}

struct astnode* crules(struct compdeps* deps, struct astnode* x1, char rule, struct astnode* container, size_t index, const char* path)
{
    switch(rule)
    {
        case ACCSSNODETYPE_ATRULES:
        {
            x1 = cleanCharset(deps, x1, rule, container, index, path);
            if(x1 == NULL)
            {
                return NULL;
            }
            x1 = cleanImport(deps, x1, rule, container, index, path);
        }
            break;

        case ACCSSNODETYPE_S:
        {
            x1 = cleanWhitespace(deps, x1, rule, container, index, path);

        }
            break;

        case ACCSSNODETYPE_NUMBER:
        {
            x1 = compressNumber(deps, x1, rule, container, index, path);
        }
            break;

        case ACCSSNODETYPE_UNARY:
        {
            x1 = cleanUnary(deps, x1, rule, container, index, path);
        }
            break;

        case ACCSSNODETYPE_VHASH:
        case ACCSSNODETYPE_FUNCTION:
        case ACCSSNODETYPE_IDENT:
        {
            x1 = compressColor(deps, x1, rule, container, index, path);
        }
            break;

        case ACCSSNODETYPE_DIMENSION:
        {
            x1 = compressDimension(deps, x1, rule, container, index, path);
        }
            break;

        case ACCSSNODETYPE_STRING:
        {
            x1 = compressString(deps, x1, rule, container, index, path);
        }
            break;

        case ACCSSNODETYPE_DECLARATION:
        {
            x1 = compressFontWeight(deps, x1, rule, container, index, path);
            if(x1 == NULL)
            {
                return NULL;
            }
            x1 = compressFont(deps, x1, rule, container, index, path);
            if(x1 == NULL)
            {
                return NULL;
            }
            x1 = compressBackground(deps, x1, rule, container, index, path);
        }
            break;

        case ACCSSNODETYPE_RULESET:
        case ACCSSNODETYPE_ATRULEB:
        case ACCSSNODETYPE_ATRULER:
        {
            x1 = cleanEmpty(deps, x1, rule, container, index, path);
        }
            break;
    }
    return x1;
}

struct astnode* srules(struct compdeps* deps, struct astnode* x1, char rule, struct astnode* container, size_t index, const char* path)
{
    switch(rule)
    {
        case ACCSSNODETYPE_VALUE:
        case ACCSSNODETYPE_FUNCTIONBODY:
        {
            x1 = reAddSpaces(deps, x1, rule, container, index, path);
        }
            break;
    }
    return x1;
}

struct astnode* prules(struct compdeps* deps, struct astnode* x1, char rule, struct astnode* container, size_t index, const char* path)
{
    switch(rule)
    {
        case ACCSSNODETYPE_DECLDELIM:
        case ACCSSNODETYPE_DELIM:
        {
            x1 = destroyDelims(deps, x1, rule, container, index, path);
        }
            break;

        case ACCSSNODETYPE_DECLARATION:
        case ACCSSNODETYPE_PROPERTY:
        case ACCSSNODETYPE_SIMPLESELECTOR:
        case ACCSSNODETYPE_FILTER:
        case ACCSSNODETYPE_VALUE:
        case ACCSSNODETYPE_NUMBER:
        case ACCSSNODETYPE_PERCENTAGE:
        case ACCSSNODETYPE_DIMENSION:
        case ACCSSNODETYPE_IDENT:
        {
            x1 = preTranslate(deps, x1, rule, container, index, path);
        }
        break;
    }
    return x1;
}

struct astnode* frrules(struct compdeps* deps, struct astnode* x1, char rule, struct astnode* container, size_t index, const char* path)
{
    if(rule == ACCSSNODETYPE_RULESET)
    {
        x1 = freezeRulesets(deps, x1, rule, container, index, path);
    }
    return x1;
}

struct astnode* rjrules(struct compdeps* deps, struct astnode* x1, char rule, struct astnode* container, size_t index, const char* path)
{
    switch(rule)
    {
        case ACCSSNODETYPE_RULESET:
        {
            x1 = rejoinRuleset(deps, x1, rule, container, index, path);
            if(x1 == NULL)
            {
                return NULL;
            }

        }

        case ACCSSNODETYPE_ATRULEB:
        case ACCSSNODETYPE_ATRULER:
        {
            x1 = cleanEmpty(deps, x1, rule, container, index, path);
        }
            break;
    }
    return x1;
}

struct astnode* msrules(struct compdeps* deps, struct astnode* x1, char rule, struct astnode* container, size_t index, const char* path)
{
    if(rule == ACCSSNODETYPE_BLOCK)
    {
        x1 = markShorthands(deps, x1, rule, container, index, path);
    }
    return x1;
}

struct astnode* csrules(struct compdeps* deps, struct astnode* x1, char rule, struct astnode* container, size_t index, const char* path)
{
    switch(rule)
    {
        case ACCSSNODETYPE_DECLARATION:
        {
            x1 = cleanShorthands(deps, x1, rule, container, index, path);
        }
            break;

        case ACCSSNODETYPE_RULESET:
        case ACCSSNODETYPE_ATRULEB:
        case ACCSSNODETYPE_ATRULER:
        {
            x1 = cleanEmpty(deps, x1, rule, container, index, path);
        }
            break;
    }
    return x1;
}

struct astnode* rbrules(struct compdeps* deps, struct astnode* x1, char rule, struct astnode* container, size_t index, const char* path)
{
    if(rule == ACCSSNODETYPE_BLOCK)
    {
        x1 = restructureBlock(deps, x1, rule, container, index, path);
    }
    return x1;
}

struct astnode* rrrules(struct compdeps* deps, struct astnode* x1, char rule, struct astnode* container, size_t index, const char* path)
{
    switch(rule)
    {
        case ACCSSNODETYPE_RULESET:
        {
            x1 = restructureRuleset(deps, x1, rule, container, index, path);
            if(x1 == NULL)
            {
                return NULL;
            }
        }

        case ACCSSNODETYPE_ATRULEB:
        case ACCSSNODETYPE_ATRULER:
        {
            x1 = cleanEmpty(deps, x1, rule, container, index, path);
        }
            break;
    }
    return x1;
}

struct astnode* srjrules(struct compdeps* deps, struct astnode* x1, char rule, struct astnode* container, size_t index, const char* path)
{
    switch(rule)
    {
        case ACCSSNODETYPE_RULESET:
        {
            x1 = superRejoinRuleset(deps, x1, rule, container, index, path);
            if(x1 == NULL)
            {
                return NULL;
            }

        }

        case ACCSSNODETYPE_ATRULEB:
        case ACCSSNODETYPE_ATRULER:
        {
            x1 = cleanEmpty(deps, x1, rule, container, index, path);
        }
            break;
    }
    return x1;
}

struct astnode* frules(struct compdeps* deps, struct astnode* x1, char rule, struct astnode* container, size_t index, const char* path)
{
    switch(rule)
    {
        case ACCSSNODETYPE_RULESET:
        case ACCSSNODETYPE_ATRULEB:
        case ACCSSNODETYPE_ATRULER:
        {
            x1 = cleanEmpty(deps, x1, rule, container, index, path);
        }
            break;

        case ACCSSNODETYPE_SELECTOR:
        {
            x1 = delimSelectors(deps, x1, rule, container, index, path);
        }
            break;

        case ACCSSNODETYPE_BLOCK:
        {
            x1 = delimBlocks(deps, x1, rule, container, index, path);
        }
            break;
    }
    return x1;
}

struct astnode* walk(struct compdeps* deps, struct astnode* (*rules)(struct compdeps* deps, struct astnode*, char, struct astnode*, size_t, const char*), struct astnode* node, const char* path)
{
    struct astnode* t, *x;

    size_t i = listLength(node->children) - 1;

    for(; i+1 > 0; i--)
    {
        t = node->children[i];
        t->parent = node;
        if (t->children != NULL)
        {
            char buffer[LDBUFFERSIZE];
            int length = sprintf(buffer, "%ld", i);
            size_t path_len = strlen(path);
            char* newpath = malloc(sizeof(char)*(length+path_len+2));
            if (newpath == NULL)
            {
                memoryFailure();
                exit(EXIT_FAILURE);
            }

            strcpy(newpath, path);
            newpath[path_len]='/';
            strcpy(&newpath[path_len+1], buffer);

            t = walk(deps, rules, t, newpath); // go inside

            free(newpath);
        }

        if (t == NULL)
        {
            node->children = removeItem(node->children, i, 1);
        }
        else
        {
            x = rules(deps, t, t->type, node, i, path);
            if (x != NULL)
            {
                node->children[i] = x;
            }
            else
            {
                node->children = removeItem(node->children, i, 1);
            }
        }

    }

    if (node->type == 0)
    {
        return NULL;
    }
    else
    {
        return node;
    }

}

struct astnode* compress(struct astnode* tree, unsigned char restructure, unsigned char mergesplitted, unsigned char compat)
{
    struct astnode* x = tree;

    struct compdeps deps;
    deps.compat = compat;
    deps.mergesplitted = mergesplitted;
    deps.props2 = NULL;
    deps.shorts2 = NULL;


    x = walk(&deps, &ccrules, x, "/0");
    x = walk(&deps, &crules, x, "/0");
    x = walk(&deps, &srules, x, "/0");
    x = walk(&deps, &prules, x, "/0");
    x = walk(&deps, &frrules, x, "/0");

    if (restructure)
    {
        size_t l0, l1 = SIZE_MAX, ls;
        struct astnode *x0, *xs;
        x0 = NULL;
        xs = NULL;

        char* str = translate(x);
        ls = strlen(str);
        free(str);

        xs = copyTree(x);

        //TODO rejoin before disjoin?
    //    x = walk(&deps, &rjrules, x, "/0");
        disjoin(x);

        x = walk(&deps, &msrules, x, "/0");



        x = walk(&deps, &csrules, x, "/0");

        freeShortHands(deps.shorts2);

#ifdef DEBUG
        printf("\n\n===================before RB======================================\n\n");
        printASTNodeJSON(x, 0);
        printf("\n\n=========================================================\n");
        fflush(stdout);
#endif

        x = walk(&deps, &rbrules, x, "/0");

        freeProps(deps.props2);
#ifdef DEBUG
        printf("\n\n===================After RB======================================\n\n");
        printASTNodeJSON(x, 0);
        printf("\n\n=========================================================\n");
        fflush(stdout);
#endif
        x = walk(&deps, &rjrules, x, "/0");
#ifdef DEBUG
        int iterations=0;
#endif
        do
        {
            l0 = l1;

            if(x0 != NULL)
            {
                deleteASTTree(x0);
            }
            x0 = copyTree(x);

            x = walk(&deps, &rrrules, x, "/0");
            x = walk(&deps, &srjrules, x, "/0");

            str = translate(x);
            l1 = strlen(str);
            free(str);
#ifdef DEBUG
            iterations++;
#endif
        } //TODO add third copy so tree can increase and then decrease
        while (l0 > l1);

#ifdef DEBUG
        printf("iterations:%i\n", iterations);
#endif

        if (ls < l0 && ls < l1)
        {
            deleteASTTree(x);
            deleteASTTree(x0);
            x = xs;
            xs = NULL;
            x0 = NULL;
        }
        else if (l0 < l1)
        {
            deleteASTTree(x);
            deleteASTTree(xs);
            x = x0;
            x0 = NULL;
            xs = NULL;
        }
        else
        {
            deleteASTTree(xs);
            deleteASTTree(x0);
        }

    }

    x = walk(&deps, &frules, x, "/0");

    return x;
}
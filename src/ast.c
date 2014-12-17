/*
 * ACCSS C Code Copyright (C) 2014 by Gandalf Sievers
 * Created by Gandalf Sievers on 29.07.14.
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
#include "ast.h"
#include "debug.h"


struct astnode* createASTNodeWithType(char type)
{
    struct astnode* node = malloc(sizeof(struct astnode));

    if(node == NULL)
    {
        memoryFailure();
        exit(EXIT_FAILURE);
    }

    memset(node, 0, sizeof(struct astnode));

    node->type = type;

    return node;
}

struct astinfo* createASTInfo()
{
    struct astinfo* info = malloc(sizeof(struct astinfo));
    if (info == NULL)
    {
        memoryFailure();
        exit(EXIT_FAILURE);
    }
    memset(info, 0, sizeof(struct astinfo));

    return info;
}

struct astnode** extractAST(struct astnode** list, size_t index)
{
    struct astnode* tmp = list[index];
    size_t i = 0, listlen = listLength(list);
    for(; i< listlen; i++)
    {
        if(i == index)
        {
            continue;
        }
        deleteASTTree(list[i]);
    }
    free(list);
    return pushASTNode(NULL, tmp);
}

struct astnode** pushASTNode(struct astnode** list, struct astnode* newnode)
{
    if(newnode == NULL)
    {
        return list;
    }
    if(list == NULL)
    {

        list = malloc(sizeof(struct astnode*)*2);

        if(list == NULL)
        {
            memoryFailure();
            exit(EXIT_FAILURE);
        }

        list[0] = newnode;
        list[1] = NULL;
        return list;
    }

    size_t size = listLength(list);
    struct astnode** newlist = realloc(list, (sizeof(struct astnode*)*(size+2)));
    if(newlist == NULL)
    {
        memoryFailure();
        exit(EXIT_FAILURE);
    }
    list = newlist;

    list[size+1] = list[size];
    list[size] = newnode;

    return list;

}

struct astnode** mergeList(struct astnode** left, struct astnode** right)
{
    if(left == NULL && right == NULL)
    {
        return NULL;
    }
    if(left != NULL && right == NULL)
    {
        return left;
    }
    if(left == NULL && right != NULL)
    {
        return right;
    }

    size_t leftsize = 1;
    struct astnode** tmp = left;
    while(*tmp != NULL)
    {
        leftsize++;
        tmp++;
    }

    leftsize--;

    size_t rightsize = 1;
    tmp = right;
    while(*tmp != NULL)
    {
        rightsize++;
        tmp++;
    }

    size_t newsize = leftsize+rightsize;
    struct astnode** newlist = realloc(left, (sizeof(struct astnode*) * newsize));
    if(newlist == NULL)
    {
        memoryFailure();
        exit(EXIT_FAILURE);
    }
    left = newlist;

	size_t i = leftsize, k=0;
    for(;i < newsize; i++, k++)
    {
        left[i] = right[k];
    }

    free(right);

    return left;
}

size_t listLength(struct astnode** list)
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
    struct astnode** tmp = list;
    while(*tmp != NULL)
    {
        listLength++;
        tmp++;
    }

    return listLength;
}

size_t indexof(struct astnode** list, struct astnode* item, char* error)
{
    *error=0;
    if(list == NULL)
    {
        *error=1;
        return 0;
    }
    if(*list == NULL)
    {
        *error=1;
        return 0;
    }

    size_t index = 0;
    struct astnode** tmp = list;
    while(*tmp != NULL && *tmp != item)
    {
        index++;
        tmp++;
    }
    if(*tmp == 0)
    {
        *error=1;
        return 0;
    }

    return index;
}

struct astnode** removeItem(struct astnode** list, size_t start, size_t len)
{
    if(len == 0)
    {
        return list;
    }

    size_t listlen = listLength(list);

    if (listlen <= start)
    {
        return list;
    }

    size_t newsize = listlen - len;

    if(newsize < 1)
    {
        deleteASTList(list);
        return NULL;
    }

    size_t ilen = start + len;

    if(ilen > listlen)
    {
        return list;
    }


    struct astnode** newlist = malloc(sizeof(struct astnode*) * (newsize+1));
    if(newlist == NULL)
	{
        memoryFailure();
        exit(EXIT_FAILURE);
    }

    size_t i = 0;
    for(; i < start; i++)
    {
        newlist[i] = list[i];
    }
    size_t k = i;

    for(; i < ilen; i++)
    {
    	deleteASTTree(list[i]);
    }

    for(; k <= newsize; k++, i++)
    {
        newlist[k] = list[i];
    }

    free(list);

    return newlist;
}

struct astnode** insertItem(struct astnode** list, size_t start, struct astnode* item)
{
    size_t listlen = listLength(list);

    struct astnode** newlist = realloc(list, sizeof(struct astnode*) * (listlen+2));
    if(newlist == NULL)
	{
        memoryFailure();
        exit(EXIT_FAILURE);
    }

    list = newlist;
    size_t i = listlen;
    for(; i >= start; i--)
    {
        list[i + 1] = list[i];
    }

    list[start] = item;

    return list;
}

void deleteASTTree(struct astnode* node)
{
    if(node->children != NULL)
    {
        size_t i = 0, listlen = listLength(node->children);
        for( ; i < listlen; i++)
        {
            deleteASTTree(node->children[i]);
        }
        free(node->children);
    }
    if(node->content != NULL)
    {
        free(node->content);
    }
    if(node->s != NULL)
    {
        free(node->s);
    }
    if(node->info != NULL)
    {
        if(node->info->freezeID != NULL)
        {
            free(node->info->freezeID);
        }
        if(node->info->pseudoID != NULL)
        {
            free(node->info->pseudoID);
        }
        if(node->info->pseudoSignature != NULL)
        {
            free(node->info->pseudoSignature);
        }
        if(node->info->sg != NULL)
        {
            freeCharList(node->info->sg);
        }
        if(node->info->shortHandKey.key != NULL)
        {
            free(node->info->shortHandKey.key);
        }

        if(node->info->id != NULL)
        {
            free(node->info->id);
        }

        free(node->info);
    }
    free(node);
}

void deleteASTList(struct astnode** list)
{
    struct astnode** tmp = list;
    while ((*tmp)!=NULL)
    {
        struct astnode* current = *tmp;
        tmp++;
        deleteASTTree(current);
    }
    free(list);
}



struct astnode* copyTree(struct astnode* node)
{
    struct astnode* copy = createASTNodeWithType(node->type);

    if(node->children != NULL)
    {
        size_t i = 0, listlen = listLength(node->children);

        for( ; i < listlen; i++)
        {
            copy->children = pushASTNode(copy->children, copyTree(node->children[i]));
        }
    }
    if(node->content != NULL)
    {
        copy->content = copyValue(node->content);
    }
    if(node->s != NULL)
    {
        copy->s = copyValue(node->s);
    }


    if(node->info != NULL)
    {
        copy->info = malloc(sizeof(struct astinfo));
        memset(copy->info, 0, sizeof(struct astinfo));
        if(node->info->freezeID != NULL)
        {
            copy->info->freezeID = copyValue(node->info->freezeID);
        }
        if(node->info->pseudoID != NULL)
        {
            copy->info->pseudoID = copyValue(node->info->pseudoID);
        }
        if(node->info->pseudoSignature != NULL)
        {
            copy->info->pseudoSignature = copyValue(node->info->pseudoSignature);
        }
        copy->info->freeze = node->info->freeze;
        copy->info->pseudo = node->info->pseudo;
        copy->info->replaceByShort = node->info->replaceByShort;
        copy->info->removeByShort = node->info->removeByShort;
        copy->info->shortHandKey.i = node->info->shortHandKey.i;
        if(node->info->shortHandKey.key != NULL)
        {
            copy->info->shortHandKey.key = copyValue(node->info->shortHandKey.key);
        }

        if(node->info->id != NULL)
        {
            copy->info->id = copyValue(node->info->id);
        }

        if (node->info->sg != NULL)
        {
            copy->info->sg = copyCharList(node->info->sg);
        }
    }

    return copy;
}

struct astnode** _copyList(struct astnode** list, size_t length, size_t start, size_t end, char deep)
{
    struct astnode** newlist = malloc(sizeof(struct astnode*) * (length+2));
    size_t i = 0;
    for(; start < end; start++, i++)
    {
        newlist[i] = deep ? copyTree(list[start]) : list[start];
    }
    newlist[i] = NULL;

    return newlist;
}
struct astnode** sslice(struct astnode** list, long start)
{
    return slice(list, start, listLength(list));
}
struct astnode** slice(struct astnode** list, long start, long end)
{
    size_t length = start-end;

    return _copyList(list,  length,  start,  end, 0);
}

struct astnode** copyList(struct astnode** list)
{
    size_t len = listLength(list);
    return _copyList(list,  len,  0,  len, 1);
}

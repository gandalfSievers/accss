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

#include <stdlib.h>

#ifndef accss_ast_h
#define accss_ast_h

#define ACCSSNODETYPE_IDENT 1
#define ACCSSNODETYPE_ATKEYWORD 2
#define ACCSSNODETYPE_STRING 3
#define ACCSSNODETYPE_SHASH 4
#define ACCSSNODETYPE_VHASH 5
#define ACCSSNODETYPE_NUMBER 6
#define ACCSSNODETYPE_PERCENTAGE 7
#define ACCSSNODETYPE_DIMENSION 8
#define ACCSSNODETYPE_CDO 9
#define ACCSSNODETYPE_CDC 10
#define ACCSSNODETYPE_DECLDELIM 11
#define ACCSSNODETYPE_S 12
#define ACCSSNODETYPE_ATTRSELECTOR 13
#define ACCSSNODETYPE_ATTRIB 14
#define ACCSSNODETYPE_NTH 15
#define ACCSSNODETYPE_NTHSELECTOR 16
#define ACCSSNODETYPE_NAMESPACE 17
#define ACCSSNODETYPE_CLAZZ 18
#define ACCSSNODETYPE_PSEUDOE 19
#define ACCSSNODETYPE_PSEUDOC 20
#define ACCSSNODETYPE_DELIM 21
#define ACCSSNODETYPE_STYLESHEET 22
#define ACCSSNODETYPE_ATRULEB 23
#define ACCSSNODETYPE_ATRULES 24
#define ACCSSNODETYPE_ATRULERQ 25
#define ACCSSNODETYPE_ATRULERS 26
#define ACCSSNODETYPE_ATRULER 27
#define ACCSSNODETYPE_BLOCK 28
#define ACCSSNODETYPE_RULESET 29
#define ACCSSNODETYPE_COMBINATOR 30
#define ACCSSNODETYPE_SIMPLESELECTOR 31
#define ACCSSNODETYPE_SELECTOR 32
#define ACCSSNODETYPE_DECLARATION 33
#define ACCSSNODETYPE_PROPERTY 34
#define ACCSSNODETYPE_IMPORTANT 35
#define ACCSSNODETYPE_UNARY 36
#define ACCSSNODETYPE_OPERATOR 37
#define ACCSSNODETYPE_BRACES 38
#define ACCSSNODETYPE_VALUE 39
#define ACCSSNODETYPE_PROGID 40
#define ACCSSNODETYPE_FILTERV 41
#define ACCSSNODETYPE_FILTER 42
#define ACCSSNODETYPE_COMMENT 43
#define ACCSSNODETYPE_URI 44
#define ACCSSNODETYPE_RAW 45
#define ACCSSNODETYPE_FUNCTIONBODY 46
#define ACCSSNODETYPE_FUNCTION 47
#define ACCSSNODETYPE_FUNCTIONEXPRESSION 48
#define ACCSSNODETYPE_UNKNOWN 49

struct pchar_i
{
    char* key;
    size_t i;
};

struct astinfo
{
    char* id;
    char freeze;
    char* freezeID;
    char pseudo;
    char* pseudoID;
    char* pseudoSignature;

    char replaceByShort;
    char removeByShort;
    struct pchar_i shortHandKey;

    char** sg;

    size_t shortGroupID;
};

struct astnode
{
    struct astinfo* info;
    char type;
    char* content;
    char* s;

    struct astnode** children;
    struct astnode* parent;
};

struct astnode* createASTNodeWithType(char type);
struct astinfo* createASTInfo();

struct astnode** pushASTNode(struct astnode** list, struct astnode* newnode);

struct astnode** mergeList(struct astnode** left, struct astnode** right);

size_t listLength(struct astnode** list);

struct astnode** removeItem(struct astnode** list, size_t start, size_t len);

struct astnode** insertItem(struct astnode** list, size_t start, struct astnode* item);

struct astnode** sslice(struct astnode** list, long start);

struct astnode** slice(struct astnode** list, long start, long end);

struct astnode** copyList(struct astnode** list);

struct astnode** extractAST(struct astnode** list, size_t index);

struct astnode* copyTree(struct astnode* root);

size_t indexof(struct astnode** list, struct astnode* item, char* error);

void deleteASTTree(struct astnode* node);

void deleteASTList(struct astnode** list);

#endif

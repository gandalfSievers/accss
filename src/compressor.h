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

#include "ast.h"

#ifndef accss_compressor_h
#define accss_compressor_h

#define ACCSSOPTION_IE7 1
#define ACCSSOPTION_IE8 2
#define ACCSSOPTION_IE9 4
#define ACCSSOPTION_IE10 8
#define ACCSSOPTION_IE11 16
#define ACCSSOPTION_CHROME 64
#define ACCSSOPTION_FUTURE 128
#define ACCSSOPTION_ALL 255
#define ACCSSOPTION_NONE 0

#define ACCSSOPTION_GE_IE8 ACCSSOPTION_ALL - ACCSSOPTION_IE7
#define ACCSSOPTION_GE_IE9 ACCSSOPTION_ALL - ACCSSOPTION_IE7 - ACCSSOPTION_IE8
#define ACCSSOPTION_GE_IE10 ACCSSOPTION_ALL - ACCSSOPTION_IE7 - ACCSSOPTION_IE8 - ACCSSOPTION_IE9
#define ACCSSOPTION_GE_IE11 ACCSSOPTION_ALL - ACCSSOPTION_IE7 - ACCSSOPTION_IE8 - ACCSSOPTION_IE9 - ACCSSOPTION_IE10
#define ACCSSOPTION_GE_CHROME ACCSSOPTION_ALL - ACCSSOPTION_IE7 - ACCSSOPTION_IE8 - ACCSSOPTION_IE9 - ACCSSOPTION_IE10 - ACCSSOPTION_IE11

#define ACCSSVENDOR_OPERA "o"
#define ACCSSVENDOR_MOZILLA "m"
#define ACCSSVENDOR_WEBKIT "w"
#define ACCSSVENDOR_MICROSOFT "i"
#define ACCSSVENDOR_EPUB "e"
#define ACCSSVENDOR_APPLE "a"
#define ACCSSVENDOR_XV "x"
#define ACCSSVENDOR_WAP "p"
#define ACCSSVENDOR_RIM "r"
#define ACCSSVENDOR_KHTML "k"

struct analyze
{
    struct astnode** eq;
    struct astnode** ne1;
    struct astnode** ne2;
};

struct shortSide
{
    char* s;
    char imp;
    struct astnode** t;
};

struct shortHand
{
    char invalid;
    char* name;

    char imp;

    struct sides
    {
        struct shortSide* top;
        struct shortSide* left;
        struct shortSide* bottom;
        struct shortSide* right;
    } sides;
};

struct prop
{
    struct astnode* block;
    char imp;
    char* id;
    char** sg;
    char freeze;
    char* path;
    char* freezeID;
    char* pseudoID;
};

struct pchar_shorts
{
    char* key;
    struct shortHand** list;
};

struct pchar_prop
{
    char* key;
    struct prop* prop;
};

struct compdeps
{
    unsigned char compat;
    unsigned char mergesplitted;
    struct pchar_shorts** shorts2;
    struct pchar_prop** props2;
    size_t shortGroupID;
    char*  lastShortSelector;
    size_t lastShortGroupID;
};

struct astnode* compress(struct astnode* tree, unsigned char restructure, unsigned char mergesplitted, unsigned char compat);

#endif

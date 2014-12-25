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

#include <stdlib.h>
 
#ifndef accss_gettokens_h
#define accss_gettokens_h

struct char_char
{
    char c;
    char type;
};

struct int_prev
{
    int pos;
    struct int_prev* prev;
};

struct token
{
    size_t tn;
    int ln;
    unsigned char type;
    char* value;
    size_t left;
    size_t right;
    
    char ws;
    char sc;
    size_t ws_last;
    size_t sc_last;
    
    size_t ruleset_l;
    size_t selector_end;
    size_t nth_last;
    size_t nthf_last;
    size_t ident_last;
    size_t clazz_l;
    size_t nm_name_last;
    size_t atrule_l;
    size_t atrulers_end;
    size_t number_l;
    size_t filterp_l;
    size_t last_progid;
    size_t progid_end;
    
    char bd_type;
    char bd_filter;
    char bd_decl;
    char atrule_type;
};

struct token_info
{
    struct token** list;
    size_t len;
};

struct token_info getTokens(const char* string, char* error);

struct token* getTokenByIndex(struct token_info* info, size_t index);

void deleteTokens(struct token_info* info);

#endif

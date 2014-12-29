/*
 * ACCSS C Code Copyright (C) 2014 by Gandalf Sievers
 * Created by Gandalf Sievers on 23.08.14.
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
#include <limits.h>

#ifndef accss_helper_h
#define accss_helper_h

#ifndef SIZE_MAX
#define SIZE_MAX ((size_t)-1)
#define SIZECASTTYPE unsigned long
#define SIZEPRI "%lu"
#else
#if ULONG_MAX < SIZE_MAX
#define SIZECASTTYPE unsigned long long
#define SIZEPRI "%llu"
#else
#define SIZECASTTYPE unsigned long
#define SIZEPRI "%lu"
#endif
#endif

void memoryFailure();
size_t casecmp(const char* str1, const char* str2);
char* resizeValue(char* old, const char* new);
char* copyValue(const char* value);
char** copyCharList(char** charList);
void freeCharList(char** charList);
size_t charListLength(char** charList);
char* lowerCase(char* string);

#endif

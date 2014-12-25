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

#ifndef accss_tokentypes_h
#define accss_tokentypes_h

#define TOKENTYPE_STRINGSQ 1
#define TOKENTYPE_STRINGDQ 2
#define TOKENTYPE_COMMENTML 3
#define TOKENTYPE_COMMENTSL 4

#define TOKENTYPE_NEWLINE 5
#define TOKENTYPE_SPACE 6
#define TOKENTYPE_TAB 7

#define TOKENTYPE_EXCLAMATIONMARK 8         /*  !  */
#define TOKENTYPE_QUOTATIONMARK 9           /*  "  */
#define TOKENTYPE_NUMBERSIGN 10             /*  #  */
#define TOKENTYPE_DOLLARSIGN 11             /*  $  */
#define TOKENTYPE_PERCENTSIGN 12            /*  %  */
#define TOKENTYPE_AMPERSAND 13              /*  &  */
#define TOKENTYPE_APOSTROPHE 14             /*  '  */
#define TOKENTYPE_LEFTPARENTHESIS 15        /*  (  */
#define TOKENTYPE_RIGHTPARENTHESIS 16       /*  )  */
#define TOKENTYPE_ASTERISK 17               /*  *  */
#define TOKENTYPE_PLUSSIGN 18               /*  +  */
#define TOKENTYPE_COMMA 19                  /*  ,  */
#define TOKENTYPE_HYPHENMINUS 20            /*  -  */
#define TOKENTYPE_FULLSTOP 21               /*  .  */
#define TOKENTYPE_SOLIDUS 22                /*  /  */
#define TOKENTYPE_COLON 23                  /*  :  */
#define TOKENTYPE_SEMICOLON 24              /*  ;  */
#define TOKENTYPE_LESSTHANSIGN 25           /*  <  */
#define TOKENTYPE_EQUALSSIGN 26             /*  =  */
#define TOKENTYPE_GREATERTHANSIGN 27        /*  >  */
#define TOKENTYPE_QUESTIONMARK 28           /*  ?  */
#define TOKENTYPE_COMMERCIALAT 29           /*  @  */
#define TOKENTYPE_LEFTSQUAREBRACKET 30      /*  [  */
#define TOKENTYPE_REVERSESOLIDUS 31         /* '\' */
#define TOKENTYPE_RIGHTSQUAREBRACKET 32     /*  ]  */
#define TOKENTYPE_CIRCUMFLEXACCENT 33       /*  ^  */
#define TOKENTYPE_LOWLINE 34                /*  _  */
#define TOKENTYPE_LEFTCURLYBRACKET 35       /*  {  */
#define TOKENTYPE_VERTICALLINE 36           /*  |  */
#define TOKENTYPE_RIGHTCURLYBRACKET 37      /*  }  */
#define TOKENTYPE_TILDE 38                  /*  ~  */

#define TOKENTYPE_IDENTIFIER 39
#define TOKENTYPE_DECIMALNUMBER 40

#endif

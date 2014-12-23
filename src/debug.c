/*
 * ACCSS C Code Copyright (C) 2014 by Gandalf Sievers
 * Created by Gandalf Sievers on 07.08.14.
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

#include "debug.h"

const char* ttypes[] = {
    "TOKENTYPE_UNINITIALIZED",
    "TOKENTYPE_STRINGSQ",
    "TOKENTYPE_STRINGDQ",
    "TOKENTYPE_COMMENTML",
    "TOKENTYPE_COMMENTSL",
    "TOKENTYPE_NEWLINE",
    "TOKENTYPE_SPACE",
    "TOKENTYPE_TAB",
    "TOKENTYPE_EXCLAMATIONMARK",
    "TOKENTYPE_QUOTATIONMARK",
    "TOKENTYPE_NUMBERSIGN",
    "TOKENTYPE_DOLLARSIGN",
    "TOKENTYPE_PERCENTSIGN",
    "TOKENTYPE_AMPERSAND",
    "TOKENTYPE_APOSTROPHE",
    "TOKENTYPE_LEFTPARENTHESIS",
    "TOKENTYPE_RIGHTPARENTHESIS",
    "TOKENTYPE_ASTERISK",
    "TOKENTYPE_PLUSSIGN",
    "TOKENTYPE_COMMA",
    "TOKENTYPE_HYPHENMINUS",
    "TOKENTYPE_FULLSTOP",
    "TOKENTYPE_SOLIDUS",
    "TOKENTYPE_COLON",
    "TOKENTYPE_SEMICOLON",
    "TOKENTYPE_LESSTHANSIGN",
    "TOKENTYPE_EQUALSSIGN",
    "TOKENTYPE_GREATERTHANSIGN",
    "TOKENTYPE_QUESTIONMARK",
    "TOKENTYPE_COMMERCIALAT",
    "TOKENTYPE_LEFTSQUAREBRACKET",
    "TOKENTYPE_REVERSESOLIDUS",
    "TOKENTYPE_RIGHTSQUAREBRACKET",
    "TOKENTYPE_CIRCUMFLEXACCENT",
    "TOKENTYPE_LOWLINE",
    "TOKENTYPE_LEFTCURLYBRACKET",
    "TOKENTYPE_VERTICALLINE",
    "TOKENTYPE_RIGHTCURLYBRACKET",
    "TOKENTYPE_TILDE",
    "TOKENTYPE_IDENTIFIER",
    "TOKENTYPE_DECIMALNUMBER"};

const char* astnodetypes[] = {
    "ACCSSNODETYPE_UNINITIALIZED",
    "ACCSSNODETYPE_IDENT",
    "ACCSSNODETYPE_ATKEYWORD",
    "ACCSSNODETYPE_STRING",
    "ACCSSNODETYPE_SHASH",
    "ACCSSNODETYPE_VHASH",
    "ACCSSNODETYPE_NUMBER",
    "ACCSSNODETYPE_PERCENTAGE",
    "ACCSSNODETYPE_DIMENSION",
    "ACCSSNODETYPE_CDO",
    "ACCSSNODETYPE_CDC",
    "ACCSSNODETYPE_DECLDELIM",
    "ACCSSNODETYPE_S",
    "ACCSSNODETYPE_ATTRSELECTOR",
    "ACCSSNODETYPE_ATTRIB",
    "ACCSSNODETYPE_NTH",
    "ACCSSNODETYPE_NTHSELECTOR",
    "ACCSSNODETYPE_NAMESPACE",
    "ACCSSNODETYPE_CLAZZ",
    "ACCSSNODETYPE_PSEUDOE",
    "ACCSSNODETYPE_PSEUDOC",
    "ACCSSNODETYPE_DELIM",
    "ACCSSNODETYPE_STYLESHEET",
    "ACCSSNODETYPE_ATRULEB",
    "ACCSSNODETYPE_ATRULES",
    "ACCSSNODETYPE_ATRULERQ",
    "ACCSSNODETYPE_ATRULERS",
    "ACCSSNODETYPE_ATRULER",
    "ACCSSNODETYPE_BLOCK",
    "ACCSSNODETYPE_RULESET",
    "ACCSSNODETYPE_COMBINATOR",
    "ACCSSNODETYPE_SIMPLESELECTOR",
    "ACCSSNODETYPE_SELECTOR",
    "ACCSSNODETYPE_DECLARATION",
    "ACCSSNODETYPE_PROPERTY",
    "ACCSSNODETYPE_IMPORTANT",
    "ACCSSNODETYPE_UNARY",
    "ACCSSNODETYPE_OPERATOR",
    "ACCSSNODETYPE_BRACES",
    "ACCSSNODETYPE_VALUE",
    "ACCSSNODETYPE_PROGID",
    "ACCSSNODETYPE_FILTERV",
    "ACCSSNODETYPE_FILTER",
    "ACCSSNODETYPE_COMMENT",
    "ACCSSNODETYPE_URI",
    "ACCSSNODETYPE_RAW",
    "ACCSSNODETYPE_FUNCTIONBODY",
    "ACCSSNODETYPE_FUNCTION",
    "ACCSSNODETYPE_FUNCTIONEXPRESSION",
    "ACCSSNODETYPE_UNKNOWN" };

void printASTNode(struct astnode* node, int depth)
{
    int i =0;
    for(; i<depth; i++)
    {
        printf("\t");
    }
    printf("[%s", astnodetypes[node->type]);
    if(node->content != NULL)
    {
        printf(" \"%s\"",node->content);
    }

    depth++;
    if(node->children != NULL)
    {
        printf("\n");
        struct astnode** tmp = node->children;

        while(*tmp != NULL)
        {
            printASTNode(*tmp, depth);
            tmp++;
        }
    }

    int k =0;
    for(; k<depth; k++)
    {
        printf("\t");
    }

    printf("]\n");
}

void printASTNodeJSON(struct astnode* node, int depth)
{
    printf("{");

    printf("'type' : '%s'", astnodetypes[node->type]);

    if(node->info != NULL)
    {
        printf(", 'info' : {");
        if(node->info->freezeID != NULL)
        {
            printf("'freezeID' : '%s'", node->info->freezeID);
        }

        if(node->info->pseudoID != NULL)
        {
            if(node->info->freezeID != NULL)
            {
                printf(", ");
            }

            printf("'pseudoID' : '%s'", node->info->pseudoID);
        }
        if(node->info->pseudoSignature != NULL)
        {
            if(node->info->freezeID != NULL || node->info->pseudoID != NULL)
            {
                printf(", ");
            }

            printf("'pseudoSignature' : '%s'", node->info->pseudoSignature);
        }

        if(node->info->freezeID != NULL || node->info->pseudoID != NULL || node->info->pseudoSignature != NULL)
        {
            printf(", ");
        }

        printf("'freeze' : '%i', ", node->info->freeze);
        printf("'pseudo' : '%i', ",  node->info->pseudo);;
        printf("'replaceByShort' : '%i', ",  node->info->replaceByShort);
        printf("'removeByShort' : '%i',", node->info->removeByShort);
        printf("'shortHandKey' : { 'i' : '%lu', 'key' : '%s' }", node->info->shortHandKey.i, node->info->shortHandKey.key != NULL ? node->info->shortHandKey.key : "");

        if(node->info->sg != NULL)
        {
            printf(", 'sg' : [");
            char** sg1 = node->info->sg;

            while(*sg1 != NULL)
            {
                if(sg1 != node->info->sg)
                {
                    printf(", ");
                }

                printf("'%s'", *sg1);
                sg1++;
            }
            printf("]");
        }
        printf("}");
    }

    if(node->content != NULL)
    {
        printf(", 'content' : '%s'", node->content);
    }

    if(node->s != NULL)
    {
        printf(", 's' : '%s'", node->s);
    }

    depth++;
    if(node->children != NULL)
    {
        printf(", 'children' : [");
        struct astnode** tmp = node->children;

        while(*tmp != NULL)
        {
            if(tmp != node->children)
            {
                printf(",");
            }

            printASTNodeJSON(*tmp, depth);
            tmp++;
        }
        printf("]");
    }

    printf("}");
}

void printASTList(struct astnode** list, char* name)
{
    struct astnode** tmp = list;
    printf("======= %s AST List =======\n\n", name);
    while((*tmp)->type != 0)
    {
        printASTNode(*tmp, 0);
        tmp++;
    }
}

void printTokens(struct token_info* info)
{
    size_t i = 0;
    for(; i < info->len; i++)
    {
        struct token* tmp = info->list[i];
        printf("%lu LN:%i Type:%s Right:%zu Value:%s  \n", i, tmp->ln, ttypes[tmp->type], tmp->right, tmp->value);
    }
}

void printSide(struct shortSide* side)
{
    if(side != NULL)
    {
        printf("{'imp' : '%i'", side->imp);

        if((side->t != NULL) || (side->t != NULL))
        {
            printf(", ");
        }

        if(side->s != NULL)
        {
            printf("'s' : '%s'", side->s);
        }

        if(side->t != NULL)
        {
            if(side->s != NULL)
            {
                printf(", ");
            }

            printf("'t' : ");
            size_t i = 0, ilen = listLength(side->t);
            for(; i<ilen; i++)
            {
                printASTNodeJSON(side->t[i], 0);
            }

        }
        printf("}");
    }
    else
    {
        printf("''");
    }
}

void printShortList(struct shortHand** sList)
{
    while(*sList != NULL)
    {
        struct shortHand* sh = *sList;
        sList++;

        printf("{");
        if(sh->name != NULL)
        {
            printf("'name' : '%s', ", sh->name);
        }

        printf("'invalid' : '%i', 'imp' : '%i', 'sides' : {", sh->invalid, sh->imp);
        printf("'top' : ");
        printSide(sh->sides.top);
        printf(", 'right' : ");
        printSide(sh->sides.right);
        printf(", 'bottom' : ");
        printSide(sh->sides.bottom);
        printf(", 'left' : ");
        printSide(sh->sides.left);
        printf("}}");

        if(*sList != NULL)
        {
            printf(", ");
        }
    }
}

void printShortHands(struct pchar_shorts** list)
{
    if(list != NULL)
    {
        struct pchar_shorts** psTmp = list;

        printf("[");
        while(*psTmp != NULL)
        {
            struct pchar_shorts* psCurrent = *psTmp;
            psTmp++;
            printf("{");
            if(psCurrent->key != NULL)
            {
                printf("'key' : '%s', ", psCurrent->key);
            }

            printf("'list' : [");
            struct shortHand** sList = psCurrent->list;
            if(sList != NULL)
            {
                printShortList(sList);
            }

            printf("]}");
            if(*psTmp != NULL)
            {
                printf(", ");
            }
        }
        printf("]");
    }
}

/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").  
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright(c) 2001 Sun Microsystems, Inc.
 * All rights reserved.
 */
#ident "@(#)pc-iscii_to_iscii91.c    1.1 01/11/13 SMI"

#include <stdio.h>
#include <errno.h>
#include "pc-iscii.h"

#define MSB          0x80
#define REPLACE_CHAR '?'

typedef struct _icv_state {
    int dummy;
} _iconv_st;

static uchar
traverse_table(Entry *entry , int num, uchar pc_iscii)
{
    int   i=0;
    uchar iscii=0;

    for ( ; i < num; ++i) {
        Entry en = entry[i];

        if ( pc_iscii < en.pc_iscii ) break;
        if ( pc_iscii >= en.pc_iscii && pc_iscii < en.pc_iscii + en.count ) {
            iscii = (pc_iscii - en.pc_iscii) + en.iscii;
            break;
        }
    }

    return iscii;
}

void *
_icv_open()
{
    _iconv_st *st;

    if ((st = (_iconv_st*)malloc(sizeof(_iconv_st))) == NULL) {
        errno = ENOMEM;
        return ((void*)-1);
    }

    bzero(st, sizeof(_iconv_st));

    return ((void*)st);
}

/*
 * Close; called from iconv_close()
 */
void
_icv_close(_iconv_st *st)
{
    if (!st)
        errno = EBADF;
    else
        free(st);
}

size_t
_icv_iconv(_iconv_st *st, char **inbuf, size_t *inbytesleft,
       char **outbuf, size_t *outbytesleft)
{
    if (st == NULL) {
        errno = EBADF;
        return ((size_t) -1);
    }

    if (inbuf == NULL || *inbuf == NULL) { /* Reset request. */
        return ((size_t)0);
    }

    /* a state machine for interpreting ISCII code */
    while (*inbytesleft > 0 && *outbytesleft > 0) {
        uchar c = (uchar)**inbuf;

        if ( c & MSB ) {
            uchar iscii = traverse_table(pciscii_isc_tbl,
                    sizeof(pciscii_isc_tbl)/sizeof(Entry), c);
            if ( iscii ) **outbuf = iscii;
            else **outbuf = REPLACE_CHAR;
        } else { /* ASCII */
            **outbuf = c;
        }

        (*inbuf)++;
        (*inbytesleft)--;
        (*outbuf)++;
        (*outbytesleft)--;
    }

    if ( *inbytesleft > 0 && *outbytesleft == 0 ) {
         errno = E2BIG;
         return ((size_t)-1);
    }

    return ((size_t)(*inbytesleft));
}


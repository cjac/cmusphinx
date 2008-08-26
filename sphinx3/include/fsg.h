/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 1999-2004 Carnegie Mellon University.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */


#ifndef __S2_FSG_H__
#define __S2_FSG_H__

#include "s3types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Structures through which an application may load an FSG into the decoder.
 * There's essentially a 1-to-1 correspondence between the FSG file format
 * and these structures.
 */
typedef struct s2_fsg_trans_s {
    int32 from_state;
    int32 to_state;
    float32 prob;                 /* Probability associated with transition */
    char *word;                   /* NULL for null transitions */
    struct s2_fsg_trans_s *next;  /* For linking together all transitions in FSG */
} s2_fsg_trans_t;

typedef struct s2_fsg_s {
    char *name;                   /* This would be the name on the FSG_BEGIN line
                                     in an FSG file.  Can be NULL or "" for unnamed
                                     FSGs */
    int32 n_state;                /* Set of states = 0 .. n_state-1 */
    int32 start_state;            /* 0 <= start_state < n_state */
    int32 final_state;            /* 0 <= final_state < n_state */
    s2_fsg_trans_t *trans_list;   /* Null-terminated list of transitions in FSG,
                                     in no particular order */
} s2_fsg_t;

#ifdef __cplusplus
}
#endif
#endif

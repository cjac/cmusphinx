/* ====================================================================
 * Copyright (c) 1993-2000 Carnegie Mellon University.  All rights 
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
 * 3. The names "Sphinx" and "Carnegie Mellon" must not be used to
 *    endorse or promote products derived from this software without
 *    prior written permission. To obtain permission, contact 
 *    sphinx@cs.cmu.edu.
 *
 * 4. Products derived from this software may not be called "Sphinx"
 *    nor may "Sphinx" appear in their names without prior written
 *    permission of Carnegie Mellon University. To obtain permission,
 *    contact sphinx@cs.cmu.edu.
 *
 * 5. Redistributions of any form whatsoever must retain the following
 *    acknowledgment:
 *    "This product includes software developed by Carnegie
 *    Mellon University (http://www.speech.cs.cmu.edu/)."
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

#ifndef _PHONE_H_
#define _PHONE_H_

#define NO_PHONE	-1

#define PT_CIPHONE	0		/* Context independent */
#define PT_CDPHONE	-1		/* Context dependent */
#define PT_WWPHONE	-2		/* With in Word */
#define PT_DIPHONE	-3		/* DiPhone */
#define PT_DIPHONE_S	-4		/* DiPhone Singleton */
#define PT_CDDPHONE	-99		/* Context dependent duration */
#define PT_WWCPHONE	1000		/* With in Word Component phone */

int phone_read(char *filename);
void phone_add_diphones(void);

/* TODO: 'extern inline' most of these if GNU C or C99 is in effect */
int32 phone_to_id(char const *phone_str, int verbose);
char const *phone_from_id(int32 phone_id);
int32 phone_id_to_base_id(int32 phone_id);
int32 phone_type(int32 phone_id);
int32 phone_len(int32 phone_id);
int32 phone_count(void);
int32 phoneCiCount (void);
int32 phoneWdCount (void);
int32 phone_map (int32 pid);
list_t *phoneList (void);


#endif /* _PHONE_H_ */

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
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
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

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <assert.h>

#include "pio.h"
#include "err.h"
#include "ckd_alloc.h"

FILE *
fopen_comp(const char *file, const char *mode, int32 * ispipe)
{
    FILE *fp;

#ifndef HAVE_POPEN
    *ispipe = 0; /* No popen() on WinCE */
#else /* HAVE_POPEN */
    int32 k, isgz;
    k = strlen(file);
#if defined(WIN32)
    *ispipe = (k > 3) && ((strcmp(file + k - 3, ".gz") == 0)
                          || (strcmp(file + k - 3, ".GZ") == 0));
    isgz = *ispipe;
#else
    *ispipe = 0;
    isgz = 0;
    if ((k > 2)
        && ((strcmp(file + k - 2, ".Z") == 0)
            || (strcmp(file + k - 2, ".z") == 0))) {
        *ispipe = 1;
    }
    else {
        if ((k > 3) && ((strcmp(file + k - 3, ".gz") == 0)
                        || (strcmp(file + k - 3, ".GZ") == 0))) {
            *ispipe = 1;
            isgz = 1;
        }
    }
#endif /* NOT WIN32 */
#endif /* HAVE_POPEN */

    if (*ispipe) {
#ifndef HAVE_POPEN
        /* Shouldn't get here, anyway */
        E_FATAL("No popen() on WinCE\n");
#else
        char command[16384];
#if defined(WIN32) 
        if (strcmp(mode, "r") == 0) {
            sprintf(command, "gzip.exe -d -c %s", file);
            if ((fp = _popen(command, mode)) == NULL) {
                E_ERROR_SYSTEM("_popen (%s,%s) failed\n", command, mode);
                return NULL;
            }
        }
        else if (strcmp(mode, "w") == 0) {
            sprintf(command, "gzip.exe > %s", file);

            if ((fp = _popen(command, mode)) == NULL) {
                E_ERROR_SYSTEM("_popen (%s,%s) failed\n", command, mode);
                return NULL;
            }
        }
        else {
            E_ERROR("fopen_comp not implemented for mode = %s\n", mode);
            return NULL;
        }
#else
        if (strcmp(mode, "r") == 0) {
            if (isgz)
                sprintf(command, "gunzip -c %s", file);
            else
                sprintf(command, "zcat %s", file);

            if ((fp = popen(command, mode)) == NULL) {
                E_ERROR_SYSTEM("popen (%s,%s) failed\n", command, mode);
                return NULL;
            }
        }
        else if (strcmp(mode, "w") == 0) {
            if (isgz)
                sprintf(command, "gzip > %s", file);
            else
                sprintf(command, "compress -c > %s", file);

            if ((fp = popen(command, mode)) == NULL) {
                E_ERROR_SYSTEM("popen (%s,%s) failed\n", command, mode);
                return NULL;
            }
        }
        else {
            E_ERROR("fopen_comp not implemented for mode = %s\n", mode);
            return NULL;
        }
#endif /* NOT WIN32 */
#endif /* HAVE_POPEN */
    }
    else {
        fp = fopen(file, mode);
    }

    return (fp);
}


void
fclose_comp(FILE * fp, int32 ispipe)
{
    if (ispipe) {
#ifdef HAVE_POPEN
#if defined(WIN32)
        _pclose(fp);
#else
        pclose(fp);
#endif
#endif
    }
    else
        fclose(fp);
}


FILE *
fopen_compchk(const char *file, int32 * ispipe)
{
#ifndef HAVE_POPEN
    *ispipe = 0; /* No popen() on WinCE */
    /* And therefore the rest of this function is useless. */
    return (fopen_comp(file, "r", ispipe));
#else /* HAVE_POPEN */
    char tmpfile[16384];
    int32 k, isgz;
    struct stat statbuf;

    k = strlen(file);

#if defined(WIN32)
    *ispipe = (k > 3) && ((strcmp(file + k - 3, ".gz") == 0)
                          || (strcmp(file + k - 3, ".GZ") == 0));
    isgz = *ispipe;
#else
    *ispipe = 0;
    isgz = 0;
    if ((k > 2)
        && ((strcmp(file + k - 2, ".Z") == 0)
            || (strcmp(file + k - 2, ".z") == 0))) {
        *ispipe = 1;
    }
    else {
        if ((k > 3) && ((strcmp(file + k - 3, ".gz") == 0)
                        || (strcmp(file + k - 3, ".GZ") == 0))) {
            *ispipe = 1;
            isgz = 1;
        }
    }
#endif

    strcpy(tmpfile, file);
    if (stat(tmpfile, &statbuf) != 0) {
        /* File doesn't exist; try other compressed/uncompressed form, as appropriate */
        E_ERROR_SYSTEM("stat(%s) failed\n", tmpfile);

        if (*ispipe) {
            if (isgz)
                tmpfile[k - 3] = '\0';
            else
                tmpfile[k - 2] = '\0';

            if (stat(tmpfile, &statbuf) != 0)
                return NULL;
        }
        else {
            strcpy(tmpfile + k, ".gz");
            if (stat(tmpfile, &statbuf) != 0) {
#if (! WIN32)
                strcpy(tmpfile + k, ".Z");
                if (stat(tmpfile, &statbuf) != 0)
                    return NULL;
#else
                return NULL;
#endif
            }
        }

        E_WARN("Using %s instead of %s\n", tmpfile, file);
    }

    return (fopen_comp(tmpfile, "r", ispipe));
#endif /* HAVE_POPEN */
}

lineiter_t *
lineiter_start(FILE *fh)
{
    lineiter_t *li;

    li = ckd_calloc(1, sizeof(*li));
    li->buf = ckd_malloc(128);
    li->buf[0] = '\0';
    li->bsiz = 128;
    li->len = 0;
    li->fh = fh;

    return lineiter_next(li);
}

lineiter_t *
lineiter_next(lineiter_t *li)
{
    /* Read a line and check for EOF. */
    if (fgets(li->buf, li->bsiz, li->fh) == NULL) {
        lineiter_free(li);
        return NULL;
    }
    /* If we managed to read the whole thing, then we are done
     * (this will be by far the most common result). */
    li->len = strlen(li->buf);
    if (li->len < li->bsiz - 1 || li->buf[li->len - 1] == '\n')
        return li;

    /* Otherwise we have to reallocate and keep going. */
    while (1) {
        li->bsiz *= 2;
        li->buf = ckd_realloc(li->buf, li->bsiz);
        /* If we get an EOF, we are obviously done. */
        if (fgets(li->buf + li->len, li->bsiz - li->len, li->fh) == NULL) {
            li->len += strlen(li->buf + li->len);
            return li;
        }
        li->len += strlen(li->buf + li->len);
        /* If we managed to read the whole thing, then we are done. */
        if (li->len < li->bsiz - 1 || li->buf[li->len - 1] == '\n')
            return li;
    }

    /* Shouldn't get here. */
    return li;
}

void
lineiter_free(lineiter_t *li)
{
    ckd_free(li->buf);
    ckd_free(li);
}

char *
fread_line(FILE *stream, size_t *out_len)
{
    char *output, *outptr;
    char buf[128];

    output = outptr = NULL;
    while (fgets(buf, sizeof(buf), stream)) {
        size_t len = strlen(buf);
        /* Append this data to the buffer. */
        if (output == NULL) {
            output = ckd_malloc(len + 1);
            outptr = output;
        }
        else {
            size_t cur = outptr - output;
            output = ckd_realloc(output, cur + len + 1);
            outptr = output + cur;
        }
        memcpy(outptr, buf, len + 1);
        outptr += len;
        /* Stop on a short read or end of line. */
        if (len < sizeof(buf)-1 || buf[len-1] == '\n')
            break;
    }
    if (out_len) *out_len = outptr - output;
    return output;
}


#define FREAD_RETRY_COUNT	60

int32
fread_retry(void *pointer, int32 size, int32 num_items, FILE * stream)
{
    char *data;
    uint32 n_items_read;
    uint32 n_items_rem;
    uint32 n_retry_rem;
    int32 loc;

    n_retry_rem = FREAD_RETRY_COUNT;

    data = pointer;
    loc = 0;
    n_items_rem = num_items;

    do {
        n_items_read = fread(&data[loc], size, n_items_rem, stream);

        n_items_rem -= n_items_read;

        if (n_items_rem > 0) {
            /* an incomplete read occurred */

            if (n_retry_rem == 0)
                return -1;

            if (n_retry_rem == FREAD_RETRY_COUNT) {
                E_ERROR_SYSTEM("fread() failed; retrying...\n");
            }

            --n_retry_rem;

            loc += n_items_read * size;
#ifdef HAVE_UNISTD_H
            sleep(1);
#endif
        }
    } while (n_items_rem > 0);

    return num_items;
}


/* Silvio Moioli: updated to use Unicode */
#ifdef _WIN32_WCE /* No stat() on WinCE */
int32
stat_retry(const char *file, struct stat * statbuf)
{
    WIN32_FIND_DATAW file_data;
    HANDLE *h;
    wchar_t *wfile;
    size_t len;

    len = mbstowcs(NULL, file, 0) + 1;
    wfile = ckd_calloc(len, sizeof(*wfile));
    mbstowcs(wfile, file, len);
    if ((h = FindFirstFileW(wfile, &file_data)) == INVALID_HANDLE_VALUE) {
        ckd_free(wfile);
        return -1;
    }
    ckd_free(wfile);
    memset(statbuf, 0, sizeof(statbuf));
    statbuf->st_mtime = file_data.ftLastWriteTime.dwLowDateTime;
    statbuf->st_size = file_data.nFileSizeLow;
    FindClose(h);

    return 0;
}


int32
stat_mtime(const char *file)
{
    struct stat statbuf;

    if (stat_retry(file, &statbuf) != 0)
        return -1;

    return ((int32) statbuf.st_mtime);
}
#else
#define STAT_RETRY_COUNT	10
int32
stat_retry(const char *file, struct stat * statbuf)
{
    int32 i;

    
    
    for (i = 0; i < STAT_RETRY_COUNT; i++) {

#ifndef HAVE_SYS_STAT_H
		FILE *fp;

		if ((fp=(FILE *)fopen(file, "r"))!= 0)
		{
		    fseek( fp, 0, SEEK_END);
		    statbuf->st_size = ftell( fp );
		    fclose(fp);
		    return 0;
		}
	
#else /* HAVE_SYS_STAT_H */
        if (stat(file, statbuf) == 0)
            return 0;
#endif
        if (i == 0) {
            E_ERROR_SYSTEM("stat(%s) failed; retrying...\n", file);
        }
#ifdef HAVE_UNISTD_H
        sleep(1);
#endif
    }

    return -1;
}

int32
stat_mtime(const char *file)
{
    struct stat statbuf;

#ifdef HAVE_SYS_STAT_H
    if (stat(file, &statbuf) != 0)
        return -1;
#else /* HAVE_SYS_STAT_H */
    if (stat_retry(file, &statbuf) != 0)
        return -1;
#endif /* HAVE_SYS_STAT_H */

    return ((int32) statbuf.st_mtime);
}
#endif /* !_WIN32_WCE */

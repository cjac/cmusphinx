
#ifndef __SPHINX3_CLASSIFIER_H
#define __SPHINX3_CLASSIFIER_H

typedef struct {
} classifier_t;

int cl_init(classifier_t *);
int cl_classify_ceps(classifier_t *, float32 **, char **);
int cl_finish(classifier_t *);

#endif

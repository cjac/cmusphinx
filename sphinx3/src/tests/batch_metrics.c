/* ====================================================================
 * Copyright (c) 1999-2001 Carnegie Mellon University.  All rights
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

/********************************************************************
 *
 * Adapted from src/programs/main_live_pretend.c
 *
 * Collects performance metrics, e.g., accuracy, runtimes
 *
 ********************************************************************/

#include <stdio.h>
#include <string.h>
#include <libutil/libutil.h>
#include <libutil/profile.h>
#include <libs3decoder/new_fe.h>
#include "live_dump.h"
#include "metrics.h"


#define MAXSAMPLES 	1000000
#define STRLEN          512
#define MAX_REFERENCES  50


void showAccuracy(FILE *rfp);
void showTiming(FILE *ftp, const char* name, 
                double audioTime, double processingTime);
void showMemory(FILE *rfp);
void analyzeResults(const char* referenceResult, const char* parthyp);
void addMatch();
void addInsert();
void addDeletion();
void addRecognitionError();
void processMismatch(char *references[], int numReferences,
                     char *parthyp[], int numHypothesis);
int countMatches(char *references[], int r, int numReferences,
                 char *parthyp[], int h, int numHypothesis);
int stringToArray(char *string, char *array[]);
void partialHypToString(partialhyp_t *parthyp, int nhypwds, 
                        char* hypothesis, int bufferSize);


int numSentences;
int numRefWords;
int numHypWords;
int numMatchingWords;
int numMatchingSentences;
int recognitionErrors;
int insertionErrors;
int deletionErrors;

int referenceIndex;
int hypothesisIndex;


int main (int argc, char *argv[])
{
    short *samps;

    int  i, buflen, endutt, blksize, nhypwds, nsamp;
    int numberFiles;
    int space, lastChar;
    
    double sampleRate;
    double totalAudioTime;
    double totalProcessingTime;
    double audioTime;
    double processingTime;

    char   *argsfile, *ctlfile, *indir, *filename, *referenceResult;
    char   cepfile[STRLEN];
    char   line[STRLEN], hypothesis[STRLEN];
    char   *word;
    char   *fileTimer = "file";

    partialhyp_t *parthyp;
    FILE *fp, *sfp, *rfp;

    space = ' ';
    numberFiles = 0;
    sampleRate = 8000;
    endutt = 0;

    totalAudioTime = 0.0;
    totalProcessingTime = 0.0;
    audioTime = 0.0;
    processingTime = 0.0;


    numSentences = 0;
    numRefWords = 0;
    numHypWords = 0;
    numMatchingWords = 0;
    numMatchingSentences = 0;
    recognitionErrors = 0;
    insertionErrors = 0;
    deletionErrors = 0;


    if (argc != 4) {
        E_FATAL("\nUSAGE: %s <ctlfile> <inrawdir> <argsfile>\n",
                argv[0]);
    }
    
    ctlfile = argv[1]; 
    indir = argv[2]; 
    argsfile = argv[3];

    samps = (short *) calloc(MAXSAMPLES,sizeof(short));
    blksize = 2000;

    if ((fp = fopen(ctlfile,"r")) == NULL)
	E_FATAL("Unable to read %s\n",ctlfile);

    rfp = stdout;

    fprintf(rfp, "BatchDecoder: decoding files in %s\n----------\n", ctlfile);

    live_initialize_decoder(argsfile);

    while (fgets(line, STRLEN, fp) != NULL) {

        /* Parse the speech file and the reference result. */
        referenceResult = strchr(line, space);
        if (referenceResult == NULL) {
            E_FATAL("No reference result\n", cepfile);
        } else {
            referenceResult++;
        }
        lastChar = strlen(referenceResult) - 1;
        if (referenceResult[lastChar] == '\n') {
            referenceResult[lastChar] = '\0';
        }
        filename = strtok(line, " ");

        numberFiles++;
        nhypwds = 0;

	sprintf(cepfile,"%s/%s",indir,filename);

        fprintf(rfp, "\nDecoding: %s\n\n", cepfile);

	if ((sfp = fopen(cepfile,"rb")) == NULL)
	    E_FATAL("Unable to read %s\n",cepfile);
        
        nsamp = fread(samps, sizeof(short), MAXSAMPLES, sfp);
        
        fflush(stdout); 
        fclose(sfp);

	if (needswap) {
	  for (i = 0; i < nsamp; i++) {
	    SWAPW(samps + i);
	  }
	}

        metricsReset(fileTimer);
        metricsStart(fileTimer);

        for (i = 0; i < nsamp; i += blksize) {

	    buflen = i+blksize < nsamp ? blksize : nsamp-i;
	    endutt = i+blksize <= nsamp-1 ? 0 : 1;
	    nhypwds = live_utt_decode_block(samps+i,buflen,endutt,&parthyp);

        }

        metricsStop(fileTimer);

        if (endutt && nhypwds > 0) {

            /* convert the hypothesis into a string */
            partialHypToString(parthyp, nhypwds, hypothesis, STRLEN);
            
            /* 
             * If hypothesis is equal to referenceResult
             * increment the number of matches
             */
            fprintf(rfp, "REF:  %s\n", referenceResult);
            fprintf(rfp, "HYP:  %s\n", hypothesis);
            
            analyzeResults(referenceResult, hypothesis);
            showAccuracy(rfp);
        }

        /* collect the processing times data */
        processingTime = metricsDuration(fileTimer);
        audioTime = ((double) nsamp) / sampleRate;
        totalProcessingTime += processingTime;
        totalAudioTime += audioTime;

        showTiming(rfp, "This", audioTime, processingTime);
        showTiming(rfp, "Total", totalAudioTime, totalProcessingTime);
        showMemory(rfp);
        fprintf(rfp, "# --------------\n");
    }

    live_print_profiles(rfp);

    metricsPrint();

    fprintf(rfp, "# ------------- Summary statistics -----------\n");
    showAccuracy(rfp);
    showTiming(rfp, "Total", totalAudioTime, totalProcessingTime);
    showMemory(rfp);

    return 0;
}


/**
 * Converts the words in the given partialhyp_t struct into a string.
 */
void partialHypToString(partialhyp_t *parthyp, int nhypwds, 
                        char* hypothesis, int bufferSize)
{
    char* word;
    int j;

    hypothesis[0] = '\0';

    for (j = 0; j < nhypwds; j++) {
        word = parthyp[j].word;
        if (strcmp(word, "<sil>") != 0 &&
            (strcmp(word, "<s>") != 0 &&
             strcmp(word, "</s>") != 0)) {
            
            if (strlen(hypothesis) > 0) {
                strcat(hypothesis, " ");
            }
            strcat(hypothesis, word);
        }
    }
}


void showAccuracy(FILE *rfp)
{
    float wordAccuracy = 
        ((float) numMatchingWords)/((float) numRefWords) * 100.0;

    float sentenceAccuracy =
        ((float) numMatchingSentences) / ((float) numSentences) * 100.0;

    int totalErrors = recognitionErrors + insertionErrors + deletionErrors;

    fprintf(rfp,
            "   Accuracy: %%%.1f   Errors: %d  (Rec: %d  Ins: %d  Del: %d)\n",
            wordAccuracy, totalErrors,
            recognitionErrors, insertionErrors, deletionErrors);

    fprintf(rfp, "   Words: %d   Matches: %d\n", 
            numRefWords, numMatchingWords);
    fprintf(rfp, "   Sentences: %d   Matches: %d   SentenceAcc: %%%.1f\n",
            numSentences, numMatchingSentences, sentenceAccuracy);
}


void showTiming(FILE *rfp, const char* name, 
                double audioTime, double processingTime)
{
    fprintf(rfp,
            "   %s Timing Audio: %.1fs  Proc: %.1fs  Speed: %.2f X real time\n",
            name, audioTime, processingTime, (processingTime/audioTime));
}


void showMemory(FILE *rfp)
{
    fprintf(rfp, "   Memory usage data unavailable\n");
}


/**
 * Compare the hypothesis to the reference string collecting
 * statistics on it.
 *
 * @param ref the reference string
 * @param hyp the hypothesis string
 */
void analyzeResults(const char* referenceResult, const char* hypotheses)
{
    int numReferences;
    int numHypothesis;

    char *references[MAX_REFERENCES];
    char myReferenceResult[STRLEN];  /* copy because we will modify it */

    char *hypothesis[MAX_REFERENCES];
    char myHypothesis[STRLEN];
    
    strcpy(myReferenceResult, referenceResult);
    numReferences = stringToArray(myReferenceResult, references);
    referenceIndex = 0;

    strcpy(myHypothesis, hypotheses);
    numHypothesis = stringToArray(myHypothesis, hypothesis);
    hypothesisIndex = 0;

    numRefWords += numReferences;
    numHypWords += numHypothesis;
    numSentences++;

    while (referenceIndex < numReferences || hypothesisIndex < numHypothesis) {

        if (referenceIndex >= numReferences) {
            addInsert();
        } else if (hypothesisIndex >= numHypothesis) {
            addDeletion();
        } else if (strcmp(references[referenceIndex], 
                          hypothesis[hypothesisIndex]) != 0) {
            processMismatch(references, numReferences, 
                            hypothesis, numHypothesis);
        } else {
            addMatch();
        }
    }

    if (strcmp(referenceResult, hypotheses) == 0) {
        numMatchingSentences++;
    }
}


/**
 * Add an insertion error corresponding to the first item
 * on the hypList
 */
void addInsert()
{
    insertionErrors++;
    hypothesisIndex++;
}


/**
 * Add a deletion error corresponding to the first item
 * on the refList
 */
void addDeletion()
{
    deletionErrors++;
    referenceIndex++;
}


/**
 * Add a recognition error
 */
void addRecognitionError()
{
    recognitionErrors++;
    referenceIndex++;
    hypothesisIndex++;
}


/**
 * Add a match.
 */
void addMatch()
{
    numMatchingWords++;
    referenceIndex++;
    hypothesisIndex++;
}


/**
 * Process a mismatch by seeing which type of error is most likely
 */
void processMismatch(char *references[], int numReferences,
                     char *parthyp[], int numHypothesis)
{
    int deletionMatches;
    int insertMatches;
    int normalMatches;

    deletionMatches = countMatches(references, referenceIndex+1, numReferences,
                                   parthyp, hypothesisIndex, numHypothesis);
    insertMatches = countMatches(references, referenceIndex, numReferences,
                                    parthyp, hypothesisIndex+1, numHypothesis);
    normalMatches = countMatches(references, referenceIndex, numReferences,
                                 parthyp, hypothesisIndex, numHypothesis);
    
    if (deletionMatches > insertMatches && deletionMatches > normalMatches) {
        addDeletion();
    } else if (insertMatches > deletionMatches && 
               insertMatches > normalMatches) {
        addInsert();
    } else {
        addRecognitionError();
    }
}



/**
 * Counts the number of matches between the two lists
 * starting at the respective indexes
 *
 * @param refList the list of reference words
 * @param r the starting point in the ref list
 * @param numReferences the number of reference words
 * @param hypList the list of hypothesis  words
 * @param h the starting point in the hyp list
 * @param numHypothesis the number of hypotheses
 *
 * @return the number of matching words
 */
int countMatches(char *references[], int r, int numReferences,
                 char *parthyp[], int h, int numHypothesis)
{
    int match;
    match = 0;

    while (r < numReferences && h < numHypothesis) {
        if (strcmp(references[r++], parthyp[h++]) == 0) {
            match++;
        }
    }

    return match;
}


/**
 * Converts the individual words in the string into an array of strings.
 *
 * @param string the string to convert
 * @param the array of string pointers
 *
 * @return the number of words
 */
int stringToArray(char *string, char *array[])
{
    int i, c;

    i = 0;

    if (strlen(string) > 0) {
        array[i++] = string;
        for (c = 0; c < strlen(string); c++) {
            if (string[c] == ' ') {
                string[c++] = '\0';
                while (string[c] == '\0') {
                    c++;
                }
                array[i++] = &string[c];
            }
        }
    }

    return i;
}

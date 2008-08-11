#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <err.h>
#include <logs3.h>


int
main(int argc, char *argv[])
{
    if (argc != 5) {
        printf
            ("testlogs3 <logbase> <language weight> <insertion penalty> <log 10 value>");
        exit(-1);
    }
    float64 logbase;
    float64 lw, wip, log10v;
    int32 ls3;
    logmath_t *logmath;

    logbase = (float64) atof(argv[1]);

    lw = (float64) atof(argv[2]);
    wip = (float64) atof(argv[3]);
    log10v = (float64) atof(argv[4]);

    if ((logmath = logs3_init(logbase, 0, 1)) == NULL) {        /*Don't report progress, use log table. */
        E_FATAL("Initialization of log table failed.\n");
    }

    ls3 = (int32) (logmath_log10_to_log(logmath, log10v) * lw - logs3(logmath, wip));

    /*logs3(wip); */

    E_INFOCONT("%d\n", ls3);
    /*  E_INFOCONT("Log 10 Value %f, Log s3 %d base %f, lw %f, wip %f\n",
       log10v,
       ls3,
       logbase,
       lw,
       wip
       ); */

    return 0;

}

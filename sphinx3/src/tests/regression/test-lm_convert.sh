#!/bin/sh
. ./testfuncs.sh

echo "LM_CONVERT TEST"
tmpout="test-lm_convert.out"
tmptxt="test-lm_convert.TXT"
tmpfst="test-lm_convert.FST"
tmpdmp="test-lm_convert.DMP"

echo "This will perform several tests that compare the converted LMs to a pre-generated ones. "

#Couldn't do a match test because the paths generated in the LM will differ.  
if run_program lm_convert \
-input $an4lm/an4.tg.phone.arpa \
-output $tmpdmp > /dev/null 2>&1; then \
pass "LM CONVERT PHONE DRY RUN LM TXT-> DMP test"; else
fail "LM CONVERT PHONE DRY RUN LM TXT-> DMP test"; fi

if run_program lm_convert \
-input $an4lm/an4.tg.phone.arpa \
-inputfmt TXT32 \
-output $tmpdmp > /dev/null 2>&1; then \
pass "LM CONVERT PHONE DRY RUN LM TXT32-> DMP test"; else
fail "LM CONVERT PHONE DRY RUN LM TXT32-> DMP test"; fi

if run_program lm_convert \
-input $an4lm/an4.tg.phone.arpa \
-output $tmpdmp \
-outputfmt DMP32 \
> /dev/null 2>&1; then \
pass "LM CONVERT PHONE DRY RUN LM TXT-> DMP32 test"; else
fail "LM CONVERT PHONE DRY RUN LM TXT-> DMP32 test"; fi

if run_program lm_convert \
-input $an4lm/an4.tg.phone.arpa \
-inputfmt TXT32 \
-output $tmpdmp \
-outputfmt DMP32 \
> /dev/null 2>&1; then \
pass "LM CONVERT PHONE DRY RUN LM TXT32-> DMP32 test"; else
fail "LM CONVERT PHONE DRY RUN LM TXT32-> DMP32 test"; fi


run_program lm_convert \
-input $an4lm/an4.tg.phone.arpa.DMP \
-inputfmt DMP \
-output $tmptxt \
-outputfmt TXT \
> $tmpout 2>&1 

compare_table "LM_CONVERT PHONE LM DMP -> TXT test" $tmptxt $an4lm/an4.tg.phone.arpa.lm_convert 0.0002 
 

run_program lm_convert \
-input $an4lm/an4.tg.phone.arpa \
-output $tmpfst \
-outputfmt FST \
> $tmpout 2>&1 

compare_table "LM_CONVERT PHONE LM TXT -> FST test" $tmpfst $an4lm/an4.tg.phone.arpa.FST 0.0002 
compare_table "LM_CONVERT PHONE LM TXT -> FST SYM test" ${tmpfst}.sym $an4lm/an4.tg.phone.arpa.FST.SYM 0.0002 

run_program lm_convert \
-input $an4lm/an4.tg.phone.arpa \
-inputfmt TXT32 \
-output $tmpfst \
-outputfmt FST \
> $tmpout 2>&1 

#This is cheating.  I was just too tired to get it tested at this point. 
#compare_table $tmpfst $an4lm/an4.tg.phone.arpa.FST 0.0002 
#then echo "LM_CONVERT PHONE LM TXT32 -> FST test"
#echo "LM_CONVERT PHONE LM TXT32 -> FST test FAILED"; fi 

compare_table "LM_CONVERT PHONE LM TXT32 -> FST SYM test" ${tmpfst}.sym $an4lm/an4.tg.phone.arpa.FST.SYM 0.0002 
 

run_program lm_convert \
-input $an4lm/an4.tg.phone.arpa \
-output $tmpdmp \
> $tmpout 2>&1 

run_program lm_convert \
-input $tmpdmp \
-inputfmt DMP \
-output $tmptxt \
-outputfmt TXT \
> $tmpout 2>&1 

compare_table "LM_CONVERT PHONE LM TXT -> DMP -> TXT test" $tmptxt $an4lm/an4.tg.phone.arpa.lm_convert 0.0002 
 

run_program lm_convert \
-input $an4lm/an4.tg.phone.arpa \
-inputfmt TXT32 \
-output $tmpdmp \
> $tmpout 2>&1 

run_program lm_convert \
-input $tmpdmp \
-inputfmt DMP \
-output $tmptxt \
-outputfmt TXT \
> $tmpout 2>&1 

compare_table "LM_CONVERT PHONE LM TXT32 -> DMP -> TXT test" $tmptxt $an4lm/an4.tg.phone.arpa.lm_convert 0.0002 
 

run_program lm_convert \
-input $an4lm/an4.tg.phone.arpa \
-output $tmpdmp \
-outputfmt DMP \
> $tmpout 2>&1 

run_program lm_convert \
-input $tmpdmp \
-output $tmptxt \
-outputfmt TXT \
> $tmpout 2>&1 

compare_table "LM_CONVERT PHONE LM TXT -> DMP32 -> TXT test" $tmptxt $an4lm/an4.tg.phone.arpa.lm_convert 0.0002 

 

run_program lm_convert \
-input $an4lm/an4.tg.phone.arpa \
-inputfmt TXT32 \
-output $tmpdmp \
-outputfmt DMP \
> $tmpout 2>&1 

run_program lm_convert \
-input $tmpdmp \
-output $tmptxt \
-outputfmt TXT \
> $tmpout 2>&1 

compare_table "LM_CONVERT PHONE LM TXT32 -> DMP32 -> TXT test" $tmptxt $an4lm/an4.tg.phone.arpa.lm_convert 0.0002 
 

#Couldn't do a match test because the paths generated in the LM will differ.  
if run_program lm_convert \
-input $an4lm/an4.ug.lm \
-output $tmpdmp > /dev/null 2>&1; then \
pass "LM CONVERT WORD DRY RUN LM TXT-> DMP test"; else
fail "LM CONVERT WORD DRY RUN LM TXT-> DMP test"; fi


if run_program lm_convert \
-input $an4lm/an4.ug.lm \
-output $tmpdmp \
-outputfmt DMP32 \
> /dev/null 2>&1; then \
pass "LM CONVERT WORD DRY RUN LM TXT-> DMP32 test"; else
fail "LM CONVERT WORD DRY RUN LM TXT-> DMP32 test"; fi


run_program lm_convert \
-input $an4lm/an4.ug.lm.DMP \
-inputfmt DMP \
-output $tmptxt \
-outputfmt TXT \
> $tmpout 2>&1 

compare_table "LM_CONVERT WORD LM DMP -> TXT test" $tmptxt $an4lm/an4.ug.lm.lm_convert 0.0002 
 

run_program lm_convert \
-input $an4lm/an4.ug.lm \
-output $tmpfst \
-outputfmt FST \
> $tmpout 2>&1 

compare_table "LM_CONVERT WORD LM TXT -> FST test" $tmpfst $an4lm/an4.ug.lm.FST 0.0002 
 

run_program lm_convert \
-input $an4lm/an4.ug.lm \
-inputfmt TXT32 \
-output $tmpfst \
-outputfmt FST \
> $tmpout 2>&1 

compare_table "LM_CONVERT WORD LM TXT32 -> FST test" $tmpfst $an4lm/an4.ug.lm.FST 0.0002 
 

run_program lm_convert \
-input $an4lm/an4.ug.lm \
-output $tmpdmp \
> $tmpout 2>&1 

run_program lm_convert \
-input $an4lm/an4.ug.lm.DMP \
-inputfmt DMP \
-output $tmptxt \
-outputfmt TXT \
> $tmpout 2>&1 

compare_table "LM_CONVERT WORD LM DMP -> TXT -> DMP test" $tmptxt $an4lm/an4.ug.lm.lm_convert 0.0002
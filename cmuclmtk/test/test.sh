#Testing of text2wfreq
echo "V3 text2wfreq"
cat English/emma11.txt.filtered | ../src/text2wfreq > English/emma11.txt.filtered.wfreq
if (diff English/emma11.txt.filtered.wfreq English/emma11.wfreq > /dev/null 2>&1); \
then echo "V3 text2wfreq PASSED"; else \
echo "V3 text2wfreq FAILED"; fi 

#echo "V2 text2wfreq"
#cat English/emma11.txt.filtered | ../../CMU-Cam_Toolkit_v2/src/text2wfreq > English/emma11.txt.filtered.wfreq
#diff English/emma11.txt.filtered.wfreq English/emma11.wfreq
#if (diff English/emma11.txt.filtered.wfreq English/emma11.wfreq > /dev/null 2>&1); \
#then echo "V2 text2wfreq PASSED"; else \
#echo "V2 text2wfreq FAILED"; fi 
#
#Testing of wfreq2vocab
echo "V3 wfreq2vocab"
cat English/emma11.txt.filtered.wfreq |../src/wfreq2vocab -top 20000  > English/emma11.txt.filtered.vocab
if (diff English/emma11.vocab English/emma11.txt.filtered.vocab > /dev/null 2>&1); \
then echo "V3 wfreq2vocab PASSED"; else \
echo "V3 wfreq2vocab FAILED"; fi 

#echo "V2 wfreq2vocab"
#cat English/emma11.txt.filtered.wfreq |../../CMU-Cam_Toolkit_v2/src/wfreq2vocab -top 20000  > English/emma11.txt.filtered.vocab
#if (diff English/emma11.vocab English/emma11.txt.filtered.vocab > /dev/null 2>&1); \
#then echo "V2 wfreq2vocab PASSED"; else \
#echo "V2 wfreq2vocab FAILED"; fi 
#
#Testing of text2idngram binary idngram
echo "V3 text2idngram BIN IDNGRAM"
cat English/emma11.txt.filtered | ../src/text2idngram -vocab English/emma11.vocab > ./English/emma11.txt.filtered.idngram  
if (cmp English/emma11.idngram ./English/emma11.txt.filtered.idngram > /dev/null 2>&1); \
then echo "V3 text2idngram BIN IDNGRAM PASSED"; else \
echo "V3 text2idngram BIN IDNGRAM FAILED"; fi 

#echo "V2 text2idngram BIN IDNGRAM"
#cat English/emma11.txt.filtered | ../../CMU-Cam_Toolkit_v2/src/text2idngram -vocab English/emma11.vocab > English/emma11.txt.filtered.idngram  
#if (cmp English/emma11.idngram ./English/emma11.txt.filtered.idngram > /dev/null 2>&1); \
#then echo "V2 text2idngram BIN IDNGRAM PASSED"; else \
#echo "V2 text2idngram BIN IDNGRAM FAILED"; fi 
#
#Testing of text2idngram text idngram 
echo "V3 text2idngram TXT IDNGRAM"
cat English/emma11.txt.filtered | ../src/text2idngram -vocab English/emma11.vocab -write_ascii > ./English/emma11.txt.filtered.idngram.txt  
if (diff English/emma11.idngram.txt ./English/emma11.txt.filtered.idngram.txt > /dev/null 2>&1); \
then echo "V3 text2idngram TXT IDNGRAM PASSED"; else \
echo "V3 text2idngram TXT IDNGRAM FAILED"; fi 

#echo "V2 text2idngram TXT IDNGRAM"
#cat English/emma11.txt.filtered | ../../CMU-Cam_Toolkit_v2/src/text2idngram -vocab English/emma11.vocab  -write_ascii > English/emma11.txt.filtered.idngram  
#if (diff English/emma11.idngram.txt ./English/emma11.txt.filtered.idngram.txt > /dev/null 2>&1); \
#then echo "V2 text2idngram TXT IDNGRAM PASSED"; else \
#echo "V2 text2idngram TXT IDNGRAM FAILED"; fi 
#
#Testing of idngram2lm
 echo "V3 idngram2lm ARPA"
../src/idngram2lm -idngram ./English/emma11.idngram -vocab ./English/emma11.vocab -arpa English/emma11.filtered.arpa
if (cmp English/emma11.arpa.V3 English/emma11.filtered.arpa  > /dev/null 2>&1); \
then echo "V3 idngram2lm ARPA PASSED"; else \
echo "V3 idngram2lm ARPA FAILED"; fi 

echo "V3 idngram2lm BIN"
../src/idngram2lm -idngram ./English/emma11.idngram -vocab ./English/emma11.vocab -binary English/emma11.filtered.bin
if (cmp English/emma11.bin English/emma11.filtered.bin  > /dev/null 2>&1); \
then echo "V3 idngram2lm BIN PASSED"; else \
echo "V3 idngram2lm BIN FAILED"; fi 

##Testing of idngram2lm
#echo "V2 idngram2lm ARPA"
#../../CMU-Cam_Toolkit_v2/src/idngram2lm -idngram ./English/emma11.idngram -vocab ./English/emma11.vocab -arpa English/emma11.filtered.arpa
#if (cmp English/emma11.arpa English/emma11.filtered.arpa  > /dev/null 2>&1); \
#then echo "V2 idngram2lm ARPA PASSED"; else \
#echo "V2 idngram2lm ARPA FAILED"; fi 
#
#echo "V2 idngram2lm BIN"
#../../CMU-Cam_Toolkit_v2/src/idngram2lm -idngram ./English/emma11.idngram -vocab ./English/emma11.vocab -binary English/emma11.filtered.bin
#if (cmp English/emma11.bin English/emma11.filtered.bin  > /dev/null 2>&1); \
#then echo "V2 idngram2lm BIN PASSED"; else \
#echo "V2 idngram2lm BIN FAILED"; fi 
#
#Testing of idngram2lm
 echo "V3 idngram2lm ARPA ABSOLUTE TXT"
../src/idngram2lm -idngram ./English/emma11.idngram -vocab ./English/emma11.vocab -absolute -arpa English/emma11.filtered.arpa
if (cmp English/emma11.absolute.arpa.V3 English/emma11.filtered.arpa  > /dev/null 2>&1); \
then echo "V3 idngram2lm ARPA ABSOLUTE TXT PASSED"; else \
echo "V3 idngram2lm ARPA ABSOLUTE TXT FAILED"; fi 

#Testing of idngram2lm
 echo "V3 idngram2lm ARPA ABSOLUTE TXT"
../src/idngram2lm -idngram ./English/emma11.idngram -vocab ./English/emma11.vocab -absolute -arpa English/emma11.filtered.arpa
if (cmp English/emma11.absolute.arpa.V3 English/emma11.filtered.arpa  > /dev/null 2>&1); \
then echo "V3 idngram2lm ARPA ABSOLUTE TXT PASSED"; else \
echo "V3 idngram2lm ARPA ABSOLUTE TXT FAILED"; fi 


#Testing of idngram2lm
 echo "V3 idngram2lm ARPA LINEAR"
../src/idngram2lm -idngram ./English/emma11.idngram -vocab ./English/emma11.vocab -linear -arpa English/emma11.filtered.arpa
if (cmp English/emma11.linear.arpa.V3 English/emma11.filtered.arpa  > /dev/null 2>&1); \
then echo "V3 idngram2lm ARPA LINEAR PASSED"; else \
echo "V3 idngram2lm ARPA LINEAR FAILED"; fi 

#Testing of idngram2lm
 echo "V3 idngram2lm ARPA WITTEN BELL"
../src/idngram2lm -idngram ./English/emma11.idngram -vocab ./English/emma11.vocab -witten_bell -arpa English/emma11.filtered.arpa
if (cmp English/emma11.witten_bell.arpa.V3 English/emma11.filtered.arpa  > /dev/null 2>&1); \
then echo "V3 idngram2lm ARPA WITTEN BELL PASSED"; else \
echo "V3 idngram2lm ARPA WITTEN BELL FAILED"; fi 

#Testing of binlm2arpa
echo "V3 of binlm2arpa"
../src/binlm2arpa -binary ./English/emma11.bin -arpa convert.arpa
if (diff convert.arpa ./English/emma11.arpa.V3 > /dev/null 2>&1); \
then echo "V3 binlm2arpa  PASSED"; else \
echo "V3 binlm2arpa FAILED"; fi 
rm convert.arpa

#echo "V2 of binlm2arpa"
#../../CMU-Cam_Toolkit_v2/src/binlm2arpa -binary ./English/emma11.bin -arpa convert.arpa
#if (diff convert.arpa ./English/emma11.arpa > /dev/null 2>&1); \
#then echo "V2 binlm2arpa  PASSED"; else \
#echo "V2 binlm2arpa FAILED"; fi 
#rm convert.arpa
#
#Testing of idngram2stats 
echo "V3 of idngram2stats"
cat ./English/emma11.idngram |  ../src/idngram2stats -n 3 > ./tmp.stats 2>&1
if (diff ./tmp.stats ./English/emma11.stats > /dev/null 2>&1); \
then echo "V3 idngram2stats PASSED"; else \
echo "V3 idngram2stats FAILED"; fi
rm ./tmp.stats

#echo "V2 of idngram2stats"
#cat ./English/emma11.idngram |  ../../CMU-Cam_Toolkit_v2/src/idngram2stats -n 3 > ./tmp.stats 2>&1 
#if (diff ./tmp.stats ./English/emma11.stats > /dev/null 2>&1); \
#then echo "V2 idngram2stats PASSED"; else \
#echo "V2 idngram2stats FAILED"; fi
#rm ./tmp.stats
#
#Testing of evallm
echo "V3 of evallm STATS"
echo "stats" | ../src/evallm -binary ./English/emma11.bin > ./tmp.stats 2>&1
if (diff ./tmp.stats ./English/emma11.bin.stats > /dev/null 2>&1); \
then echo "V3 evallm STATS PASSED"; else \
echo "V3 evallm STATS FAILED"; fi

#Testing of evallm
echo "V3 of evallm PERPLEXITY"
echo "perplexity -text ./English/pandp12.txt.filtered"  | ../src/evallm -binary ./English/emma11.bin > ./tmp.perplexity
if (diff ./tmp.perplexity ./English/emma11.bin.perplexity > /dev/null 2>&1); \
then echo "V3 evallm PERPLEXITY PASSED"; else \
echo "V3 evallm PERPLEXITY FAILED"; fi
rm ./tmp.perplexity

#Testing of ngram2mgram TXT
echo "V3 of ngram2mgram TO 2-GRAM TXT"
../src/ngram2mgram -n 3 -m 2 -ascii < ./English/emma11.idngram.txt > ./English/emma11.id2gram.txt.filtered
if (diff English/emma11.id2gram.txt ./English/emma11.id2gram.txt.filtered > /dev/null 2>&1); \
then echo "V3 ngram2mgram TXT TO 2-GRAM TXT PASSED"; else \
echo "V3 ngram2mgram TXT TO 2-GRAM TXT FAILED"; fi 

echo "V3 of ngram2mgram TO 1-GRAM TXT"
../src/ngram2mgram -n 3 -m 1 -ascii < ./English/emma11.idngram.txt > ./English/emma11.id1gram.txt.filtered
if (diff English/emma11.id1gram.txt ./English/emma11.id1gram.txt.filtered > /dev/null 2>&1); \
then echo "V3 ngram2mgram TXT TO 1-GRAM TXT PASSED"; else \
echo "V3 ngram2mgram TXT TO 1-GRAM TXT FAILED"; fi 

#Testing of ngram2mgram BIN
echo "V3 of ngram2mgram TO 2-GRAM BIN"
../src/ngram2mgram -n 3 -m 2 -binary < ./English/emma11.idngram > ./English/emma11.id2gram.filtered
if (cmp English/emma11.id2gram ./English/emma11.id2gram.filtered > /dev/null 2>&1); \
then echo "V3 ngram2mgram TXT TO 2-GRAM BIN PASSED"; else \
echo "V3 ngram2mgram BIN TO 2-GRAM BIN FAILED"; fi 

echo "V3 of ngram2mgram TO 1-GRAM BIN"
../src/ngram2mgram -n 3 -m 1 -binary < ./English/emma11.idngram > ./English/emma11.id1gram.filtered
if (cmp English/emma11.id1gram ./English/emma11.id1gram.filtered > /dev/null 2>&1); \
then echo "V3 ngram2mgram TXT TO 1-GRAM BIN PASSED"; else \
echo "V3 ngram2mgram BIN TO 1-GRAM BIN FAILED"; fi 

cat ./English/pandp12.txt.filtered | ../src/text2wfreq > ./English/pandp12.wfreq
cat ./English/pandp12.wfreq | ../src/wfreq2vocab > ./English/pandp12.vocab

cat ./English/pandp12.txt.filtered | ../src/text2idngram -vocab ./English/pandp12.vocab  > ./English/pandp12.txt.idngram
cat ./English/pandp12.txt.filtered | ../src/text2idngram -vocab ./English/pandp12.vocab -write_ascii  > ./English/pandp12.idngram.txt

echo "V3 of mergeidngram BIN INPUT TXT OUTPUT"
../src/mergeidngram -n 3 ./English/emma11.txt.idngram ./English/pandp12.txt.idngram -ascii_output > ./English/combine.idngram.txt.filtered
if (diff ./English/combine.idngram.txt  ./English/combine.idngram.txt.filtered > /dev/null 2>&1); \
then echo "V3 mergeidngram BIN INPUT TXT OUTPUT PASSED "; else \
echo "V3 mergeidngram BIN INPUT TXT OUTPUT FAILED"; fi 

echo "V3 of mergeidngram BIN INPUT BIN OUTPUT"
../src/mergeidngram -n 3 ./English/emma11.txt.idngram ./English/pandp12.txt.idngram  > ./English/combine.idngram.filtered
if (diff ./English/combine.idngram  ./English/combine.idngram.filtered > /dev/null 2>&1); \
then echo "V3 mergeidngram BIN INPUT BIN OUTPUT PASSED "; else \
echo "V3 mergeidngram BIN INPUT BIN OUTPUT FAILED"; fi 

echo "V3 of mergeidngram TXT INPUT TXT OUTPUT"
../src/mergeidngram -n 3 ./English/emma11.idngram.txt ./English/pandp12.idngram.txt -ascii_input -ascii_output > ./English/combine.idngram.txt.filtered
if (diff ./English/combine.idngram.txt  ./English/combine.idngram.txt.filtered > /dev/null 2>&1); \
then echo "V3 mergeidngram TXT INPUT TXT OUTPUT PASSED "; else \
echo "V3 mergeidngram TXT INPUT TXT OUTPUT FAILED"; fi 

echo "V3 of mergeidngram TXT INPUT BIN OUTPUT"
../src/mergeidngram -n 3 ./English/emma11.idngram.txt ./English/pandp12.idngram.txt -ascii_input  > ./English/combine.idngram.filtered
if (diff ./English/combine.idngram  ./English/combine.idngram.filtered > /dev/null 2>&1); \
then echo "V3 mergeidngram TXT INPUT BIN OUTPUT PASSED "; else \
echo "V3 mergeidngram TXT INPUT BIN OUTPUT FAILED"; fi 


#Append test here. 
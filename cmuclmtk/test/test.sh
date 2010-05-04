#
# [20080328] (air) added  DIFF to deal with win/linux texts
#
DIFF='diff --strip-trailing-cr'
BIN=../src/programs

set -e

#Testing of text2wfreq
echo "V3 32BITS text2wfreq"
cat English/emma11.txt.filtered | $BIN/text2wfreq > English/emma11.txt.filtered.wfreq 2>text2wfreq.log
if ($DIFF English/emma11.txt.filtered.wfreq English/emma11.wfreq > /dev/null 2>&1); \
then echo "V3 32BITS text2wfreq PASSED"; else \
echo "V3 32BITS text2wfreq FAILED"; fi 

#Testing of wfreq2vocab
echo "V3 32BITS wfreq2vocab"
cat English/emma11.txt.filtered.wfreq |$BIN/wfreq2vocab -top 20000 > English/emma11.txt.filtered.vocab 2>wfreq2vocab.log
if ($DIFF English/emma11.vocab English/emma11.txt.filtered.vocab > /dev/null 2>&1); \
then echo "V3 32BITS wfreq2vocab PASSED"; else \
echo "V3 32BITS wfreq2vocab FAILED"; fi 

#Testing of text2idngram binary idngram
echo "V3 32BITS text2idngram BIN IDNGRAM"
cat English/emma11.txt.filtered | $BIN/text2idngram -vocab English/emma11.vocab -idngram ./English/emma11.txt.filtered.idngram.32bits 2>text2idngram.log
if (cmp English/emma11.idngram.32bits ./English/emma11.txt.filtered.idngram.32bits > /dev/null 2>&1); \
then echo "V3 32BITS text2idngram BIN IDNGRAM PASSED"; else \
echo "V3 32BITS text2idngram BIN IDNGRAM FAILED"; fi 

#Testing of text2idngram text idngram 
echo "V3 32BITS text2idngram TXT IDNGRAM"
cat English/emma11.txt.filtered | $BIN/text2idngram -vocab English/emma11.vocab -write_ascii -idngram ./English/emma11.txt.filtered.idngram.txt 2>text2idngramtxt.log
if ($DIFF English/emma11.idngram.txt ./English/emma11.txt.filtered.idngram.txt > /dev/null 2>&1); \
then echo "V3 32BITS text2idngram TXT IDNGRAM PASSED"; else \
echo "V3 32BITS text2idngram TXT IDNGRAM FAILED"; fi 

#Testing of idngram2lm
 echo "V3 32BITS idngram2lm ARPA"
$BIN/idngram2lm -idngram ./English/emma11.idngram.32bits -vocab ./English/emma11.vocab -arpa English/emma11.filtered.arpa 2>idngram2lm.log
if ($DIFF English/emma11.arpa.V3 English/emma11.filtered.arpa  > /dev/null 2>&1); \
then echo "V3 32BITS idngram2lm ARPA PASSED"; else \
echo "V3 32BITS idngram2lm ARPA FAILED"; fi 

echo "V3 32BITS idngram2lm BIN"
$BIN/idngram2lm -idngram ./English/emma11.idngram.32bits -vocab ./English/emma11.vocab -binary English/emma11.filtered.bin 2>idngram2lmbin.log
if (cmp English/emma11.bin.32bits English/emma11.filtered.bin  > /dev/null 2>&1); \
then echo "V3 32BITS idngram2lm BIN PASSED"; else \
echo "V3 32BITS idngram2lm BIN FAILED"; fi 

#Testing of binlm2arpa
echo "V3 32BITS of binlm2arpa"
$BIN/binlm2arpa -binary ./English/emma11.bin.32bits -arpa convert.arpa 2>binlm2arpa.log
if ($DIFF convert.arpa ./English/emma11.arpa.V3 > /dev/null 2>&1); \
then echo "V3 32BITS binlm2arpa  PASSED"; else \
echo "V3 32BITS binlm2arpa FAILED"; fi 
rm convert.arpa

#Testing of idngram2stats 
echo "V3 32BITS of idngram2stats"
cat ./English/emma11.idngram.32bits |  $BIN/idngram2stats -n 3 > ./tmp.stats 2>&1
if ($DIFF ./tmp.stats ./English/emma11.stats > /dev/null 2>&1); \
then echo "V3 32BITS idngram2stats PASSED"; else \
 echo "V3 32BITS idngram2stats FAILED"; fi
rm ./tmp.stats

#Testing of evallm
echo "V3 32BITS of evallm STATS"
echo "stats" | $BIN/evallm -binary ./English/emma11.bin.32bits > ./tmp.stats 2>&1
sed "s/\.32bits//" tmp.stats > tmp
mv tmp tmp.stats
if ($DIFF ./tmp.stats ./English/emma11.bin.stats > /dev/null 2>&1); \
then echo "V3 32BITS evallm STATS PASSED"; else \
echo "V3 32BITS evallm STATS FAILED"; fi

#Testing of evallm
echo "V3 32BITS of evallm PERPLEXITY"
echo "perplexity -text ./English/pandp12.txt.filtered"  | $BIN/evallm -binary ./English/emma11.bin.32bits > ./tmp.perplexity 2>evallmpp.log
if ($DIFF ./tmp.perplexity ./English/emma11.bin.perplexity > /dev/null 2>&1); \
then echo "V3 32BITS evallm PERPLEXITY PASSED"; else \
echo "V3 32BITS evallm PERPLEXITY FAILED"; fi
rm ./tmp.perplexity

#Testing of ngram2mgram TXT
echo "V3 of ngram2mgram TO 2-GRAM TXT"
$BIN/ngram2mgram -n 3 -m 2 -ascii < ./English/emma11.idngram.txt > ./English/emma11.id2gram.txt.filtered 2>ngram2mgram32.log
if ($DIFF English/emma11.id2gram.txt ./English/emma11.id2gram.txt.filtered > /dev/null 2>&1); \
then echo "V3 ngram2mgram TXT TO 2-GRAM TXT PASSED"; else \
echo "V3 ngram2mgram TXT TO 2-GRAM TXT FAILED"; fi 

echo "V3 of ngram2mgram TO 1-GRAM TXT"
$BIN/ngram2mgram -n 3 -m 1 -ascii < ./English/emma11.idngram.txt > ./English/emma11.id1gram.txt.filtered 2>ngram2mgram31.log
if ($DIFF English/emma11.id1gram.txt ./English/emma11.id1gram.txt.filtered > /dev/null 2>&1); \
then echo "V3 ngram2mgram TXT TO 1-GRAM TXT PASSED"; else \
echo "V3 ngram2mgram TXT TO 1-GRAM TXT FAILED"; fi 

#Testing of ngram2mgram BIN
echo "V3 of ngram2mgram TO 2-GRAM BIN"
$BIN/ngram2mgram -n 3 -m 2 -binary < ./English/emma11.idngram.32bits > ./English/emma11.id2gram.filtered 2>ngram2mgram32bin.log
if (cmp English/emma11.id2gram.32bits ./English/emma11.id2gram.filtered > /dev/null 2>&1); \
then echo "V3 ngram2mgram TXT TO 2-GRAM BIN PASSED"; else \
echo "V3 ngram2mgram BIN TO 2-GRAM BIN FAILED"; fi 

echo "V3 of ngram2mgram TO 1-GRAM BIN"
$BIN/ngram2mgram -n 3 -m 1 -binary < ./English/emma11.idngram.32bits > ./English/emma11.id1gram.filtered 2>ngram2mgram31bin.log
if (cmp English/emma11.id1gram.32bits ./English/emma11.id1gram.filtered > /dev/null 2>&1); \
then echo "V3 ngram2mgram TXT TO 1-GRAM BIN PASSED"; else \
echo "V3 ngram2mgram BIN TO 1-GRAM BIN FAILED"; fi 

cat ./English/pandp12.txt.filtered | $BIN/text2wfreq > ./English/pandp12.wfreq 2>/dev/null
cat ./English/pandp12.wfreq | $BIN/wfreq2vocab > ./English/pandp12.vocab 2>/dev/null

cat ./English/pandp12.txt.filtered | $BIN/text2idngram -vocab ./English/pandp12.vocab > ./English/pandp12.txt.idngram 2>/dev/null
cat ./English/pandp12.txt.filtered | $BIN/text2idngram -vocab ./English/pandp12.vocab -write_ascii > ./English/pandp12.idngram.txt 2>/dev/null

echo "V3 of mergeidngram BIN INPUT TXT OUTPUT"
$BIN/mergeidngram -n 3 ./English/emma11.idngram.32bits ./English/pandp12.txt.idngram -ascii_output > ./English/combine.idngram.txt.filtered 2>mergeidngrambintxt.log
if ($DIFF ./English/combine.idngram.txt  ./English/combine.idngram.txt.filtered > /dev/null 2>&1); \
then echo "V3 mergeidngram BIN INPUT TXT OUTPUT PASSED "; else \
echo "V3 mergeidngram BIN INPUT TXT OUTPUT FAILED"; fi 

echo "V3 of mergeidngram BIN INPUT BIN OUTPUT"
$BIN/mergeidngram -n 3 ./English/emma11.idngram.32bits ./English/pandp12.txt.idngram > ./English/combine.idngram.filtered 2>mergeidngrambinbin.log
if ($DIFF ./English/combine.idngram.32bits  ./English/combine.idngram.filtered > /dev/null 2>&1); \
then echo "V3 mergeidngram BIN INPUT BIN OUTPUT PASSED "; else \
echo "V3 mergeidngram BIN INPUT BIN OUTPUT FAILED"; fi 

echo "V3 of mergeidngram TXT INPUT TXT OUTPUT"
$BIN/mergeidngram -n 3 ./English/emma11.idngram.txt ./English/pandp12.idngram.txt -ascii_input -ascii_output 2>mergeidntramtxttxt.log  |sed 's/\r//'> ./English/combine.idngram.txt.filtered
if ($DIFF ./English/combine.idngram.txt  ./English/combine.idngram.txt.filtered > /dev/null 2>&1); \
then echo "V3 mergeidngram TXT INPUT TXT OUTPUT PASSED "; else \
echo "V3 mergeidngram TXT INPUT TXT OUTPUT FAILED"; fi 

echo "V3 of mergeidngram TXT INPUT BIN OUTPUT"
$BIN/mergeidngram -n 3 ./English/emma11.idngram.txt ./English/pandp12.idngram.txt -ascii_input > ./English/combine.idngram.filtered 2>mergeidngramtxtbin.log
if ($DIFF ./English/combine.idngram.32bits  ./English/combine.idngram.filtered > /dev/null 2>&1); \
then echo "V3 mergeidngram TXT INPUT BIN OUTPUT PASSED "; else \
echo "V3 mergeidngram TXT INPUT BIN OUTPUT FAILED"; fi 



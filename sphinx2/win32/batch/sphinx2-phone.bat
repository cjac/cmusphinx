echo off

set S2BATCH=..\msdev\SphinxExamples\sphinx2_allphone\Debug\sphinx2_allphone
set HMM=..\..\model\hmm\6k
set TASK=..\..\model\lm\turtle
set DICT=..\..\model\lm\turtle\turtle.dic

%S2BATCH% -allphone TRUE -adcin TRUE -adcext 16k -agcmax FALSE -datadir %TASK% -langwt 6.5 -fwdflatlw 8.5 -rescorelw 9.5 -ugwt 0.5 -fillpen 1e-10 -silpen 0.005 -inspen 0.65 -top 1 -topsenfrm 3 -topsenthresh -70000 -beam 2e-06 -npbeam 2e-06 -lpbeam 2e-05 -lponlybeam 0.0005 -nwbeam 0.0005 -fwdflat FALSE -fwdflatbeam 1e-08 -fwdflatnwbeam 0.0003 -bestpath TRUE -kbdumpdir %TASK% -dictfn %DICT% -ndictfn %HMM%\noisedict -phnfn %HMM%\phone -mapfn %HMM%\map -hmmdir %HMM% -hmmdirlist %HMM% -8bsen TRUE -sendumpfn %HMM%\sendump -cbdir %HMM%



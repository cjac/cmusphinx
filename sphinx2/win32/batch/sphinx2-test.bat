echo off

set S2BATCH=..\msdev\SphinxExamples\sphinx2_continuous\Debug\sphinx2_continuous.exe
set HMM=..\..\model\hmm\6k
set TASK=..\..\model\lm\turtle
set CTLFILE=..\..\model\lm\turtle\turtle.ctl

echo " "
echo "sphinx2_test"
echo "Run CMU Sphinx2 in Batch mode to decode an example utterance."
echo " "

%S2BATCH% -verbose 9 -adcin TRUE -adcext 16k -ctlfn %CTLFILE% -ctloffset 0 -ctlcount 100000000 -datadir %TASK% -agcmax FALSE -langwt 6.5 -fwdflatlw 8.5 -rescorelw 9.5 -ugwt 0.5 -fillpen 1e-10 -silpen 0.005 -inspen 0.65 -top 1 -topsenfrm 3 -topsenthresh -70000 -beam 2e-06 -npbeam 2e-06 -lpbeam 2e-05 -lponlybeam 0.0005 -nwbeam 0.0005 -fwdflat FALSE -fwdflatbeam 1e-08 -fwdflatnwbeam 0.0003 -bestpath TRUE -kbdumpdir %TASK% -lmfn %TASK%\turtle.lm -dictfn %TASK%\turtle.dic -ndictfn %HMM%\noisedict -phnfn %HMM%\phone -mapfn %HMM%\map -hmmdir %HMM% -hmmdirlist %HMM% -8bsen TRUE -sendumpfn %HMM%\sendump -cbdir %HMM%

echo off

set S2CONTINUOUS=..\msdev\SphinxExamples\sphinx2_continuous\Debug\sphinx2_continuous.exe
set HMM=..\..\model\hmm\6k
set TASK=..\..\model\lm\turtle
set DICT=..\..\model\lm\turtle\turtle.dic

echo sphinx2_simple:
echo   Demo CMU Sphinx2 decoder called with command line arguments.
echo   Run sphinx2_demo and read the docs to grok the arguments.

echo executing %S2CONTINUOUS%, please wait
%S2CONTINUOUS% -live TRUE -ctloffset 0 -ctlcount 100000000 -cepdir %TASK%\ctl -datadir %TASK%\ctl -agcemax TRUE -langwt 6.5 -fwdflatlw 8.5 -rescorelw 9.5 -ugwt 0.5 -fillpen 1e-10 -silpen 0.005 -inspen 0.65 -top 1 -topsenfrm 3 -topsenthresh -70000 -beam 2e-06 -npbeam 2e-06 -lpbeam 2e-05 -lponlybeam 0.0005 -nwbeam 0.0005 -fwdflat FALSE -fwdflatbeam 1e-08 -fwdflatnwbeam 0.0003 -bestpath TRUE -kbdumpdir %TASK% -lmfn %TASK%\turtle.lm -dictfn %DICT% -noisedict %HMM%\noisedict -phnfn %HMM%\phone -mapfn %HMM%\map -hmmdir %HMM% -hmmdirlist %HMM% -8bsen TRUE -sendumpfn %HMM%\sendump -cbdir %HMM%


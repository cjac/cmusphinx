rem example batch file to call sphinx2_batch to do forced alignment

echo off

rem the binary to run
set S2BATCH=..\msdev\SphinxExamples\sphinx2_batch\Debug\sphinx2_batch.exe

rem the HMM directory 
set HMM=..\..\model\hmm\6k

rem the 'task'; the turtle demo by default.
set TASK=..\..\model\lm\turtle

rem the control file, which lists the utterance names, one per line
set CTLFILE=..\..\model\lm\turtle\turtle.ctl

rem the transcripts, one per line, to be used for the alignment
set TACTLFN=..\..\model\lm\turtle\turtle.cor

rem the dictionary
set DICT=..\..\model\lm\turtle\turtle.dic

echo sphinx2_align:
echo    Use CMU Sphinx2 to perform phonetic alignment of an audio file
echo    to a known text transcription.  Shows an example.

%S2BATCH% -adcin TRUE -adcext 16k -ctlfn %CTLFILE% -tactlfn %TACTLFN% -ctloffset 0 -ctlcount 100000000 -datadir %TASK% -agcmax TRUE -langwt 6.5 -fwdflatlw 8.5 -rescorelw 9.5 -ugwt 0.5 -fillpen 1e-10 -silpen 0.005 -inspen 0.65 -top 1 -topsenfrm 3 -topsenthresh -70000 -beam 2e-06 -npbeam 2e-06 -lpbeam 2e-05 -lponlybeam 0.0005 -nwbeam 0.0005 -fwdflat FALSE -fwdflatbeam 1e-08 -fwdflatnwbeam 0.0003 -bestpath TRUE -kbdumpdir %TASK% -dictfn %DICT% -ndictfn %HMM%\noisedict -phnfn %HMM%\phone -mapfn %HMM%\map -hmmdir %HMM% -hmmdirlist %HMM% -8bsen TRUE -sendumpfn %HMM%\sendump -cbdir %HMM%


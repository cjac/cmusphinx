echo off

set S3ROOT=..\..
cd %S3ROOT%
set S3BATCH=.\bin\Debug\astar.exe
set TASK=.\model\lm\an4
set CTLFILE=.\win32\batch\an4.ctl

echo . 
echo sphinx3-astar
echo Run CMU Sphinx-3 astar in Batch mode to decode an example utterance.
echo .
echo This batch script assumes all files are relative to the main
echo directory (S3ROOT).
echo .

%S3BATCH%  -mdeffn ./model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/hub4opensrc.6000.mdef -fdictfn ./model/lm/an4/filler.dict -dictfn ./model/lm/an4/an4.dict -lmfn ./model/lm/an4/an4.ug.lm.DMP -inspen 0.2 -beam 1e-64 -nbest 5 -ctlfn ./model/lm/an4/an4.ctl.platform_independent -inlatdir ./model/lm/an4/ -logbase 1.0003 -nbestdir . 




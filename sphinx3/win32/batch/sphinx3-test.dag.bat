echo off

set S3ROOT=..\..
cd %S3ROOT%
set S3BATCH=.\bin\Debug\dag.exe
set TASK=.\model\lm\an4
set CTLFILE=.\win32\batch\an4.ctl

echo . 
echo sphinx3-dag
echo Run CMU Sphinx-3 dag in Batch mode to decode an example utterance.
echo .
echo This batch script assumes all files are relative to the main
echo directory (S3ROOT).
echo .
echo If the installation is correct, this line should read:
echo P I T T S B U R G H (null)

%S3BATCH%  -mdeffn ./model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/hub4opensrc.6000.mdef -fdictfn ./model/lm/an4/filler.dict -dictfn ./model/lm/an4/an4.dict -lmfn ./model/lm/an4/an4.ug.lm.DMP -langwt 13.0 -inspen 0.2 -ctlfn ./model/lm/an4/an4.ctl.platform_independent -inlatdir ./model/lm/an4/ -logbase 1.0003 -backtrace 1 




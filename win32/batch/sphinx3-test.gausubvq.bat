echo off

set S3ROOT=..\..
cd %S3ROOT%
set S3BATCH=.\bin\Debug\gausubvq.exe
set TASK=.\model\lm\an4
set CTLFILE=.\win32\batch\an4.ctl

echo . 
echo sphinx3-dag
echo Run CMU Sphinx-3 gausubvq in Batch mode to decode an example utterance.
echo .
echo This batch script assumes all files are relative to the main
echo directory (S3ROOT).
echo .


%S3BATCH%  -mean ./model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/means -var  ./model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/variances -mixw  ./model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/mixture_weights -svspec 0-38 -iter 20 -svqrows 16 -subvq subvq.out 





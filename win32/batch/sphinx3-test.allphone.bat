echo off

set S3ROOT=..\..
pushd %S3ROOT%
set S3BATCH=.\bin\Debug\allphone.exe
set TASK=.\model\lm\an4
set CTLFILE=.\win32\batch\an4.ctl

echo . 
echo sphinx3-test
echo Run CMU Sphinx-3 align in Batch mode to decode an example utterance.
echo .
echo This batch script assumes all files are relative to the main
echo directory (S3ROOT).
echo .

%S3BATCH% -logbase 1.0003 -mdeffn ./model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/hub4opensrc.6000.mdef -meanfn ./model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/means -varfn ./model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/variances -mixwfn ./model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/mixture_weights -tmatfn ./model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/transition_matrices -feat 1s_c_d_dd -topn 1000 -beam 1e-80 -senmgaufn .cont. -ctlfn ./model/lm/an4/an4.ctl -cepdir ./model/lm/an4/  -phsegdir ./


popd

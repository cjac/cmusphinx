echo off

set S3ROOT=..\..
pushd %S3ROOT%
set S3BATCH=.\bin\Debug\decode.exe
set TASK=.\model\lm\an4
set CTLFILE=.\win32\batch\an4.ctl

echo . 
echo sphinx3-test
echo Run CMU Sphinx-3's decode in Batch mode to decode an example utterance.
echo .
echo This batch script assumes all files are relative to the main
echo directory (S3ROOT).
echo .
echo When running this, look for a line that starts with "FWDVIT:"
echo If the installation is correct, this line should read:
echo FWDVID: P I T T S B U R G H (null)


%S3BATCH% -mdef ./model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/hub4opensrc.6000.mdef -fdict ./model/lm/an4/filler.dict -dict ./model/lm/an4/an4.dict -mean ./model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/means -var ./model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/variances -mixw ./model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/mixture_weights -tmat ./model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd/transition_matrices -lm ./model/lm/an4/an4.ug.lm.DMP -ctl ./model/lm/an4/an4.ctl -cepdir ./model/lm/an4/ -agc none -varnorm no -cmn current -subvqbeam 1e-02 -epl 4 -fillprob 0.02 -feat 1s_c_d_dd -lw 9.5 -maxwpf 1 -beam 1e-40 -pbeam 1e-30 -wbeam 1e-20 -maxhmmpf 1500 -wend_beam 1e-1 -ci_pbeam 1e-3 -ds 2  

popd

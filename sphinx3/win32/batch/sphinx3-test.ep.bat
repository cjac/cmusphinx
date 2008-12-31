echo off

set S3ROOT=..\..
pushd %S3ROOT%
set S3BATCH=.\bin\Debug\ep.exe
set TASK=.\model\lm\an4
set CTLFILE=.\win32\batch\an4.ctl

echo . 
echo sphinx3-test.ep
echo Run CMU Sphinx-3's decode in Batch mode to decode an example utterance.
echo .
echo This batch script assumes all files are relative to the main
echo directory (S3ROOT).
echo .

%S3BATCH% -i ./model/ep/chan3.raw -srate 11025 -frate 105 -wlen 0.024 -alpha 0.97 -ncep 13 -nfft 512 -nfilt 36 -upperf 5400 -lowerf 130 -blocksize 262500 -mean ./model/ep/means -var ./model/ep/variances -mixw ./model/ep/mixture_weights 

popd

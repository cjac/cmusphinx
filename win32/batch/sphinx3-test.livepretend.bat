echo off

set S3ROOT=..\..
cd %S3ROOT%
set S3BATCH=.\bin\Debug\livepretend.exe
set TASK=.\model\lm\an4
set CTLFILE=.\win32\batch\an4.ctl
set ARGS=.\model\lm\an4\args.an4.test


echo . 
echo sphinx3-test
echo Run CMU Sphinx-3's livepretend in Batch mode to decode an example utterance.
echo .
echo This batch script assumes all files are relative to the main
echo directory (S3ROOT).
echo .
echo When running this, look for a line that starts with "FWDVIT:"
echo If the installation is correct, this line should read:
echo FWDVID: P I T T S B U R G H (null)


%S3BATCH% %CTLFILE% %TASK% %ARGS%


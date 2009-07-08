echo off
echo .
REM You can set S3ROOT in your environment and it won't be overridden here.
if not defined S3ROOT set S3ROOT=..\..
echo S3ROOT == %S3ROOT%

pushd %S3ROOT%

REM You can set S3BATCH in you environment and it won't be overridden here.
if not defined S3BATCH set S3BATCH=.\win32\msdev\programs\livepretend\Debug\livepretend.exe
echo S3BATCH == %S3BATCH%

REM you can set TASK in your environment and it won't be overridden here.
if not defined TASK set TASK=.\model\lm\an4
echo TASK == %TASK%

REM you can set CTLFILE in your environment and it won't be overridden here.
if not defined CTLFILE set CTLFILE=.\win32\batch\an4.ctl
echo CTLFILE == %CTLFILE%

REM you can set ARGS in your environment and it won't be overridden here.
if not defined ARGS set ARGS=.\model\lm\an4\args.an4.test
echo ARGS == %ARGS%

echo . 
echo sphinx3-test
echo Run CMU Sphinx-3 in Batch mode to decode an example utterance.
echo .
echo This batch script assumes all files are relative to the main
echo directory (S3ROOT).
echo .
echo When running this, look for a line that starts with "FWDVIT:"
echo If the installation is correct, this line should read:
echo FWDVID: P I T T S B U R G H (null)

echo on

%S3BATCH% %CTLFILE% %TASK% %ARGS%

echo off

popd


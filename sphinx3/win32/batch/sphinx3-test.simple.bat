echo off

set S3ROOT=..\..
cd %S3ROOT%
set S3BATCH=.\bin\Debug\align.exe
set TASK=.\model\lm\an4
set CTLFILE=.\win32\batch\an4.ctl

echo . 
echo sphinx3-test
echo Simple test
echo .

cd .\win32\batch\
.\sphinx3-test.livepretend.bat
cd ..\..\

echo off

rem This batch script assumes all files are relative to the main
rem directory (S3ROOT).

set S3ROOT=..\..
cd %S3ROOT%
set S3CONTINUOUS=.\bin\Debug\livedecode.exe

set ARGS=.\model\lm\an4\args.an4.test

echo " "
echo "sphinx3-simple:"
echo "  Demo CMU Sphinx-3 decoder called with command line arguments."
echo " "

echo "<executing $S3CONTINUOUS, please wait>"
%S3CONTINUOUS% %ARGS%


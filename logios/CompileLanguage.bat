:: recompile language without going through the entire build process
:: [20080220] (air)

::::::  SPECIFY LOGIOS ROOT  ( assume that script is run in expected location)  ::::::
SET LOGIOS_ROOT=%CD%

::::::  CHANGE THESE LABELS AS PER YOUR PROJECT   ::::::
:: INSTANCE will be the base name for all generated files
SET PROJECT=MeetingLineDomain
SET INSTANCE=MeetingLine

:: Compile language knowledge bases from the grammar
echo Executing MakeLanguage
cd Resources
perl %LOGIOS_ROOT%\Tools\MakeLanguage\make_language.pl --logios %LOGIOS_ROOT% --project %PROJECT% --instance %INSTANCE% --logfile %INSTANCE%.log
cd ..

:: give user a chance to examine the trace
pause
::

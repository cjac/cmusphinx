:: Compile a language knowledge base from a grammar
:: [20080707] (air)

::::::  ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::  ::::::
::::::  FIRST: Copy this script into your Project root folder                             ::::::
::::::  SECOND: Change the following line to point to your Logios installation            ::::::
::::::  or have the environment variable set appropriately                                ::::::

IF NOT DEFINED LOGIOS_ROOT SET LOGIOS_ROOT=%CD%

::::::  THIRD: (for Olympus) create your project grammar in Resources\Grammar\GRAMMAR     ::::::
::::::  (the following script creates the folder tree)                                    ::::::
::::::  If this is not for Olympus, you'll need to specify --inpath and --outpath, below  ::::::

::::::  CHANGE THESE LABELS AS PER YOUR PROJECT; OUTPUT FILES WILL BE NAMED ACCORDINGLY   ::::::

SET PROJECT=MeetingLineDomain
SET INSTANCE=MeetingLine

::::::  ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::  ::::::


:::::: Compile language knowledge bases from the grammar
:::::: --olympus and --resources flag must be used together, in which case the Resources/ tree is used

CALL %LOGIOS_ROOT%\makeTree.bat
SET RESOURCES=%CD%\Resources
perl %LOGIOS_ROOT%\Tools\MakeLanguage\make_language.pl --logios %LOGIOS_ROOT% --olympus --resources %RESOURCES% --project %PROJECT% --instance %INSTANCE% --logfile %INSTANCE%.log

:::::: otherwise --inpath and --outpath give you control over actual locations
:: SET INPATH=%CD%\Resources\Grammar\GRAMMAR
:: SET OUTPATH=%CD%\Resources\Grammar\GRAMMAR
::perl %LOGIOS_ROOT%\Tools\MakeLanguage\make_language.pl --logios %LOGIOS_ROOT% --inpath %INPATH% --outpath %OUTPATH% --project %PROJECT% --instance %INSTANCE% --logfile %INSTANCE%.log

:: give user a chance to examine the trace
pause
::

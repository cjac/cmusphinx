:: recompile language without going through the entire build process
:: [20080220] (air)
:: should be run in the dialog project folder

:: Change these labels as per the project
SET PROJECT=TeamTalk
SET INSTANCE=TreasureHunt

:: Run MakeLanguage
:: will compile language knowledge bases from the grammar
echo Executing MakeLanguage
cd Resources
perl ..\Tools\MakeLanguage\make_language.pl --project %PROJECT% --instance %INSTANCE% --logfile logfile
cd ..

::

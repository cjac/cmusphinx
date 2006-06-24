del /s /q classes

mkdir classes

javac -cp ..\..\common\lib\sphinx4.jar;..\..\common\lib\batch.jar;..\..\common\lib\javolution.jar;..\..\common\lib\forms_rt.jar -sourcepath ..\src\java;..\..\common\src\java -d classes ..\src\java\edu\cmu\sphinx\tools\dictator\DictatorView.java

cd classes
jar -xvf ..\..\..\common\lib\sphinx4.jar
jar -xvf ..\..\..\common\lib\forms_rt.jar
jar -xvf ..\..\..\common\data\WSJ_8gau_13dCep_16k_40mel_130Hz_6800Hz.jar

cd ..

copy ..\..\common\data\wsj5k.dmp

jar -cmvf MANIFEST.MF dictator.jar -C classes\ . 
jar -uvf dictator.jar dictator.config.xml

del /s /q classes

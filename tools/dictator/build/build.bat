del /s /q classes
del wsj5k.DMP
del dictator.jar

mkdir classes

javac -cp "C:\Program Files\Java\jdk1.5.0_04\jre\lib\javaws.jar";..\..\common\lib\sphinx4.jar;..\..\common\lib\batch.jar;..\..\common\lib\javolution.jar;..\..\common\lib\forms_rt.jar -sourcepath ..\src\java;..\..\common\src\java -d classes ..\src\java\edu\cmu\sphinx\tools\dictator\DictatorView.java

cd classes

jar -xvf ..\..\..\common\lib\sphinx4.jar
jar -xvf ..\..\..\common\lib\forms_rt.jar
jar -xvf ..\..\..\common\data\WSJ_8gau_13dCep_16k_40mel_130Hz_6800Hz.jar
copy ..\..\..\common\data\wsj5k.dmp
copy ..\dictator.config.xml

cd ..

jar -cmvf MANIFEST.MF dictator.jar -C classes\ .

del /s /q classes

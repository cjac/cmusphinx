import platform
import os
import fnmatch

Import('common')

srcDir = '../tools/dictator/src/java'
classDir = os.path.normpath('../../scons_build/classes/dictator')

libpath = '..' + os.sep + 'tools' + os.sep + 'common' + os.sep + 'lib' + os.sep
classpath =  libpath + 'batch.jar' + os.pathsep
classpath += libpath + 'dom4j-1.6.1.jar' + os.pathsep
classpath += libpath + 'forms_rt_license.jar' + os.pathsep
classpath += libpath + 'forms_rt.jar' + os.pathsep
classpath += libpath + 'javolution.jar' + os.pathsep
classpath += libpath + 'sphinx4.jar' + os.pathsep
classpath += str(common[0])
wsjdmp = Install(classDir, '../tools/common/data/wsj5k.DMP')

env = Environment(ENV=os.environ, JAVACFLAGS='-source 1.5 -classpath ' + classpath, JARCHDIR=classDir)
classes =  env.Java(target = classDir, source = srcDir )

jarFile = os.path.normpath('../../scons_build/jars/dictator.jar')
dictator = env.Jar(target = jarFile, source = [classDir, '../tools/dictator/build/MANIFEST.MF'])

Depends(classes, common)
Depends(wsjdmp,classes)
Depends(dictator, wsjdmp)

Export('dictator')
import platform
import os
import fnmatch

Import('javapath')
Import('sphinx4')
Import('common')

srcDir = os.path.normpath('../tools/dictator/src/java')
classDir = os.path.normpath('../../scons_build/classes/dictator')

libpath = '..' + os.sep + 'tools' + os.sep + 'common' + os.sep + 'lib' + os.sep
classpath =  libpath + 'batch.jar' + os.pathsep
classpath += libpath + 'dom4j-1.6.1.jar' + os.pathsep
classpath += libpath + 'forms_rt_license.jar' + os.pathsep
classpath += libpath + 'forms_rt.jar' + os.pathsep
classpath += libpath + 'javolution.jar' + os.pathsep
classpath += str(sphinx4[0]) + os.pathsep
classpath += str(common[0])

wsjdmp = Install(classDir, '../tools/common/data/wsj5k.DMP')

env = Environment(ENV = {'PATH' : javapath }, JAVACFLAGS = '-source 1.5 -classpath "' + classpath + '"', JARCHDIR = classDir)
classes =  env.Java(target = classDir, source = srcDir )

jarFile = os.path.normpath('../../scons_build/jars/dictator.jar')
manifest =  os.path.normpath('../tools/dictator/build/MANIFEST.MF')
dictator = env.Jar(target = jarFile, source = [classDir, manifest])

Depends(classes, common)
Depends(wsjdmp, classes)
Depends(dictator, wsjdmp)

Export('dictator')

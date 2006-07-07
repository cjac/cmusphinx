import platform
import os
import fnmatch

Import('sphinx4')
Import('common')

srcDir = os.path.normpath('../tools/dictator/src/java')
classDir = os.path.normpath('../../scons_build/classes/dictator')

classpath =  '../tools/common/lib/batch.jar:'
classpath += '../tools/common/lib/dom4j-1.6.1.jar:'
classpath += '../tools/common/lib/forms_rt_license.jar:'
classpath += '../tools/common/lib/forms_rt.jar:'
classpath += '../tools/common/lib/javolution.jar:'
classpath += str(sphinx4[0]) + ':'
classpath += str(common[0])
classpath = classpath.replace('/',os.sep).replace(':',os.pathsep)

wsjdmp = Install(classDir, '../tools/common/data/wsj5k.DMP')

env = Environment(ENV=os.environ, JAVACFLAGS='-source 1.5 -classpath ' + classpath, JARCHDIR=classDir)
classes =  env.Java(target = classDir, source = srcDir )

jarFile = os.path.normpath('../../scons_build/jars/dictator.jar')
manifest =  os.path.normpath('../tools/dictator/build/MANIFEST.MF')
dictator = env.Jar(target = jarFile, source = [classDir, manifest])

Depends(classes, common)
Depends(wsjdmp,classes)
Depends(dictator, wsjdmp)

Export('dictator')
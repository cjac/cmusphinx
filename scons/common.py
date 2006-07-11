import platform
import os
import fnmatch
Import('javapath')
Import('sphinx4')

srcDir = os.path.normpath('../tools/common/src/java:../../')
classDir = os.path.normpath('../../scons_build/classes/common')

libpath = '..' + os.sep + 'tools' + os.sep + 'common' + os.sep + 'lib' + os.sep
classpath =  libpath + 'batch.jar' + os.pathsep
classpath += libpath + 'dom4j-1.6.1.jar' + os.pathsep
classpath += libpath + 'forms_rt_license.jar' + os.pathsep
classpath += libpath + 'forms_rt.jar' + os.pathsep
classpath += libpath + 'javolution.jar' + os.pathsep
classpath += str(sphinx4[0])

env = Environment(ENV = {'PATH' : javapath }, JAVACFLAGS = '-source 1.5 -classpath "' + classpath + '"', JARCHDIR = classDir)
classes = env.Java(target = classDir, source = srcDir)
Depends(classes, sphinx4)

jarFile = os.path.normpath('../../scons_build/jars/common.jar')
common = env.Jar(target = jarFile, source = classDir)


Export('common')

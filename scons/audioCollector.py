import platform
import os
import fnmatch

Import('common')

srcDir = '../tools/audioCollector/src/java'
classDir = os.path.normpath('../../scons_build/classes/audioCollector')
javaHome = os.path.normpath(os.environ['JAVA_HOME'])

libpath = '..' + os.sep + 'tools' + os.sep + 'common' + os.sep + 'lib' + os.sep
classpath =  libpath + 'batch.jar' + os.pathsep
classpath += libpath + 'dom4j-1.6.1.jar' + os.pathsep
classpath += libpath + 'forms_rt_license.jar' + os.pathsep
classpath += libpath + 'forms_rt.jar' + os.pathsep
classpath += libpath + 'javolution.jar' + os.pathsep
classpath += libpath + 'sphinx4.jar' + os.pathsep
classpath += javaHome + os.sep + 'lib' + os.sep + 'plugin.jar' + os.pathsep
classpath += str(common[0])

env = Environment(ENV=os.environ, JAVACFLAGS='-source 1.5 -classpath ' + classpath, JARCHDIR=classDir)
classes =  env.Java(target = classDir, source = srcDir )
Depends(classes, common)
jarFile = os.path.normpath('../../scons_build/jars/audioCollector.jar')
audioCollector = env.Jar(target = jarFile, source = classDir)

Export('audioCollector')
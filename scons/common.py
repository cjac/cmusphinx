import platform
import os
import fnmatch

srcDir = '../tools/common/src/java:../../'
classDir = os.path.normpath('../../scons_build/classes/common')

libpath = '..' + os.sep + 'tools' + os.sep + 'common' + os.sep + 'lib' + os.sep
classpath =  libpath + 'batch.jar' + os.pathsep
classpath += libpath + 'dom4j-1.6.1.jar' + os.pathsep
classpath += libpath + 'forms_rt_license.jar' + os.pathsep
classpath += libpath + 'forms_rt.jar' + os.pathsep
classpath += libpath + 'javolution.jar' + os.pathsep
classpath += libpath + 'sphinx4.jar' + os.pathsep

env = Environment(ENV=os.environ, JAVACFLAGS='-source 1.5 -classpath ' + classpath, JARCHDIR=classDir)
env.Java(target = classDir, source = srcDir)


jarFile = os.path.normpath('../../scons_build/jars/common.jar')
common = env.Jar(target = jarFile, source = classDir)

Export('common')
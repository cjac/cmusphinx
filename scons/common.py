import platform
import os
import fnmatch

srcDir = '../tools/common/src/java:../../'
classDir = os.path.normpath('../../scons_build/classes/common')

classpath =  '../tools/common/lib/batch.jar:'
classpath += '../tools/common/lib/dom4j-1.6.1.jar:'
classpath += '../tools/common/lib/forms_rt_license.jar:'
classpath += '../tools/common/lib/forms_rt.jar:'
classpath += '../tools/common/lib/javolution.jar:'
classpath += '../tools/common/lib/sphinx4.jar:'

env = Environment(ENV=os.environ, JAVACFLAGS='-source 1.5 -classpath ' + classpath, JARCHDIR=classDir)
env.Java(target = classDir, source = srcDir)


jarFile = os.path.normpath('../../scons_build/jars/common.jar')
common = env.Jar(target = jarFile, source = classDir)

Export('common')
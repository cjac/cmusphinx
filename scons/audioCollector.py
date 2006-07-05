import platform
import os
import fnmatch

Import('common')

srcDir = '../tools/audioCollector/src/java'
classDir = os.path.normpath('../../scons_build/classes/audioCollector')
javaHome = os.path.normpath(os.environ['JAVA_HOME'])

classpath =  '../tools/common/lib/batch.jar:'
classpath += '../tools/common/lib/dom4j-1.6.1.jar:'
classpath += '../tools/common/lib/forms_rt_license.jar:'
classpath += '../tools/common/lib/forms_rt.jar:'
classpath += '../tools/common/lib/javolution.jar:'
classpath += '../tools/common/lib/sphinx4.jar:'
classpath += javaHome + '/lib/plugin.jar:'
classpath += str(common[0])

env = Environment(ENV=os.environ, JAVACFLAGS='-source 1.5 -classpath ' + classpath, JARCHDIR=classDir)
classes =  env.Java(target = classDir, source = srcDir )
Depends(classes, common)
jarFile = os.path.normpath('../../scons_build/jars/audioCollector.jar')
audioCollector = env.Jar(target = jarFile, source = classDir)

Export('audioCollector')
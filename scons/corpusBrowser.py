import platform
import os
import fnmatch

Import('common')

srcDir = '../tools/corpusBrowser/src/java'
classDir = os.path.normpath('../../scons_build/classes/corpusBrowser')

classpath =  '../tools/common/lib/batch.jar:'
classpath += '../tools/common/lib/dom4j-1.6.1.jar:'
classpath += '../tools/common/lib/forms_rt_license.jar:'
classpath += '../tools/common/lib/forms_rt.jar:'
classpath += '../tools/common/lib/javolution.jar:'
classpath += '../tools/common/lib/sphinx4.jar:'
classpath += str(common[0])

env = Environment(ENV=os.environ, JAVACFLAGS='-source 1.5 -classpath ' + classpath, JARCHDIR=classDir)
classes =  env.Java(target = classDir, source = srcDir )
Depends(classes, common)
jarFile = os.path.normpath('../../scons_build/jars/corpusBrowser.jar')
corpusBrowser = env.Jar(target = jarFile, source = classDir)

Export('corpusBrowser')
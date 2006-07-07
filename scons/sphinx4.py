import platform
import os
import fnmatch

srcDir = os.path.normpath('../sphinx4/src/sphinx4')
classDir = os.path.normpath('../../scons_build/classes/sphinx4')

classpath =  '../sphinx4/lib/js.jar'
classpath = classpath.replace('/',os.sep).replace(':',os.pathsep)

env = Environment(ENV=os.environ, JAVACFLAGS='-source 1.5 -classpath ' + classpath, JARCHDIR=classDir)
classes =  env.Java(target = classDir, source = srcDir )

jarFile = os.path.normpath('../../scons_build/jars/sphinx4.jar')
sphinx4 = env.Jar(target = jarFile, source = classDir)

Export('sphinx4')
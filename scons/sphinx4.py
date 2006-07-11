import platform
import os
import fnmatch
Import('javapath')

srcDir = os.path.normpath('../sphinx4/src/sphinx4')
classDir = os.path.normpath('../../scons_build/classes/sphinx4')

classpath =  '../sphinx4/lib/js.jar'
classpath = classpath.replace('/', os.sep)

env = Environment(ENV = {'PATH' : javapath }, JAVACFLAGS = '-source 1.5 -classpath "' + classpath + '"', JARCHDIR = classDir)
classes =  env.Java(target = classDir, source = srcDir )

#Windows sucks but the next three lines make it suck less
if platform.system() == 'Windows':
    env['_JAVACCOM']         = '$JAVAC $JAVACFLAGS -d ${TARGET.attributes.java_classdir} -sourcepath ${SOURCE.dir.rdir()} $SOURCES'
    env['JAVACCOM']="${TEMPFILE('$_JAVACCOM')}"

jarFile = os.path.normpath('../../scons_build/jars/sphinx4.jar')
sphinx4 = env.Jar(target = jarFile, source = classDir)

Export('sphinx4')

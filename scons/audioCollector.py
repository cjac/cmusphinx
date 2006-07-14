import platform
import os
import fnmatch

Import('javapath')
Import('common')

srcDir = os.path.normpath('../tools/audioCollector/src/java')
clientSrcDir = os.path.normpath(srcDir + '/edu/cmu/sphinx/tools/datacollection/client/')
serverSrcDir = os.path.normpath(srcDir + '/edu/cmu/sphinx/tools/datacollection/server/')
clientClassDir = os.path.normpath('../../scons_build/classes/audioCollector/client')
serverClassDir = os.path.normpath('../../scons_build/classes/audioCollector/server')
sphinx_store = os.path.normpath('../tools/audioCollector/build/sphinx_store')

javaHome = os.path.normpath(os.environ['JAVA_HOME'])

libpath = '..' + os.sep + 'tools' + os.sep + 'common' + os.sep + 'lib' + os.sep
classpath =  libpath + 'batch.jar' + os.pathsep
classpath += libpath + 'dom4j-1.6.1.jar' + os.pathsep
classpath += libpath + 'forms_rt_license.jar' + os.pathsep
classpath += libpath + 'forms_rt.jar' + os.pathsep
classpath += libpath + 'javolution.jar' + os.pathsep
classpath += libpath + 'sphinx4.jar' + os.pathsep
classpath += libpath + 'plugin.jar' + os.pathsep
classpath += str(common[0])

print classpath


envServer = Environment(ENV = {'PATH' : javapath }, JAVACFLAGS='-source 1.5 -classpath ' + classpath)
serverClasses =  envServer.Java(target = serverClassDir, source = serverSrcDir )
Depends(serverClasses, common)
serverJarFile = os.path.normpath('../../scons_build/jars/datacollection-server.jar')
serverJar = envServer.Jar(target = serverJarFile, source = serverClassDir, JARCHDIR=serverClassDir)

envClient = Environment(ENV = {'PATH' : javapath }, JAVACFLAGS='-source 1.5 -classpath ' + classpath)
clientClasses =  envClient.Java(target = clientClassDir, source = clientSrcDir )
Depends(clientClasses, common)
clientJarFile = os.path.normpath('../../scons_build/jars/datacollection-client.jar')
clientJar = envServer.Jar(target = clientJarFile, source = clientClassDir, JARCHDIR=clientClassDir)

# Add images to the class dir so they'll end up in the jar
for root, dirs, files in os.walk(top=clientSrcDir, topdown=False):
    for name in files:
        if fnmatch.fnmatch(name, '*.gif') == True:
            f = Install(root.replace(srcDir,clientClassDir), os.path.join(root, name))
            Depends(f, clientClasses)
            Depends(clientJar, f)


shipDir = '../../scons_ship/audioCollector'
shipAudioCollector = Alias('foo', shipDir)

Install(shipDir + '/jsps', '../tools/audioCollector/src/jsps/audio_collector.jsp')
Install(shipDir + '/jsps', '../tools/audioCollector/src/jsps/corpus_store.jsp')
Install(shipDir + '/jsps', '../tools/audioCollector/src/jsps/login_process.jsp')
Install(shipDir + '/jsps', '../tools/audioCollector/src/jsps/login.jsp')
Install(shipDir + '/configs', '../tools/audioCollector/data/samples/recorder.config.xml')
Install(shipDir + '/configs', '../tools/audioCollector/data/samples/subjects.xml')
Install(shipDir + '/configs', '../tools/audioCollector/data/samples/transcripts_en_us.txt')
Install(shipDir + '/configs', '../tools/audioCollector/data/samples/transcripts_zh.txt')

#Install server jars
Install(shipDir +  '/WEB-INF', '../tools/audioCollector/data/configs/web.xml')
loc = Install(shipDir + '/WEB-INF/lib', '../tools/common/lib/batch.jar')
sign = envServer.Command('noSuchFile0', loc, "jarsigner -keystore " + sphinx_store + "  -storepass sphinx4 $SOURCE audiocollector")
Depends(shipAudioCollector, sign)


loc = Install(shipDir + '/WEB-INF/lib', '../tools/common/lib/javolution.jar')
sign = envServer.Command('noSuchFile1', loc, "jarsigner -keystore " + sphinx_store + "  -storepass sphinx4 $SOURCE audiocollector")
Depends(shipAudioCollector, sign)

loc = Install(shipDir + '/WEB-INF/lib', '../tools/common/lib/sphinx4.jar')
sign = envServer.Command('noSuchFile2', loc, "jarsigner -keystore " + sphinx_store + "  -storepass sphinx4 $SOURCE audiocollector")
Depends(shipAudioCollector, sign)

loc = Install(shipDir + '/WEB-INF/lib', serverJarFile)
sign = envServer.Command('noSuchFile3', loc, "jarsigner -keystore " + sphinx_store + "  -storepass sphinx4 $SOURCE audiocollector")
Depends(shipAudioCollector, sign)

loc = InstallAs(shipDir + '/WEB-INF/lib/corpus.jar', str(common[0]))
sign = envServer.Command('noSuchFile4', loc, "jarsigner -keystore " + sphinx_store + "  -storepass sphinx4 $SOURCE audiocollector")
Depends(shipAudioCollector, sign)

#Install client jars & sign them
loc = Install(shipDir, '../tools/common/lib/batch.jar')
sign = envServer.Command('noSuchFile5', loc, "jarsigner -keystore " + sphinx_store + "  -storepass sphinx4 $SOURCE audiocollector")
Depends(shipAudioCollector, sign)

loc = Install(shipDir, '../tools/common/lib/javolution.jar')
sign = envServer.Command('noSuchFile6', loc, "jarsigner -keystore " + sphinx_store + " -storepass sphinx4 $SOURCE audiocollector")
Depends(shipAudioCollector, sign)

loc = Install(shipDir, '../tools/common/lib/sphinx4.jar')
sign = envServer.Command('noSuchFile7', loc, "jarsigner -keystore " + sphinx_store + " -storepass sphinx4 $SOURCE audiocollector")
Depends(shipAudioCollector, sign)

loc = Install(shipDir, clientJarFile)
sign = envServer.Command('noSuchFile8', loc, "jarsigner -keystore " + sphinx_store + " -storepass sphinx4 $SOURCE audiocollector")
Depends(shipAudioCollector, sign)

loc = InstallAs(shipDir + '/corpus.jar', str(common[0]))
sign = envServer.Command('noSuchFile9', loc, "jarsigner -keystore " + sphinx_store + " -storepass sphinx4 $SOURCE audiocollector")
Depends(shipAudioCollector, sign)


audioCollector = Alias('foo2', [serverJar, clientJar])



Export('audioCollector')
Export('shipAudioCollector')


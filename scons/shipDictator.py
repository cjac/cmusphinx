import subprocess
import platform
import os
import sys
import string

Import('javapath')
Import('dictator')

dir = '../../scons_ship/dictator'

Install(dir, '../../scons_build/jars/common.jar')
Install(dir, '../tools/common/lib/batch.jar')
Install(dir, '../tools/common/lib/dom4j-1.6.1.jar')
Install(dir, '../tools/common/lib/forms_rt_license.txt')
Install(dir, '../tools/common/lib/forms_rt.jar')
Install(dir, '../tools/common/lib/javolution.jar')
Install(dir, '../tools/common/lib/sphinx4.jar')
Install(dir, '../tools/common/data/WSJ_8gau_13dCep_16k_40mel_130Hz_6800Hz.jar')
Install(dir, '../tools/dictator/build/dictator.config.xml')
Install(dir, str(dictator[0]))

shipDictator = Alias('foo', dir)


classpath =  'WSJ_8gau_13dCep_16k_40mel_130Hz_6800Hz.jar' + os.pathsep
classpath += 'batch.jar' + os.pathsep
classpath += 'common.jar' + os.pathsep
classpath += 'dictator.jar' + os.pathsep
classpath += 'dom4j-1.6.1.jar' + os.pathsep
classpath += 'forms_rt.jar' + os.pathsep
classpath += 'javolution.jar' + os.pathsep
classpath += 'sphinx4.jar'

def build(target, source, env):
    execenv = Environment(ENV = os.environ)
    execenv['ENV']['PATH'] = javapath
    subprocessenv = execenv['ENV']
    cmd = 'java -Xmx256m -classpath "' + classpath + '" edu.cmu.sphinx.tools.dictator.DictatorView'
    P = subprocess.Popen(cmd, cwd = dir, env = subprocessenv, shell = True)
    return P.wait()

env = Environment(ENV = {'PATH' : javapath }, BUILDERS = {'RunDictator' : Builder(action = build)})
runDictator = env.RunDictator('bar', shipDictator)
Depends(runDictator, shipDictator)

Export('shipDictator')
Export('runDictator')

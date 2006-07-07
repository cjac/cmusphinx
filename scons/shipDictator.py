import platform
import os

dir = '../../scons_ship/dictator'
Import('dictator')

Install(dir, '../../scons_build/jars/common.jar')
Install(dir, '../tools/common/lib/batch.jar')
Install(dir, '../tools/common/lib/dom4j-1.6.1.jar')
Install(dir, '../tools/common/lib/forms_rt_license.txt')
Install(dir, '../tools/common/lib/forms_rt.jar')
Install(dir, '../tools/common/lib/javolution.jar')
Install(dir, '../tools/common/lib/sphinx4.jar')
Install(dir, '../tools/common/data/WSJ_8gau_13dCep_16k_40mel_130Hz_6800Hz.jar')
Install(dir, '../tools/dictator/build/dictator.config.xml')
Install(dir, '../tools/dictator/run/run.sh')
Install(dir, str(dictator[0]))

shipDictator = Alias('foo', dir)


classpath =  'WSJ_8gau_13dCep_16k_40mel_130Hz_6800Hz.jar:batch.jar:'
classpath += 'common.jar:'
classpath += 'dictator.jar:'
classpath += 'dom4j-1.6.1.jar:'
classpath += 'forms_rt.jar:'
classpath += 'javolution.jar:'
classpath += 'sphinx4.jar'
classpath = classpath.replace('/',os.sep).replace(':',os.pathsep)

def build(target, source, env):
    os.chdir(dir)
    os.system('java -Xmx256m -classpath ' + classpath + ' edu.cmu.sphinx.tools.dictator.DictatorView')
    return None

env = Environment(BUILDERS = {'RunDictator' : Builder(action = build)})
runDictator = env.RunDictator('bar', shipDictator)
Depends(runDictator, shipDictator)


Export('shipDictator')
Export('runDictator')
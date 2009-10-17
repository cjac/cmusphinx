from distutils.core import setup, Extension
import os

# For no good reason, CL.EXE doesn't actually define this constant
if os.name == 'nt':
    define_macros = [('WIN32', '1')]
else:
    define_macros = []

module = Extension('_sphinx3',
                   include_dirs = ['../../sphinxbase/include',
                                   '../include',
                                   '/usr/local/include/sphinxbase/',
                                   '/usr/local/include/sphinx3',
                                   ],
		   define_macros = define_macros,
		   library_dirs = ['../../sphinxbase/src/libsphinxbase/.libs',
				'../src/libs3decoder/.libs',
				'../../sphinxbase/lib/debug',
				'../lib/debug',
				],
                   libraries = ['sphinxbase', 's3decoder'],
                   sources = ['_sphinx3module.c'])

setup(name = 'Sphinx3',
      version = '0.1',
      description = 'Python interface to Sphinx3 speech recognition',
      ext_modules = [module])

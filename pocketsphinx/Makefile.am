SUBDIRS = src \
	  doc \
	  model \
	  include \
	  scripts \
	  test \
	python

EXTRA_DIST = autogen.sh								\
	pocketsphinx.sln							\
	pocketsphinx.pc.in							\
	pocketsphinx.sln							\
	win32/msdev/pocketsphinx/pocketsphinx.vcproj				\
	win32/msdev/pocketsphinx_batch/pocketsphinx.args			\
	win32/msdev/pocketsphinx_batch/pocketsphinx_batch.vcproj		\
	win32/msdev/pocketsphinx_continuous/pocketsphinx.args			\
	win32/msdev/pocketsphinx_continuous/pocketsphinx_continuous.vcproj	\
	win32/msdev/pocketsphinx_ptt/pocketsphinx.args				\
	win32/msdev/pocketsphinx_ptt/pocketsphinx_ptt.vcproj			\
	m4/pkg.m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = pocketsphinx.pc
CLEANFILES = pocketsphinx.pc

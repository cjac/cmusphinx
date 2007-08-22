my_plugins = 
if BUILD_POCKETSPHINX
my_plugins += libgstpocketsphinx.la
endif
if BUILD_SPHINX3
my_plugins += libgstsphinx3.la
endif

plugin_LTLIBRARIES = $(my_plugins)

libgstpocketsphinx_la_SOURCES = gstpocketsphinx.c
libgstpocketsphinx_la_CFLAGS = $(GST_CFLAGS) $(PocketSphinx_CFLAGS) $(SphinxBase_CFLAGS) \
	-DPOCKETSPHINX_PREFIX=\"$(POCKETSPHINX_PREFIX)\"
libgstpocketsphinx_la_LIBADD = $(GST_LIBS) $(PocketSphinx_LIBS) $(SphinxBase_LIBS) \
	  -lgstaudio-$(GST_MAJORMINOR)
libgstpocketsphinx_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

libgstsphinx3_la_SOURCES = gstsphinx3.c
libgstsphinx3_la_CFLAGS = $(GST_CFLAGS) $(Sphinx3_CFLAGS) $(SphinxBase_CFLAGS) \
	-DSPHINX3_PREFIX=\"$(SPHINX3_PREFIX)\"
libgstsphinx3_la_LIBADD = $(GST_LIBS) $(Sphinx3_LIBS) $(SphinxBase_LIBS) \
	  -lgstaudio-$(GST_MAJORMINOR)
libgstsphinx3_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

# headers we need but don't want installed
noinst_HEADERS = gstpocketsphinx.h gstsphinx3.h

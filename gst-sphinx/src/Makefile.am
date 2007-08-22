# plugindir is set in configure

##############################################################################
# change libgstplugin.la to something more suitable, e.g. libmysomething.la  #
##############################################################################
plugin_LTLIBRARIES = libgstpocketsphinx.la

##############################################################################
# for the next set of variables, rename the prefix if you renamed the .la,   #
#  e.g. libgstplugin_la_SOURCES => libmysomething_la_SOURCES                 #
#       libgstplugin_la_CFLAGS  => libmysomething_la_CFLAGS                  #
#       libgstplugin_la_LIBADD  => libmysomething_la_LIBADD                  #
#       libgstplugin_la_LDFLAGS => libmysomething_la_LDFLAGS                 #
##############################################################################

# sources used to compile this plug-in
libgstpocketsphinx_la_SOURCES = gstpocketsphinx.c

# flags used to compile this plugin
# add other _CFLAGS and _LIBS as needed
libgstpocketsphinx_la_CFLAGS = $(GST_CFLAGS) $(PocketSphinx_CFLAGS) $(SphinxBase_CFLAGS)
libgstpocketsphinx_la_LIBADD = $(GST_LIBS) $(PocketSphinx_LIBS) $(SphinxBase_LIBS) \
	  -lgstaudio-$(GST_MAJORMINOR)
libgstpocketsphinx_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)

# headers we need but don't want installed
noinst_HEADERS = gstpocketsphinx.h

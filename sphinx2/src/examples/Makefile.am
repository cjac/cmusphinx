bin_PROGRAMS = \
	sphinx2-continuous \
	sphinx2-ptt \
	sphinx2-batch \
	sphinx2-server \
	sphinx2-client \
	sphinx2-allphone \
	adrec \
	adpow \
	cont_adseg \
	cont_fileseg \
        lm3g2dmp
	# cdcn_test

sphinx2_continuous_SOURCES =  tty-continuous.c

sphinx2_ptt_SOURCES = tty-ptt.c

sphinx2_batch_SOURCES = batch.c

sphinx2_server_SOURCES = \
	server.c \
	srvcore.c

sphinx2_client_SOURCES = \
	client.c \
	clicore.c

sphinx2_allphone_SOURCES = allphone-test.c

adrec_SOURCES = adrec.c

adpow_SOURCES = adpow.c

cont_adseg_SOURCES = cont_adseg.c

cont_fileseg_SOURCES = cont_fileseg.c

lm3g2dmp_SOURCES = lm3g2dmp.c

# cdcn_test_SOURCES = cdcn_test.c

EXTRA_DIST = clicore.h  srvcore.h

LDADD = $(top_builddir)/src/libsphinx2/.libs/libsphinx2.a -lm

INCLUDES = -I$(top_srcdir)/include  -I$(top_srcdir)/src/libsphinx2/include \
		-I$(top_srcdir)/src/examples



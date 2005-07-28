bin_PROGRAMS = adpow \
	adrec \
	cdcn_test \
	cont_adseg \
	cont_fileseg \
	lm_fsg_test \
	lm_fsg_test2 \
        lm3g2dmp \
	pdf32to8b \
	raw2cep \
	sphinx2-allphone \
	sphinx2-batch \
	sphinx2-continuous \
	sphinx2-ptt \
	sphinx2-server \
	sphinx2-client

EXTRA_DIST = README \
	README.bin \
	clicore.h \
	srvcore.h

adpow_SOURCES = adpow.c

adrec_SOURCES = adrec.c

cdcn_test_SOURCES = cdcn_test.c

cont_adseg_SOURCES = cont_adseg.c

cont_fileseg_SOURCES = cont_fileseg.c

lm_fsg_test_SOURCES = lm_fsg_test.c

lm_fsg_test2_SOURCES = lm_fsg_test2.c

lm3g2dmp_SOURCES = lm3g2dmp.c

raw2cep_SOURCES = raw2cep.c

pdf32to8b_SOURCES = pdf32to8b.c

sphinx2_allphone_SOURCES = allphone-test.c

sphinx2_batch_SOURCES = batch.c

sphinx2_continuous_SOURCES =  tty-continuous.c

sphinx2_ptt_SOURCES = tty-ptt.c

sphinx2_server_SOURCES = server.c \
	srvcore.c

sphinx2_client_SOURCES = client.c \
	clicore.c

LDADD = $(top_builddir)/src/libsphinx2/libsphinx2.la \
	$(top_builddir)/src/libsphinx2fe/libsphinx2fe.la \
	$(top_builddir)/src/libsphinx2ad/libsphinx2ad.la \
	-lm @ad_libs@ 
# add these for solaris
# -L/usr/demo/SOUND/lib/ -laudio -lnsl -lsocket

INCLUDES = -I$(top_srcdir)/include \
       -I$(top_srcdir)/src/libsphinx2/include \
       -I$(top_srcdir)/src/examples \
       -I$(top_builddir)/include

# add for solaris
# -I/usr/demo/SOUND/include

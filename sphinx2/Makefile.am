SUBDIRS = src \
		  model \
		  include \
		  doc \
		  scripts

EXTRA_DIST = sphinx2.spec \
		autogen.sh \
	sphinx2.dsw \
	win32/batch/sphinx2-align.bat \
	win32/batch/sphinx2-phone.bat \
	win32/batch/sphinx2-simple.bat \
	win32/batch/sphinx2-test.bat \
	win32/msdev/SphinxExamples/adpow/adpow.dsp \
	win32/msdev/SphinxExamples/adrec/adrec.dsp \
	win32/msdev/SphinxExamples/adrec/adrec.dsp \
	win32/msdev/SphinxExamples/cont_adseg/cont_adseg.dsp \
	win32/msdev/SphinxExamples/cont_fileseg/cont_fileseg.dsp \
	win32/msdev/SphinxExamples/lm3g2dmp/lm3g2dmp.dsp \
	win32/msdev/SphinxExamples/raw2cep/raw2cep.dsp \
	win32/msdev/SphinxExamples/sphinx2_allphone/sphinx2_allphone.dsp \
	win32/msdev/SphinxExamples/sphinx2_batch/sphinx2_batch.dsp \
	win32/msdev/SphinxExamples/sphinx2_client/sphinx2_client.dsp \
	win32/msdev/SphinxExamples/sphinx2_continuous/sphinx2_continuous.dsp \
	win32/msdev/SphinxExamples/sphinx2_ptt/sphinx2_ptt.dsp \
	win32/msdev/SphinxExamples/sphinx2_client/sphinx2_client.dsp \
	win32/msdev/SphinxExamples/sphinx2_server/sphinx2_server.dsp \
        win32/msdev/SphinxOCX/ReadMe.txt \
        win32/msdev/SphinxOCX/Sphinx.cpp \
        win32/msdev/SphinxOCX/Sphinx.def \
        win32/msdev/SphinxOCX/Sphinx.dep \
        win32/msdev/SphinxOCX/Sphinx.dsp \
        win32/msdev/SphinxOCX/Sphinx.dsw \
        win32/msdev/SphinxOCX/Sphinx.h \
        win32/msdev/SphinxOCX/Sphinx.ico \
        win32/msdev/SphinxOCX/Sphinx.odl \
        win32/msdev/SphinxOCX/Sphinx.rc \
        win32/msdev/SphinxOCX/SphinxArgfileUpdate.cpp \
        win32/msdev/SphinxOCX/SphinxArgfileUpdate.h \
        win32/msdev/SphinxOCX/SphinxBuildLm.cpp \
        win32/msdev/SphinxOCX/SphinxBuildLm.h \
        win32/msdev/SphinxOCX/SphinxConf.cpp \
        win32/msdev/SphinxOCX/SphinxConf.h \
        win32/msdev/SphinxOCX/SphinxCtl.bmp \
        win32/msdev/SphinxOCX/SphinxCtl.cpp \
        win32/msdev/SphinxOCX/SphinxCtl.h \
        win32/msdev/SphinxOCX/SphinxPpg.cpp \
        win32/msdev/SphinxOCX/SphinxPpg.h \
        win32/msdev/SphinxOCX/StdAfx.cpp \
        win32/msdev/SphinxOCX/StdAfx.h \
        win32/msdev/SphinxOCX/resource.h \
        win32/msdev/libsphinx2/libsphinx2.dsp \
        win32/msdev/libsphinx2ad/libsphinx2ad.dsp \
        win32/msdev/libsphinx2fe/libsphinx2fe.dsp

test:
	$(LIBTOOL) --mode=execute src/examples/sphinx2-continuous \
		-verbose 9 -adcin TRUE -adcext 16k \
		-ctlfn $(top_srcdir)/model/lm/turtle/turtle.ctl \
		-ctloffset 0 -ctlcount 100000000 \
		-datadir $(top_srcdir)/model/lm/turtle \
		-agcmax TRUE -langwt 6.5 -fwdflatlw 8.5 -rescorelw 9.5 \
		-ugwt 0.5 -fillpen 1e-10 -silpen 0.005 -inspen 0.65 \
		-top 1 -topsenfrm 3 -topsenthresh -70000 -beam 2e-06 \
		-npbeam 2e-06 -lpbeam 2e-05 -lponlybeam 0.0005 \
		-nwbeam 0.0005 -fwdflat FALSE -fwdflatbeam 1e-08 \
		-fwdflatnwbeam 0.0003 -bestpath TRUE \
		-kbdumpdir $(top_srcdir)/model/lm/turtle \
		-lmfn $(top_srcdir)/model/lm/turtle/turtle.lm \
		-dictfn $(top_srcdir)/model/lm/turtle/turtle.dic \
		-ndictfn $(top_srcdir)/model/hmm/6k/noisedict \
		-phnfn $(top_srcdir)/model/hmm/6k/phone \
		-mapfn $(top_srcdir)/model/hmm/6k/map \
		-hmmdir $(top_srcdir)/model/hmm/6k \
		-hmmdirlist $(top_srcdir)/model/hmm/6k \
		-8bsen TRUE -sendumpfn $(top_srcdir)/model/hmm/6k/sendump \
		-cbdir $(top_srcdir)/model/hmm/6k

TESTS = \
	test-decode-simple.sh \
	test-decode-raw.sh \
	test-decode.sh \
	test-decode-clm.sh \
	test-decode-mllr.sh \
	test-decode-s3cont.sh \
	test-decode-mode1.sh \
	test-decode-mode2.sh \
	test-decode-s2semi.sh \
	test-decode-lts.sh \
	test-decode-mode3.sh \
	test-decode-2ndstage.sh \
	test-decode-mode1368.sh \
	test-decode-mode1369.sh \
	test-lm_convert-and-decode.sh \
	test-dp.sh \
	test-align.sh \
	test-align-mllr.sh \
	test-lattice-htk.sh \
	test-livepretend.sh \
	test-livepretend-clm.sh \
	test-livepretend-mllr.sh \
	test-livepretend-fsg.sh \
	test-lm_convert.sh \
	test-astar.sh \
	test-astar-clm.sh \
	test-dag.sh \
	test-dag-clm.sh \
	test-gausubvq.sh \
	test-ep.sh \
	test-conf.sh \
	print-summary.sh

DISABLED_TESTS = \
	test-decode-noncomp.sh \
	test-decode-mode5.sh \
	test-decode-mode5-noncomp.sh 

EXTRA_DIST = \
	$(TESTS) \
	$(DISABLED_TESTS)

# Add an item for each and every test target to the CLEANFILES variable
CLEANFILES = chan3.mfc \
	test.out \
	test-*.cepview \
	test-*.log \
	test-*.mfc \
	test-*.log \
	test-*.out \
	test-*.seg \
	test-*.tar \
	test-*.hypseg \
	test-*.confhypseg \
	tmp.* \
	pittsburgh.* \
	tmp \
	test-align-mllr.log \
	allphone.match \
	allphone.matchseg \
	test-decode-mode1*.match \
	test-decode-mode2.match \
	test-lm_convert.DMP \
	test-lm_convert.FST.sym \
	test-lm_convert.FST \
	test-lm_convert.TXT \
	test-lm_convert-and-decode.DMP \
	test-lm_convert-and-decode.DMP32


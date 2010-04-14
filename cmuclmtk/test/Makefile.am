check:
	./test.sh | tee test.log
	if grep -q FAILED test.log; then exit 1; fi

EXTRA_DIST =					\
	test.sh					\
	English/emma11.bin.32bits		\
	English/emma11.arpa.V3			\
	English/emma11.bin.perplexity		\
	English/filterlist			\
	English/emma11.bin.stats		\
	English/emma11.txt			\
	English/emma11.id1gram.32bits		\
	English/emma11.txt.filtered.wfreq	\
	English/emma11.txt.filtered		\
	English/emma11.stats			\
	English/emma11.id1gram.txt		\
	English/pandp12.txt.filtered		\
	English/combine.idngram.txt		\
	English/emma11.vocab			\
	English/emma11.txt.filtered.vocab	\
	English/combine.idngram.32bits		\
	English/emma11.id2gram.txt		\
	English/emma11.idngram.32bits		\
	English/emma11.id2gram.32bits		\
	English/emma11.arpa			\
	English/emma11.wfreq			\
	English/emma11.idngram.txt

CLEANFILES = *.log tmp.stats				\
	English/combine.idngram.filtered		\
	English/combine.idngram.txt.filtered	\
	English/emma11.filtered.arpa		\
	English/emma11.filtered.bin		\
	English/emma11.id1gram.filtered		\
	English/emma11.id1gram.txt.filtered	\
	English/emma11.id2gram.filtered		\
	English/emma11.id2gram.txt.filtered	\
	English/emma11.txt.filtered.idngram.32bits	\
	English/emma11.txt.filtered.idngram.txt	\
	English/emma11.txt.filtered.idngram	\
	English/pandp12.idngram.txt		\
	English/pandp12.txt.idngram		\
	English/pandp12.vocab			\
	English/pandp12.wfreq

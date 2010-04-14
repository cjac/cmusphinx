check:
	./test.sh | tee test.log
	if grep -q FAILED test.log; then exit 1; fi

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

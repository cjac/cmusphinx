use ExtUtils::MakeMaker;

WriteMakefile(
	      NAME => 'LMTraining',
	      VERSION_FROM => 'LMTraining.pm',
	      PREREQ_PM => { XML::Parser => 0 },
	      EXE_FILES => [ qw(lm_train ngram_train ngram_test
				ngram_interp ngram_pronounce
				build_vocab normalize_text) ],
	      PMLIBDIRS => [ qw(InputFilter NGramModel) ],
	      clean => { FILES => "t/*.test.arpa t/*.test" },
);

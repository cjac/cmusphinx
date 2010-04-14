use ExtUtils::MakeMaker;

WriteMakefile(
	      NAME => 'Text::CMU::LMTraining',
	      VERSION_FROM => 'LMTraining.pm',
	      PREREQ_PM => { XML::Parser => 0 },
	      EXE_FILES => [ qw(cmu_lm_train cmu_ngram_train
				cmu_ngram_test cmu_ngram_interp
				cmu_ngram_pronounce
				cmu_build_vocab cmu_normalize_text) ],
	      PMLIBDIRS => [ qw(InputFilter NGramModel) ],
	      clean => { FILES => "t/*.test.arpa t/*.test" },
);

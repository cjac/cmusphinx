int32 hmm_num_sseq(void);
int32 hmm_pid2sid (int32 pid);

void hmm_tied_read_bin (char const *dir_list,
			char const *file,
			SMD *smd,
			double transSmooth,
			int32 numAlphaExpected,
			int norm,
			double arcWeight);

void hmm_tied_read_big_bin (char const  *dir_list,
			    char const  *file,
			    SMD   *smds,
			    double transSmooth,
			    int32  numAlphaExpected,
			    int    norm,
			    double arcWeight);

void read_dists (char const *distDir,
		 char const *Code_Ext0, char const *Code_Ext1,
		 char const *Code_Ext2, char const *Code_Ext3,
		 int32 numAlphabet,
		 double SmoothMin,
		 int32 useCiDistsOnly);

void readDistsOnly (
	char const *distDir,
	char const *Code_Ext0, char const *Code_Ext1,
	char const *Code_Ext2, char const *Code_Ext3,
	int32 numAlphabet,
	int32 useCiDistsOnly);

void read_map (char const *map_file, int32 compress);
void remap (SMD *smdV);

int32 senid2pid (int32 senid);
int32 *hmm_get_psen (void);

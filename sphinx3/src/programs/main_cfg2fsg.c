
#include "s3_cfg.h"
#include "fsg.h"
#include "logs3.h"

/*
 * Command line arguments.
 */
static arg_t defn[] = {
    {"-cfg",
     ARG_STRING,
     NULL,
     "The input CFG"},
    {"-fsg",
     ARG_STRING,
     NULL,
     "The output FSG"},
    {"-max_expansion",
     ARG_INT32,
     "3",
     "A limit on the non-terminal expansions"},
    {NULL, ARG_INT32, NULL, NULL}
};

int
main(int _argc, char **_argv)
{
  char *cfg_file;
  char *fsg_file;
  int max_expansion;

  s3_cfg_t *cfg;
  s2_fsg_t *fsg;
  s2_fsg_trans_t *trans;
  FILE *out;
  int n_trans = 0;

  print_appl_info(_argv[0]);
  cmd_ln_parse(defn, _argc, _argv, TRUE);

  cfg_file = cmd_ln_str("-cfg");
  fsg_file = cmd_ln_str("-fsg");
  max_expansion = cmd_ln_int32("-max_expansion");

  if ((out = fopen(fsg_file, "w")) == NULL)
    E_FATAL("Error opening output FSG file\n");

  logs3_init(1.0001, 0, 0);
  cfg = s3_cfg_read_simple(cfg_file);
  s3_cfg_compile_rules(cfg);
  fsg = s3_cfg_convert_to_fsg(cfg, max_expansion);

  fprintf(out, "# Automatic CFG to FSG conversion\n");
  fprintf(out, "FSG_BEGIN converted\n");
  fprintf(out, "NUM STATES %d\n", fsg->n_state);
  fprintf(out, "START_STATE %d\n", fsg->start_state);
  fprintf(out, "FINAL_STATE %d\n", fsg->final_state);
  
  for (trans = fsg->trans_list; trans != NULL; trans = trans->next) {
    fprintf(out, "TRANSITION\t%d\t%d\t%0.5f\t",
	    trans->from_state, trans->to_state, trans->prob);
    if (trans->word == NULL)
      fprintf(out, "\n");
    else
      fprintf(out, "%s\n", trans->word);
    n_trans++;
  }
  
  fprintf(out, "FSG_END\n");

  fclose(out);

  printf("Conversion complete\n"
	 "CFG: %d rules, %d symbols\n"
	 "FSG: %d states, %d transitions\n",
	 s3_arraylist_count(&cfg->rules),
	 s3_arraylist_count(&cfg->item_info),
	 fsg->n_state,
	 n_trans);
  
  return 0;
}

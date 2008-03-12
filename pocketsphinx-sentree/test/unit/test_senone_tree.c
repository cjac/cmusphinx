#include <stdio.h>
#include <string.h>

#include "senone_tree.h"
#include "test_macros.h"

int
main(int argc, char *argv[])
{
	senone_tree_t *stree;
	int i;

	stree = senone_tree_read(DATADIR "/mixw_trees", -1);
	TEST_ASSERT(stree);
	for (i = 0; i < 5; ++i) {
		int k, n_bits;

		n_bits = 0;
		for (k = 0; k < stree->nodes[i].n_leaf_bits; ++k)
			if (bitvec_is_set(stree->nodes[i].leaves, k))
				++n_bits;
		printf("%d => %d,%d (%d senones)\n", i,
		       stree->nodes[i].left,
		       stree->nodes[i].right, n_bits);
	}
	senone_tree_free(stree);

	stree = senone_tree_read(DATADIR "/mixw_trees", 256);
	TEST_ASSERT(stree);
	for (i = 0; i < 5; ++i) {
		int k, n_bits;

		n_bits = 0;
		for (k = 0; k < stree->nodes[i].n_leaf_bits; ++k)
			if (bitvec_is_set(stree->nodes[i].leaves, k))
				++n_bits;
		printf("%d => %d,%d (%d senones)\n", i,
		       stree->nodes[i].left,
		       stree->nodes[i].right, n_bits);
	}
	senone_tree_free(stree);

	return 0;
}

#define NUM_ALPHABET	256
/* centered 5 frame difference of c[0]...c[12] with centered 9 frame
 * of c[1]...c[12].  Don't ask me why it was designed this way! */
#define DCEP_VECLEN	25
#define DCEP_LONGWEIGHT	0.5
#define POW_VECLEN	3	/* pow, diff pow, diff diff pow */
#define DEF_VAR_FLOOR	0.00001

#define MAX_DIFF_WINDOW	9	/* # frames include cur */

#define INPUT_MASK	0xf
#define DIFF_MASK	0x7

#define INDEX(x,m) ((x)&(m))	/* compute new circular buff index */

#define INPUT_INDEX(x) INDEX(x,INPUT_MASK)
#define DIFF_INDEX(x) INDEX(x,DIFF_MASK)

typedef struct {
  union {
    int32	score;
    int32	dist;	/* distance to next closest vector */
  } val;
  int32 codeword;		/* codeword (vector index) */
} vqFeature_t;
typedef vqFeature_t *vqFrame_t;

typedef enum {SCVQ_HEAD, SCVQ_TAIL, SCVQ_DEQUEUE} ht_t;

#define FBUFMASK	0x3fff	/* (# frames)-1 of topN features to keep in circ buff */

#define WORST_SCORE	(int32)(0x80000000)
#define WORST_DIST	(int32)(0x80000000)
#define BTR		>
#define NEARER		>
#define WORSE		<
#define FARTHER		<


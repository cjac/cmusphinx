#include <libutil/libutil.h>

static int32 N_CB;	/* E.g., 256 */
#define N_SEN		6000
#define N_DEN		16
#define N_DIM		39


static int32 dist2 (float32 *pre, int32 ***id, float32 **diff, int32 n_cl)
{
    int32 c, d, i;
    float64 s;
    int32 *idl;

    for (c = 0; c < N_SEN; c++) {
	for (d = 0; d < N_DEN; d++) {
	    idl = id[c][d];

	    s = 0;
	    for (i = 0; i < n_cl; i++)
		s += pre[idl[i]];

	    diff[c][d] = s;
	}
    }
}


static int32 gauden_dist (float32 ***mean, float32 ***var, float32 *obs, float32 **diff)
{
    float32 *m1, *v1, *m2, *v2;
    float64 dd1, dd2, s1, s2;
    int32 c, d, i;

    for (c = 0; c < N_CB; c++) {
	for (d = 0; d < N_DEN; d += 2) {
	    m1 = mean[c][d];
	    m2 = mean[c][d+1];
	    v1 = var[c][d];
	    v2 = var[c][d+1];

	    s1 = 0.0;
	    s2 = 0.0;
	    for (i = 0; i < N_DIM; i++) {
		dd1 = obs[i] - m1[i];
		s1 += dd1 * dd1 * v1[i];
		dd2 = obs[i] - m2[i];
		s2 += dd2 * dd2 * v2[i];
	    }

	    diff[c][d] = s1;
	    diff[c][d+1] = s2;
	}
    }
}


main (int32 argc, char *argv[])
{
    float32 ***mean, ***var, *obs, **diff, *pre;
    int32 c, d, i, j, r, f, n_cl;
    float32 *m, *v;
    int32 ***id;
    timing_t *tm;

    if (argc != 4)
	E_FATAL("Usage: %s <random-seed> <#clusters> <#cb>\n", argv[0]);
    if (sscanf (argv[1], "%d", &r) != 1)
	E_FATAL("Usage: %s <random-seed> <#clusters> <#cb>\n", argv[0]);
    if (sscanf (argv[2], "%d", &n_cl) != 1)
	E_FATAL("Usage: %s <random-seed> <#clusters> <#cb>\n", argv[0]);
    if (sscanf (argv[3], "%d", &N_CB) != 1)
	E_FATAL("Usage: %s <random-seed> <#clusters> <#cb>\n", argv[0]);

    srandom (r);

    mean = (float32 ***) ckd_calloc_3d (N_CB, N_DEN, N_DIM, sizeof(float32));
    var = (float32 ***) ckd_calloc_3d (N_CB, N_DEN, N_DIM, sizeof(float32));
    obs = (float32 *) ckd_calloc (N_DIM, sizeof(float32));
    
    pre = (float32 *) ckd_calloc (N_CB * N_DEN * n_cl, sizeof(float32));
    id = (int32 ***) ckd_calloc_3d (N_SEN, N_DEN, n_cl, sizeof(int32));

    diff = (float32 **) ckd_calloc_2d (N_SEN, N_DEN, sizeof(float32));

    for (c = 0; c < N_CB; c++) {
	for (d = 0; d < N_DEN; d++) {
	    m = mean[c][d];
	    v = var[c][d];

	    for (i = 0; i < N_DIM; i++) {
		r = random() & 0x00007fff;
		if (r == 0)
		    r = 1;
		
		m[i] = (float64)r * (1.0 / 32768.0);

		r = random() & 0x00007fff;
		if (r == 0)
		    r = 1;
		
		v[i] = (float64)r * (1.0 / 32768.0);
	    }
	}
    }

    j = 0;
    for (c = 0; c < N_CB; c++) {
	for (d = 0; d < N_DEN; d++) {
	    for (i = 0; i < n_cl; i++) {
		r = random() & 0x00007fff;
		if (r == 0)
		    r = 1;
		
		pre[j] = (float64)r * (1.0 / 32768.0);
		j++;
	    }
	}
    }

    for (c = 0; c < N_SEN; c++) {
	for (d = 0; d < N_DEN; d++) {
	    for (i = 0; i < n_cl; i++) {
		r = (random() & 0x7fffffff) % (N_CB * N_DEN);
		r *= n_cl;

		id[c][d][i] = r+i;
	    }
	}
    }

    tm = timing_new ("timer");

    timing_reset (tm);
    timing_start (tm);
    
    for (i = 0; i < N_DIM; i++) {
	r = random() & 0x00007fff;
	if (r == 0)
	    r = 1;
	
	obs[i] = (float64)r * (1.0 / 32768.0);
    }
    
    for (f = 0; f < 100; f++) {
	gauden_dist (mean, var, obs, diff);
	dist2 (pre, id, diff, n_cl);
    }

    timing_stop (tm);

    printf ("%.2f sec CPU, %.2f sec elapsed\n", tm->t_tot_cpu, tm->t_tot_elapsed);
}

/* -*- tab-width: 8; c-file-style: "linux" -*- */
/* Copyright (c) 2007 Carnegie Mellon University. All rights *
 * reserved.
 *
 * You may copy, modify, and distribute this code under the same terms
 * as Sphinx3 or Python, at your convenience, as long as this notice
 * is not removed.
 *
 * Author: David Huggins-Daines <dhuggins@cs.cmu.edu>
 *
 * This work was partially funded by BAE Systems under contract from
 * the Defense Advanced Research Projects Agency.
 */

#include "Python.h"

#include "s3_decode.h"
#include "utt.h"

/* This is a singleton. */
static s3_decode_t decoder;
/* And this. */
static fe_t *fe;
/* And these. */
static char **argv;
static int argc;

/* Parse command line from file */
static PyObject *
sphinx3_parse_argfile(PyObject *self, PyObject *args)
{
	const char *filename;

	if (!PyArg_ParseTuple(args, "s", &filename))
		return NULL;
	if (cmd_ln_parse_file(S3_DECODE_ARG_DEFS, (char *)filename) == -1) {
		/* Raise an IOError, the file did not exist (probably). */
		PyErr_SetString(PyExc_IOError, "Argument file could not be read");
		return NULL;
	}
	Py_INCREF(Py_None);
	return Py_None;
}

/* Parse command line from Python dictionary */
static PyObject *
sphinx3_parse_argdict(PyObject *self, PyObject *args)
{
	PyObject *seq;
	int i;

	if (!PyArg_ParseTuple(args, "O", &seq))
		return NULL;

	if ((seq = PyMapping_Items(seq)) == NULL) {
		return NULL;
	}
	
	if (argv) {
		for (i = 0; i < argc; ++i) {
			free(argv[i]);
		}
		free(argv);
		argv = NULL;
	}

	argc = PySequence_Size(seq);
	/* Allocate bogus initial and NULL final entries */
	if ((argv = calloc(argc * 2 + 2, sizeof(*argv))) == NULL)
		return PyErr_NoMemory();
	argv[0] = strdup("sphinx3_python");

	for (i = 0; i < argc; ++i) {
		PyObject *pair, *str;
		const char *key, *val;

		if ((pair = PySequence_GetItem(seq, i)) == NULL)
			return NULL;
		if ((str = PyTuple_GetItem(pair, 0)) == NULL)
			return NULL;
		if ((str = PyObject_Str(str)) == NULL)
			return NULL;
		if ((key = PyString_AsString(str)) == NULL)
			return NULL;
		Py_DECREF(str);
		if (key[0] != '-') {
			argv[i*2+1] = calloc(strlen(key) + 2, 1);
			argv[i*2+1][0] = '-';
			strcat(argv[i*2+1], key);
		}
		else
			argv[i*2+1] = strdup(key);

		if ((str = PyTuple_GetItem(pair, 1)) == NULL)
			return NULL;
		if ((str = PyObject_Str(str)) == NULL)
			return NULL;
		Py_DECREF(str);
		if ((val = PyString_AsString(str)) == NULL)
			return NULL;
		argv[i*2+2] = strdup(val);
	}

	argc = argc * 2 + 1;
	if (cmd_ln_parse(S3_DECODE_ARG_DEFS, argc, argv) == -1) {
		/* This actually won't ever happen */
		PyErr_SetString(PyExc_ValueError, "Arguments are invalid");
		return NULL;
	}

	Py_DECREF(seq);
	Py_INCREF(Py_None);
	return Py_None;
}

/* Parse command line from Python array or sequence */
static PyObject *
sphinx3_parse_argv(PyObject *self, PyObject *args)
{
	PyObject *seq;
	int i;

	if (!PyArg_ParseTuple(args, "O", &seq))
		return NULL;

	if (!PySequence_Check(seq)) {
		PyErr_SetString(PyExc_TypeError, "Argument is not a sequence");
		return NULL;
	}

	if (argv) {
		for (i = 0; i < argc; ++i) {
			free(argv[i]);
		}
		free(argv);
		argv = NULL;
	}

	argc = PySequence_Size(seq);
	if ((argv = calloc(argc + 1, sizeof(*argv))) == NULL)
		return PyErr_NoMemory();

	for (i = 0; i < argc; ++i) {
		PyObject *str;
		const char *arg;

		if ((str = PySequence_GetItem(seq, i)) == NULL)
			return NULL;
		if ((str = PyObject_Str(str)) == NULL)
			return NULL;
		if ((arg = PyString_AsString(str)) == NULL)
			return NULL;
		argv[i] = strdup(arg);

		Py_DECREF(str);
	}

	if (cmd_ln_parse(S3_DECODE_ARG_DEFS, argc, argv) == -1) {
		/* This actually won't ever happen */
		PyErr_SetString(PyExc_ValueError, "Arguments are invalid");
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

/* Initialize live decoder */
static PyObject *
sphinx3_init(PyObject *self, PyObject *args)
{
	s3_decode_init(&decoder);
	fe = fe_init_auto();
	Py_INCREF(Py_None);
	return Py_None;
}

/* Wrap up the live decoder */
static PyObject *
sphinx3_close(PyObject *self, PyObject *args)
{
	s3_decode_close(&decoder);
	fe_close(fe);
	cmd_ln_free();
	Py_INCREF(Py_None);
	return Py_None;
}

/* Get hypothesis string and segmentation. */
static PyObject *
sphinx3_get_hypothesis(PyObject *self, PyObject *args)
{
	PyObject *hypstr_obj, *hypseg_obj;
	hyp_t **hypsegs, **h;
	char *hypstr, *uttid;
	int nhyps, i, allphone;

	s3_decode_hypothesis(&decoder, &uttid, &hypstr, &hypsegs);
	nhyps = 0;
	for (h = hypsegs; *h; ++h)
		++nhyps;

	allphone = (cmd_ln_int32("-op_mode") == 1);

	hypstr_obj = PyString_FromString(hypstr);
	hypseg_obj = PyTuple_New(nhyps);
	for (i = 0; i < nhyps; ++i) {
		PyObject *seg_obj;
		const char *wordstr;

		/* hyp_t is BOGUS, it should have a string, then we
		 * wouldn't have to screw around like this for
		 * allphones. */
		if (allphone) {
			wordstr = mdef_ciphone_str(kbcore_mdef(decoder.kbcore),
						   hypsegs[i]->id);
		}
		else {
			wordstr = dict_wordstr(kbcore_dict(decoder.kbcore),
					       hypsegs[i]->id);
		}
		seg_obj = Py_BuildValue("(siiii)",
					wordstr,
					hypsegs[i]->sf,
					hypsegs[i]->ef,
					hypsegs[i]->ascr,
					hypsegs[i]->lscr);
		PyTuple_SET_ITEM(hypseg_obj, i, seg_obj);
	}

	return Py_BuildValue("(OO)", hypstr_obj, hypseg_obj);
}

static PyObject *
sphinx3_begin_utt(PyObject *self, PyObject *args)
{
	char *uttid = NULL;

	if (!PyArg_ParseTuple(args, "|s", &uttid))
		return NULL;

	if (s3_decode_begin_utt(&decoder, uttid) < 0) {
		PyErr_SetString(PyExc_RuntimeError, "s3_decode_begin_utt() failed");
		return NULL;
	}

	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
sphinx3_end_utt(PyObject *self, PyObject *args)
{
	s3_decode_end_utt(&decoder);
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
sphinx3_process_raw(PyObject *self, PyObject *args)
{
	PyObject *str;
	int16 *data;
	int32 nsamps;
	mfcc_t **cep_block;
	int32 nframes;

	if (!PyArg_ParseTuple(args, "O", &str))
		return NULL;
	if ((data = (int16 *)PyString_AsString(str)) == NULL)
		return NULL;
	nsamps = PyString_Size(str)/2;

	if (fe_process_utt(fe, data, nsamps, &cep_block, &nframes) == -1) {
		PyErr_SetString(PyExc_ValueError, "Problem in fe_process_utt()");
		return NULL;
	}
	s3_decode_process(&decoder, cep_block, nframes);
	ckd_free_2d((void **)cep_block);

	return Py_BuildValue("i", nframes);
}

static PyObject *
sphinx3_read_lm(PyObject *self, PyObject *args)
{
	const char *lmfile, *lmname;

	if (!PyArg_ParseTuple(args, "ss", &lmfile, &lmname))
		return NULL;

	s3_decode_read_lm(&decoder, lmfile, lmname);
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
sphinx3_set_lm(PyObject *self, PyObject *args)
{
	const char *lmname;

	if (!PyArg_ParseTuple(args, "s", &lmname))
		return NULL;

	s3_decode_set_lm(&decoder, lmname);
	Py_INCREF(Py_None);
	return Py_None;
}

static PyObject *
sphinx3_delete_lm(PyObject *self, PyObject *args)
{
	const char *lmname;

	if (!PyArg_ParseTuple(args, "s", &lmname))
		return NULL;

	s3_decode_set_lm(&decoder, lmname);
	Py_INCREF(Py_None);
	return Py_None;
}

/* Decode raw waveform data */
static PyObject *
sphinx3_decode_raw(PyObject *self, PyObject *args)
{
	PyObject *str;
	int16 *data;
	int32 nsamps;
	mfcc_t **cep_block, ***feat_block;
	int32 nframes;
	char *uttid = NULL;

	if (!PyArg_ParseTuple(args, "O|s", &str, &uttid))
		return NULL;
	if ((data = (int16 *)PyString_AsString(str)) == NULL)
		return NULL;
	nsamps = PyString_Size(str)/2;

	if (fe_process_utt(fe, data, nsamps, &cep_block, &nframes) == -1) {
		PyErr_SetString(PyExc_ValueError, "Problem in fe_process_utt()");
		return NULL;
	}
	feat_block = feat_array_alloc(kbcore_fcb(decoder.kbcore),nframes);

	s3_decode_begin_utt(&decoder, uttid);
	/* In theory we should check the return from this, but it will
	 * always process the whole thing if both beginutt and endutt
	 * are TRUE. */
	decoder.num_frames_entered
		= feat_s2mfc2feat_block(kbcore_fcb(decoder.kbcore),
					cep_block, nframes, TRUE, TRUE,
					feat_block);
	ckd_free_2d((void **)cep_block);
	if (nframes == 0) {
		PyErr_SetString(PyExc_ValueError, "Utterance too short");
		ckd_free_2d((void **)feat_block);
		return NULL;
	}

	/* Unfortunately we have to bypass s3_decode.c a bit here. */
	utt_decode_block(feat_block, nframes, &decoder.num_frames_decoded, &decoder.kb);
	feat_array_free(feat_block);
	s3_decode_end_utt(&decoder);

	/* Now get the results and return them. */
	return sphinx3_get_hypothesis(self, args);
}

/* Decode a feature file */
static PyObject *
sphinx3_decode_cep_file(PyObject *self, PyObject *args)
{
	const char *filename;
	char *uttid = NULL;
	int sf = 0;
	int ef = -1;

	if (!PyArg_ParseTuple(args, "s|iis", &filename, &sf, &ef, &uttid))
		return NULL;

	/* Unfortunately we have to bypass s3_decode.c a bit here. */
	s3_decode_begin_utt(&decoder, uttid);
	decoder.num_frames_entered
		= feat_s2mfc2feat(kbcore_fcb(decoder.kbcore),
				  filename,
				  NULL, "",
				  sf, ef,
				  decoder.kb.feat, S3_MAX_FRAMES);
	if (decoder.num_frames_entered < 0) {
		PyErr_SetString(PyExc_IOError, "Could not read feature file");
		return NULL;
	}
	else if (decoder.num_frames_entered == 0) {
		PyErr_SetString(PyExc_ValueError, "Utterance too short");
		return NULL;
	}
	utt_decode_block(decoder.kb.feat, decoder.num_frames_entered,
			 &decoder.num_frames_decoded, &decoder.kb);
	s3_decode_end_utt(&decoder);

	/* Now get the results and return them. */
	return sphinx3_get_hypothesis(self, args);
}

static PyMethodDef sphinx3methods[] = {
	{ "init", sphinx3_init, METH_VARARGS,
	  "Initialize the Sphinx3 decoder.\n"
	  "You must first call one of parse_argfile, parse_argdict, parse_argv." },
	{ "close", sphinx3_close, METH_VARARGS,
	  "Shut down the Sphinx3 decoder." },
	{ "parse_argfile", sphinx3_parse_argfile, METH_VARARGS,
	  "Load Sphinx3 parameters from a file." },
	{ "parse_argv", sphinx3_parse_argv, METH_VARARGS,
	  "Set Sphinx3 parameters from an argv array (e.g. sys.argv)\n"
	  "Note that the first element of this array is IGNORED." },
	{ "parse_argdict", sphinx3_parse_argdict, METH_VARARGS,
	  "Load Sphinx3 parameters from a dictionary.\n"
	  "Keys can optionally begin with a - as in the command line."	},
	{ "begin_utt", sphinx3_begin_utt, METH_VARARGS,
	  "Mark the start of the current utterance.\n" },
	{ "end_utt", sphinx3_end_utt, METH_VARARGS,
	  "Mark the end of the current utterance, doing final search if necessary.\n" },
	{ "process_raw", sphinx3_process_raw, METH_VARARGS,
	  "Process a block of raw audio.\n" },
	/* Processing cepstra might wait for a bit until I decide if
	 * it's worth pulling in NumPy */
	{ "get_hypothesis", sphinx3_get_hypothesis, METH_VARARGS,
	  "Get current hypothesis string and segmentation.\n" },
	{ "set_lm", sphinx3_set_lm, METH_VARARGS,
	  "Set the current language model to the one named (must be previously loaded).\n" },
	{ "read_lm", sphinx3_read_lm, METH_VARARGS,
	  "Load a language model from a file and associate it with a name.\n" },
	{ "delete_lm", sphinx3_delete_lm, METH_VARARGS,
	  "Unload and free resources used by the named language model.\n" },
	{ "decode_raw", sphinx3_decode_raw, METH_VARARGS,
	  "Decode an entire utterance of raw waveform data from a Python string.\n" },
	{ "decode_cep_file", sphinx3_decode_cep_file, METH_VARARGS,
	  "Decode a Sphinx-format feature (MFCC) file.\n" },
	{ NULL, NULL, 0, NULL }
};

PyMODINIT_FUNC
init_sphinx3(void)
{
	(void) Py_InitModule("_sphinx3", sphinx3methods);
}

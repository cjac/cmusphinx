/* -*- c-basic-offset: 4; indent-tabs-mode: nil -*- */
/* ====================================================================
 * Copyright (c) 2007 Carnegie Mellon University.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 * Author: David Huggins-Daines <dhuggins@cs.cmu.edu>
 */
/* Based on gstsphinxsink.c from gnome-voice-control:
 *
 * Copyright (C) 2007  Nickolay V. Shmyrev  <nshmyrev@yandex.ru>
 *
 * gstsphinxsink.c:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
/* Based on GStreamer plug-in template code, license follows: */
/*
 * GStreamer
 * Copyright 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-plugin
 * @short_description: sink for speech recognition using Sphinx3
 * <refsect2>
 * <title>Example launch line</title>
 * <para>
 * <programlisting>
 * gst-launch -v -m audiotestsrc ! sphinx3
 * </programlisting>
 * </para>
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <gst/gstmarshal.h>
#include <sphinx_config.h>
#include <string.h>

#include "gstsphinx3.h"

GST_DEBUG_CATEGORY_STATIC (gst_sphinx3_debug);
#define GST_CAT_DEFAULT gst_sphinx3_debug

/* Filter signals and args */
enum
{
    SIGNAL_INITIALIZATION,
    SIGNAL_AFTER_INITIALIZATION,
    SIGNAL_CALIBRATION,
    SIGNAL_LISTENING,
    SIGNAL_READY,
    SIGNAL_PARTIAL_RESULT,
    SIGNAL_RESULT,
    LAST_SIGNAL
};

enum
{
    PROP_0,
    PROP_HMM_DIR,
    PROP_LM_FILE,
    PROP_DICT_FILE
};

/* Default command line. (will go away soon and be constructed using properties) */
static char *default_argv[] = {
    "gst-sphinx3",
    "-hmm", SPHINX3_PREFIX "/share/sphinx3/model/hmm/hub4_cd_continuous_8gau_1s_c_d_dd",
    "-lm", SPHINX3_PREFIX "/share/sphinx3/model/lm/an4/an4.ug.lm.DMP",
    "-fdict", SPHINX3_PREFIX "/share/sphinx3/model/lm/an4/filler.dict",
    "-dict", SPHINX3_PREFIX "/share/sphinx3/model/lm/an4/an4.dict",
    "-samprate", "16000",
};
static const int default_argc = sizeof(default_argv)/sizeof(default_argv[0]);

static GstStaticPadTemplate sink_factory =
    GST_STATIC_PAD_TEMPLATE ("sink",
                             GST_PAD_SINK,
                             GST_PAD_ALWAYS,
                             GST_STATIC_CAPS("audio/x-raw-int, "
                                             "width = (int) 16, "
                                             "depth = (int) 16, "
                                             "endianness = (int) BYTE_ORDER, "
                                             "channels = (int) 1, "
                                             "rate = (int) 16000")
        );
	
static void gst_sphinx3_set_property (GObject * object, guint prop_id,
					   const GValue * value, GParamSpec * pspec);
static void gst_sphinx3_get_property (GObject * object, guint prop_id,
					   GValue * value, GParamSpec * pspec);
static gboolean gst_sphinx3_start(GstBaseSink * asink);
static gboolean gst_sphinx3_stop (GstBaseSink * asink);
static GstFlowReturn gst_sphinx3_render (GstBaseSink * sink, GstBuffer * buffer);
static void gst_sphinx3_do_init (GType type);

static guint gst_sphinx3_signals[LAST_SIGNAL];

GST_BOILERPLATE_FULL (GstSphinx3, gst_sphinx3, GstBaseSink,
		      GST_TYPE_BASE_SINK, gst_sphinx3_do_init);

static void
gst_sphinx3_do_init (GType type)
{
    GST_DEBUG_CATEGORY_INIT (gst_sphinx3_debug, "sphinx3",
                             0, "Sphinx3 plugin");
}


static void
gst_sphinx3_base_init (gpointer gclass)
{
    static GstElementDetails element_details = {
        "Sphinx3",
        "Sink/Audio",
        "Perform speech recognition on a stream",
        "David Huggins-Daines <dhuggins@cs.cmu.edu>"
    };
    GstElementClass *element_class = GST_ELEMENT_CLASS (gclass);

    gst_element_class_add_pad_template (element_class,
                                        gst_static_pad_template_get (&sink_factory));
    gst_element_class_set_details (element_class, &element_details);
}

static void
string_disposal (gpointer key, gpointer value, gpointer user_data)
{
    g_free(value);
}

static void
gst_sphinx3_finalize (GObject * gobject)
{
    GstSphinx3 *sphinxsink = GST_SPHINX3 (gobject);

    gst_adapter_clear (sphinxsink->adapter);
    g_object_unref (sphinxsink->adapter);

    if (sphinxsink->ad.initialized) {
        fe_close(sphinxsink->fe);
        s3_decode_close(&sphinxsink->decoder);
    }

    g_hash_table_foreach (sphinxsink->arghash, string_disposal, NULL);
    g_hash_table_destroy (sphinxsink->arghash);

    GST_CALL_PARENT (G_OBJECT_CLASS, finalize, (gobject));
}

static void
gst_sphinx3_class_init (GstSphinx3Class * klass)
{
    GstBaseSinkClass *basesink_class;
    GObjectClass *gobject_class;

    gobject_class = (GObjectClass *) klass;
    basesink_class = (GstBaseSinkClass *) klass;

    gobject_class->set_property = gst_sphinx3_set_property;
    gobject_class->get_property = gst_sphinx3_get_property;
    gobject_class->finalize = GST_DEBUG_FUNCPTR (gst_sphinx3_finalize);
    basesink_class->start = GST_DEBUG_FUNCPTR (gst_sphinx3_start);
    basesink_class->stop = GST_DEBUG_FUNCPTR (gst_sphinx3_stop);
    basesink_class->render = GST_DEBUG_FUNCPTR (gst_sphinx3_render);

    /* TODO: We will bridge cmd_ln.h properties to GObject
     * properties here somehow eventually. */
    g_object_class_install_property
        (gobject_class, PROP_HMM_DIR,
         g_param_spec_string ("hmm_dir", "HMM Directory",
                              "Directory containing acoustic model parameters",
                              SPHINX3_PREFIX "/share/sphinx3/model/hmm/wsj1",
                              G_PARAM_READWRITE));
    g_object_class_install_property
        (gobject_class, PROP_LM_FILE,
         g_param_spec_string ("lm_file", "LM File",
                              "Language model file",
                              SPHINX3_PREFIX "/share/sphinx3/model/lm/swb/swb.lm.DMP",
                              G_PARAM_READWRITE));
    g_object_class_install_property
        (gobject_class, PROP_DICT_FILE,
         g_param_spec_string ("dict_file", "Dictionary File",
                              "Dictionary File",
                              SPHINX3_PREFIX "/share/sphinx3/model/lm/swb/swb.dic",
                              G_PARAM_READWRITE));

    gst_sphinx3_signals[SIGNAL_INITIALIZATION] =
        g_signal_new ("initialization", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GstSphinx3Class, initialization), NULL, NULL,
                      g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, G_TYPE_NONE);

    gst_sphinx3_signals[SIGNAL_AFTER_INITIALIZATION] =
        g_signal_new ("after_initialization", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GstSphinx3Class, after_initialization), NULL, NULL,
                      g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, G_TYPE_NONE);

    gst_sphinx3_signals[SIGNAL_CALIBRATION] =
        g_signal_new ("calibration", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GstSphinx3Class, calibration), NULL, NULL,
                      g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, G_TYPE_NONE);

    gst_sphinx3_signals[SIGNAL_LISTENING] =
        g_signal_new ("listening", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GstSphinx3Class, listening), NULL, NULL,
                      g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, G_TYPE_NONE);

    gst_sphinx3_signals[SIGNAL_READY] =
        g_signal_new ("ready", G_TYPE_FROM_CLASS (klass), G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GstSphinx3Class, calibration), NULL, NULL,
                      g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0, G_TYPE_NONE);

    gst_sphinx3_signals[SIGNAL_PARTIAL_RESULT] = 
        g_signal_new ("partial_result",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GstSphinx3Class, partial_result),
                      NULL, NULL,
                      gst_marshal_VOID__STRING,
                      G_TYPE_NONE,
                      1, G_TYPE_STRING
            );

    gst_sphinx3_signals[SIGNAL_RESULT] = 
        g_signal_new ("result",
                      G_TYPE_FROM_CLASS (klass),
                      G_SIGNAL_RUN_LAST,
                      G_STRUCT_OFFSET (GstSphinx3Class, result),
                      NULL, NULL,
                      gst_marshal_VOID__STRING,
                      G_TYPE_NONE,
                      1, G_TYPE_STRING
            );
}

static void
gst_sphinx3_set_string (GstSphinx3 *sink,
                             const gchar *key, const GValue *value)
{
    /* NOTE: This is an undocumented feature of SphinxBase's cmd_ln.h.
     * However it will be officially supported in future releases. */
    anytype_t *val;
    gchar *str;

    val = cmd_ln_access(key);
    val->ptr = g_strdup(g_value_get_string(value));
    if ((str = g_hash_table_lookup(sink->arghash, key)))
        g_free(str);
    g_hash_table_insert(sink->arghash, (gpointer)key, val->ptr);
}

static void
gst_sphinx3_set_property (GObject * object, guint prop_id,
			       const GValue * value, GParamSpec * pspec)
{
    GstSphinx3 *sink = GST_SPHINX3(object);

    switch (prop_id) {
    case PROP_HMM_DIR:
        gst_sphinx3_set_string(sink, "-hmm", value);
        break;
    case PROP_LM_FILE:
        gst_sphinx3_set_string(sink, "-lm", value);
        break;
    case PROP_DICT_FILE:
        gst_sphinx3_set_string(sink, "-dict", value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        return;
    }
}

static void
gst_sphinx3_get_property (GObject * object, guint prop_id,
			       GValue * value, GParamSpec * pspec)
{
    switch (prop_id) {
    case PROP_HMM_DIR:
        g_value_set_string(value, cmd_ln_str("-hmm"));
        break;
    case PROP_LM_FILE:
        g_value_set_string(value, cmd_ln_str("-lm"));
        break;
    case PROP_DICT_FILE:
        g_value_set_string(value, cmd_ln_str("-dict"));
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
        break;
    }
}

static void
gst_sphinx3_init (GstSphinx3 * rec,
		       GstSphinx3Class * gclass)
{
    /* Create the hash table to store argument strings. */
    rec->arghash = g_hash_table_new(g_str_hash, g_str_equal);

    /* Parse default command-line options. */
    cmd_ln_parse(S3_DECODE_ARG_DEFS, default_argc, default_argv, FALSE);

    /* Create an adapter object to "pull" data from the pipeline */
    GST_BASE_SINK(rec)->sync = FALSE;
    rec->adapter = gst_adapter_new ();
}

#define REQUIRED_FRAME_SAMPLES 1024
#define REQUIRED_FRAME_BYTES  REQUIRED_FRAME_SAMPLES * 2

int32 
gst_sphinx3_ad_read(ad_rec_t *ad, int16 *buf, int32 max)
{
    GstSphinx3 *sphinxsink = GST_SPHINX3 (((GstSphinxSinkAd *)ad)->self);

    memcpy ((void *)buf,
            gst_adapter_peek (sphinxsink->adapter, REQUIRED_FRAME_BYTES),
            REQUIRED_FRAME_BYTES);
  
    return REQUIRED_FRAME_SAMPLES;
}

static gboolean
gst_sphinx3_start (GstBaseSink * asink)
{
    GstSphinx3 *sphinxsink = GST_SPHINX3 (asink);
    GstSphinxSinkAd *ad = &sphinxsink->ad;

    ad->self = sphinxsink;
    ad->sps = 16000;
    ad->self = sphinxsink;
    ad->bps = sizeof(int16);
    ad->calibrated = FALSE;
    ad->calibrate_started = FALSE;
  
    sphinxsink->cont = cont_ad_init ((ad_rec_t *)ad, gst_sphinx3_ad_read);
  
    return TRUE;
}

static gboolean
gst_sphinx3_stop (GstBaseSink * asink)
{
    GstSphinx3 *sphinxsink = GST_SPHINX3 (asink);
  
    cont_ad_close (sphinxsink->cont);
  
    return TRUE;
}

static void gst_sphinx3_calibrate_chunk (GstSphinx3 *sphinxsink)
{
    int result;

    if (!sphinxsink->ad.calibrate_started) {
        g_signal_emit (sphinxsink,
                       gst_sphinx3_signals[SIGNAL_CALIBRATION], 0, NULL);
        sphinxsink->ad.calibrate_started = TRUE;
    }

    result = cont_ad_calib_loop (sphinxsink->cont, 
                                 (int16 *)gst_adapter_peek(sphinxsink->adapter,
                                                           REQUIRED_FRAME_BYTES),
                                 REQUIRED_FRAME_SAMPLES);
	
    if (result == 0) {
        sphinxsink->ad.calibrated = TRUE;
        sphinxsink->ad.listening = 0;
    	    
        g_signal_emit (sphinxsink,
                       gst_sphinx3_signals[SIGNAL_READY], 0, NULL);
    }
}

static void gst_sphinx3_process_chunk (GstSphinx3 *sphinxsink)
{
    int32 k;
    int16 adbuf[REQUIRED_FRAME_SAMPLES];
	
    k = cont_ad_read (sphinxsink->cont, adbuf, REQUIRED_FRAME_SAMPLES);
	
    if (k == 0 && sphinxsink->last_ts == 0) {
        return;
    } else if (k == 0 && sphinxsink->cont->read_ts - sphinxsink->last_ts > 
               DEFAULT_SAMPLES_PER_SEC) {
        char *hyp = NULL;
        char *uttid;
        hyp_t **segs;

        s3_decode_end_utt(&sphinxsink->decoder);
        if (s3_decode_hypothesis(&sphinxsink->decoder, &uttid, &hyp, &segs) < 0) {
            g_warning ("uttproc_result failed");
        } else {
            if (hyp != NULL)
                g_signal_emit (sphinxsink,
                               gst_sphinx3_signals[SIGNAL_RESULT], 
                               0, hyp);
        }

        sphinxsink->last_ts = 0;
        sphinxsink->ad.listening = 0;
        g_signal_emit (sphinxsink,
                       gst_sphinx3_signals[SIGNAL_READY], 0, NULL);

    } else if (k != 0) {
        int32 nfr;
        float32 **mfcc;

        if (sphinxsink->ad.listening == 0) {
            s3_decode_begin_utt (&sphinxsink->decoder, NULL);
            sphinxsink->ad.listening = 1;
        }
	
        if (fe_process_utt (sphinxsink->fe, adbuf, k, &mfcc, &nfr) < 0) {
            g_warning("fe_process_utt failed");
        }
        else {
            s3_decode_process (&sphinxsink->decoder, mfcc, nfr);
            fe_free_2d(mfcc);
        }
        sphinxsink->last_ts = sphinxsink->cont->read_ts;

        g_signal_emit (sphinxsink,
                       gst_sphinx3_signals[SIGNAL_LISTENING], 0, NULL);
    }
}

static GstFlowReturn gst_sphinx3_render (GstBaseSink * asink, GstBuffer * buffer)
{
    GstSphinx3 *sphinxsink = GST_SPHINX3 (asink);

    if (!sphinxsink->ad.initialized) {
        g_signal_emit (sphinxsink,
                       gst_sphinx3_signals[SIGNAL_INITIALIZATION], 0, NULL);
        /* We initialized the cmd_ln module earlier. */
        s3_decode_init (&sphinxsink->decoder);
        sphinxsink->fe = fe_init_auto();
        g_signal_emit (sphinxsink,
                       gst_sphinx3_signals[SIGNAL_AFTER_INITIALIZATION], 0, NULL);
        sphinxsink->ad.initialized = TRUE;
    }

    gst_buffer_ref (buffer);
    gst_adapter_push (sphinxsink->adapter, buffer);
  
    while (gst_adapter_available (sphinxsink->adapter) >= REQUIRED_FRAME_BYTES) {
	if (sphinxsink->ad.calibrated) {
            gst_sphinx3_process_chunk (sphinxsink);
    	} else {
            gst_sphinx3_calibrate_chunk (sphinxsink);
    	}
        gst_adapter_flush (sphinxsink->adapter, REQUIRED_FRAME_BYTES);
    }

    return GST_FLOW_OK;
}

static gboolean
plugin_init (GstPlugin * plugin)
{
    return gst_element_register (plugin, "sphinx3",
                                 GST_RANK_NONE, GST_TYPE_SPHINX3);
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
		   GST_VERSION_MINOR,
		   "sphinx3",
		   "Sphinx3 plugin",
		   plugin_init, VERSION,
		   "LGPL", "Sphinx3", "http://www.cmusphinx.org/")

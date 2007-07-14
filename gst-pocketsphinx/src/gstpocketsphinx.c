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
 *
 * <refsect2>
 * <title>Example launch line</title>
 * <para>
 * <programlisting>
 * gst-launch -v -m audiotestsrc ! plugin ! fakesink silent=TRUE
 * </programlisting>
 * </para>
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>
#include <sphinx_config.h>
#include <ad.h>
#include <string.h>

#include "gstpocketsphinx.h"

GST_DEBUG_CATEGORY_STATIC (gst_pocketsphinx_debug);
#define GST_CAT_DEFAULT gst_pocketsphinx_debug

/* Filter signals and args */
enum
{
	LAST_SIGNAL
};

enum
{
	ARG_0
};

static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
			 GST_PAD_SINK,
			 GST_PAD_ALWAYS,
			 GST_STATIC_CAPS("audio/x-raw-int, "
					 "width = (int) 16, "
					 "depth = (int) 16, "
					 "endianness = (int) BYTE_ORDER, "
					 "channels = (int) 1, "
					 "rate = (int) 16000")
	);
	
static void gst_pocketsphinx_set_property (GObject * object, guint prop_id,
					   const GValue * value, GParamSpec * pspec);
static void gst_pocketsphinx_get_property (GObject * object, guint prop_id,
					   GValue * value, GParamSpec * pspec);
static gboolean gst_pocketsphinx_event (GstBaseSink *basesink, GstEvent *event);
static GstFlowReturn gst_pocketsphinx_render (GstBaseSink * basesink, GstBuffer * buf);
static void gst_pocketsphinx_do_init (GType type);

GST_BOILERPLATE_FULL (GstPocketSphinx, gst_pocketsphinx, GstBaseSink,
		      GST_TYPE_BASE_SINK, gst_pocketsphinx_do_init);

static void
gst_pocketsphinx_do_init (GType type)
{
	GST_DEBUG_CATEGORY_INIT (gst_pocketsphinx_debug, "pocketsphinx",
				 0, "PocketSphinx plugin");
}


static void
gst_pocketsphinx_base_init (gpointer gclass)
{
	static GstElementDetails element_details = {
		"PocketSphinx",
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
gst_pocketsphinx_class_init (GstPocketSphinxClass * klass)
{
	GstBaseSinkClass *basesink_class;
	GObjectClass *gobject_class;

	gobject_class = (GObjectClass *) klass;
	basesink_class = (GstBaseSinkClass *) klass;

	gobject_class->set_property = gst_pocketsphinx_set_property;
	gobject_class->get_property = gst_pocketsphinx_get_property;
	basesink_class->event = gst_pocketsphinx_event;
	basesink_class->render = gst_pocketsphinx_render;
	/* TODO: We will bridge cmd_ln.h properties to GObject properties here */
}

static void
gst_pocketsphinx_init (GstPocketSphinx * rec,
		       GstPocketSphinxClass * gclass)
{
	/* This is all totally bogus of course */
	static char *argv[] = {
		"gst-pocketsphinx",
		"-hmm", "/usr/local/share/pocketsphinx/model/hmm/wsj0",
		"-lm", "/usr/local/share/pocketsphinx/model/lm/swb/swb.lm.DMP",
		"-dict", "/usr/local/share/pocketsphinx/model/lm/swb/swb.dic",
		"-fwdflat", "no"
	};
	static const int argc = sizeof(argv)/sizeof(argv[0]);
	ad_rec_t ad;

	/* Initialize the decoder and stuff here */
	fbs_init(argc, argv);

	memset(&ad, 0, sizeof(ad));
	ad.sps = 16000;
	rec->cad = cont_ad_init(&ad, NULL);
	cont_ad_detach(rec->cad);
	rec->ts = 0;
	rec->listening = FALSE;
}

static void
gst_pocketsphinx_set_property (GObject * object, guint prop_id,
			       const GValue * value, GParamSpec * pspec)
{
	switch (prop_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

static void
gst_pocketsphinx_get_property (GObject * object, guint prop_id,
			       GValue * value, GParamSpec * pspec)
{
	switch (prop_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
		break;
	}
}

/* handle events from upstream elements */
static gboolean
gst_pocketsphinx_event (GstBaseSink *basesink, GstEvent *event)
{
	GstPocketSphinx *rec;

	rec = GST_POCKETSPHINX (basesink);

	switch (GST_EVENT_TYPE (event)) {
	case GST_EVENT_EOS:
		/* End audio and utterance processing and spit out any remaining results */
		break;
	default:
		break;
	}

	return TRUE;
}

/* process stream data */
static GstFlowReturn
gst_pocketsphinx_render (GstBaseSink * basesink, GstBuffer * buf)
{
	GstBuffer *writebuf;
	GstPocketSphinx *rec;
	GstPad *sink;
	int32 n;

	rec = GST_POCKETSPHINX (basesink);
	sink = GST_BASE_SINK_PAD (basesink);
	writebuf = gst_buffer_make_writable(buf);
	if ((n = cont_ad_read(rec->cad, (short *)writebuf->data,
			      writebuf->size / sizeof(short))) == 0) {
		if (rec->listening
		    && rec->cad->read_ts - rec->ts > 16000) {
			int32 fr;
			char *hyp;

			rec->listening = FALSE;
			cont_ad_reset(rec->cad);
			uttproc_end_utt();
			uttproc_result(&fr, &hyp, 1);
		}
		return GST_FLOW_OK;
	}

	if (rec->listening == FALSE) {
		rec->listening = TRUE;
		uttproc_begin_utt(NULL);
	}
	uttproc_rawdata((short *)writebuf->data, n, TRUE);
	rec->ts = rec->cad->read_ts;

	return GST_FLOW_OK;
}

static gboolean
plugin_init (GstPlugin * plugin)
{
	return gst_element_register (plugin, "pocketsphinx",
				     GST_RANK_NONE, GST_TYPE_POCKETSPHINX);
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
		   GST_VERSION_MINOR,
		   "pocketsphinx",
		   "PocketSphinx plugin",
		   plugin_init, VERSION,
		   /* FIXME: This should say MIT, but for some reason
		    * that isn't a "valid" license. */
		   "LGPL", "PocketSphinx", "http://pocketsphinx.org/")

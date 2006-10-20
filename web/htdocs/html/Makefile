###########################################################################
##                                                                       ##
##                     Carnegie Mellon University                        ##
##                        Copyright (c) 2003                             ##
##                        All Rights Reserved.                           ##
##                                                                       ##
##  Permission is hereby granted, free of charge, to use and distribute  ##
##  this software and its documentation without restriction, including   ##
##  without limitation the rights to use, copy, modify, merge, publish,  ##
##  distribute, sublicense, and/or sell copies of this work, and to      ##
##  permit persons to whom this work is furnished to do so, subject to   ##
##  the following conditions:                                            ##
##   1. The code must retain the above copyright notice, this list of    ##
##      conditions and the following disclaimer.                         ##
##   2. Any modifications must be clearly marked as such.                ##
##   3. Original authors' names are not deleted.                         ##
##   4. The authors' names are not used to endorse or promote products   ##
##      derived from this software without specific prior written        ##
##      permission.                                                      ##
##                                                                       ##
##  CARNEGIE MELLON UNIVERSITY AND THE CONTRIBUTORS TO THIS WORK         ##
##  DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING      ##
##  ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT   ##
##  SHALL CARNEGIE MELLON UNIVERSITY NOR THE CONTRIBUTORS BE LIABLE      ##
##  FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES    ##
##  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN   ##
##  AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,          ##
##  ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF       ##
##  THIS SOFTWARE.                                                       ##
##                                                                       ##
###########################################################################
##                                                                       ##
##  html pages                                                           ##
##                                                                       ##
###########################################################################
TOP=.
DIRNAME=.
BUILD_DIRS = 
ALL_DIRS= $(BUILD_DIRS)
HTMLBODIES= cmusphinx_body.html contact_body.html download_body.html compare_body.html system_body.html
HTMLBITS=header.html sidebar.html footer.html
SCRIPTS=make_html
FINALHTML= $(HTMLBODIES:_body.html=.php) $(SCRIPTS)
FILES = Makefile $(HTMLBODIES) $(HTMLBITS)
LOCAL_CLEAN = $(HTMLBODIES:_body.html=.php) *.tmp.html

INSTALLDIR=shell.cmusphinx.net:/home/groups/c/cm/cmusphinx/htdocs/

ALL = $(FINALHTML)

include $(TOP)/config/common_make_rules

%.php : %_body.html $(HTMLBITS) make_html
	@ ./make_html $< "" header.html footer.html sidebar.html >$@

install:
ifdef ALL_DIRS
	@ for i in $(ALL_DIRS) ; \
	do \
	   $(MAKE) -C $$i --no-print-directory install INSTALLDIR=$(INSTALLDIR) ; \
	done
endif
	scp -p $(FINALHTML) $(INSTALLDIR)



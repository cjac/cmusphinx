#********************************************************************************
#                                                                               *
#       Copyright (c) 1999                                                      *
#	Center for Speech and Language Research					*
#       University of Colorado at Boulder                                       *
#       All Rights Reserved                                                     *
#                                                                               *
#********************************************************************************

MAKEFILE = Makefile
include ../../include/${MAKEFILE}.defines

ROOT_DIR = $(GALAXY_ROOT)
TEMPLATES = $(ROOT_DIR)/templates
PACKAGE = galaxy
SLS_HOME = $(ROOT_DIR)/sls
NL_HOME = $(ROOT_DIR)/nl
GALAXY_HOME = $(ROOT_DIR)/galaxy

CPPFLAGS = -I$(CSLR_BASE)/lib/Phoenix/ParserLib
LDFLAGS  = 
COMMON_LIBS = $(CSLR_BASE)/lib/Phoenix/ParserLib/libparse.a
EXECDIR = ../../bin/
SERVER = phoenix_serv
SOURCES = phoenix_serv.c
include ${ROOT_DIR}/templates/rules.make
include ${MAKEFILE}.depend

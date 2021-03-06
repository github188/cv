#***************************************************************************
#                                  _   _ ____  _
#  Project                     ___| | | |  _ \| |
#                             / __| | | | |_) | |
#                            | (__| |_| |  _ <| |___
#                             \___|\___/|_| \_\_____|
#
# Copyright (C) 1998 - 2007, Daniel Stenberg, <daniel@haxx.se>, et al.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at http://curl.haxx.se/docs/copyright.html.
#
# You may opt to use, copy, modify, merge, publish, distribute and/or sell
# copies of the Software, and permit persons to whom the Software is
# furnished to do so, under the terms of the COPYING file.
#
# This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
# KIND, either express or implied.
#
# $Id: Makefile.am,v 1.44 2007-01-29 10:09:06 bagder Exp $
###########################################################################
AUTOMAKE_OPTIONS = foreign nostdinc

# $(top_srcdir)/include/curl is for the main curl include files, to make it
# easier to include this specific set of curl headers, and NOT the ones
# possibly already installed in the system.
# -I$(top_srcdir)/lib is for the setup.h file, included by test.h
# -I$(top_builddir)/lib is for the config.h file, possibly included by the
# setup.h file

INCLUDES = -I$(top_srcdir)/include/curl \
           -I$(top_builddir)/lib   \
           -I$(top_srcdir)/lib 

LIBDIR = $(top_builddir)/lib

EXTRA_DIST = test307.pl

# *NOTICE* that some test cases use curlx_* functions that are not in the API
# but must be provided by source
TIMEVAL=$(top_srcdir)/lib/timeval.c

# these files are used in every single test program below
SUPPORTFILES = first.c test.h

# These are all libcurl test programs
noinst_PROGRAMS = lib500 lib501 lib502 lib503 lib504 lib505 lib506	\
  lib507 lib508 lib509 lib510 lib511 lib512 lib513 lib514 lib515 lib516	\
  lib517 lib518 lib519 lib520 lib521 lib523 lib524 lib525 lib526 lib527	\
  lib529 lib530 lib532 lib533 lib536 lib537

# Dependencies (may need to be overriden)
LDADD = $(LIBDIR)/libcurl.la
DEPENDENCIES = $(LIBDIR)/libcurl.la

lib500_SOURCES = lib500.c $(SUPPORTFILES)
lib500_DEPENDENCIES = $(DEPENDENCIES)

lib501_SOURCES = lib501.c $(SUPPORTFILES)
lib501_DEPENDENCIES = $(DEPENDENCIES)

lib502_SOURCES = lib502.c $(SUPPORTFILES) $(TIMEVAL)
lib502_DEPENDENCIES = $(DEPENDENCIES)

lib503_SOURCES = lib503.c $(SUPPORTFILES) $(TIMEVAL)
lib503_DEPENDENCIES = $(DEPENDENCIES)

lib504_SOURCES = lib504.c $(SUPPORTFILES) $(TIMEVAL)
lib504_DEPENDENCIES = $(DEPENDENCIES)

lib505_SOURCES = lib505.c $(SUPPORTFILES)
lib505_DEPENDENCIES = $(DEPENDENCIES)

lib506_SOURCES = lib506.c $(SUPPORTFILES)
lib506_DEPENDENCIES = $(DEPENDENCIES)

lib507_SOURCES = lib507.c $(SUPPORTFILES) $(TIMEVAL)
lib507_DEPENDENCIES = $(DEPENDENCIES)

lib508_SOURCES = lib508.c $(SUPPORTFILES)
lib508_DEPENDENCIES = $(DEPENDENCIES)

lib509_SOURCES = lib509.c $(SUPPORTFILES) $(TIMEVAL)
lib509_DEPENDENCIES = $(DEPENDENCIES)

lib510_SOURCES = lib510.c $(SUPPORTFILES)
lib510_DEPENDENCIES = $(DEPENDENCIES)

lib511_SOURCES = lib511.c $(SUPPORTFILES)
lib511_DEPENDENCIES = $(DEPENDENCIES)

lib512_SOURCES = lib512.c $(SUPPORTFILES)
lib512_DEPENDENCIES = $(DEPENDENCIES)

lib513_SOURCES = lib513.c $(SUPPORTFILES)
lib513_DEPENDENCIES = $(DEPENDENCIES)

lib514_SOURCES = lib514.c $(SUPPORTFILES)
lib514_DEPENDENCIES = $(DEPENDENCIES)

lib515_SOURCES = lib515.c $(SUPPORTFILES)
lib515_DEPENDENCIES = $(DEPENDENCIES)

lib516_SOURCES = lib516.c $(SUPPORTFILES)
lib516_DEPENDENCIES = $(DEPENDENCIES)

lib517_SOURCES = lib517.c $(SUPPORTFILES)
lib517_DEPENDENCIES = $(DEPENDENCIES)

lib518_SOURCES = lib518.c $(SUPPORTFILES)
lib518_DEPENDENCIES = $(DEPENDENCIES)

lib519_SOURCES = lib519.c $(SUPPORTFILES)
lib519_DEPENDENCIES = $(DEPENDENCIES)

lib520_SOURCES = lib520.c $(SUPPORTFILES)
lib520_DEPENDENCIES = $(DEPENDENCIES)

lib521_SOURCES = lib521.c $(SUPPORTFILES)
lib521_DEPENDENCIES = $(DEPENDENCIES)

lib523_SOURCES = lib523.c $(SUPPORTFILES)
lib523_DEPENDENCIES = $(DEPENDENCIES)

lib524_SOURCES = lib524.c $(SUPPORTFILES)
lib524_DEPENDENCIES = $(DEPENDENCIES)

lib525_SOURCES = lib525.c $(SUPPORTFILES) $(TIMEVAL)
lib525_DEPENDENCIES = $(DEPENDENCIES)

lib526_SOURCES = lib526.c $(SUPPORTFILES) $(TIMEVAL)
lib526_CFLAGS = -DLIB526
lib526_DEPENDENCIES = $(DEPENDENCIES)

lib527_SOURCES = lib526.c $(SUPPORTFILES)
lib527_CFLAGS = -DLIB527
lib527_DEPENDENCIES = $(DEPENDENCIES)

lib529_SOURCES = lib525.c $(SUPPORTFILES)
lib529_CFLAGS = -DLIB529
lib529_DEPENDENCIES = $(DEPENDENCIES)

lib530_SOURCES = lib530.c $(SUPPORTFILES) $(TIMEVAL)
lib530_CFLAGS = -DLIB530
lib530_DEPENDENCIES = $(DEPENDENCIES)

lib532_SOURCES = lib526.c $(SUPPORTFILES)
lib532_CFLAGS = -DLIB532
lib532_DEPENDENCIES = $(DEPENDENCIES)

lib533_SOURCES = lib533.c $(SUPPORTFILES) $(TIMEVAL)
lib533_DEPENDENCIES = $(DEPENDENCIES)

lib536_SOURCES = lib536.c $(SUPPORTFILES) $(TIMEVAL)
lib536_DEPENDENCIES = $(DEPENDENCIES)

lib537_SOURCES = lib537.c $(SUPPORTFILES)
lib537_DEPENDENCIES = $(DEPENDENCIES)


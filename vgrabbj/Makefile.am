#
# Copyright (C) 2000, 2001, 2002 Jens Gecius, Hannover, Germany
# eMail: devel@gecius.de
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at you option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston MA 02111-1307,
# USA  

bin_PROGRAMS = vgrabbj
vgrabbj_SOURCES = vgrabbj.c vgrabbj.h v_config.c v_config.h font.c ccvt_c.c ccvt.h v_ftp.c v_ftp.h v_writer.c v_writer.h v_utils.c v_utils.h v_stamp.c v_stamp.h v_plist.h v_options.h
vgrabbj_CFLAGS = @LIBV4L1_CFLAGS@
vgrabbj_LDADD = @LIBV4L1_LIBS@

#sysconf_DATA = vgrabbj.conf.default
EXTRA_DIST = vgrabbj.conf.default $(man_MANS)

man_MANS = vgrabbj.1 vgrabbj.conf.5

DEFAULT_CONFIG_FILE = $(sysconfdir)/vgrabbj.conf

install-data-local: #install-sysconfDATA
	@if test -f $(DESTDIR)$(sysconfdir)/vgrabbj.conf ; then \
	        echo "$@ will not overwrite existing $(DESTDIR)$(sysconfdir)/vgrabbj.conf" ; \
	else \
		if ! test -d $(DESTDIR)$(sysconfdir); then \
		    mkdir $(DESTDIR)$(sysconfdir); \
		fi; \
	        echo "$(INSTALL_DATA) $(srcdir)/vgrabbj.conf.default $(DESTDIR)$(sysconfdir)/vgrabbj.conf" ;\
	        $(INSTALL_DATA) $(srcdir)/vgrabbj.conf.default $(DESTDIR)$(sysconfdir)/vgrabbj.conf; \
	fi

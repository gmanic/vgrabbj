#
# Copyright (C) 2000, 2001 Jens Gecius, Larchmont, USA
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
vgrabbj_SOURCES = vgrabbj.c v_config.c font.c ccvt_c.c vgrabbj.h ccvt.h v_ftp.c ftplib.c ftplib.h

#sysconf_DATA = vgrabbj.conf.default
EXTRA_DIST = vgrabbj.conf.default

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

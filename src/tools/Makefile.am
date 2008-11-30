AM_CPPFLAGS = -I$(top_srcdir)/src/libofflinefs
bin_PROGRAMS = offimport offmedia
offimport_SOURCES = offimport.cxx format.hxx
offimport_LDADD = $(top_srcdir)/src/libofflinefs/libofflinefs.la -lboost_program_options
offmedia_SOURCES = offmedia.cxx
offmedia_LDADD = $(top_srcdir)/src/libofflinefs/libofflinefs.la -lboost_program_options

bin_SCRIPTS = offimport_cd.sh
EXTRA_DIST = offimport_cd.sh.in
CLEANFILES = offimport_cd.sh

edit = sed \
	-e 's|@sysconfdir[@]|$(sysconfdir)|g'

offimport_cd.sh: Makefile offimport_cd.sh.in
	rm -f $@
	$(edit) $@.in > $@
	chmod a+x $@

AM_CPPFLAGS = -I$(top_srcdir)/src/libofflinefs
dist_bin_SCRIPTS = offimport_cd.sh
bin_PROGRAMS = offimport offmedia
offimport_SOURCES = offimport.cxx format.hxx
offimport_LDADD = $(top_srcdir)/src/libofflinefs/libofflinefs.la -lboost_program_options
offmedia_SOURCES = offmedia.cxx
offmedia_LDADD = $(top_srcdir)/src/libofflinefs/libofflinefs.la -lboost_program_options

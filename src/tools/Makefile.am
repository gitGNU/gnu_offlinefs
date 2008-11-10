AM_CPPFLAGS = -I$(top_srcdir)/src/libofflinefs
dist_bin_SCRIPTS = offimport_cd.sh
bin_PROGRAMS = offimport
offimport_SOURCES = offimport.cxx format.hxx
offimport_LDADD = $(top_srcdir)/src/libofflinefs/libofflinefs.la -lboost_program_options $(libconfig_LIBS)

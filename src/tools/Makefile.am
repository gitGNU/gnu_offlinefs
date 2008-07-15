AM_CPPFLAGS = -I$(top_srcdir)/src/libofflinefs
bin_PROGRAMS = offmedia offimport
offmedia_SOURCES =  offmedia.cxx
offmedia_LDADD = $(top_srcdir)/src/libofflinefs/libofflinefs.la
offimport_SOURCES = offimport.cxx format.hxx
offimport_LDADD = $(top_srcdir)/src/libofflinefs/libofflinefs.la -lboost_program_options

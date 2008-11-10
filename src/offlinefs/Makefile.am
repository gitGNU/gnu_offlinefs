AM_CPPFLAGS= -I$(top_srcdir)/src/libofflinefs
bin_PROGRAMS=offlinefs
offlinefs_SOURCES = fs.cxx fs.hxx main.cxx scontextcache.cxx scontextcache.hxx
AM_CXXFLAGS = $(libconfig_CFLAGS)
offlinefs_LDADD = $(top_srcdir)/src/libofflinefs/libofflinefs.la $(libconfig_LIBS)

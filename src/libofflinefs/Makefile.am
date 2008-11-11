AUTOMAKE_OPTIONS = subdir-objects
lib_LTLIBRARIES = libofflinefs.la
libofflinefs_la_SOURCES = database.cxx database.hxx fsdb.cxx fsdb.hxx nodes/directory.cxx nodes/directory.hxx nodes/file.cxx nodes/file.hxx nodes.hxx nodes/node.cxx nodes/node.hxx nodes/symlink.cxx nodes/symlink.hxx media/medium.cxx media/medium.hxx media/directory.hxx media/directory.cxx media/insert.hxx media/insert.cxx register.hxx sources/source.cxx sources/source.hxx sources/single.hxx sources/single.cxx chunks/chunk.hxx chunks/file.hxx chunks/file.cxx common.hxx scontext.hxx scontext.cxx pathcache.hxx pathcache.cxx mediacache.hxx mediacache.cxx path.hxx
AM_CXXFLAGS = $(libconfig_CFLAGS)
libofflinefs_la_LDFLAGS = -release @PACKAGE_VERSION@ $(libconfig_LIBS)

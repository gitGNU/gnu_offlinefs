#ifndef COMMON_HXX
#define COMMON_HXX

#include "config.h"
#define _FILE_OFFSET_BITS 64
#define FUSE_USE_VERSION 26
#include <fuse.h>
#include <iostream>
#include <sys/stat.h>
#include <errno.h>
#include <attr/xattr.h>
#include <time.h>
#include <stdexcept>
#include <list>
#include <memory>
#include <unistd.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <fcntl.h>
#include <utility>

#ifdef HAVE_DB_CXX_H
#include <db_cxx.h>
#elif defined HAVE_DB4_5_DB_CXX_H
#include <db4.5/db_cxx.h>
#endif

#endif

//     Copyright (C) 2008 Francisco Jerez
//     This file is part of offlinefs.

//     offlinefs is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.

//     offlinefs is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.

//     You should have received a copy of the GNU General Public License
//     along with offlinefs.  If not, see <http://www.gnu.org/licenses/>.

#ifndef COMMON_HXX
#define COMMON_HXX

#include <config.h>
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

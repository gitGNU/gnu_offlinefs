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

#ifndef FS_HXX
#define FS_HXX

#include <common.hxx>
#include <fsdb.hxx>
#include <fsnodes.hxx>
#include <sources.hxx>
#include <media.hxx>
#include <pthread.h>
#include "scontextcache.hxx"
#include "pathcache.hxx"

#define MAX_OPEN_FILES 512

class FS{
   private:      
      pthread_mutex_t openmutex;
      Source* openFiles[MAX_OPEN_FILES];
      FsDb dbs;

      SContextCache scache;
      PathCache_hash pcache;
      inline SContext userctx();

      uint32_t defmedium;
      //Get the medium associated to f
      //If it isn't associated with any medium, associate it with the default one
      std::auto_ptr<Medium> getmedium(FsTxn& txns,File& f,std::string phid);
   public:
      FS(std::string dbroot,uint32_t defmedium);
      ~FS();

      int getattr(const char* path, struct stat* st);
      int readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, fuse_file_info* fi);
      int readlink(const char* path, char* buf, size_t bufsiz);
      int mknod(const char* path , mode_t mode, dev_t dev);
      int mkdir(const char* path, mode_t mode);
      int unlink(const char* path);
      int rmdir(const char* path);
      int symlink(const char* oldpath, const char* newpath); 
      int rename(const char* oldpath, const char* newpath);
      int link(const char* oldpath, const char* newpath);
      int chmod(const char* path, mode_t mode);
      int chown(const char* path, uid_t owner, gid_t group);
      int truncate(const char* path, off_t length);
      int open(const char* path, struct fuse_file_info* fi);
      int read(const char* path, char* buf, size_t nbyte, off_t offset, struct fuse_file_info* fi);
      int write(const char* path, const char* buf, size_t nbyte, off_t offset, struct fuse_file_info* fi);
      int statfs(const char* path, struct statvfs* buf);
      int flush(const char* path, struct fuse_file_info* fi);
      int release(const char* path, struct fuse_file_info* fi);
      int fsync(const char* path, int datasync, struct fuse_file_info* fi);
      int setxattr(const char* path, const char* name, const char* value, size_t size, int flags);
      int getxattr(const char* path, const char* name, char* value, size_t size);
      int listxattr(const char* path , char* list, size_t size);
      int removexattr(const char* path, const char* name);
      int utimens(const char* path, const struct timespec tv[2]);
      int access(const char* path, int mode);
};

#endif

#ifndef FS_HXX
#define FS_HXX

#include "common.hxx"
#include "fsdb.hxx"
#include "fsnodes.hxx"
#include "source.hxx"
#include "medium.hxx"

#define MAX_OPEN_FILES 512

class FS{
   private:      
      Source* openFiles[MAX_OPEN_FILES];
      FsDb dbs;
   public:
      FS(std::string dbroot);
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

#include "fs.hxx"
#include <iostream>
#include <sys/stat.h>
#include <errno.h>
#include <attr/xattr.h>

#define MIN(a,b) (a<b?a:b)

using std::auto_ptr;
using std::string;
using std::exception;
using std::list;

FS::FS():db("/home/curro/metafs/offlinefs/db/") {
   memset(openFiles,0,sizeof(openFiles));
}

FS::~FS(){
   for(int i=0;i<MAX_OPEN_FILES;i++)
      if(openFiles[i]!=NULL)
	 delete openFiles[i];
}

int errcode(exception& e){
   if(typeid(e)==typeid(Node::ENotFound))
      return -ENOENT;
   else if(typeid(e)==typeid(std::bad_cast))
      return -ENOTDIR;
   else if(typeid(e)==typeid(Node::EAttrNotFound))
      return -EIO;
   else if(typeid(e)==typeid(Node::ENotDir))
      return -ENOTDIR;
   else if(typeid(e)==typeid(Node::EExists))
      return -EEXIST;
   else{
      std::cout << e.what() << "!!" <<std::endl;
      return -EIO;
   }
}

int FS::getattr(const char* path, struct stat* st){
   memset(st, 0, sizeof(struct stat));
   try{
      auto_ptr<Node> n=Node::getnode(db,string(path));
      st->st_ino=(ino_t)n->getid();
      st->st_dev=n->getattr<dev_t>("offlinefs.dev");
      st->st_nlink=n->getattr<nlink_t>("offlinefs.nlink");
      st->st_mode=n->getattr<mode_t>("offlinefs.mode");
      st->st_uid=n->getattr<uid_t>("offlinefs.uid");
      st->st_gid=n->getattr<gid_t>("offlinefs.gid");
      st->st_size=n->getattr<off_t>("offlinefs.size");
      st->st_atime=n->getattr<time_t>("offlinefs.atime");
      st->st_mtime=n->getattr<time_t>("offlinefs.mtime");
      st->st_ctime=n->getattr<time_t>("offlinefs.ctime");
   }catch(exception& e){
      return errcode(e);
   }

  return 0;
}

int FS::readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, fuse_file_info* fi){ 
   try{
      auto_ptr<Node> n=Node::getnode(db,string(path));
      Directory& d=dynamic_cast<Directory&>(*n);
      Directory::dirlist l=d.getchildren();
      for(Directory::dirlist::iterator it=l.begin();it!=l.end();it++)
	 filler(buf,it->first.c_str(),NULL,0);
   }catch(exception& e){
      return errcode(e);
   }
  return 0;
}

int FS::readlink(const char* path, char* buf, size_t bufsiz){
   try{
      auto_ptr<Node> n=Node::getnode(db,string(path));
      Symlink& sl=dynamic_cast<Symlink&>(*n);
      Buffer target=sl.getattrv("offlinefs.symlink");
      memcpy(buf,target.data,MIN(bufsiz,target.size));
      if(target.size<bufsiz)
	 buf[target.size]=0;
      return 0;
   }catch(exception& e){
      return errcode(e);
   }
}

int FS::mknod(const char* path , mode_t mode, dev_t dev){
   try{
      auto_ptr<Node> n=Node::create(db,string(path));
      n->setattr<mode_t>("offlinefs.mode",mode);
      if(S_ISCHR(mode)||S_ISBLK(mode))
	 n->setattr<dev_t>("offlinefs.dev",dev);
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}

int FS::mkdir(const char* path, mode_t mode){
   try{
      auto_ptr<Directory> n=Directory::create(db,string(path));
      n->setattr<mode_t>("offlinefs.mode",S_IFDIR|mode);
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}

int FS::unlink(const char* path){
   try{
      Path p(db,path);
      if(!p.nparent.get())
	 return -ENOTDIR;
      else if(!p.nchild.get())
	 return -ENOENT;
      p.nparent->delchild(p.child);
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}

int FS::rmdir(const char* path){
   try{
      Path p(db,path);
      if(!p.nchild.get())
	 return -ENOENT;
      if(dynamic_cast<Directory&>(*p.nchild).getchildren().size()>2)
	 return -ENOTEMPTY;
      p.nparent->delchild(p.child);
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}

int FS::symlink(const char* oldpath, const char* newpath){
   try{
      auto_ptr<Symlink> sl=Symlink::create(db,newpath);
      sl->setattrv("offlinefs.symlink",Buffer(oldpath,strlen(oldpath)));
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}
 
int FS::rename(const char* oldpath, const char* newpath){
   int err;
   if((err=link(oldpath,newpath)))
      return err;
   if((err=unlink(oldpath)))
      return err;
   return 0;
}

int FS::link(const char* oldpath, const char* newpath){
   try{
      Path p(db,newpath);
      if(p.nchild.get())
	 return -EEXIST;
      p.nparent->addchild(p.child,Node::getnode(db,oldpath)->getid());
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}

int FS::chmod(const char* path, mode_t mode){
   try{
      auto_ptr<Node> n=Node::getnode(db,path);
      n->setattr<mode_t>("offlinefs.mode",mode&(~S_IFMT)|n->getattr<mode_t>("offlinefs.mode")&S_IFMT);
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}

int FS::chown(const char* path, uid_t owner, gid_t group){
   try{
      auto_ptr<Node> n=Node::getnode(db,path);
      if(owner!=(uid_t)-1)
	 n->setattr<uid_t>("offlinefs.uid",owner);
      if(group!=(gid_t)-1)
	 n->setattr<gid_t>("offlinefs.gid",group);
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}

int FS::truncate(const char* path, off_t length){
   try{
      return Medium::getmedium(db,Node::getnode(db,path)->getattr<uint32_t>("offlinefs.mediumid"))->truncate(length);
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}

int FS::open(const char* path, struct fuse_file_info* fi){
   try{
      auto_ptr<Node> n=Node::getnode(db,path);
      auto_ptr<Source> s=Medium::getmedium(db,n->getattr<uint32_t>("offlinefs.mediumid"))->getsource(dynamic_cast<File&>(*n));
      for(int i=0;i<MAX_OPEN_FILES;i++)
	 if(openFiles[i]==NULL){
	    openFiles[i]=s.release();
	    fi->fh=i;
	    return 0;
	 }
	 return -ENFILE;
   }catch(exception& e){
      return errcode(e);
   }
}

int FS::read(const char* path, char* buf, size_t nbyte, off_t offset, struct fuse_file_info* fi){
   if(fi->fh>=MAX_OPEN_FILES || !openFiles[fi->fh])
      return -EBADF;
   return openFiles[fi->fh]->read(buf,nbyte,offset);
}

int FS::write(const char* path, const char* buf, size_t nbyte, off_t offset, struct fuse_file_info* fi){
   if(fi->fh>=MAX_OPEN_FILES || !openFiles[fi->fh])
      return -EBADF;
   return openFiles[fi->fh]->write(buf,nbyte,offset);
}

int FS::statfs(const char* path, struct statvfs* buf){
   memset(buf,0,sizeof(struct statvfs));
   return 0;
}

int FS::flush(const char* path, struct fuse_file_info* fi){
   if(fi->fh>=MAX_OPEN_FILES || !openFiles[fi->fh])
      return -EBADF;
   return openFiles[fi->fh]->flush();
}

int FS::release(const char* path, struct fuse_file_info* fi){
   if(fi->fh>=MAX_OPEN_FILES || !openFiles[fi->fh])
      return -EBADF;
   delete openFiles[fi->fh];
   openFiles[fi->fh]=NULL;
   return 0;
}

int FS::fsync(const char* path,int datasync, struct fuse_file_info* fi){
   if(fi->fh>=MAX_OPEN_FILES || !openFiles[fi->fh])
      return -EBADF;
   return openFiles[fi->fh]->fsync(datasync);
}

int FS::setxattr(const char* path, const char* name, const char* value, size_t size, int flags){
   try{
      auto_ptr<Node> n=Node::getnode(db,path);
      if(flags){
	 if(flags==XATTR_CREATE){
	    try{
	       n->getattrv(name);
	       return -EEXIST;
	    }catch(Node::EAttrNotFound& e){}
	 }else if(flags==XATTR_REPLACE){
	    n->getattrv(name);
	 }else{
	    return -EINVAL;
	 }
      }
      n->setattrv(name,Buffer(value,size));
   }catch(Node::EAttrNotFound& e){
      return -ENOATTR;
   }catch(exception& e){
      return errcode(e);
   }
   return 0;   
}

int FS::getxattr(const char* path, const char* name, char* value, size_t size){
   try{
      auto_ptr<Node> n=Node::getnode(db,path);
      Buffer b=n->getattrv(name);
      memcpy(value,b.data,MIN(size,b.size));
      return b.size;
   }catch(Node::EAttrNotFound& e){
      return -ENOATTR;
   }catch(exception& e){
      return errcode(e);
   }
}

int FS::listxattr(const char* path , char* list, size_t size){
   try{
      auto_ptr<Node> n=Node::getnode(db,path);
      std::list<string> l=n->getattrs();
      size_t count=0;
      for(std::list<string>::iterator it=l.begin();it!=l.end();it++){
	 int namesize=it->size()+1;
	 count+=namesize;
	 if(count<=size){
	    memcpy(list,it->c_str(),namesize);
	    list+=namesize;
	 }
      }
      return count;
   }catch(exception& e){
      return errcode(e);
   }
}

int FS::removexattr(const char* path, const char* name){
   try{
      auto_ptr<Node> n=Node::getnode(db,path);
      n->delattr(name);
   }catch(Node::EAttrNotFound& e){
      return -ENOATTR;
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}

int FS::utimens(const char* path, const struct timespec tv[2]){
   try{
      auto_ptr<Node> n=Node::getnode(db,path);
      n->setattr<time_t>("offlinefs.atime",tv[0].tv_sec);
      n->setattr<time_t>("offlinefs.mtime",tv[1].tv_sec);
   }catch(exception& e){
      return errcode(e);
   }
   return 0;   
}


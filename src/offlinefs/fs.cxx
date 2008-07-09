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

#include "fs.hxx"

#define MIN(a,b) (a<b?a:b)

using std::auto_ptr;
using std::string;
using std::exception;
using std::list;

inline SContext FS::userctx(){
   return scache.get(fuse_get_context()->uid,fuse_get_context()->gid);
}

FS::FS(string dbroot):dbs(dbroot){
   memset(openFiles,0,sizeof(openFiles));
   dbs.open();
   if(pthread_mutex_init(&openmutex,NULL))
      throw std::runtime_error("FS::FS: error initializing the mutex.");
}

FS::~FS(){
   pthread_mutex_lock(&openmutex);
   for(int i=0;i<MAX_OPEN_FILES;i++)
      if(openFiles[i]!=NULL)
	 delete openFiles[i];
   pthread_mutex_unlock(&openmutex);
   pthread_mutex_destroy(&openmutex);
}

// Translate an exception into an error code
int errcode(exception& e){
   if(typeid(e)==typeid(Node::ENotFound))
      return -ENOENT;
   else if(typeid(e)==typeid(Node::EBadCast<Directory>))
      return -ENOTDIR;
   else if(typeid(e)==typeid(Node::EBadCast<Symlink>))
      return -EINVAL;
   else if(typeid(e)==typeid(Node::EBadCast<File>))
      return -EINVAL;
   else if(typeid(e)==typeid(Node::EAttrNotFound))
      return -EIO;
   else if(typeid(e)==typeid(Directory::EExists))
      return -EEXIST;
   else if(typeid(e)==typeid(Node::EAccess))
      return -EACCES;
   else{
      std::cerr << e.what() << "!!" <<std::endl;
      return -EIO;
   }
}

int FS::getattr(const char* path, struct stat* st){
   memset(st, 0, sizeof(struct stat));
   try{
      FsTxn txns(dbs);
      SContext sctx=userctx();
      auto_ptr<Node> n=Node::getnode(txns,sctx,pcache,string(path));
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
      FsTxn txns(dbs);
      SContext sctx=userctx();
      auto_ptr<Directory> n=Node::cast<Directory>(Node::getnode(txns,sctx,pcache,string(path)));
      n->access(sctx,R_OK|X_OK);
      n->setattr<time_t>("offlinefs.atime",time(NULL));
      list<string> l=n->getchildren();
      for(list<string>::iterator it=l.begin();it!=l.end();it++)
	 filler(buf,it->c_str(),NULL,0);
   }catch(exception& e){
      return errcode(e);
   }
  return 0;
}

int FS::readlink(const char* path, char* buf, size_t bufsiz){
   try{
      FsTxn txns(dbs);
      SContext sctx=userctx();
      auto_ptr<Symlink> n=Node::cast<Symlink>(Node::getnode(txns,sctx,pcache,string(path)));
      Buffer target=n->getattrv("offlinefs.symlink");
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
      FsTxn txns(dbs);
      SContext sctx=userctx();
      auto_ptr<Node> n=Node::create(txns,sctx,pcache,string(path));
      n->setattr<time_t>("offlinefs.atime",time(NULL));
      n->setattr<time_t>("offlinefs.mtime",time(NULL));
      n->setattr<time_t>("offlinefs.ctime",time(NULL));
      n->setattr<uid_t>("offlinefs.uid",fuse_get_context()->uid);
      n->setattr<gid_t>("offlinefs.gid",fuse_get_context()->gid);
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
      FsTxn txns(dbs);
      SContext sctx=userctx();
      auto_ptr<Directory> n=Directory::create(txns,sctx,pcache,string(path));
      n->setattr<time_t>("offlinefs.atime",time(NULL));
      n->setattr<time_t>("offlinefs.mtime",time(NULL));
      n->setattr<time_t>("offlinefs.ctime",time(NULL));
      n->setattr<uid_t>("offlinefs.uid",fuse_get_context()->uid);
      n->setattr<gid_t>("offlinefs.gid",fuse_get_context()->gid);
      n->setattr<mode_t>("offlinefs.mode",S_IFDIR|mode);
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}

int FS::unlink(const char* path){
   try{
      FsTxn txns(dbs);
      SContext sctx=userctx();
      Directory::Path p(txns,sctx,pcache,path);
      p.parent->access(sctx,W_OK|X_OK);
      p.parent->getchild(p.leaf)->setattr<time_t>("offlinefs.ctime",time(NULL));
      p.parent->delchild(p.leaf);
      pcache.invalidate(path);
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}

int FS::rmdir(const char* path){
   try{
      FsTxn txns(dbs);
      SContext sctx=userctx();
      Directory::Path p(txns,sctx,pcache,path);
      p.parent->access(sctx,X_OK|W_OK);
      auto_ptr<Directory> n=Node::cast<Directory>(p.parent->getchild(p.leaf));
      if(n->getchildren().size()>2)
	 return -ENOTEMPTY;
      p.parent->delchild(p.leaf);
      pcache.invalidate(path);
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}

int FS::symlink(const char* oldpath, const char* newpath){
   try{
      FsTxn txns(dbs);
      SContext sctx=userctx();
      auto_ptr<Symlink> sl=Symlink::create(txns,sctx,pcache,newpath);
      sl->setattr<time_t>("offlinefs.atime",time(NULL));
      sl->setattr<time_t>("offlinefs.mtime",time(NULL));
      sl->setattr<time_t>("offlinefs.ctime",time(NULL));
      sl->setattr<uid_t>("offlinefs.uid",fuse_get_context()->uid);
      sl->setattr<gid_t>("offlinefs.gid",fuse_get_context()->gid);
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
      FsTxn txns(dbs);
      SContext sctx=userctx();
      auto_ptr<Node> n=Node::getnode(txns,sctx,pcache,oldpath);
      Directory::Path p(txns,sctx,pcache,newpath);
      p.parent->access(sctx,W_OK);
      n->setattr<time_t>("offlinefs.ctime",time(NULL));
      p.parent->addchild(p.leaf,*n);
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}

int FS::chmod(const char* path, mode_t mode){
   try{
      FsTxn txns(dbs);
      SContext sctx=userctx();
      auto_ptr<Node> n=Node::getnode(txns,sctx,pcache,path);
      if(fuse_get_context()->uid!=0){
	 gid_t fgid=n->getattr<gid_t>("offlinefs.gid");
	 uid_t fuid=n->getattr<uid_t>("offlinefs.uid");
	 if(sctx.uid!=fuid)
	    return -EPERM;
	 if(!sctx.groups.count(fgid))
	    mode&=~S_ISGID;
      }
      n->setattr<time_t>("offlinefs.ctime",time(NULL));
      n->setattr<mode_t>("offlinefs.mode",mode&(~S_IFMT)|n->getattr<mode_t>("offlinefs.mode")&S_IFMT);
      pcache.invalidateAccess(path);
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}

int FS::chown(const char* path, uid_t owner, gid_t group){
   try{
      FsTxn txns(dbs);
      SContext sctx=userctx();
      auto_ptr<Node> n=Node::getnode(txns,sctx,pcache,path);
      if(owner!=(uid_t)-1 && owner!=n->getattr<uid_t>("offlinefs.uid")){
	 if(sctx.uid!=0)
	    return -EACCES;
	 n->setattr<uid_t>("offlinefs.uid",owner);
      }
      if(group!=(gid_t)-1 && group!=n->getattr<gid_t>("offlinefs.gid")){
	 if(sctx.uid!=0){
	    if(!sctx.groups.count(group))
	       return -EACCES;
	    else{
	       //Clear setuid/setgid bit
	       mode_t m=n->getattr<mode_t>("offlinefs.mode")&(~S_ISUID);
	       if(m&S_IXGRP)
		  m&=~S_ISGID;
	       n->setattr<mode_t>("offlinefs.mode",m);
	    }
	 }
	 n->setattr<gid_t>("offlinefs.gid",group);
      }
      n->setattr<time_t>("offlinefs.ctime",time(NULL));

      pcache.invalidateAccess(path);
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}

int FS::truncate(const char* path, off_t length){
   try{
      FsTxn txns(dbs);
      SContext sctx=userctx();
      auto_ptr<File> n=Node::cast<File>(Node::getnode(txns,sctx,pcache,path));
      n->access(sctx,W_OK);
      n->setattr<time_t>("offlinefs.mtime",time(NULL));
      n->setattr<time_t>("offlinefs.ctime",time(NULL));
      return n->getmedium(path)->truncate(*n,length);
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}

int FS::open(const char* path, struct fuse_file_info* fi){
   try{
      FsTxn txns(dbs);
      SContext sctx=userctx();
      auto_ptr<File> n=Node::cast<File>(Node::getnode(txns,sctx,pcache,path));
      if((fi->flags&O_ACCMODE)==O_RDONLY||(fi->flags&O_ACCMODE)==O_RDWR)
	 n->access(sctx,R_OK);
      if((fi->flags&O_ACCMODE)==O_WRONLY||(fi->flags&O_ACCMODE)==O_RDWR)
	 n->access(sctx,W_OK);
      auto_ptr<Source> s=n->getmedium(path)->getsource(*n,fi->flags);

      for(int i=0;i<MAX_OPEN_FILES;i++)
	 if(openFiles[i]==NULL){
	    if(pthread_mutex_lock(&openmutex))
	       throw std::runtime_error("FS::open: error locking mutex.");
	    if(openFiles[i]==NULL){
	       openFiles[i]=s.release();
	       fi->fh=i;
	       if(pthread_mutex_unlock(&openmutex))
		  throw std::runtime_error("FS::open: error unlocking mutex.");
	       return 0;
	    }
	    if(pthread_mutex_unlock(&openmutex))
	       throw std::runtime_error("FS::open: error unlocking mutex.");
	 }

      return -ENFILE;
   }catch(Node::EBadCast<File>& e){
      return -EISDIR;
   }catch(exception& e){
      return errcode(e);
   }
}

int FS::read(const char* path, char* buf, size_t nbyte, off_t offset, struct fuse_file_info* fi){
   if(fi->fh>=MAX_OPEN_FILES || !openFiles[fi->fh])
      return -EBADF;

   if(nbyte>0){
      FsTxn txns(dbs);
      File f(txns,openFiles[fi->fh]->getfileid());
      f.setattr<time_t>("offlinefs.atime",time(NULL));
   }

   return openFiles[fi->fh]->read(buf,nbyte,offset);
}

int FS::write(const char* path, const char* buf, size_t nbyte, off_t offset, struct fuse_file_info* fi){
   if(fi->fh>=MAX_OPEN_FILES || !openFiles[fi->fh])
      return -EBADF;
   if(nbyte>0){
      FsTxn txns(dbs);
      File f(txns,openFiles[fi->fh]->getfileid());
      f.setattr<time_t>("offlinefs.mtime",time(NULL));
   }
   return openFiles[fi->fh]->write(buf,nbyte,offset);
}

int FS::statfs(const char* path, struct statvfs* buf){
   try{
      FsTxn txns(dbs);
      Medium::Stats st=Medium::collectstats(txns);
      memset(buf,0,sizeof(struct statvfs));
      buf->f_bsize=4096;
      buf->f_frsize=4096;
      buf->f_blocks=st.blocks;
      buf->f_bfree=st.freeblocks;
      buf->f_bavail=st.freeblocks;
      buf->f_namemax=0;
      return 0;
   }catch(exception& e){
      return errcode(e);
   }
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
      FsTxn txns(dbs);
      SContext sctx=userctx();
      auto_ptr<Node> n=Node::getnode(txns,sctx,pcache,path);
      if(sctx.uid!=0 && sctx.uid!=getuid() && (string(name).find("offlinefs.")==0 || sctx.uid!=n->getattr<uid_t>("offlinefs.uid")) )
	 return -EACCES;
      n->setattr<time_t>("offlinefs.ctime",time(NULL));
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

      if(string(name)=="offlinefs.uid" || string(name)=="offlinefs.gid" || string(name)=="offlinefs.mode")
	 pcache.invalidateAccess(path);

   }catch(Node::EAttrNotFound& e){
      return -ENOATTR;
   }catch(exception& e){
      return errcode(e);
   }
   return 0;   
}

int FS::getxattr(const char* path, const char* name, char* value, size_t size){
   try{
      FsTxn txns(dbs);
      SContext sctx=userctx();
      auto_ptr<Node> n=Node::getnode(txns,sctx,pcache,path);
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
      FsTxn txns(dbs);
      SContext sctx=userctx();
      auto_ptr<Node> n=Node::getnode(txns,sctx,pcache,path);
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
      FsTxn txns(dbs);
      SContext sctx=userctx();
      auto_ptr<Node> n=Node::getnode(txns,sctx,pcache,path);
      if(sctx.uid!=0 && sctx.uid!=getuid() && (string(name).find("offlinefs.")==0 || sctx.uid!=n->getattr<uid_t>("offlinefs.uid")) )
	 return -EACCES;
      n->setattr<time_t>("offlinefs.ctime",time(NULL));
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
      FsTxn txns(dbs);
      SContext sctx=userctx();
      auto_ptr<Node> n=Node::getnode(txns,sctx,pcache,path);
      if(tv){
	 if(sctx.uid!=0 && sctx.uid!=getuid() && sctx.uid!=n->getattr<uid_t>("offlinefs.uid"))
	    return -EACCES;
	 n->setattr<time_t>("offlinefs.atime",tv[0].tv_sec);
	 n->setattr<time_t>("offlinefs.mtime",tv[1].tv_sec);
      }else{
	 n->access(sctx,W_OK);
	 n->setattr<time_t>("offlinefs.atime",time(NULL));
	 n->setattr<time_t>("offlinefs.mtime",time(NULL));	 
      }
   }catch(exception& e){
      return errcode(e);
   }
   return 0;   
}

int FS::access(const char* path, int mode){
   try{
      FsTxn txns(dbs);
      SContext sctx=userctx();
      auto_ptr<Node> n=Node::getnode(txns,sctx,pcache,path);
      n->access(sctx,mode);
   }catch(Node::ENotFound& e){
      return -EACCES;
   }catch(exception& e){
      return errcode(e);
   }
   return 0;
}

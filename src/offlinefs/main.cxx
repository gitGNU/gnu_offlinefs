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

#include "common.hxx"
#include "fs.hxx"
#include <fuse/fuse_opt.h>
#include <stddef.h>
#include <stdlib.h>
#include <grp.h>
#include <sstream>

struct Params{
      const char* dbroot;
      const char* dbgroup;
      const char* dbumask;
      bool help;
      bool rebuild;
} params={NULL,NULL,NULL,false,false};

std::string dbroot_default;

enum{
   KEY_VERSION,
   KEY_HELP,
   KEY_REBUILDDB,
};

struct fuse_opt opts[] ={
   {"dbroot=%s",offsetof(Params,dbroot),0},
   {"dbgroup=%s",offsetof(Params,dbgroup),0},
   {"dbumask=%s",offsetof(Params,dbumask),0},
   FUSE_OPT_KEY("-V", KEY_VERSION),
   FUSE_OPT_KEY("--version", KEY_VERSION),
   FUSE_OPT_KEY("-h", KEY_HELP),
   FUSE_OPT_KEY("--help", KEY_HELP),
   FUSE_OPT_KEY("--rebuilddb", KEY_REBUILDDB),
   {NULL,0,0}
};

void usage(){
   std::cerr << "usage: offlinefs [mountpoint] [options]\n";
   std::cerr << "\n";
   std::cerr << "offlinefs options:\n";
   std::cerr << "\t-h\t--help\t\tprint help\n";
   std::cerr << "\t-V\t--version\tprint version\n";
   std::cerr << "\t-o dbroot=<dbroot>\tDatabase root\n";
   std::cerr << "\t-o dbgroup=<dbroot>\tDatabase group\n";
   std::cerr << "\t-o dbumask=<dbroot>\tDatabase umask (octal)\n";
   std::cerr << "\t--rebuilddb\t\tRebuild DB mode\n";
   std::cerr << std::endl;
}

int opt_proc(void* data, const char* arg, int key, struct fuse_args* outargs){
   if(key==KEY_VERSION){
      std::cerr << "offlinefs version: " << VERSION << std::endl;
      ((Params*)data)->help=true;
      return 1;
   }else if(key==KEY_HELP){
      usage();
      fuse_opt_add_arg(outargs,"-ho");
      ((Params*)data)->help=true;
      return 0;
   }else if(key==KEY_REBUILDDB){
      ((Params*)data)->rebuild=true;
      return 0;
   }else
      return 1;
}

void* init_(fuse_conn_info* conn){
   Params* p=(Params*)fuse_get_context()->private_data;
   return new FS(p->dbroot);
}

void destroy_(void *userdata){
   delete (FS*)fuse_get_context()->private_data;
}

int getattr_(const char* path, struct stat* st){
  return ((FS*)fuse_get_context()->private_data)->getattr(path,st);
}

int readdir_(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, fuse_file_info* fi){
  return ((FS*)fuse_get_context()->private_data)->readdir(path,buf,filler,offset,fi);
}

int readlink_(const char* path, char* buf, size_t bufsiz){
   return ((FS*)fuse_get_context()->private_data)->readlink(path,buf,bufsiz);
}

int mknod_(const char* path , mode_t mode, dev_t dev){
   return ((FS*)fuse_get_context()->private_data)->mknod(path,mode,dev);
}

int mkdir_(const char* path, mode_t mode){
   return ((FS*)fuse_get_context()->private_data)->mkdir(path,mode);
}

int unlink_(const char* path){
   return ((FS*)fuse_get_context()->private_data)->unlink(path);
}

int rmdir_(const char* path){
   return ((FS*)fuse_get_context()->private_data)->rmdir(path);
}

int symlink_(const char* oldpath, const char* newpath){
   return ((FS*)fuse_get_context()->private_data)->symlink(oldpath,newpath);
}
 
int rename_(const char* oldpath, const char* newpath){
   return ((FS*)fuse_get_context()->private_data)->rename(oldpath,newpath);
}

int link_(const char* oldpath, const char* newpath){
   return ((FS*)fuse_get_context()->private_data)->link(oldpath,newpath);
}

int chmod_(const char* path, mode_t mode){
   return ((FS*)fuse_get_context()->private_data)->chmod(path,mode);
}

int chown_(const char* path, uid_t owner, gid_t group){
   return ((FS*)fuse_get_context()->private_data)->chown(path,owner,group);
}

int truncate_(const char* path, off_t length){
   return ((FS*)fuse_get_context()->private_data)->truncate(path,length);
}

int open_(const char* path, struct fuse_file_info* fi){
   return ((FS*)fuse_get_context()->private_data)->open(path,fi);
}

int read_(const char* path, char* buf, size_t nbyte, off_t offset, struct fuse_file_info* fi){
   return ((FS*)fuse_get_context()->private_data)->read(path,buf,nbyte,offset,fi);
}

int write_(const char* path, const char* buf, size_t nbyte, off_t offset, struct fuse_file_info* fi){
   return ((FS*)fuse_get_context()->private_data)->write(path,buf,nbyte,offset,fi);
}

int statfs_(const char* path, struct statvfs* buf){
   return ((FS*)fuse_get_context()->private_data)->statfs(path,buf);
}

int flush_(const char* path, struct fuse_file_info* fi){
   return ((FS*)fuse_get_context()->private_data)->flush(path,fi);
}

int release_(const char* path, struct fuse_file_info* fi){
   return ((FS*)fuse_get_context()->private_data)->release(path,fi);
}

int fsync_(const char* path, int datasync, struct fuse_file_info* fi){
   return ((FS*)fuse_get_context()->private_data)->fsync(path,datasync,fi);
}

int setxattr_(const char* path, const char* name, const char* value, size_t size, int flags){
   return ((FS*)fuse_get_context()->private_data)->setxattr(path,name,value,size,flags);
}

int getxattr_(const char* path, const char* name, char* value, size_t size){
   return ((FS*)fuse_get_context()->private_data)->getxattr(path,name,value,size);
}

int listxattr_(const char* path , char* list, size_t size){
   return ((FS*)fuse_get_context()->private_data)->listxattr(path,list,size);
}

int removexattr_(const char* path, const char* name){
   return ((FS*)fuse_get_context()->private_data)->removexattr(path,name);
}

int utimens_(const char* path, const struct timespec tv[2]){
   return ((FS*)fuse_get_context()->private_data)->utimens(path,tv);
}

int access_(const char* path, int mode){
   return ((FS*)fuse_get_context()->private_data)->access(path,mode);
}

int main(int argc, char** argv){
  fuse_operations ops;

  memset(&ops,0,sizeof(fuse_operations));
  ops.init=init_;
  ops.destroy=destroy_;
  ops.getattr=getattr_;
  ops.readdir=readdir_;
  ops.readlink=readlink_;
  ops.mknod=mknod_;
  ops.mkdir=mkdir_;
  ops.unlink=unlink_;
  ops.rmdir=rmdir_;
  ops.symlink=symlink_;
  ops.rename=rename_;
  ops.link=link_;
  ops.chmod=chmod_;
  ops.chown=chown_;
  ops.truncate=truncate_;
  ops.open=open_;
  ops.read=read_;
  ops.write=write_;
  ops.statfs=statfs_;
  ops.flush=flush_;
  ops.release=release_;
  ops.fsync=fsync_;
  ops.setxattr=setxattr_;
  ops.getxattr=getxattr_;
  ops.listxattr=listxattr_;
  ops.removexattr=removexattr_;
  ops.utimens=utimens_;
  ops.access=access_;

  struct fuse_args args = FUSE_ARGS_INIT(argc,argv);

  if(getenv("HOME")){
     dbroot_default=std::string(getenv("HOME"))+"/.offlinefs/";
     params.dbroot=dbroot_default.c_str();
  }


  if(fuse_opt_parse(&args,&params,opts,opt_proc)){
     std::cerr << "Error parsing command line" << std::endl;
     return -1;
  }
   if(params.dbgroup){
      struct group* gr=getgrnam(params.dbgroup);
      if(!gr){
	 std::cerr << "Specified dbgroup doesn't exist.\n";
	 return 1;
      }
      if(setegid(gr->gr_gid)){
	 std::cerr << "Error calling setegid().\n";
	 return 1;
      }
   }
   if(params.dbumask){
      std::istringstream is(params.dbumask);
      is.setf(std::ios::oct);
      mode_t dbumask;
      is >> dbumask;
      if(!is){
	 std::cerr << "Error parsing dbumask.\n";
	 return 1;
      }
      umask(dbumask);
   }


  if(!params.help){
     if(!params.dbroot){
	std::cerr << "No database root found!\n";
	usage();
	return -1;
     }
     if(params.dbroot[0]!='/'){
	std::cerr << "Error: expecting absolute path for dbroot.\n";
	return -1;
     }
     if(params.rebuild){
	std::cerr << "Rebuilding database at " << params.dbroot << " ...\n";
	FsDb(params.dbroot).rebuild();
	std::cerr << "Done.\n";
	return 0;
     }
  }

  int err= fuse_main(args.argc, args.argv,&ops,&params);

  fuse_opt_free_args(&args);

  return err;
}

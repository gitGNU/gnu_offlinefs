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

// Option processing stuff

struct Params{
      std::string dbroot;
      std::string dbgroup;
      std::string dbumask;
      std::string defmedium;
};

struct CmdParams{
      const char* dbroot;
      const char* dbgroup;
      const char* dbumask;
      const char* defmedium;
      bool help;
      bool rebuild;
};

static CmdParams cmdparams={NULL,NULL,NULL,NULL,false,false};

enum{
   KEY_VERSION,
   KEY_HELP,
   KEY_REBUILDDB,
};

struct fuse_opt opts[] ={
   {"dbroot=%s",offsetof(CmdParams,dbroot),0},
   {"dbgroup=%s",offsetof(CmdParams,dbgroup),0},
   {"dbumask=%s",offsetof(CmdParams,dbumask),0},
   {"defmedium=%s",offsetof(CmdParams,defmedium),0},
   FUSE_OPT_KEY("-V", KEY_VERSION),
   FUSE_OPT_KEY("--version", KEY_VERSION),
   FUSE_OPT_KEY("-h", KEY_HELP),
   FUSE_OPT_KEY("--help", KEY_HELP),
   FUSE_OPT_KEY("--rebuilddb", KEY_REBUILDDB),
   {NULL,0,0}
};

static void usage(){
   std::cerr << "usage: offlinefs [mountpoint] [options]\n";
   std::cerr << "\n";
   std::cerr << "offlinefs options:\n";
   std::cerr << "\t-h\t--help\t\tprint help\n";
   std::cerr << "\t-V\t--version\tprint version and license information\n";
   std::cerr << "\t-o dbroot=<dbroot>\tDatabase root\n";
   std::cerr << "\t-o dbgroup=<dbroot>\tDatabase group\n";
   std::cerr << "\t-o dbumask=<dbroot>\tDatabase umask (octal)\n";
   std::cerr << "\t-o defmedium=<medium id> Default medium a file is created in\n";
   std::cerr << "\t--rebuilddb\t\tRebuild DB mode\n";
   std::cerr << std::endl;
}

static void version(){
   std::cerr << "offlinefs version: " << VERSION << "\n"\
      "Copyright (C) 2008 Francisco Jerez\n"				\
      "This program is free software: you can redistribute it and/or modify\n" \
      "it under the terms of the GNU General Public License as published by\n" \
      "the Free Software Foundation, either version 3 of the License, or\n" \
      "(at your option) any later version.\n"				\
      "\n"								\
      "This program is distributed in the hope that it will be useful,\n" \
      "but WITHOUT ANY WARRANTY; without even the implied warranty of\n" \
      "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"	\
      "GNU General Public License for more details.\n"			\
      "\n"								\
      "You should have received a copy of the GNU General Public License\n" \
      "along with offlinefs.  If not, see <http://www.gnu.org/licenses/>.\n";
   std::cerr << std::endl;
      
}

static int opt_proc(void* data, const char* arg, int key, struct fuse_args* outargs){
   if(key==KEY_VERSION){
      version();      
      ((CmdParams*)data)->help=true;
      return 1;
   }else if(key==KEY_HELP){
      usage();
      fuse_opt_add_arg(outargs,"-ho");
      ((CmdParams*)data)->help=true;
      return 0;
   }else if(key==KEY_REBUILDDB){
      ((CmdParams*)data)->rebuild=true;
      return 0;
   }else
      return 1;
}

// Glue to relay the FS operations to a C++ object

static void* init_(fuse_conn_info* conn){
   Params* p=(Params*)fuse_get_context()->private_data;
   return new FS(p->dbroot,p->defmedium);
}

static void destroy_(void *userdata){
   delete (FS*)fuse_get_context()->private_data;
}

static int getattr_(const char* path, struct stat* st){
  return ((FS*)fuse_get_context()->private_data)->getattr(path,st);
}

static int readdir_(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, fuse_file_info* fi){
  return ((FS*)fuse_get_context()->private_data)->readdir(path,buf,filler,offset,fi);
}

static int readlink_(const char* path, char* buf, size_t bufsiz){
   return ((FS*)fuse_get_context()->private_data)->readlink(path,buf,bufsiz);
}

static int mknod_(const char* path , mode_t mode, dev_t dev){
   return ((FS*)fuse_get_context()->private_data)->mknod(path,mode,dev);
}

static int mkdir_(const char* path, mode_t mode){
   return ((FS*)fuse_get_context()->private_data)->mkdir(path,mode);
}

static int unlink_(const char* path){
   return ((FS*)fuse_get_context()->private_data)->unlink(path);
}

static int rmdir_(const char* path){
   return ((FS*)fuse_get_context()->private_data)->rmdir(path);
}

static int symlink_(const char* oldpath, const char* newpath){
   return ((FS*)fuse_get_context()->private_data)->symlink(oldpath,newpath);
}
 
static int rename_(const char* oldpath, const char* newpath){
   return ((FS*)fuse_get_context()->private_data)->rename(oldpath,newpath);
}

static int link_(const char* oldpath, const char* newpath){
   return ((FS*)fuse_get_context()->private_data)->link(oldpath,newpath);
}

static int chmod_(const char* path, mode_t mode){
   return ((FS*)fuse_get_context()->private_data)->chmod(path,mode);
}

static int chown_(const char* path, uid_t owner, gid_t group){
   return ((FS*)fuse_get_context()->private_data)->chown(path,owner,group);
}

static int truncate_(const char* path, off_t length){
   return ((FS*)fuse_get_context()->private_data)->truncate(path,length);
}

static int open_(const char* path, struct fuse_file_info* fi){
   return ((FS*)fuse_get_context()->private_data)->open(path,fi);
}

static int read_(const char* path, char* buf, size_t nbyte, off_t offset, struct fuse_file_info* fi){
   return ((FS*)fuse_get_context()->private_data)->read(path,buf,nbyte,offset,fi);
}

static int write_(const char* path, const char* buf, size_t nbyte, off_t offset, struct fuse_file_info* fi){
   return ((FS*)fuse_get_context()->private_data)->write(path,buf,nbyte,offset,fi);
}

static int statfs_(const char* path, struct statvfs* buf){
   return ((FS*)fuse_get_context()->private_data)->statfs(path,buf);
}

static int flush_(const char* path, struct fuse_file_info* fi){
   return ((FS*)fuse_get_context()->private_data)->flush(path,fi);
}

static int release_(const char* path, struct fuse_file_info* fi){
   return ((FS*)fuse_get_context()->private_data)->release(path,fi);
}

static int fsync_(const char* path, int datasync, struct fuse_file_info* fi){
   return ((FS*)fuse_get_context()->private_data)->fsync(path,datasync,fi);
}

static int setxattr_(const char* path, const char* name, const char* value, size_t size, int flags){
   return ((FS*)fuse_get_context()->private_data)->setxattr(path,name,value,size,flags);
}

static int getxattr_(const char* path, const char* name, char* value, size_t size){
   return ((FS*)fuse_get_context()->private_data)->getxattr(path,name,value,size);
}

static int listxattr_(const char* path , char* list, size_t size){
   return ((FS*)fuse_get_context()->private_data)->listxattr(path,list,size);
}

static int removexattr_(const char* path, const char* name){
   return ((FS*)fuse_get_context()->private_data)->removexattr(path,name);
}

static int utimens_(const char* path, const struct timespec tv[2]){
   return ((FS*)fuse_get_context()->private_data)->utimens(path,tv);
}

static int access_(const char* path, int mode){
   return ((FS*)fuse_get_context()->private_data)->access(path,mode);
}

static int opendir_(const char* path, struct fuse_file_info* fi){
   return ((FS*)fuse_get_context()->private_data)->opendir(path,fi);
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
   ops.opendir=opendir_;

   struct fuse_args args = FUSE_ARGS_INIT(argc,argv);
   
   Params params;

   // Parse the command line
   if(fuse_opt_parse(&args,&cmdparams,opts,opt_proc)){
      std::cerr << "Error parsing command line" << std::endl;
      return -1;
   }

   // Default parameters
   if(getenv("HOME")){
      params.dbroot=std::string(getenv("HOME"))+"/.offlinefs/";
   }

   // Parameters from command line
   if(cmdparams.dbroot)
      params.dbroot = cmdparams.dbroot;

   if(cmdparams.dbgroup)
      params.dbgroup = cmdparams.dbgroup;

   if(cmdparams.dbumask)
      params.dbumask = cmdparams.dbumask;

   if(cmdparams.defmedium)
      params.defmedium = cmdparams.defmedium;

   // Apply the selected parameters
   if(!params.dbgroup.empty()){
      struct group* gr=getgrnam(cmdparams.dbgroup);
      if(!gr){
	 std::cerr << "Specified dbgroup doesn't exist.\n";
	 return 1;
      }
      if(setegid(gr->gr_gid)){
	 std::cerr << "Error calling setegid().\n";
	 return 1;
      }
   }

   if(!params.dbumask.empty()){
      mode_t dbumask;
      std::istringstream is(params.dbumask);

      is.setf(std::ios::oct);
      is >> dbumask;
      if(!is){
	 std::cerr << "Error parsing dbumask.\n";
	 return 1;
      }

      umask(dbumask);
   }

   if(!cmdparams.help){
      if(params.dbroot.empty()){
	 std::cerr << "No database root found!\n";
	 usage();
	 return -1;
      }
      
      if(cmdparams.rebuild){
	 std::cerr << "Rebuilding database at " << params.dbroot << " ...\n";
	 FsDb(params.dbroot).rebuild();
	 std::cerr << "Done.\n";
	 return 0;
      }
   }

   // Main loop
   int err = fuse_main(args.argc, args.argv,&ops,&params);
   
   fuse_opt_free_args(&args);
   
   return err;
}

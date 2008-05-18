#include "insert.hxx"
#include <stdlib.h>

using std::auto_ptr;
using std::string;
using std::list;

std::auto_ptr<Medium_insert> Medium_insert::create(FsDb& dbs){
   auto_ptr<Medium_directory> r=Medium_directory::create(dbs);
   string mediumtype("insert");
   r->setattrv("mediumtype",Buffer(mediumtype.c_str(),mediumtype.size()));
   r->setattrv("label",Buffer(NULL,0));
   r->setattrv("checkcmd",Buffer(NULL,0));
   r->setattrv("insertscript",Buffer(NULL,0));
   return auto_ptr<Medium_insert>(new Medium_insert(dbs,r->getid()));
}

std::auto_ptr<Source> Medium_insert::getsource(File& f,int mode){
   insert();
   return Medium_directory::getsource(f,mode);
}

int Medium_insert::truncate(File& f,off_t length){
   insert();
   return Medium_directory::truncate(f,length);
}

Medium::Stats Medium_insert::getstats(){
   insert();
   return Medium_directory::getstats();
}

void Medium_insert::addfile(File& f,string phid){
   Medium_directory::addfile(f,phid);
}

void Medium_insert::delfile(File& f){
   Buffer b=getattrv("unlink_files");
   if(string(b.data,b.size)=="true")
      insert();
   Medium_directory::delfile(f);
}

bool Medium_insert::check(){
   Buffer b=getattrv("checkcmd");
   string command(b.data,b.size);
   int err=system(command.c_str());
   if(err==-1)
      throw std::runtime_error("Medium_insert::check: error calling system.");
   return (WEXITSTATUS(err)==0);
}

void Medium_insert::insert(){
   if(!check()){
      Buffer b=getattrv("label");
      string label(b.data,b.size);
      b=getattrv("directory");
      string directory(b.data,b.size);
      b=getattrv("insertscript");
      string insertscript(b.data,b.size);
      string insertcmd="sh "+insertscript+" \""+label+"\" \""+directory+"\"";
      int err=system(insertcmd.c_str());
      if(err==-1)
	 throw std::runtime_error("Medium_insert::insert: error calling system.");
      if(WEXITSTATUS(err))
	 throw std::runtime_error("Medium_insert::insert: error inserting the volume.");
      if(!check())
	 throw std::runtime_error("Medium_insert::insert: check failed.");
   }
}

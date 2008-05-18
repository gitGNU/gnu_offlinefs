#include <common.hxx>

#include <iostream>
#include <string>
#include <list>
#include <sstream>

#include <fsdb.hxx>
#include <medium.hxx>
#include <fsnodes.hxx>

using std::pair;
using std::string;
using std::list;
using std::cout;
using std::cerr;
using std::endl;
using std::istringstream;
using std::auto_ptr;

typedef list<pair<string,string> > attrlist;

int usage(std::string prog){
   cerr << "Usage:\n";
   cerr << prog << " --add <type> --<attr0> <value0> --<attr1> <value1> ... [dbroot]\n";
   cerr << prog << " --list [dbroot]\n";
   cerr << prog << " --rm <id> [dbroot]\n";
   cerr << prog << " --mod <id> --<attr0> <value0> --<attr1> <value1> ... [dbroot]\n";
   cerr << prog << " --addfile <id> <path> [refpath] [dbroot]\n";
   cerr << "\t--add: Add medium of type <type> while setting specified attributes to the specified values.\n";
   cerr << "\t--list: List existing media.\n";
   cerr << "\t--rm: Remove specified medium.\n";
   cerr << "\t--mod: Modify specified attributes on medium <id>.\n";
   cerr << "\t--addfile: Create file <path> (relative to mount point) by cloning the metadata from [refpath], and add it to medium <id>.\n";
   cerr << "[dbroot] defaults to ~/.offlinefs/\n";
   return 1;
}

int addm(string type,attrlist attrs, string dbroot){
   FsDb dbs(dbroot);
   dbs.open();
   auto_ptr<Medium> m=Medium::create(dbs,type);
   for(attrlist::iterator it=attrs.begin();it!=attrs.end();it++)
      m->setattrv(it->first,Buffer(it->second.c_str(),it->second.size()));
   return 0;
}

int listm(string dbroot){
   FsDb dbs(dbroot);
   dbs.open();
   list<uint32_t> l=dbs.media.listregisters();
   for(list<uint32_t>::iterator it=l.begin();it!=l.end();it++){
      cout << "#MEDIUM: "<< *it << "\n";
      Database<uint32_t>::Register r(dbs.media,*it);
      list<string> attrs=r.getattrs();
      for(list<string>::iterator it=attrs.begin();it!=attrs.end();it++){
	 Buffer b=r.getattrv(*it);
	 cout << "\t" << *it << "=" << string(b.data,b.size) << "\n";
      }
   }
   return 0;
}

int rmm(uint32_t id,string dbroot){
   FsDb dbs(dbroot);
   dbs.open();
   auto_ptr<Medium> m=Medium::getmedium(dbs,id);
   m->remove();
   return 0;
}

int modm(uint32_t id,attrlist attrs, string dbroot){
   FsDb dbs(dbroot);
   dbs.open();
   auto_ptr<Medium> m=Medium::getmedium(dbs,id);
   for(attrlist::iterator it=attrs.begin();it!=attrs.end();it++)
      m->setattrv(it->first,Buffer(it->second.c_str(),it->second.size()));
   return 0;
}

int addf(uint32_t id,string path,string refpath,string dbroot){
   FsDb dbs(dbroot);
   dbs.open();
   auto_ptr<File> f=File::create(dbs,"/"+path);
   auto_ptr<Medium> m=Medium::getmedium(dbs,id);
   Buffer b=m->getattrv("directory");
   string basedir(b.data,b.size);
   basedir=basedir.substr(0,basedir.find_last_not_of("/")+1);
   if(refpath.find(basedir)!=0){
      cerr << "Error: cannot recognize refpath as a subdirectory of the specified medium." << endl;
      return 1;
   }
   m->addfile(*f,refpath.substr(basedir.size()));
   if(!refpath.empty()){
      struct stat st;
      stat(refpath.c_str(),&st);
      f->setattr<time_t>("offlinefs.atime",st.st_atime);
      f->setattr<time_t>("offlinefs.mtime",st.st_mtime);
      f->setattr<time_t>("offlinefs.ctime",time(NULL));
      f->setattr<mode_t>("offlinefs.mode",st.st_mode&(~S_IFMT)|S_IFREG);
      f->setattr<uid_t>("offlinefs.uid",st.st_uid);
      f->setattr<uid_t>("offlinefs.gid",st.st_gid);
      f->setattr<off_t>("offlinefs.size",st.st_size);
   }
   return 0;
}

int main(int argc,char* argv[]){
   string dbroot;
   if(getenv("HOME"))
      dbroot=string(getenv("HOME"))+"/.offlinefs/";
//Parse the commandline
   try{
      if(argc<2)
	 return usage(argv[0]);
      if(string(argv[1])=="--add"){
	 if(argc<3){
	    cerr << "Too few arguments" << endl;
	    return usage(argv[0]);
	 }
	 string type(argv[2]);
	 attrlist attrs;
	 int i;
	 for(i=3;i<argc-1;i+=2){
	    if(string(argv[i],2)!="--"){
	       cerr << "Expecting \"--<attr>\", got \"" << argv[i] << "\"" << endl;
	       return usage(argv[0]);
	    }
	    attrs.push_back(pair<string,string>(string(argv[i],2,string::npos),string(argv[i+1])));
	 }
	 if(i<argc){
	    if(string(argv[i],2)=="--"){
	       cerr << "Expecting <value>, got \"" << argv[i] << "\"" << endl;
	       return 1;
	    }
	    dbroot=string(argv[argc-1]);
	 }else if(dbroot.empty()){
	    cerr << "No default dbroot found!";
	    return 1;
	 }
	    
	 return addm(type,attrs,dbroot);
      }else if(string(argv[1])=="--list"){
	 if(argc==3)
	    dbroot=string(argv[argc-1]);
	 else if(argc==2){
	    if(dbroot.empty()){
	       cerr << "No default dbroot found!";
	       return 1;
	    }
	 }else{
	    cerr << "Wrong number of arguments" << endl;
	    return usage(argv[0]);
	 }
	 return listm(dbroot);
      }else if(string(argv[1])=="--rm"){
	 if(argc==4){
	    dbroot=string(argv[argc-1]);
	 }else if(argc==3){
	    if(dbroot.empty()){
	       cerr << "No default dbroot found!";
	       return 1;
	    }
	 }else{
	    cerr << "Wrong number of arguments" << endl;
	    return usage(argv[0]);
	 }
	 istringstream is(argv[2]);
	 is.exceptions(std::ios::failbit);
	 uint32_t id;
	 is >> id;
	 return rmm(id,dbroot);
      }else if(string(argv[1])=="--mod"){
	 if(argc<3){
	    cerr << "Too few arguments" << endl;
	    return usage(argv[0]);
	 }
	 istringstream is(argv[2]);
	 is.exceptions(std::ios::failbit);
	 uint32_t id;
	 is >> id;
	 attrlist attrs;
	 int i;
	 for(i=3;i<argc-1;i+=2){
	    if(string(argv[i],2)!="--"){
	       cerr << "Expecting \"--<attr>\", got \"" << argv[i] << "\"" << endl;
	       return usage(argv[0]);
	    }
	    attrs.push_back(pair<string,string>(string(argv[i],2,string::npos),string(argv[i+1])));
	 }
	 if(i<argc){
	    if(string(argv[i],2)=="--"){
	       cerr << "Expecting <value>, got \"" << argv[i] << "\"" << endl;
	       return 1;
	    }
	    dbroot=string(argv[argc-1]);
	 }else if(dbroot.empty()){
	    cerr << "No default dbroot found!";
	    return 1;
	 }
	    
	 return modm(id,attrs,dbroot);
      }else if(string(argv[1])=="--addfile"){
 	 if(argc<4||argc>6){
	    cerr << "Wrong number of arguments" << endl;
	    return usage(argv[0]);
	 }
	 istringstream is(argv[2]);
	 is.exceptions(std::ios::failbit);
	 uint32_t id;
	 is >> id;
	 string path(argv[3]);
	 string refpath;
	 if(argc>4)
	    refpath=string(argv[4]);
	 if(argc>5)
	    dbroot=string(argv[5]);
	 return addf(id,path,refpath,dbroot);
      }else{
	 return usage(argv[0]);
      }
      return 0;
   }catch(std::ios::failure& e){
      cerr << "Error parsing commandline\n";
      return usage(argv[0]);
   }catch(std::exception& e){
      cerr << "offmedia: Exception: " << e.what() << endl;
      return 1;
   }
}

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

#include <common.hxx>

#include <iostream>
#include <string>
#include <list>
#include <sstream>
#include <iomanip>
#include <boost/program_options.hpp>
#include <signal.h>
#include <stdio.h>
#include <grp.h>
#include <pwd.h>
#include <fsdb.hxx>
#include <fsnodes.hxx>
#include <media.hxx>
#include "format.hxx"

using std::string;
using std::auto_ptr;
using std::list;
using std::runtime_error;
using std::istringstream;
using std::exception;
using std::tr1::unordered_map;
namespace po=boost::program_options;

void sigint_handler(int){
   fclose(stdin);
}

template<typename T>
T parse(string str){
   istringstream is(str);
   T val;
   is >> val;
   if(is.fail())
      throw runtime_error("Error parsing \""+str+"\"");
   return val;
}

struct FMap{
      string user;
      string numuser;
      string group;
      string numgroup;
      string link;
      string perms;
      string path;
      string path2;
      string type;
      string atime;
      string ctime;
      string mtime;
      string size;
      unordered_map<string,string> z;
      unordered_map<string,string> x8;
      unordered_map<string,string> x16;
      unordered_map<string,string> x32;
      unordered_map<string,string> x64;
};

int main(int argc, char** argv){
   po::options_description desc("Options");
   desc.add_options()
      ("help,h","Print help")
      ("version,V","Print version")
      ("create,c","Allow creating new files")
      ("modify,m","Allow modifying already existing files")
      ("medium,M",po::value<uint32_t>(),"Associate every created file with the medium <arg>, using the input path as phid.")
      ("dbroot,b",po::value<string>(),"Database root")
      ("format,f",po::value<string>(),"Input format string")
      ("prefix,p",po::value<string>(),"Path where the input tree will be reproduced");
   
   po::positional_options_description pdesc;
   po::variables_map vm;
   try{
      po::store(po::command_line_parser(argc,argv).options(desc).positional(pdesc).run(),vm);
   }catch(std::exception& e){
      std::cerr << "Error parsing command line: " << e.what() << std::endl;
      std::cerr << "Use --help for more information." << std::endl;
      return 1;
   }
   po::notify(vm);

   if(vm.count("help")){
      std::cerr << "This program accepts a directory listing (formatted as specified with -f) on standard input, reproducing it inside a offlinefs with the specified attributes.\n";
      std::cerr << desc;
      std::cerr << "\n"\
	 "Valid input formats:\n"\
	 "\t%u\t\tSymbolic/Numeric user id\n"\
	 "\t%U\t\tNumeric user id\n"\
	 "\t%g\t\tSymbolic/Numeric group id\n"\
	 "\t%G\t\tNumeric group id\n"\
	 "\t%l\t\tSymbolic link target\n"\
	 "\t%m\t\tNumeric permissions (octal)\n"\
	 "\t%p,%P\t\tPath\n"\
	 "\t%y\t\tNode type: (f)ile (d)ir sym(l)ink (c)har dev (b)lock dev (p)ipe (s)ock\n"\
	 "\t%A@\t\tAccess time\n"\
	 "\t%C@\t\tChange time\n"\
	 "\t%T@\t\tModification time\n"\
	 "\t%s\t\tFile size (bytes)\n"\
	 "\t%z{xattr name}\tSet the specified xattr to the matched text (as string)\n"\
	 "\t%x8{xattr name}\txattr, as unsigned (8 bit)\n"\
	 "\t%x16{xattr name} xattr, as unsigned (16 bit)\n"\
	 "\t%x32{xattr name} xattr, as unsigned (32 bit)\n"\
	 "\t%x64{xattr name} xattr, as unsigned (64 bit)\n";
      return 0;
   }
   if(vm.count("version")){
      std::cerr << "offlinefs version: " << VERSION << "\n"		\
	 "Copyright (C) 2008 Francisco Jerez\n"				\
	 "This program is free software: you can redistribute it and/or modify\n" \
	 "it under the terms of the GNU General Public License as published by\n" \
	 "the Free Software Foundation, either version 3 of the License, or\n" \
	 "(at your option) any later version.\n"			\
	 "\n"								\
	 "This program is distributed in the hope that it will be useful,\n" \
	 "but WITHOUT ANY WARRANTY; without even the implied warranty of\n" \
	 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n" \
	 "GNU General Public License for more details.\n"		\
	 "\n"								\
	 "You should have received a copy of the GNU General Public License\n" \
	 "along with offlinefs.  If not, see <http://www.gnu.org/licenses/>.\n";
      std::cerr << std::endl;
      return 0;
   }

   string dbroot;
   if(!vm.count("dbroot")){
      char* home=getenv("HOME");
      if(!home){
	 std::cerr << "Error: Database root required." << std::endl;
	 std::cerr << "Use --help for more information." << std::endl;
	 return 1;
      }
      dbroot=string(home)+"/.offlinefs/";
   }else
      dbroot=vm["dbroot"].as<string>();

   if(!vm.count("format")){
      std::cerr << "Error: Input format string required." << std::endl;
      std::cerr << "Use --help for more information." << std::endl;
      return 1;
   }
   string prefix("/");
   if(vm.count("prefix")){
      prefix=vm["prefix"].as<string>();
      if(prefix[0]!='/')
	 prefix="/"+prefix;
   }

   bool allow_modify=vm.count("modify");
   bool allow_create=vm.count("create");
   if(!(allow_modify || allow_create)){
      std::cerr << "Please specify -m or -c.\n";
      std::cerr << "Use --help for more information." << std::endl;
      return 1;
   }

   struct sigaction sact;
   memset(&sact,0,sizeof(struct sigaction));
   sact.sa_handler=sigint_handler;
   sigaction(SIGINT,&sact,NULL);

   FsDb dbs(dbroot);
   dbs.open();

   FsTxn mtxns(dbs);
   auto_ptr<Medium> medium;
   if(vm.count("medium"))
      medium=Medium::getmedium(mtxns,vm["medium"].as<uint32_t>());
   else
      mtxns.abort();

   SContext sctx(0,0);
   PathCache_hash pch;

   FParser<FMap> fpar;
   fpar.add(FSpec<FMap>("u",&FMap::user))
      .add(FSpec<FMap>("U",&FMap::numuser))
      .add(FSpec<FMap>("g",&FMap::group))
      .add(FSpec<FMap>("G",&FMap::numgroup))
      .add(FSpec<FMap>("l",&FMap::link))
      .add(FSpec<FMap>("m",&FMap::perms))
      .add(FSpec<FMap>("p",&FMap::path))
      .add(FSpec<FMap>("P",&FMap::path2))
      .add(FSpec<FMap>("y",&FMap::type))
      .add(FSpec<FMap>("A@",&FMap::atime))
      .add(FSpec<FMap>("C@",&FMap::ctime))
      .add(FSpec<FMap>("T@",&FMap::mtime))
      .add(FSpec<FMap>("s",&FMap::size))
      .add(FSpec<FMap>("z{",&FMap::z))
      .add(FSpec<FMap>("x8{",&FMap::x8))
      .add(FSpec<FMap>("x16{",&FMap::x16))
      .add(FSpec<FMap>("x32{",&FMap::x32))
      .add(FSpec<FMap>("x64{",&FMap::x64));
   Format<FMap> f;
   try{
      f=fpar.parse(vm["format"].as<string>()+"\n");
   }catch(exception& e){
      std::cerr << "Error parsing the format string:" << std::endl;
      std::cerr << e.what() << std::endl;
      return 1;
   }


   char buf[1024];

   while(std::cin.good()){
      string line;
      do{
	 std::cin.clear();
	 std::cin.getline(buf,1024);
	 line+=string(buf);
      }while(std::cin.fail()&&!std::cin.eof());

      if(line.empty())
	 continue;

      FsTxn txns(dbs);

      try{
	 FMap m;
	 f.match(m,line+"\n");

	 auto_ptr<Node> n;
	 mode_t nodetype=0;
	 bool creating=false;
	 string path;

	 if(!m.type.empty()){
	    if(m.type=="f"){
	       nodetype=S_IFREG;
	    }else if(m.type=="d"){
	       nodetype=S_IFDIR;
	    }else if(m.type=="l"){
	       nodetype=S_IFLNK;
	    }else if(m.type=="c"){
	       nodetype=S_IFCHR;
	    }else if(m.type=="b"){
	       nodetype=S_IFBLK;
	    }else if(m.type=="p"){
	       nodetype=S_IFIFO;
	    }else if(m.type=="s"){
	       nodetype=S_IFSOCK;
	    }else{
	       throw runtime_error("Error parsing input: unknown file type");
	    }
	 }

	 if(!(path=m.path).empty() || !(path=m.path2).empty()){
	    if(allow_modify){
	       try{
		  n=Node::getnode(txns,sctx,pch,prefix+path);
	       }catch(Node::ENotFound& e){}
	    }
	    if(!n.get() && allow_create){
	       if(!nodetype)
		  nodetype=S_IFREG;
	       switch(nodetype){
		  case S_IFREG:
		     n=File::create(txns,sctx,pch,prefix+path);
		     if(medium.get())
			medium->addfile(*(File*)n.get(),path);
 		     break;
		  case S_IFDIR:
		     n=Directory::create(txns,sctx,pch,prefix+path);
		     break;
		  case S_IFLNK:
		     n=Symlink::create(txns,sctx,pch,prefix+path);
		     break;
		  default:
		     n=Node::create(txns,sctx,pch,prefix+path);
	       }
	       creating=true;
	    }
	 }
	 if(!n.get())
	    throw runtime_error("Error parsing input: no valid objects specified");


	 if(!m.link.empty())
	    n->setattrv("offlinefs.symlink",Buffer(m.link.c_str(),m.link.size()));

	 if(!m.numgroup.empty()){
	    n->setattr<gid_t>("offlinefs.gid",parse<gid_t>(m.numgroup));
	 }else if(!m.group.empty()){
	    try{
	       n->setattr<gid_t>("offlinefs.gid",parse<gid_t>(m.group));
	    }catch(...){
	       struct group* gr=getgrnam(m.group.c_str());
	       if(!gr)
		  throw runtime_error("Group \""+m.group+"\" doesn't exist.");
	       n->setattr<gid_t>("offlinefs.gid",gr->gr_gid);
	    }
	 }else if(creating){
	    n->setattr<gid_t>("offlinefs.gid",getgid());
	 }

	 if(!m.numuser.empty()){
	    n->setattr<uid_t>("offlinefs.uid",parse<uid_t>(m.numuser));
	 }else if(!m.user.empty()){
	    try{
	       n->setattr<uid_t>("offlinefs.uid",parse<uid_t>(m.user));
	    }catch(...){
	       struct passwd* pw=getpwnam(m.user.c_str());
	       if(!pw)
		  throw runtime_error("User \""+m.user+"\" doesn't exist.");
	    }
	 }else if(creating){
	    n->setattr<uid_t>("offlinefs.uid",getuid());
	 }

	 if(!m.atime.empty())
	    n->setattr<time_t>("offlinefs.atime",parse<time_t>(m.atime));
	 else if(creating)
	    n->setattr<time_t>("offlinefs.atime",time(NULL));

	 if(!m.mtime.empty())
	    n->setattr<time_t>("offlinefs.mtime",parse<time_t>(m.mtime));
	 else if(creating)
	    n->setattr<time_t>("offlinefs.mtime",time(NULL));
	 
	 if(!m.ctime.empty())
	    n->setattr<time_t>("offlinefs.ctime",parse<time_t>(m.ctime));
	 else if(creating)
	    n->setattr<time_t>("offlinefs.ctime",time(NULL));
	 

	 if(!m.perms.empty()){
	    istringstream is(m.perms);
	    mode_t perms;
	    is >> std::setbase(8) >> perms;
	    if(is.fail())
	       throw runtime_error("Error parsing permissions");
	    if(!nodetype)
	       nodetype=n->getattr<mode_t>("offlinefs.mode")&S_IFMT;
	    n->setattr<mode_t>("offlinefs.mode",nodetype|((~S_IFMT)&perms));
	 }else if(creating){
	    mode_t m=umask(0);
	    umask(m);
	    if(nodetype==S_IFDIR)
	       m=(~m)&0777;
	    else if(nodetype==S_IFLNK)
	       m=0;
	    else
	       m=(~m)&0666;
	    n->setattr<mode_t>("offlinefs.mode",nodetype|m);
	 }else if(!creating && nodetype){
	    mode_t perms=n->getattr<mode_t>("offlinefs.mode")&(~S_IFMT);
	    n->setattr<mode_t>("offlinefs.mode",nodetype|perms);
	 }

	 if(!m.size.empty()){
	    if(!nodetype)
	       nodetype=n->getattr<mode_t>("offlinefs.mode")&S_IFMT;	       
	    if(nodetype==S_IFREG)
	       n->setattr<off_t>("offlinefs.size",parse<off_t>(m.size));
	 }

	 for(unordered_map<string,string>::iterator it=m.z.begin();it!=m.z.end();++it)
	    n->setattrv(it->first,Buffer(it->second.c_str(),it->second.size()));

	 for(unordered_map<string,string>::iterator it=m.x8.begin();it!=m.x8.end();++it)
	    n->setattr<uint8_t>(it->first,parse<uint8_t>(it->second));

	 for(unordered_map<string,string>::iterator it=m.x16.begin();it!=m.x16.end();++it)
	    n->setattr<uint16_t>(it->first,parse<uint16_t>(it->second));

	 for(unordered_map<string,string>::iterator it=m.x32.begin();it!=m.x32.end();++it)
	    n->setattr<uint32_t>(it->first,parse<uint32_t>(it->second));

	 for(unordered_map<string,string>::iterator it=m.x64.begin();it!=m.x64.end();++it)
	    n->setattr<uint64_t>(it->first,parse<uint64_t>(it->second));

      }catch(exception& e){
	 txns.abort();
	 std::cerr << "Near: " << line << std::endl;
	 std::cerr << e.what() << std::endl;
	 return 1;
      }
   }
   
   return 0;
}

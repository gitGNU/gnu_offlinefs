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

#include "directory.hxx"

using std::auto_ptr;
using std::string;
using std::pair;

Directory::EExists::EExists():runtime_error("Directory::EExists") {}

Directory::Directory(FsDb& dbs,uint64_t id):Node(dbs,id),rdir(dbs.directories,id) {}

void Directory::addchild(std::string name, Node& n){
   n.link();
   try{
      try{
	 rdir.getattrv(name);
      }catch(EAttrNotFound& e){
	 rdir.setattr(name,n.getid());
	 return;
      }
      throw EExists();
   }catch(...){
      n.unlink();
      throw;
   }
}

void Directory::delchild(std::string name){
   std::auto_ptr<Node> child=getchild(name);
   rdir.delattr(name);
   child->unlink();
}

std::auto_ptr<Node> Directory::getchild(std::string name){
   try{
      return Node::getnode(dbs,rdir.getattr<uint64_t>(name)); 
   }catch(Register::EAttrNotFound& e){
      throw ENotFound();
   }
}

void Directory::remove(){
   rdir.remove();
   Node::remove();
}

std::auto_ptr<Directory> Directory::create(FsDb& dbs){
   auto_ptr<Directory> n(new Directory(dbs,Node::create(dbs)->getid()));
   n->setattr<mode_t>("offlinefs.mode",S_IFDIR);
   n->addchild(".",*n);
   return n;
}

auto_ptr<Directory> Directory::create(FsDb& dbs,const SContext& sctx,string path){
   auto_ptr<Directory> n=Directory::create(dbs);
   try{
      Path p(dbs,sctx,path);
      p.parent->access(sctx,W_OK|X_OK);
      p.parent->addchild(p.leaf,*n);
      n->addchild("..",*p.parent);
   }catch(...){
      n->remove();
      throw;
   }
   return n;
}

Directory::Path::Path(FsDb& dbs,const SContext& sctx, string path){
   std::string parentpath;
   string::size_type slashpos=0;
   string::size_type notslash=path.find_last_not_of("/");
   if(notslash!=string::npos){
      slashpos=path.rfind("/",notslash);
      leaf=path.substr(slashpos+1,notslash-slashpos);
      parentpath=path.substr(0,slashpos);
   }
   parent=cast<Directory>(Node::getnode(dbs,sctx,parentpath));
}

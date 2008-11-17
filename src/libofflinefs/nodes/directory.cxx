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
#include "types.hxx"

using std::auto_ptr;
using std::string;
using std::pair;
namespace off = offlinefs;

Directory::EExists::EExists():runtime_error("The file already exists.") {}

Directory::Directory(FsTxn& txns,uint64_t id):Node(txns,id),rdir(txns.directories,id) {}

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
      return Node::getnode(txns,rdir.getattr<uint64_t>(name)); 
   }catch(Register::EAttrNotFound& e){
      throw ENotFound();
   }
}

void Directory::remove(){
   rdir.remove();
   Node::remove();
}

std::auto_ptr<Directory> Directory::create(FsTxn& txns){
   auto_ptr<Directory> n(new Directory(txns,Node::create(txns)->getid()));
   n->setattr<off::mode_t>("offlinefs.mode",S_IFDIR);
   n->addchild(".",*n);
   return n;
}

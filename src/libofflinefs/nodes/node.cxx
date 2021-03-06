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

#include "node.hxx"
#include "file.hxx"
#include "directory.hxx"
#include "symlink.hxx"
#include "types.hxx"

using std::auto_ptr;
using std::string;
namespace off = offlinefs;

Node::ENotFound::ENotFound():runtime_error("Node::ENotFound") {}

Node::EAccess::EAccess():runtime_error("Node::EAccess") {}


Node::Node(FsTxn& txns,uint64_t id):Register(txns.nodes,id),txns(txns) {}

Node::~Node() {}

auto_ptr<Node> Node::create(FsTxn& txns){
   auto_ptr<Node> n(new Node(txns,txns.dbs.nodes.createregister(txns.nodes)));
   n->setattr<off::dev_t>("offlinefs.dev",0);
   n->setattr<off::nlink_t>("offlinefs.nlink",0);
   n->setattr<off::off_t>("offlinefs.size",0);
   n->setattr<off::time_t>("offlinefs.atime",time(NULL));
   n->setattr<off::time_t>("offlinefs.mtime",time(NULL));
   n->setattr<off::time_t>("offlinefs.ctime",time(NULL));
   return n;
}

void Node::link(){
   setattr<off::nlink_t>("offlinefs.nlink",1+getattr<off::nlink_t>("offlinefs.nlink"));
}

void Node::unlink(){
   int nlink=getattr<off::nlink_t>("offlinefs.nlink")-1;
   if(nlink<=0)
      remove();
   else
      setattr<off::nlink_t>("offlinefs.nlink",nlink);
}


std::auto_ptr<Node> Node::getnode(FsTxn& txns, uint64_t id){
   Node n(txns,id);
   try{
      mode_t m=n.getattr<off::mode_t>("offlinefs.mode");
      if(S_ISREG(m))
	 return auto_ptr<Node>(new File(txns,id));
      else if(S_ISDIR(m))
	 return auto_ptr<Node>(new Directory(txns,id));
      else if(S_ISLNK(m))
	 return auto_ptr<Node>(new Symlink(txns,id));
      else
	 return auto_ptr<Node>(new Node(txns,id));
   }catch(EAttrNotFound& e){
      throw ENotFound();
   }
}

void Node::access(const SContext& sctx, int mode){
   if(sctx.uid !=0){
      mode_t fmode=getattr<off::mode_t>("offlinefs.mode");

      fmode = (sctx.uid == getattr<off::uid_t>("offlinefs.uid")? (fmode>>6) & 07 :
	       sctx.groups.count(getattr<off::gid_t>("offlinefs.gid"))? (fmode>>3) & 07 :
	       fmode & 07);

      if(mode & ~fmode)
	 throw EAccess();
   }
}

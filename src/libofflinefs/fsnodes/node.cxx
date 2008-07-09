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

using std::auto_ptr;
using std::string;

Node::ENotFound::ENotFound():runtime_error("Node::ENotFound") {}

Node::EAccess::EAccess():runtime_error("Node::EAccess") {}


Node::Node(FsTxn& txns,uint64_t id):Register(txns.nodes,id),txns(txns) {}

Node::~Node() {}

auto_ptr<Node> Node::create(FsTxn& txns){
   auto_ptr<Node> n(new Node(txns,txns.dbs.nodes.createregister(txns.nodes)));
   n->setattr<dev_t>("offlinefs.dev",0);
   n->setattr<nlink_t>("offlinefs.nlink",0);
   n->setattr<mode_t>("offlinefs.mode",0);
   n->setattr<uid_t>("offlinefs.uid",0);
   n->setattr<gid_t>("offlinefs.gid",0);
   n->setattr<off_t>("offlinefs.size",0);
   n->setattr<time_t>("offlinefs.atime",0);
   n->setattr<time_t>("offlinefs.mtime",0);
   n->setattr<time_t>("offlinefs.ctime",0);
   return n;
}

auto_ptr<Node> Node::create(FsTxn& txns,const SContext& sctx, PathCache& pch, string path){
   return Node::create_<Node>(txns,sctx,pch,path);
}
void Node::link(){
   setattr<nlink_t>("offlinefs.nlink",1+getattr<nlink_t>("offlinefs.nlink"));
}

void Node::unlink(){
   int nlink=getattr<nlink_t>("offlinefs.nlink")-1;
   if(nlink<=0)
      remove();
   else
      setattr<nlink_t>("offlinefs.nlink",nlink);
}


std::auto_ptr<Node> Node::getnode(FsTxn& txns, uint64_t id){
   Node n(txns,id);
   try{
      mode_t m=n.getattr<mode_t>("offlinefs.mode");
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
   if(sctx.uid==0)
      return;
   mode_t fmode=getattr<mode_t>("offlinefs.mode");

   int mode_ok=fmode&07;

   if(sctx.groups.count(getattr<gid_t>("offlinefs.gid")))
      mode_ok|=(fmode>>3)&07;

   if(sctx.uid==getattr<uid_t>("offlinefs.uid"))
      mode_ok|=(fmode>>6)&07;

   if(mode&~mode_ok)
      throw EAccess();
}

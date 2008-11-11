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

#include <pathcache.hxx>
#include <nodes.hxx>

using std::string;
using std::auto_ptr;
using std::list;
using std::pair;

std::auto_ptr<Node> PathCache_null::getnode(FsTxn& txns,const SContext& sctx, std::string path){
   auto_ptr<Node> n = Node::getnode(txns,0);
   string::size_type pos=0;
   string::size_type newpos=0;

   //Check search permission on every parent directory
   while((newpos=path.find("/",pos))!=string::npos){
      if(newpos!=pos){
	 n->access(sctx,X_OK);
	 n=Node::cast<Directory>(n)->getchild(path.substr(pos,newpos-pos));
      }	 
      pos=newpos+1;
   }

   string leaf=path.substr(pos);
   if(leaf.empty())
      return n;
   else{
      n->access(sctx,X_OK);
      return Node::cast<Directory>(n)->getchild(leaf);
   }
}

PathCache_hash::PathCache_hash(): nelems(0){
   if(pthread_mutex_init(&mutex,NULL))
      throw std::runtime_error("PathCache_hash::PathCache_hash: error initializing the mutex.");
}

PathCache_hash::~PathCache_hash(){
   pthread_mutex_destroy(&mutex);
}

void PathCache_hash::insert(Cache::iterator cit,uint64_t nodeid){
   if(nelems==PATHCACHE_MAX_ELEMS){
      Cache::iterator last=cache.find(queue.back().c_str());
      if(last->second.access)
	 delete last->second.access;
      cache.erase(last);
      queue.pop_back();
   }else
      nelems++;
   cit->second.nodeid=nodeid;
   queue.push_front(cit->first);
   cit->second.qit=queue.begin();
}

void PathCache_hash::promote(Cache::iterator cit){
   queue.erase(cit->second.qit);
   queue.push_front(cit->first);
   cit->second.qit=queue.begin();
}

PathCache_hash::Cache::iterator PathCache_hash::promote(FsTxn& txns,string path,Cache::iterator parent,string leaf){
   pair<Cache::iterator,bool> p=cache.insert(pair<string,CElem>(path,CElem()));
   if(p.second)
      try{
	 insert(p.first,Node::cast<Directory>(Node::getnode(txns,parent->second.nodeid))->getchild(leaf)->getid());
      }catch(...){
	 cache.erase(p.first);
	 throw;
      }
   else
      promote(p.first);
   return p.first;
}

PathCache_hash::Cache::iterator PathCache_hash::promoteRoot(FsTxn& txns){
   pair<Cache::iterator,bool> p=cache.insert(pair<string,CElem>("/",CElem()));
   if(p.second)
      insert(p.first,0);
   else
      promote(p.first);
   return p.first;
}

void PathCache_hash::checkaccess(FsTxn& txns,const SContext& sctx,PathCache_hash::CElem& ce){
   if(!ce.access){
      Node n(txns,ce.nodeid);
      ce.access=new Access(n.getattr<uid_t>("offlinefs.uid"),n.getattr<gid_t>("offlinefs.gid"),n.getattr<mode_t>("offlinefs.mode"));
   }
   if(!(sctx.uid==0 || ce.access->mode&S_IXOTH ||
	(ce.access->uid==sctx.uid && ce.access->mode&S_IXUSR) ||
	(sctx.groups.count(ce.access->gid) && ce.access->mode&S_IXGRP)))
      throw Node::EAccess();
}

std::auto_ptr<Node> PathCache_hash::getnode(FsTxn& txns, const SContext& sctx, std::string path){
   Cache::iterator cit;

   pthread_mutex_lock(&mutex);
   try{
      cit = promoteRoot(txns);
      string::size_type pos=0;
      string::size_type newpos=0;

      // Check every parent directory for search permission
      while((newpos=path.find("/",pos))!=string::npos){
	 if(newpos!=pos){
	    checkaccess(txns,sctx,cit->second);
	    cit=promote(txns,path.substr(0,newpos),cit,path.substr(pos,newpos-pos));
	 }	 
	 pos=newpos+1;
      }
      
      string leaf=path.substr(pos);
      if(!leaf.empty()){
	 checkaccess(txns,sctx,cit->second);
	 cit=promote(txns,path,cit,leaf);
      }

   }catch(...){
      pthread_mutex_unlock(&mutex);
      throw;
   }
   pthread_mutex_unlock(&mutex);

   return auto_ptr<Node>(Node::getnode(txns,cit->second.nodeid));
}

void PathCache_hash::invalidate(std::string path){
   pthread_mutex_lock(&mutex);
   Cache::iterator it=cache.find(path.c_str());
   if(it!=cache.end()){
      queue.erase(it->second.qit);
      if(it->second.access)
	 delete it->second.access;
      cache.erase(it);
   }
   pthread_mutex_unlock(&mutex);
}

void PathCache_hash::invalidateAccess(std::string path){
   pthread_mutex_lock(&mutex);
   Cache::iterator it=cache.find(path.c_str());
   if(it!=cache.end() && it->second.access){
	 delete it->second.access;
	 it->second.access=NULL;
   }
   pthread_mutex_unlock(&mutex);
}

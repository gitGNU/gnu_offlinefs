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

#include <boost/bind.hpp>
#include <pathcache.hxx>
#include <nodes.hxx>
#include <types.hxx>
#include <token.hxx>

using std::string;
using std::auto_ptr;
using std::list;
using std::pair;
using std::tr1::shared_ptr;
using boost::bind;
namespace off = offlinefs;

std::auto_ptr<Node> PathCache_null::getnode(FsTxn& txns,const SContext& sctx, const std::string& path){
   list<Token> toks;
   auto_ptr<Node> n = Node::getnode(txns,0);

   tokenize("/",toks,path);

   //Check search permission on every parent directory
   for(list<Token>::iterator it=toks.begin();
       it!=toks.end(); it++){
      n->access(sctx,X_OK);
      n=Node::cast<Directory>(n)->getchild(string(it->first,it->second));
   }

   return n;
}

PathCache_hash::PathCache_hash(): maxentries(PATHCACHE_MAX_ENTRIES), nentries(0){
   if(pthread_mutex_init(&mutex,NULL))
      throw std::runtime_error("PathCache_hash::PathCache_hash: error initializing the mutex.");

   paths.rehash(PATHCACHE_MAX_ENTRIES);
}

PathCache_hash::~PathCache_hash(){
   pthread_mutex_destroy(&mutex);
}

template<typename F, typename ItT>
void PathCache_hash::withPath(F f, FsTxn& txns,const ItT& tok0,
			      const ItT& tok1){
   ItT tok = tok1;
   CEntry* cen = lookup(txns,tok0,tok).get();

   while(true){
      f(cen);

      if(tok == tok0)
	 break;

      tok--;

      if(cen->data->parent->isValid()){
	 cen = cen->data->parent.get();

      }else{
	 shared_ptr<CEntry> parent=lookup(txns,tok0,tok);
	 cen->data->parent = parent;
	 cen = parent.get();
      }
   }
}

template<typename ItT>
shared_ptr<PathCache_hash::CEntry> PathCache_hash::lookup(FsTxn& txns,const ItT& tok0, const ItT& tok1){
   ItT tok = tok1;
   list<pair<PathMap::iterator, bool> > cens;
   shared_ptr<CEntry> cen;

   try{
      // Traverse up the branch until a cached ancestor is found
      while(true){
	 string path;
	 join("/",path, tok0,tok);

	 cens.push_back(paths.insert(PathMap::value_type(
					path, shared_ptr<CEntry>(new CEntry))));

	 if(!cens.back().second){
	    break;
	 }else if(tok == tok0){
	    initEntry(txns, cens.back().first->second, shared_ptr<CEntry>(),
		      tok0, tok0);
	    break;
	 }

	 tok--;
      }

      cen = cens.back().first->second;
      cens.pop_back();

      // Insert into the cache every missing intermediate ancestor
      while(tok != tok1){
	 shared_ptr<CEntry> parent = cen;
	 cen = cens.back().first->second;

	 tok++;

	 initEntry(txns, cen, parent, tok0, tok);

	 cens.pop_back();

      }
   }catch(...){
      // Clean up the inserted uninitialized cache entries
      while(!cens.empty()){
	 paths.erase(cens.back().first);
	 cens.pop_back();
      }

      throw;
   }

   return cen;
}

template<typename ItT>
void PathCache_hash::initEntry(FsTxn& txns, const shared_ptr<CEntry>& cen,
			       const shared_ptr<CEntry>& parent,
			       ItT tok0, ItT tok1){
   auto_ptr<Node> n;
   if(parent){
      join("/",cen->data->path, tok0, tok1);

      tok1--;

      auto_ptr<Directory> d = Node::cast<Directory>(
	 Node::getnode(txns,parent->data->nodeid));
      n = d->getchild(string(tok1->first, tok1->second));

   }else{
      n = Node::getnode(txns,0);
      cen->data->path = "";
   }

   cen->data->nodeid = n->getid();
   cen->data->parent = parent;
   cen->data->uid = n->getattr<off::uid_t>("offlinefs.uid");
   cen->data->gid = n->getattr<off::gid_t>("offlinefs.gid");
   cen->data->mode = n->getattr<off::mode_t>("offlinefs.mode");

   queue.push_back(cen);
   cen->data->queue_it = --queue.end();

   nentries++;
}

void PathCache_hash::delEntry(shared_ptr<CEntry> cen){
   queue.erase(cen->data->queue_it);
   paths.erase(cen->data->path);

   cen->invalidate();

   nentries--;
}

void PathCache_hash::checkAccess(const SContext& sctx, PathCache_hash::CEntry* cen){
   if(sctx.uid != 0 && (cen->data->uid == sctx.uid? ! (cen->data->mode & S_IXUSR) :
			sctx.groups.count(cen->data->gid)? ! (cen->data->mode & S_IXGRP) :
			! (cen->data->mode & S_IXOTH)))
      throw Node::EAccess();
}

std::auto_ptr<Node> PathCache_hash::getnode(FsTxn& txns, const SContext& sctx, const std::string& path){
   list<Token> toks;
   shared_ptr<CEntry> cen;

   tokenize("/",toks, path);

   pthread_mutex_lock(&mutex);
   try{
      if(toks.begin() != toks.end())
	 withPath(bind<void>(&PathCache_hash::checkAccess, this, sctx, _1),
		  txns, toks.begin(), --toks.end());

      cen = lookup(txns, toks.begin(), toks.end());

      // Promote the cache entry
      queue.erase(cen->data->queue_it);
      queue.push_back(cen);
      cen->data->queue_it= --queue.end();

      // If the cache is oversized, remove the least used entries
      while(nentries > maxentries)
	 delEntry(queue.front());

   }catch(...){
      pthread_mutex_unlock(&mutex);
      throw;
   }
   pthread_mutex_unlock(&mutex);

   return Node::getnode(txns,cen->data->nodeid);
}

void PathCache_hash::invalidate(FsTxn& txns, const std::string& path){
   pthread_mutex_lock(&mutex);

   try{
      // Tokenize and rejoin the path to ensure it is on the same form
      // that would have been inserted into the path map
      list<Token> toks;
      string normalized_path;

      tokenize("/",toks, path);
      join("/",normalized_path, toks.begin(), toks.end());

      PathMap::iterator cen = paths.find(normalized_path);
      if(cen != paths.end())
	 delEntry(cen->second);

   }catch(...){
      pthread_mutex_unlock(&mutex);
      throw;
   }

   pthread_mutex_unlock(&mutex);
}

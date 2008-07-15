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

#ifndef PATHCACHE_HXX
#define PATHCACHE_HXX

#include <common.hxx>
#include <scontext.hxx>
#include <fsdb.hxx>

#include <tr1/unordered_map>
#include <pthread.h>

#define PATHCACHE_MAX_ELEMS 2000

class Node;

class PathCache{
   public:
      virtual ~PathCache() {}
      // Instance a Node derived object, possibly using the cached data.
      // It can throw ENotFound, EBadCast<Directory> (if any of the specified parent nodes isn't a directory) 
      // and EAccess (if any of them doesn't have search permission for the caller).
      virtual std::auto_ptr<Node> getnode(FsTxn& txns,const SContext& sctx, std::string path)=0;
      //Remove all the cached data related to the specified file.
      virtual void invalidate(std::string path)=0;
      //Remove all the cached permission data related to the specified file.
      virtual void invalidateAccess(std::string path)=0;
};

// Implementation of PathCache that doesn't actually cache anything.
class PathCache_null:public PathCache{
   public:
      virtual ~PathCache_null() {}
      virtual std::auto_ptr<Node> getnode(FsTxn& txns,const SContext& sctx, std::string path);
      virtual void invalidate(std::string path) {}
      virtual void invalidateAccess(std::string path) {}
};

// Thread-safe implementation of PathCache by using a hash map.
class PathCache_hash:public PathCache{
      pthread_mutex_t mutex;

      class CElem;

      typedef std::tr1::unordered_map<std::string,CElem> Cache;
      typedef std::list<std::string> Queue;

      class Access{
	 public:
	    Access(uid_t uid,gid_t gid,mode_t mode):uid(uid),gid(gid),mode(mode) {}
	    uid_t uid;
	    gid_t gid;
	    mode_t mode;
      };

      class CElem{
	  public:
	    CElem():nodeid(0),access(NULL) {}
	    uint64_t nodeid;
	    Queue::iterator qit;
	    Access* access;
      };

      Cache cache;
      Queue queue;
      int nelems;

      // Initialize a new cache entry in the position specified by cit.
      // If the cache has reached its size limit, remove some files from the bottom of the queue.
      void insert(Cache::iterator cit,uint64_t nodeid);
      // Move the specified cache entry to the top of the queue.
      void promote(Cache::iterator cit);
      // Move the cache entry for the specified file to the top of the queue
      // If it doesn't have a cache entry, create it by using the parent's entry
      Cache::iterator promote(FsTxn& txns,std::string path,Cache::iterator parent,std::string leaf);
      Cache::iterator promoteRoot(FsTxn& txns);
      //Throw an exception if the specified cache element doesn't have search permission for the caller
      void checkaccess(FsTxn& txns,const SContext& sctx,CElem& ce);

   public:
      PathCache_hash();
      virtual ~PathCache_hash();

      virtual std::auto_ptr<Node> getnode(FsTxn& txns, const SContext& sctx, std::string path);
      virtual void invalidate(std::string path);
      virtual void invalidateAccess(std::string path);
};

#endif

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

#include <ext/hash_map>
#include <tr1/functional>
#include <pthread.h>

#define PATHCACHE_MAX_ELEMS 2000

class Node;

class PathCache{
   public:
      virtual std::auto_ptr<Node> getnode(FsTxn& txns,const SContext& sctx, std::string path)=0;
      virtual void invalidate(std::string path)=0;
      virtual void invalidateAccess(std::string path)=0;
};

class PathCache_null:public PathCache{
   public:
      virtual std::auto_ptr<Node> getnode(FsTxn& txns,const SContext& sctx, std::string path);
      virtual void invalidate(std::string path) {}
      virtual void invalidateAccess(std::string path) {}
};

class PathCache_hash:public PathCache{
      pthread_mutex_t mutex;

      class CElem;

      typedef __gnu_cxx::hash_map<std::string,CElem,std::tr1::hash<std::string> > Cache;
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

      void insert(Cache::iterator cit,uint64_t nodeid);
      void promote(Cache::iterator cit);
      Cache::iterator promote(FsTxn& txns,std::string path,Cache::iterator parent,std::string leaf);
      Cache::iterator promoteRoot(FsTxn& txns);
      void checkaccess(FsTxn& txns,const SContext& sctx,CElem& ce);

   public:
      PathCache_hash();
      ~PathCache_hash();

      virtual std::auto_ptr<Node> getnode(FsTxn& txns, const SContext& sctx, std::string path);
      virtual void invalidate(std::string path);
      virtual void invalidateAccess(std::string path);
};

#endif

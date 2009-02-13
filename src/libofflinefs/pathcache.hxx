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
#include <tr1/memory>
#include <pthread.h>

#define PATHCACHE_MAX_ENTRIES 4096

class Node;

class PathCache{
   public:
      virtual ~PathCache() {}

      // Instance a Node derived object, possibly using the cached data.
      // It can throw ENotFound, EBadCast<Directory> (if any of the specified parent nodes isn't a directory) 
      // and EAccess (if any of them doesn't have search permission for the caller).
      virtual std::auto_ptr<Node> getnode(FsTxn& txns,const SContext& sctx, const std::string& path)=0;

      //Remove all the cached data related to the specified file.
      virtual void invalidate(FsTxn& txns, const std::string& path)=0;
};

// Implementation of PathCache that doesn't actually cache anything.
class PathCache_null:public PathCache{
   public:
      virtual ~PathCache_null() {}
      virtual std::auto_ptr<Node> getnode(FsTxn& txns,const SContext& sctx, const std::string& path);
      virtual void invalidate(FsTxn& txns, const std::string& path) {}
};

// Thread-safe implementation of PathCache by using a hash map.
class PathCache_hash:public PathCache{
      pthread_mutex_t mutex;

      class CEntry;

      typedef std::tr1::unordered_map<std::string, std::tr1::shared_ptr<CEntry> > PathMap;
      typedef std::list<std::tr1::shared_ptr<CEntry> > Queue;

      class CEntry{
	 public:
	    class Data{
	       public:
		  std::string path;
		  uint64_t nodeid;
		  std::tr1::shared_ptr<CEntry> parent;

		  uid_t uid;
		  gid_t gid;
		  mode_t mode;

		  Queue::iterator queue_it;
	    } *data;

	    CEntry(){
	       data=new Data;
	    }

	    ~CEntry(){
	       invalidate();
	    }

	    void invalidate(){
	       if(data){
		  delete data;
		  data = NULL;
	       }
	    }

	    bool isValid(){
	       return data!=NULL;
	    }
      };

      PathMap paths;
      Queue queue;
      int maxentries;
      int nentries;

      // Run f for each cache entry corresponding to a token in the
      // path
      template<typename F, typename ItT>
      void withPath(F f, FsTxn& txns, const ItT& tok0, const ItT& tok1);

      // Return the cache entry corresponding to the specified
      // tokenized path
      template<typename ItT>
      std::tr1::shared_ptr<CEntry> lookup(FsTxn& txns, const ItT& tok0, const ItT& tok1);

      // Initialize the specified cache entry
      template<typename ItT>
      void initEntry(FsTxn& txns, const std::tr1::shared_ptr<CEntry>& cen,
		     const std::tr1::shared_ptr<CEntry>& parent,
		     ItT tok0, ItT tok1);

      // Remove the specified cache entry
      void delEntry(std::tr1::shared_ptr<CEntry> cen);

      // Throw an exception if the specified cache entry doesn't have
      // search permissions for the caller
      void checkAccess(const SContext& sctx, PathCache_hash::CEntry* cen);

   public:
      PathCache_hash();
      virtual ~PathCache_hash();

      virtual std::auto_ptr<Node> getnode(FsTxn& txns, const SContext& sctx, const std::string& path);
      virtual void invalidate(FsTxn& txns, const std::string& path);

};

#endif

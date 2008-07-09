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

#ifndef SCONTEXTCACHE_HXX
#define SCONTEXTCACHE_HXX

#include <common.hxx>
#include <scontext.hxx>
#include <ext/hash_map>
#include <pthread.h>


#define SCONTEXTCACHE_MAX_ELEMS 40

class SContextCache{
      pthread_mutex_t mutex;

      class CKey;
      class CKHash;
      class CElem;

      typedef __gnu_cxx::hash_map<CKey,CElem,CKHash> Cache;
      typedef std::list<CKey> Queue;

      class CKey{
	 public:
	    CKey(uid_t uid,gid_t gid):uid(uid),gid(gid) {}
	    bool operator==(const CKey& ck) const { return uid==ck.uid && gid==ck.gid; }
	    uid_t uid;
	    gid_t gid;
      };
      class CKHash{
	 public:
	    std::size_t operator()(const CKey& ck) const;
      };
      class CElem{
	  public:
	    CElem(uid_t uid,gid_t gid,Queue::iterator qit):sctx(uid,gid),qit(qit) {}
	    SContext sctx;
	    Queue::iterator qit;
      };



      Cache cache;
      Queue queue;
      int nelems;
      
      time_t checktime;
      void uptodate();
   public:
      SContextCache();
      ~SContextCache();
      SContext get(uid_t uid,gid_t gid);
      
};
#endif

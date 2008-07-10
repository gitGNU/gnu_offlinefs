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

#include "scontextcache.hxx"

SContextCache::SContextCache(): nelems(0),checktime(0){
   if(pthread_mutex_init(&mutex,NULL))
      throw std::runtime_error("SContextCache::SContextCache: error initializing the mutex.");
   uptodate();
}

SContextCache::~SContextCache(){
   pthread_mutex_destroy(&mutex);
}

void SContextCache::uptodate(){
   struct stat st;
   if(stat("/etc/passwd", &st) || st.st_mtime>checktime ||
      stat("/etc/group",&st) || st.st_mtime>checktime){
      checktime=time(NULL);
      nelems=0;
      cache.clear();
      queue.clear();
   }
}

SContext SContextCache::get(uid_t uid,gid_t gid){
   pthread_mutex_lock(&mutex);
   Cache::iterator it;
   try{
      uptodate();
      it=cache.find(CKey(uid,gid));
      if(it==cache.end()){
	 //Keep the cache size less than SCONTEXTCACHE_MAX_ELEMS
	 if(nelems==SCONTEXTCACHE_MAX_ELEMS){
	    cache.erase(queue.back());
	    queue.pop_back();
	 }else
	    nelems++;
	 queue.push_front(CKey(uid,gid));
	 try{
	    it=cache.insert(std::pair<CKey,CElem>(*queue.begin(),CElem(uid,gid,queue.begin()))).first;
	 }catch(...){
	    queue.pop_front();
	    nelems--;
	    throw;
	 }
      }else{
	 queue.erase(it->second.qit);
	 queue.push_front(it->first);
	 it->second.qit=queue.begin();
      }

   }catch(...){
      pthread_mutex_unlock(&mutex);
      throw;
   }
   pthread_mutex_unlock(&mutex);

   return it->second.sctx;
}

std::size_t SContextCache::CKHash::operator()(const CKey& ck) const{
   return 5*ck.uid+ck.gid;
}

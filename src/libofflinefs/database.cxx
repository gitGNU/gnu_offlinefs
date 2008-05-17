#include "database.hxx"
#include "fsnodes.hxx"

Environment::Environment(std::string path){
   struct stat st;
   if(stat(path.c_str(),&st) &&  errno==ENOENT)
      mkdir(path.c_str(),0777);

   dbenv=new DbEnv(0);
   dbenv->open(path.c_str(),DB_INIT_LOCK|DB_INIT_LOG|DB_INIT_MPOOL|DB_INIT_TXN|DB_CREATE|DB_RECOVER|DB_REGISTER|DB_THREAD,0);
   dbenv->set_lk_detect(DB_LOCK_RANDOM);
   dbenv->log_archive(NULL,DB_ARCH_REMOVE);
}

Environment::~Environment(){
   if(dbenv){
      dbenv->close(0);
      delete dbenv;
   }
}

void Buffer::clean(){
   if(data){
      delete[] data;
      data=NULL;
      size=0;
   }
}

void Buffer::copy(const char* data, size_t size){
   this->data=new char[size];
   this->size=size;
   memcpy(this->data,data,size);
}


template<>
uint32_t Database<uint32_t>::incrId(uint32_t id){
   return __cpu_to_be32(__be32_to_cpu(id)+1);
}

template<>
uint64_t Database<uint64_t>::incrId(uint64_t id){
   return __cpu_to_be64(__be64_to_cpu(id)+1);
}


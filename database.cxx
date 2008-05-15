#include "database.hxx"
#include "fsnodes.hxx"

Environment::Environment(std::string path){
   dbenv=new DbEnv(0);
   dbenv->open(path.c_str(),DB_INIT_LOG|DB_INIT_MPOOL|DB_INIT_TXN|DB_CREATE|DB_RECOVER|DB_REGISTER|DB_THREAD,0);
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



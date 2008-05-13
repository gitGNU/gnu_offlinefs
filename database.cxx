#include "database.hxx"
#include "fsnodes.hxx"
#include <iostream>

using std::auto_ptr;
using std::string;

void Database::close(){
   if(nodes){
      nodes->close(0);
      delete nodes;
   }
   if(directories){
      directories->close(0);
      delete directories;
   }
}

void Database::open(){
   nodes=new Db(dbenv,0);
   nodes->open(NULL,"offlinefs.db","nodes",DB_BTREE,DB_AUTO_COMMIT|DB_THREAD,0);
   directories=new Db(dbenv,0);
   directories->open(NULL,"offlinefs.db","directories",DB_BTREE,DB_AUTO_COMMIT|DB_THREAD,0);
}

void Database::rebuild(){
   nodes=new Db(dbenv,0);
   nodes->open(NULL,"offlinefs.db","nodes",DB_BTREE,DB_AUTO_COMMIT|DB_CREATE|DB_THREAD,0);
   nodes->truncate(NULL,NULL,0);
   directories=new Db(dbenv,0);
   directories->open(NULL,"offlinefs.db","directories",DB_BTREE,DB_AUTO_COMMIT|DB_CREATE|DB_THREAD,0);
   directories->truncate(NULL,NULL,0);

   auto_ptr<Directory> d(Directory::create(*this));
   d->addchild("..",d->getid());
}

Database::Database(string path):dbenv(NULL),nodes(NULL),directories(NULL){
   dbenv=new DbEnv(0);
   dbenv->open(path.c_str(),DB_INIT_LOG|DB_INIT_MPOOL|DB_INIT_TXN|DB_CREATE|DB_RECOVER|DB_REGISTER|DB_THREAD,0);
   try{
      try{
	 open();
      }catch(...){
	 close();
	 rebuild();
      }
   }catch(...){
      close();
      throw;
   }
}

Database::~Database(){
   close();
   if(dbenv){
      dbenv->close(0);
      delete dbenv;
   }
}

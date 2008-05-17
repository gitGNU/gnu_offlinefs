#ifndef DATABASE_HXX
#define DATABASE_HXX

#include "common.hxx"
#include <asm/byteorder.h>

class Environment{
   public:
      Environment(std::string path);
      virtual ~Environment();

      DbEnv* dbenv;

};

class Buffer{
      void clean();
      void copy(const char* data, size_t size);
   public:
      Buffer():data(NULL),size(0) {}
      Buffer(const char* data,size_t size) { copy(data,size); }
      Buffer(const Buffer& v){ copy(v.data,v.size); }
      ~Buffer() {clean();}
      Buffer& operator=(const Buffer& v) { clean(); copy(v.data,v.size); return *this;}
      char* data;
      size_t size;
};

template<typename T>
class Database{
      T incrId(T id);
      DbEnv* dbenv;
      std::string name;
      Db* db;
   public:
      class Register{
	 private:
	    Database<T>& db;
	    T id;
	    Buffer mkey(std::string attr);
	 public:
	    struct Key{
		  T id;
		  char text[];
	    };

	    class EAttrNotFound:public std::runtime_error{
	       public:
		  EAttrNotFound();
	    };
	    
	    Register(Database<T>& db, T id);
	    Register(const Register& r);
	    virtual ~Register();
	    
	    T getid() { return id; }
	    Database<T>& getdb();
	    virtual void remove();
	    
	    Buffer getattrv(std::string name);
	    void setattrv(std::string name,const Buffer& v);
	    
	    template<typename S> S getattr(std::string name);
	    template<typename S> void setattr(std::string name,S v);
	    void delattr(std::string name);
	    std::list<std::string> getattrs();    
      };

      Database(Environment& env,std::string name);
      void open();
      void close();
      void rebuild();
      ~Database();

      T createregister();
      std::list<T> listregisters();
};

template<typename T>
T Database<T>::createregister(){
   T id=(T)0;
   Dbc* cur;
   DbTxn* txn;
   Dbt key;
   Dbt v;
   int err;
   dbenv->txn_begin(NULL,&txn,0);
   try{
      db->cursor(txn,&cur,0);
      try{
	 err=cur->get(&key,&v,DB_LAST);
      
	 if(err!=DB_NOTFOUND){
	    if(err || key.get_size()<sizeof(typename Register::Key))
	       throw std::runtime_error("Database::createregister: Error reading from the database.");
	    id=incrId(((typename Register::Key*)key.get_data())->id);
	 }
	 key.set_data(&id);
	 key.set_size(sizeof(T));
	 v.set_data(NULL);
	 v.set_size(0);
	 cur->put(&key,&v,DB_KEYFIRST);
      }catch(...){
	 cur->close();
	 throw;
      }
      cur->close();
   }catch(...){
      txn->abort();
      throw;
   }
   txn->commit(0);
   return id;
}

template<typename T>
std::list<T> Database<T>::listregisters(){
   Dbc* cur;
   T lastid=(T)0;
   Dbt k;
   Dbt v;
   std::list<T> l;
   db->cursor(NULL,&cur,0);
   int err=cur->get(&k,&v,DB_FIRST);
   l.push_back(lastid);
   while(!err && k.get_size()>=sizeof(typename Register::Key)){
      T id=((typename Register::Key*)k.get_data())->id;
      if(id!=lastid){
	 l.push_back(id);
	 lastid=id;
      }
      err=cur->get(&k,&v,DB_NEXT);
   }
   cur->close();
   if(err && err!=DB_NOTFOUND)
      throw std::runtime_error("Database::listregisters: error accessing the database.");
   return l;
}

template<typename T>
Database<T>::Database(Environment& env, std::string name):dbenv(env.dbenv),name(name),db(NULL) {}

template<typename T>
Database<T>::~Database(){
   close();
}

template<typename T>
void Database<T>::open(){
      db=new Db(dbenv,0);
      db->open(NULL,"offlinefs.db",name.c_str(),DB_BTREE,DB_AUTO_COMMIT|DB_THREAD,0);
}

template<typename T>
void Database<T>::close(){
      if(db){
	 db->close(0);
	 delete db;
	 db=0;
      }
}

template<typename T>
void Database<T>::rebuild(){
   close();
   db=new Db(dbenv,0);
   db->open(NULL,"offlinefs.db",name.c_str(),DB_BTREE,DB_AUTO_COMMIT|DB_CREATE|DB_THREAD,0);
   db->truncate(NULL,NULL,0);
}

template<typename T>
T Database<T>::incrId(T id){
//Dirty hack to allow using arbitrary T...
   std::cerr << "!!!!!!" << std::endl;
   char* p=(char*)&id+sizeof(T);
   do{
      p--;
      (*p)++;
   }while(p!=(char*)&id && !*p);
   return id;
}

#include "register.hxx"

#endif

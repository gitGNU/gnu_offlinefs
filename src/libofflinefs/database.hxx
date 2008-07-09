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

#ifndef DATABASE_HXX
#define DATABASE_HXX

#include "common.hxx"
#include <asm/byteorder.h>

class Environment{
      unsigned int opcount;
   public:
      Environment(std::string path);
      virtual ~Environment();
      void cleanlogs();

      DbEnv* dbenv;

};

// Object automating memory management
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


//
// Class to abstract a table mapping an id of type T into a register
// Each register maps an attribute name (a string) into an arbitrary data type
//
template<typename T>
class Database{
      T incrId(T id);
      Environment& env;
      DbEnv* dbenv;
      std::string name;
      Db* db;
   public:

      class Txn{
	    DbTxn* txn;
	 public:
	    Txn(Database<T>& db);
	    ~Txn();
	    void commit();
	    void abort();
	    Dbc* cur;
	    Database<T>& db;
      };

      class Register{
	 private:
	    typename Database<T>::Txn& txn;
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
	    
	    Register(typename Database<T>::Txn& txn, T id);
	    Register(const Register& r);
	    virtual ~Register();
	    
	    T getid() { return id; }
	    Database<T>& getdb();
	    virtual void remove();
	    
	    // Get the specified attribute, throw EAttrNotFound if it doesn't exist
	    Buffer getattrv(std::string name);

	    // Set the specified attribute
	    void setattrv(std::string name,const Buffer& v);

	    //Get the specified attribute (as a type S)
	    //If the type's size doesn't match the stored one,
	    //throw std::runtime_exception
	    template<typename S> S getattr(std::string name);

	    //Set the specified attribute
	    template<typename S> void setattr(std::string name,S v);

	    //Delete the attribute "name", throw EAttrNotFound if it
	    // doesn't exist
	    void delattr(std::string name);

	    std::list<std::string> getattrs();    
      };

      Database(Environment& env,std::string name);
      void open();
      void close();
      //Erase the contents of the database
      void rebuild();
      ~Database();

      // Initialize a new register, and return its ID
      T createregister(Txn& txn);

      std::list<T> listregisters(Txn& txn);

};


template<typename T>
Database<T>::Txn::Txn(Database<T>& db):db(db){
   db.dbenv->txn_begin(NULL,&txn,0);
   try{
      db.db->cursor(txn,&cur,0);
   }catch(...){
      txn->abort();
      throw;
   }
}

template<typename T>
Database<T>::Txn::~Txn(){
   commit();
}   

template<typename T>
void Database<T>::Txn::commit(){
   if(cur){
      cur->close();
      cur=NULL;
   }
   if(txn){
      txn->commit(0);
      txn=NULL;
   }
}

template<typename T>
void Database<T>::Txn::abort(){
   if(cur){
      cur->close();
      cur=NULL;
   }
   if(txn){
      txn->abort();
      txn=NULL;
   }
}

template<typename T>
T Database<T>::createregister(Txn& txn){
   T id=(T)0;
   Dbt key;
   Dbt v;
   int err;

   err=txn.cur->get(&key,&v,DB_LAST|DB_RMW);
   if(err!=DB_NOTFOUND){
      if(err || key.get_size()<sizeof(typename Register::Key))
	 throw std::runtime_error("Database::createregister: Error reading from the database.");
      id=incrId(((typename Register::Key*)key.get_data())->id);
   }
   key.set_data(&id);
   key.set_size(sizeof(T));
   v.set_data(NULL);
   v.set_size(0);
   txn.cur->put(&key,&v,DB_KEYFIRST);

   env.cleanlogs();
   return id;
}

template<typename T>
std::list<T> Database<T>::listregisters(Txn& txn){
   T lastid=(T)-1;
   Dbt k;
   Dbt v;
   std::list<T> l;

   int err=txn.cur->get(&k,&v,DB_FIRST|DB_RMW);
   while(!err && k.get_size()>=sizeof(typename Register::Key)){
      T id=((typename Register::Key*)k.get_data())->id;
      if(id!=lastid){
	 l.push_back(id);
	 lastid=id;
      }
      err=txn.cur->get(&k,&v,DB_NEXT|DB_RMW);
   }

   if(err && err!=DB_NOTFOUND)
      throw std::runtime_error("Database::listregisters: error accessing the database.");
   return l;
}

template<typename T>
Database<T>::Database(Environment& env, std::string name):env(env),dbenv(env.dbenv),name(name),db(NULL) {}

template<typename T>
Database<T>::~Database(){
   close();
}

template<typename T>
void Database<T>::open(){
      db=new Db(dbenv,0);
      db->open(NULL,(name+".db").c_str(),NULL,DB_BTREE,DB_AUTO_COMMIT|DB_THREAD,0);
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
   db->open(NULL,(name+".db").c_str(),NULL,DB_BTREE,DB_AUTO_COMMIT|DB_CREATE|DB_THREAD,0);
   db->truncate(NULL,NULL,0);
}

template<typename T>
T Database<T>::incrId(T id){
//Dirty hack to allow using arbitrary T...
   char* p=(char*)&id+sizeof(T);
   do{
      p--;
      (*p)++;
   }while(p!=(char*)&id && !*p);
   return id;
}


#include "register.hxx"

#endif

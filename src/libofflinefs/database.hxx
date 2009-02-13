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
      char* data;
      size_t size;

      Buffer():data(NULL),size(0) {}
      Buffer(const char* data,size_t size) { copy(data,size); }
      Buffer(const Buffer& v){ copy(v.data,v.size); }
      Buffer(const std::string& s) { copy(s.c_str(),s.size()); }
      ~Buffer() {clean();}

      Buffer& operator=(const Buffer& v) { clean(); copy(v.data,v.size); return *this;}
      operator std::string() { return std::string(data,size); }
};


//
// Class to abstract a table mapping an id of type IdT into a register
// Each register maps an attribute name (a string) into an arbitrary data type
//
template<typename IdT>
class Database{
      Environment& env;
      DbEnv* dbenv;
      std::string name;
      Db* db;
   public:

      //Every database operation requires a transaction object:
      class Txn{
	    DbTxn* txn;
	 public:
	    Txn(Database<IdT>& db);
	    ~Txn();
	    void commit();
	    void abort();
	    Dbc* cur;
	    Database<IdT>& db;
      };

      class Register{
	    friend class Database<IdT>;
	 private:
	    typename Database<IdT>::Txn& txn;
	    IdT id;

	    struct Key{
		  IdT id;
		  char text[];
	    };

	    Buffer getkey(std::string attr);
	 public:
	    class EAttrNotFound:public std::runtime_error{
	       public:
		  EAttrNotFound(std::string attr);
	    };

	    Register(typename Database<IdT>::Txn& txn, IdT id);
	    Register(const Register& r);
	    virtual ~Register();

	    IdT getid() const { return id; }
	    Database<IdT>& getdb();

	    virtual void remove();

	    // Get the specified attribute, throw EAttrNotFound if it
	    // doesn't exist
	    Buffer getattrv(std::string name);

	    // Set the specified attribute
	    void setattrv(std::string name,const Buffer& v);

	    // Get the specified attribute (as a type AttrT) If the
	    // type's size doesn't match the stored one, throw
	    // std::runtime_exception
	    template<typename AttrT> AttrT getattr(std::string name);

	    // Set the specified attribute
	    template<typename AttrT> void setattr(std::string name,AttrT v);

	    // Delete the attribute "name", throw EAttrNotFound if it
	    // doesn't exist
	    void delattr(std::string name);

	    // Append the existing attribute names to the specified container
	    template<typename ConT> void getattrs(ConT& attrs);

	    // Return the number of existing attributes
	    int countattrs();
      };

      Database(Environment& env,std::string name);
      void open();
      void close();
      // Erase the contents of the database
      void rebuild();
      ~Database();

      // Initialize a new register, and return its ID
      IdT createregister(Txn& txn);

      // Append the existing register IDs to the specified container
      template<typename ConT> void getregisters(Txn& txn, ConT& regs);
};

// Endianness conversion functions
template<typename T> T host_to_db(T v);
template<typename T> T db_to_host(T v);

#include "register.hxx"

template<typename IdT>
Database<IdT>::Txn::Txn(Database<IdT>& db):db(db){
   db.dbenv->txn_begin(NULL,&txn,0);
   try{
      db.db->cursor(txn,&cur,0);
   }catch(...){
      txn->abort();
      throw;
   }
}

template<typename IdT>
Database<IdT>::Txn::~Txn(){
   commit();
}   

template<typename IdT>
void Database<IdT>::Txn::commit(){
   if(cur){
      cur->close();
      cur=NULL;
   }
   if(txn){
      txn->commit(0);
      txn=NULL;
   }
}

template<typename IdT>
void Database<IdT>::Txn::abort(){
   if(cur){
      cur->close();
      cur=NULL;
   }
   if(txn){
      txn->abort();
      txn=NULL;
   }
}

template<typename IdT>
IdT Database<IdT>::createregister(Txn& txn){
   IdT id=(IdT)0;
   Dbt key;
   Dbt v;
   int err;

   // Acquire the register creation lock
   uint32_t locker;
   DbLock lock;

   dbenv->lock_id(&locker);

   std::string lockname = name + "_createregister_lock";
   key.set_data((void*)lockname.c_str());
   key.set_size(lockname.size());

   dbenv->lock_get(locker,0,&key,DB_LOCK_WRITE,&lock);

   try{
      // Get the last register ID
      err=txn.cur->get(&key,&v,DB_LAST|DB_RMW);
      if(err!=DB_NOTFOUND){
	 if(err || key.get_size()<sizeof(typename Register::Key))
	    throw std::runtime_error("Database::createregister:"
				     " Error reading from the database.");

	 id=host_to_db<IdT>(
	    db_to_host<IdT>(
	       ((typename Register::Key*)key.get_data())->id)+1);
      }

      // Add a dummy attribute->value mapping to allocate the computed ID
      key.set_data(&id);
      key.set_size(sizeof(IdT));
      v.set_data(NULL);
      v.set_size(0);
      txn.cur->put(&key,&v,DB_KEYFIRST);
   }catch(...){
      // Release the register creation lock
      dbenv->lock_put(&lock);
      dbenv->lock_id_free(locker);
      throw;
   }
   // Release the register creation lock
   dbenv->lock_put(&lock);
   dbenv->lock_id_free(locker);

   env.cleanlogs();

   return id;
}

template<typename IdT>
template<typename ConT>
void Database<IdT>::getregisters(Txn& txn, ConT& regs){
   IdT lastid=(IdT)-1;
   Dbt k;
   Dbt v;

   int err=txn.cur->get(&k,&v,DB_FIRST|DB_RMW);
   while(!err && k.get_size()>=sizeof(typename Register::Key)){
      IdT id=((typename Register::Key*)k.get_data())->id;
      if(id!=lastid){
	 regs.push_back(id);
	 lastid=id;
      }
      err=txn.cur->get(&k,&v,DB_NEXT|DB_RMW);
   }

   if(err && err!=DB_NOTFOUND)
      throw std::runtime_error("Database::listregisters: error accessing the database.");
}

template<typename IdT>
Database<IdT>::Database(Environment& env, std::string name):env(env),dbenv(env.dbenv),name(name),db(NULL) {}

template<typename IdT>
Database<IdT>::~Database(){
   close();
}

template<typename IdT>
void Database<IdT>::open(){
      db=new Db(dbenv,0);
      db->open(NULL,(name+".db").c_str(),NULL,DB_BTREE,DB_AUTO_COMMIT|DB_THREAD,0);
}

template<typename IdT>
void Database<IdT>::close(){
   if(db){
      db->close(0);
      delete db;
      db=0;
   }
}

template<typename IdT>
void Database<IdT>::rebuild(){
   close();

   db=new Db(dbenv,0);
   try{
      db->remove((name+".db").c_str(),NULL,0);
   }catch(...) {}
   delete db;

   db=new Db(dbenv,0);
   db->open(NULL,(name+".db").c_str(),NULL,DB_BTREE,DB_AUTO_COMMIT|DB_CREATE|DB_THREAD,0);
}

#endif

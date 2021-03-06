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


template<typename IdT> Database<IdT>::Register::EAttrNotFound::EAttrNotFound(std::string attr):
   runtime_error("Attribute not found: " + attr) {}

template<typename IdT>
template<typename AttrT> AttrT Database<IdT>::Register::getattr(std::string name){
   Buffer b=getattrv(name);
   if(b.size!=sizeof(AttrT))
      throw std::runtime_error("Database::Register::getattr: Sizes do not match.");

   return db_to_host<AttrT>(*(AttrT*)b.data);
}

template<typename IdT>
template<typename AttrT> void Database<IdT>::Register::setattr(std::string name,AttrT v){
   v = host_to_db<AttrT>(v);
   Buffer b((char*)&v,sizeof(AttrT));
   setattrv(name,b);
}

template<typename IdT>
Database<IdT>::Register::Register(typename Database<IdT>::Txn& txn,IdT id):txn(txn),id(id) {}

template<typename IdT>
Database<IdT>::Register::Register(const Register& r):txn(r.txn),id(r.id) {}

template<typename IdT>
Database<IdT>::Register::~Register(){}


// Each berkeley DB key stores the concatenation of the ID and the attribute name
template<typename IdT>
Buffer Database<IdT>::Register::getkey(std::string name){
   Buffer b;
   b.size=sizeof(Key)+name.size();
   b.data=new char[b.size];
   Key* p=(Key*)b.data;
   p->id=id;
   memcpy(&p->text,name.c_str(),name.size());
   return b;
}

template<typename IdT>
Buffer Database<IdT>::Register::getattrv(std::string name){
   Buffer bk=getkey(name);
   Dbt key(bk.data,bk.size);
   Dbt v;
   v.set_flags(DB_DBT_MALLOC);


   if(txn.cur->get(&key,&v,DB_SET|DB_RMW))
      throw EAttrNotFound(name);

   bk=Buffer((char*)v.get_data(),v.get_size());

   if(v.get_data())
      free(v.get_data());
   return bk;
}

template<typename IdT>
void Database<IdT>::Register::setattrv(std::string name,const Buffer& bv){
   Buffer bk=getkey(name);
   Dbt key(bk.data,bk.size);
   Dbt v(bv.data,bv.size);

   if(txn.cur->put(&key,&v,DB_KEYFIRST))
      throw std::runtime_error("Database::Register::setattrv: Error writing into the database.");

   txn.db.env.cleanlogs();
}

template<typename IdT>
void Database<IdT>::Register::delattr(std::string name){
   Buffer bk=getkey(name);
   Dbt key(bk.data,bk.size);

   Dbt v;
   v.set_flags(DB_DBT_PARTIAL);
   v.set_dlen(0);
   if(txn.cur->get(&key,&v,DB_SET|DB_RMW))
      throw EAttrNotFound(name);
   txn.cur->del(0);

   txn.db.env.cleanlogs();
}
 
template<typename IdT>
template<typename ConT>
void Database<IdT>::Register::getattrs(ConT& attrs){
   Dbt key(&id,sizeof(IdT));
   Dbt v;

   int err=txn.cur->get(&key,&v,DB_SET_RANGE|DB_RMW);

   while(!err && key.get_size()>=sizeof(Key) && ((Key*)key.get_data())->id==id){
      if(key.get_size()>sizeof(Key))
	 attrs.push_back(std::string(((Key*)key.get_data())->text,key.get_size()-sizeof(IdT)));

      err=txn.cur->get(&key,&v,DB_NEXT|DB_RMW);
   }

   if(err && err!=DB_NOTFOUND)
      throw std::runtime_error("Database::Register::getattrs: Error accessing the database.");
}

template<typename IdT>
int Database<IdT>::Register::countattrs(){
   Dbt key(&id,sizeof(IdT));
   Dbt v;
   int count=0;

   // We aren't interested on the actual attribute values
   v.set_flags(DB_DBT_PARTIAL);
   v.set_dlen(0);

   int err=txn.cur->get(&key,&v,DB_SET_RANGE|DB_RMW);

   while(!err && key.get_size()>=sizeof(Key) && ((Key*)key.get_data())->id==id){
      if(key.get_size()>sizeof(Key))
	 count++;

      err=txn.cur->get(&key,&v,DB_NEXT|DB_RMW);
   }

   if(err && err!=DB_NOTFOUND)
      throw std::runtime_error("Database::Register::getattrs: Error accessing the database.");

   return count;
}

template<typename IdT>
void Database<IdT>::Register::remove(){
   std::list<std::string> l;
   getattrs(l);

   for(std::list<std::string>::iterator it=l.begin();it!=l.end();it++)
      delattr(*it);

   try{
      delattr("");
   }catch(EAttrNotFound& e) {}

   txn.db.env.cleanlogs();
}


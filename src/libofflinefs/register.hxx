

template<typename T> Database<T>::Register::EAttrNotFound::EAttrNotFound():runtime_error("Database::Register::EAttrNotFound") {}

template<typename T>
template<typename S> S Database<T>::Register::getattr(std::string name){
   Buffer b=getattrv(name);
   if(b.size!=sizeof(S))
      throw std::runtime_error("Database::Register::getattr: Sizes do not match.");
   return *(S*)b.data;
}

template<typename T>
template<typename S> void Database<T>::Register::setattr(std::string name,S v){
   Buffer b((char*)&v,sizeof(S));
   setattrv(name,b);
}

template<typename T>
Database<T>::Register::Register(Database<T>& db,T id):db(db),id(id) {}

template<typename T>
Database<T>::Register::Register(const Register& r):db(r.db),id(r.id) {}

template<typename T>
Database<T>::Register::~Register(){}


template<typename T>
Buffer Database<T>::Register::mkey(std::string name){
   Buffer b;
   b.size=sizeof(Key)+name.size();
   b.data=new char[b.size];
   Key* p=(Key*)b.data;
   p->id=id;
   memcpy(&p->text,name.c_str(),name.size());
   return b;
}

template<typename T>
Buffer Database<T>::Register::getattrv(std::string name){
   Buffer bk=mkey(name);
   Dbt key(bk.data,bk.size);
   Dbt v;
   v.set_flags(DB_DBT_MALLOC);

   if(db.db->get(NULL,&key,&v,0))
      throw EAttrNotFound();
   bk=Buffer((char*)v.get_data(),v.get_size());

   if(v.get_data())
      free(v.get_data());
   return bk;
}

template<typename T>
void Database<T>::Register::setattrv(std::string name,const Buffer& bv){
   Buffer bk=mkey(name);
   Dbt key(bk.data,bk.size);
   Dbt v(bv.data,bv.size);
   if(db.db->put(NULL,&key,&v,0))
      throw std::runtime_error("Database::Register::setattrv: Error writing into the database.");
}

template<typename T>
void Database<T>::Register::delattr(std::string name){
   Buffer bk=mkey(name);
   Dbt key(bk.data,bk.size);
   if(db.db->del(NULL,&key,0))
      throw EAttrNotFound();
}
 
template<typename T>
std::list<std::string> Database<T>::Register::getattrs(){
   std::list<std::string> l;
   Dbt key(&id,sizeof(T));
   Dbt v;
   Dbc* cur=NULL;
   db.db->cursor(NULL,&cur,0);
   try{
      int err=cur->get(&key,&v,DB_SET_RANGE);
      
      while(!err && key.get_size()>=sizeof(Key) && ((Key*)key.get_data())->id==id){      
	 if(key.get_size()>sizeof(Key))
	    l.push_back(std::string(((Key*)key.get_data())->text,key.get_size()-sizeof(T)));
	 err=cur->get(&key,&v,DB_NEXT);
      }
      if(err && err!=DB_NOTFOUND)
	 throw std::runtime_error("Database::Register::getattrs: Error accessing the database.");
      
   }catch(...){
      cur->close();
      cur=NULL;
      throw;
   }
   cur->close();
   cur=NULL;


   return l;
}

template<typename T>
void Database<T>::Register::remove(){
   std::list<std::string> l=getattrs();
   for(std::list<std::string>::iterator it=l.begin();it!=l.end();it++)
      delattr(*it);
   Dbt key(&id,sizeof(T));
   db.db->del(NULL,&key,0);
}


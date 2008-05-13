#include "fsnodes.hxx"
#include <string.h>
#include <sys/stat.h>
#include <iostream>

using std::string;
using std::runtime_error;
using std::auto_ptr;
using std::list;

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

Node::EAttrNotFound::EAttrNotFound():runtime_error("Node::EAttrNotFound") {
}

Node::ENotFound::ENotFound():runtime_error("Node::ENotFound") {
}

Node::EExists::EExists():runtime_error("Node::EExists") {
}

Node::ENotDir::ENotDir():runtime_error("Node::NotDir") {
}


Node::Node(Database& db, uint64_t id):id(id),db(db),cur(NULL) {
   db.nodes->cursor(NULL,&cur,0);
}

Node::~Node(){
   try{
      if(cur){
	 cur->close();
      }
   }catch(...){}
}

struct Key{
      uint64_t id;
      char text[];
};

Buffer Node::mkey(std::string name){
   Buffer b;
   b.size=sizeof(Key)+name.size();
   b.data=new char[b.size];
   Key* p=(Key*)b.data;
   p->id=id;
   memcpy(&p->text,name.c_str(),name.size());
   return b;
}

Buffer Node::getattrv(std::string name){
   Buffer bk=mkey(name);
   Dbt key(bk.data,bk.size);
   Dbt v;
   v.set_flags(DB_DBT_MALLOC);
   key.set_flags(DB_DBT_MALLOC);

   if(cur->get(&key,&v,DB_SET))
      throw EAttrNotFound();
   return Buffer((char*)v.get_data(),v.get_size());
}

void Node::setattrv(std::string name,const Buffer& bv){
   Buffer bk=mkey(name);
   Dbt key(bk.data,bk.size);
   Dbt v(bv.data,bv.size);
   if(db.nodes->put(NULL,&key,&v,0))
      throw runtime_error("Node::setattrv: Error writing into the database.");
}

void Node::delattr(std::string name){
   Buffer bk=mkey(name);
   Dbt key(bk.data,bk.size);
   if(db.nodes->del(NULL,&key,0))
      throw EAttrNotFound();
}
 
std::list<std::string> Node::getattrs(){
   list<string> l;
   Dbt key(&id,sizeof(uint64_t));
   Dbt v;
   int err=cur->get(&key,&v,DB_SET_RANGE);

   while(!err && key.get_size()>=sizeof(Key) && ((Key*)key.get_data())->id==id){      
      l.push_back(string(((Key*)key.get_data())->text,key.get_size()-sizeof(uint64_t)));
      err=cur->get(&key,&v,DB_NEXT);
   }

   if(err && err!=DB_NOTFOUND)
      throw runtime_error("Node::getattrs: Error accessing the database.");

   return l;
}


auto_ptr<Node> Node::create(Database& db){
   auto_ptr<Node> n(new Node(db,0));
   Dbt key;
   Dbt v;
   int err=n->cur->get(&key,&v,DB_LAST);

   if(err!=DB_NOTFOUND){
      if(err || key.get_size()<sizeof(Key))
 	 throw runtime_error("Node::create: Error reading from the database.");
      n->id=((Key*)key.get_data())->id+1;   
   }
   n->setattr<dev_t>("offlinefs.dev",0);
   n->setattr<nlink_t>("offlinefs.nlink",0);
   n->setattr<mode_t>("offlinefs.mode",0);
   n->setattr<uid_t>("offlinefs.uid",0);
   n->setattr<gid_t>("offlinefs.gid",0);
   n->setattr<off_t>("offlinefs.size",0);
   n->setattr<time_t>("offlinefs.atime",0);
   n->setattr<time_t>("offlinefs.mtime",0);
   n->setattr<time_t>("offlinefs.ctime",0);
   return n;
}

std::auto_ptr<Node> Node::create(Database& db,string path){
   Path p(db,path);
   if(p.nchild.get())
      throw EExists();
   auto_ptr<Node> n=Node::create(db);
   p.nparent->addchild(p.child,n->getid());
   return n;
}

void Node::remove(){
   list<string> l=getattrs();
   for(list<string>::iterator it=l.begin();it!=l.end();it++)
      delattr(*it);
}


void Node::link(){
   setattr<nlink_t>("offlinefs.nlink",1+getattr<nlink_t>("offlinefs.nlink"));
}

void Node::unlink(){
   int nlink=getattr<nlink_t>("offlinefs.nlink")-1;
   if(nlink<=0)
      remove();
   else
      setattr<nlink_t>("offlinefs.nlink",nlink);
}


std::auto_ptr<Node> Node::getnode(Database& db, uint64_t id){
   Node n(db,id);
   mode_t m=n.getattr<mode_t>("offlinefs.mode");
   if(S_ISREG(m))
      return auto_ptr<Node>(new File(db,id));
   else if(S_ISDIR(m))
      return auto_ptr<Node>(new Directory(db,id));
   else if(S_ISLNK(m))
      return auto_ptr<Node>(new Symlink(db,id));
   else
      return auto_ptr<Node>(new Node(db,id));
}

std::auto_ptr<Node> Node::getnode(Database& db, std::string path){
   auto_ptr<Directory> d(new Directory(db,0));
   string::size_type pos=0;
   string::size_type newpos=0;
   while((newpos=path.find("/",pos))!=string::npos){
      if(newpos!=pos)
	 d.reset(new Directory(db,d->getchild(path.substr(pos,newpos-pos))));
      pos=newpos+1;
   }
   string name=path.substr(pos,newpos);
   if(name.empty())
      return auto_ptr<Node>(d);
   else
      return getnode(db,d->getchild(path.substr(pos,newpos)));
}


Directory::Directory(Database& db, uint64_t id):Node(db,id),dcur(NULL) {
   db.directories->cursor(NULL,&dcur,0);
}

Directory::~Directory(){
   try{
      if(dcur){
	 dcur->close();
      }
   }catch(...){}
   this->~Node();
}


void Directory::addchild(std::string name, uint64_t id){
   Buffer bk=mkey(name);
   Dbt key(bk.data,bk.size);
   Dbt v(&id,sizeof(uint64_t));
   Node n(db,id);
   n.link();
   try{
      if(db.directories->put(NULL,&key,&v,0))
	 throw runtime_error("Directory::addchild: Error writing into the database.");   
   }catch(...){
      n.unlink();
      throw;
   }
}

void Directory::delchild(std::string name){
   Node n(db,getchild(name));
   Buffer bk=mkey(name);
   Dbt key(bk.data,bk.size);
   if(db.directories->del(NULL,&key,0))
      throw ENotFound();
  
   n.unlink();
}

uint64_t Directory::getchild(std::string name){
   Buffer bk=mkey(name);
   Dbt key(bk.data,bk.size);
   Dbt v;
   if(dcur->get(&key,&v,DB_SET) || v.get_size()<sizeof(uint64_t))
      throw ENotFound();
   return *(uint64_t*)v.get_data();
}

Directory::dirlist Directory::getchildren(){
   dirlist l;
   Dbt key(&id,sizeof(uint64_t));
   Dbt v;
   int err=dcur->get(&key,&v,DB_SET_RANGE);

   while(!err && key.get_size()>=sizeof(Key) && v.get_size()==sizeof(uint64_t) && ((Key*)key.get_data())->id==id){
      l.push_back(std::pair<string,uint64_t>(string(((Key*)key.get_data())->text,key.get_size()-sizeof(uint64_t)),*(uint64_t*)v.get_data()));
      err=dcur->get(&key,&v,DB_NEXT);
   }

   if(err && err!=DB_NOTFOUND)
      throw runtime_error("Directory::getchildren: Error accessing the database.");
   return l;
}

void Directory::remove(){
   Dbt key(&id,sizeof(uint64_t));
   Dbt v;
   int err=dcur->get(&key,&v,DB_SET_RANGE);

   while(!err && key.get_size()>=sizeof(Key) && ((Key*)key.get_data())->id==id){
      if(!dcur->del(0))
	 throw runtime_error("Directory::remove: Error accessing the database");
      err=dcur->get(&key,&v,DB_NEXT);
   }

   if(err && err!=DB_NOTFOUND)
      throw runtime_error("Directory::remove: Error accessing the database.");
   
   Node::remove();
}

auto_ptr<Directory> Directory::create(Database& db){
   auto_ptr<Directory> n(new Directory(db,Node::create(db)->getid()));
   n->setattr<mode_t>("offlinefs.mode",S_IFDIR);
   n->addchild(".",n->getid());
   return n;
}

auto_ptr<Directory> Directory::create(Database& db,string path){
   Path p(db,path);
   if(p.nchild.get())
      throw EExists();
   auto_ptr<Directory> d=Directory::create(db);
   p.nparent->addchild(p.child,d->getid());
   d->addchild("..",p.nparent->getid());
   return d;
}


File::File(Database& db,uint64_t id):Node(db,id) {
}


auto_ptr<File> File::create(Database& db){
   auto_ptr<Node> n(Node::create(db));
   n->setattr<mode_t>("offlinefs.mode",S_IFREG);
   return auto_ptr<File>(new File(db,n->getid()));
}

auto_ptr<File> File::create(Database& db,string path){
   auto_ptr<Node> n(Node::create(db,path));
   n->setattr<mode_t>("offlinefs.mode",S_IFREG);
   return auto_ptr<File>(new File(db,n->getid()));
}


Symlink::Symlink(Database& db,uint64_t id):Node(db,id) {
}


auto_ptr<Symlink> Symlink::create(Database& db){
   auto_ptr<Node> n(Node::create(db));
   n->setattr<mode_t>("offlinefs.mode",S_IFLNK);
   n->setattrv("offlinefs.symlink",Buffer());
   return auto_ptr<Symlink>(new Symlink(db,n->getid()));
}

auto_ptr<Symlink> Symlink::create(Database& db,string path){
   auto_ptr<Node> n(Node::create(db,path));
   n->setattr<mode_t>("offlinefs.mode",S_IFLNK);
   n->setattrv("offlinefs.symlink",Buffer());
   return auto_ptr<Symlink>(new Symlink(db,n->getid()));
}

Path::Path(Database& db,string path){
   int slashpos=path.rfind("/");
   parent=path.substr(0,slashpos);
   child=path.substr(slashpos+1);
   nparent=auto_ptr<Directory>(dynamic_cast<Directory*>(Node::getnode(db,parent).release()));
   if(!nparent.get())
      throw Node::ENotDir();
   try{
      nchild=auto_ptr<Node>(Node::getnode(db,nparent->getchild(child)));
   }catch(...){}
}

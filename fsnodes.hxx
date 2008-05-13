#ifndef FSNODES_HXX
#define FSNODES_HXX

#include <string>
#include <list>
#include <stdexcept>
#include "database.hxx"

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

class Node{
   protected:
      Buffer mkey(std::string attr);
      uint64_t id;
      Database& db;
      Dbc* cur;
   public:
      class EAttrNotFound:public std::runtime_error{
	 public:
	    EAttrNotFound();
      };
      class ENotFound:public std::runtime_error{
	 public:
	    ENotFound();
      };
      class EExists:public std::runtime_error{
	 public:
	    EExists();
      };
      class ENotDir:public std::runtime_error{
	 public:
	    ENotDir();
      };

      Node(Database& db, uint64_t id);
      virtual ~Node();

      uint64_t getid() {return id;}

      Buffer getattrv(std::string name);
      void setattrv(std::string name,const Buffer& v);

      template<typename T> T getattr(std::string name);
      template<typename T> void setattr(std::string name,T v);
      void delattr(std::string name);
      std::list<std::string> getattrs();

      static std::auto_ptr<Node> create(Database& db);
      static std::auto_ptr<Node> create(Database& db,std::string path);
      virtual void remove();

      static std::auto_ptr<Node> getnode(Database& db, uint64_t id);
      static std::auto_ptr<Node> getnode(Database& db, std::string path);

      void link();
      void unlink();
};

class Directory:public Node{
      Dbc* dcur;
   public:
      typedef std::list< std::pair< std::string, uint64_t > > dirlist;

      Directory(Database& db, uint64_t id);
      virtual ~Directory();

      void addchild(std::string name, uint64_t id);
      void delchild(std::string name);
      uint64_t getchild(std::string name);
      dirlist getchildren();

      virtual void remove();

      static std::auto_ptr<Directory> create(Database& db);
      static std::auto_ptr<Directory> create(Database& db,std::string path);
};

class File:public Node{
   public:
      File(Database& db, uint64_t id);

      static std::auto_ptr<File> create(Database& db);
      static std::auto_ptr<File> create(Database& db,std::string path);
};

class Symlink:public Node{
   public:
      Symlink(Database& db, uint64_t id);

      static std::auto_ptr<Symlink> create(Database& db);
      static std::auto_ptr<Symlink> create(Database& db,std::string path);
};

class Path{
   public:
      Path(Database& db, std::string path);
      std::auto_ptr<Directory> nparent;
      std::auto_ptr<Node> nchild;
      std::string parent;
      std::string child;
};

template<typename T> T Node::getattr(std::string name){
   Buffer b=getattrv(name);
   if(b.size!=sizeof(T))
      throw std::runtime_error("Node::getattr: Sizes do not match.");
   return *(T*)b.data;
}

template<typename T> void Node::setattr(std::string name,T v){
   Buffer b((char*)&v,sizeof(T));
   setattrv(name,b);
}

#endif

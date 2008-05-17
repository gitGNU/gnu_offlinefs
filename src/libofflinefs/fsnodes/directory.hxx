#ifndef FSNODES_DIRECTORY_HXX
#define FSNODES_DIRECTORY_HXX

#include "node.hxx"

class Directory:public Node{
   protected:
      Database<uint64_t>::Register rdir;
   public:
      class EExists:public std::runtime_error{
	 public:
	    EExists();
      };

      Directory(FsDb& dbs,uint64_t id);
      virtual ~Directory() {}

      void addchild(std::string name, Node& n);
      void delchild(std::string name);
      std::auto_ptr<Node> getchild(std::string name);
      std::list<std::string> getchildren() { return rdir.getattrs(); }

      virtual void remove();

      static std::auto_ptr<Directory> create(FsDb& dbs);
      static std::auto_ptr<Directory> create(FsDb& dbs, std::string path);

      class Path{
	 public:
	    Path(FsDb& dbs,std::string path);
	    std::auto_ptr<Directory> parent;
	    std::string leaf;
      };
};

template <typename T> 
std::auto_ptr<T> Node::create_(FsDb& dbs, std::string path){
   std::auto_ptr<T> n=T::create(dbs);
   try{
      Directory::Path p(dbs,path);
      p.parent->addchild(p.leaf,*n);
   }catch(...){
      n->remove();
      throw;
   }
   return n;
}

#endif

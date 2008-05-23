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

      // Link n at the specified name, it throws EExists if there is already a child
      // with the same name
      void addchild(std::string name, Node& n);
      void delchild(std::string name); // It throws ENotFound if the file doesn't exist
      std::auto_ptr<Node> getchild(std::string name); // It can throw ENotFound
      std::list<std::string> getchildren() { return rdir.getattrs(); }

      virtual void remove();

      //Initialize an empty directory
      static std::auto_ptr<Directory> create(FsDb& dbs);
      //Initialize an empty directory an link it at path, it can throw EAccess, ENotFound and EBadCast<Directory>
      static std::auto_ptr<Directory> create(FsDb& dbs,const SContext& sctx, std::string path);

      class Path{
	 public:
	    //Parse the path, copy the leaf name (the last token of the path) to leaf,
	    // even if it doesn't exist, and instance a Directory object representing
	    // leaf's parent
	    Path(FsDb& dbs,const SContext& sctx, std::string path);
	    std::auto_ptr<Directory> parent;
	    std::string leaf;
      };
};

template <typename T> 
std::auto_ptr<T> Node::create_(FsDb& dbs,const SContext& sctx, std::string path){
   std::auto_ptr<T> n=T::create(dbs);
   try{
      Directory::Path p(dbs,sctx,path);
      p.parent->access(sctx,W_OK|X_OK);
      p.parent->addchild(p.leaf,*n);
   }catch(...){
      n->remove();
      throw;
   }
   return n;
}

#endif

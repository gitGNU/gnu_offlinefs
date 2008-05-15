#ifndef FSNODES_NODE_HXX
#define FSNODES_NODE_HXX

#include "../common.hxx"
#include "../fsdb.hxx"

class Node:public Database<uint64_t>::Register{
   protected:
      template <typename T> static std::auto_ptr<T> create_(FsDb& dbs, std::string path);
   public:
      class ENotFound:public std::runtime_error{
	 public:
	    ENotFound();
      };
      class ENotDir:public std::runtime_error{
	 public:
	    ENotDir();
      };
      class EAccess:public std::runtime_error{
	 public:
	    EAccess();
      };
      FsDb& dbs;

      Node(FsDb& dbs,uint64_t id);
      virtual ~Node();

      static std::auto_ptr<Node> create(FsDb& dbs);
      static std::auto_ptr<Node> create(FsDb& dbs, std::string path);

      static std::auto_ptr<Node> getnode(FsDb& dbs, uint64_t id);
      static std::auto_ptr<Node> getnode(FsDb& dbs, std::string path);

      void link();
      void unlink();

      void access(uid_t uid,gid_t gid,int mode);
};

#endif

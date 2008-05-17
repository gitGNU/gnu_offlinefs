#ifndef FSNODES_NODE_HXX
#define FSNODES_NODE_HXX

#include <common.hxx>
#include <fsdb.hxx>

class Node:public Database<uint64_t>::Register{
   protected:
      template <typename T> static std::auto_ptr<T> create_(FsDb& dbs, std::string path);
   public:
      template<typename T>
      class EBadCast:public std::runtime_error{
	 public:
	    EBadCast():runtime_error(std::string("Node::EBadCast<")+typeid(T).name()+">") {}
      };
      class ENotFound:public std::runtime_error{
	 public:
	    ENotFound();
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

      template<typename T>
      static std::auto_ptr<T> cast(std::auto_ptr<Node> n){
	 if(typeid(*n)!=typeid(T))
	    throw EBadCast<T>();
	 return std::auto_ptr<T>((T*)n.release());
      }

      void link();
      void unlink();

      void access(uid_t uid,gid_t gid,int mode);
};

#endif

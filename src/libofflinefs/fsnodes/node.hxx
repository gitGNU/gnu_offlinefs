#ifndef FSNODES_NODE_HXX
#define FSNODES_NODE_HXX

#include <common.hxx>
#include <fsdb.hxx>
#include <scontext.hxx>

class Node:public Database<uint64_t>::Register{
   protected:
      template <typename T> static std::auto_ptr<T> create_(FsDb& dbs,const SContext& sctx, std::string path);
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

      //Initialize a "neutral" node (with its mode attribute = 0)
      static std::auto_ptr<Node> create(FsDb& dbs);
      //The same as the previous one, but also link it to the specified path.
      // It throws EBadCast<Directory> if one of the intermediate path elements
      // isn't a directory, ENotFound if one of them doesn't exist, EAccess if the
      // caller doesn't have enough permissions.
      static std::auto_ptr<Node> create(FsDb& dbs, const SContext& sctx, std::string path); 

      // Instance a Node derived object, depending on the type stored in the database
      // It throws ENotFound if the node doesn't exist
      static std::auto_ptr<Node> getnode(FsDb& dbs, uint64_t id);
      // The same as before. It can also throw EBadCast<Directory> and EAccess
      static std::auto_ptr<Node> getnode(FsDb& dbs, const SContext& sctx, std::string path);

      template<typename T>
      static std::auto_ptr<T> cast(std::auto_ptr<Node> n){
	 if(typeid(*n)!=typeid(T))
	    throw EBadCast<T>();
	 return std::auto_ptr<T>((T*)n.release());
      }

      // Increment link count
      void link();
      // Decrement link count and remove it if it has reached zero
      void unlink();

      // Throw EAccess if the node doesn't have all the specified permissions
      // for the caller
      void access(const SContext& sctx, int mode);
};

#endif

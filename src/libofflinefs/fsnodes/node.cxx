#include "node.hxx"
#include "file.hxx"
#include "directory.hxx"
#include "symlink.hxx"

using std::auto_ptr;
using std::string;

Node::ENotFound::ENotFound():runtime_error("Node::ENotFound") {}

Node::EAccess::EAccess():runtime_error("Node::EAccess") {}


Node::Node(FsDb& dbs,uint64_t id):Register(dbs.nodes,id),dbs(dbs) {}

Node::~Node() {}

auto_ptr<Node> Node::create(FsDb& dbs){
   auto_ptr<Node> n(new Node(dbs,dbs.nodes.createregister()));
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

auto_ptr<Node> Node::create(FsDb& dbs,const SContext& sctx, string path){
   return Node::create_<Node>(dbs,sctx,path);
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


std::auto_ptr<Node> Node::getnode(FsDb& dbs, uint64_t id){
   Node n(dbs,id);
   try{
      mode_t m=n.getattr<mode_t>("offlinefs.mode");
      if(S_ISREG(m))
	 return auto_ptr<Node>(new File(dbs,id));
      else if(S_ISDIR(m))
	 return auto_ptr<Node>(new Directory(dbs,id));
      else if(S_ISLNK(m))
	 return auto_ptr<Node>(new Symlink(dbs,id));
      else
	 return auto_ptr<Node>(new Node(dbs,id));
   }catch(EAttrNotFound& e){
      throw ENotFound();
   }
}

std::auto_ptr<Node> Node::getnode(FsDb& dbs, const SContext& sctx, std::string path){
   auto_ptr<Node> n = Node::getnode(dbs,0);
   string::size_type pos=0;
   string::size_type newpos=0;

   while((newpos=path.find("/",pos))!=string::npos){
      if(newpos!=pos){
	 n->access(sctx,X_OK);
	 n=cast<Directory>(n)->getchild(path.substr(pos,newpos-pos));
      }	 
      pos=newpos+1;
   }
      
   string name=path.substr(pos,newpos);
   if(name.empty())
      return n;
   else{
      n->access(sctx,X_OK);
      return cast<Directory>(n)->getchild(path.substr(pos,newpos));
   }
}

void Node::access(const SContext& sctx, int mode){
   if(sctx.uid==0)
      return;
   mode_t fmode=getattr<mode_t>("offlinefs.mode");

   int mode_ok=fmode&07;

   if(sctx.groups.count(getattr<gid_t>("offlinefs.gid")))
      mode_ok|=(fmode>>3)&07;

   if(sctx.uid==getattr<uid_t>("offlinefs.uid"))
      mode_ok|=(fmode>>6)&07;

   if(mode&~mode_ok)
      throw EAccess();
}

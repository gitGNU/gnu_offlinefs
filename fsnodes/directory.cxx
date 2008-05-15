#include "directory.hxx"

using std::auto_ptr;
using std::string;
using std::pair;

Directory::EExists::EExists():runtime_error("Directory::EExists") {}

Directory::Directory(FsDb& dbs,uint64_t id):Node(dbs,id),rdir(dbs.directories,id) {}

void Directory::addchild(std::string name, Node& n){
   n.link();
   try{
      try{
	 rdir.getattrv(name);
      }catch(...){
	 rdir.setattr(name,n.getid());
	 return;
      }
      throw EExists();
   }catch(...){
      n.unlink();
      throw;
   }
}

void Directory::delchild(std::string name){
   std::auto_ptr<Node> child=getchild(name);
   rdir.delattr(name);
   child->unlink();
}

std::auto_ptr<Node> Directory::getchild(std::string name){
   try{
      return Node::getnode(dbs,rdir.getattr<uint64_t>(name)); 
   }catch(Register::EAttrNotFound& e){
      throw ENotFound();
   }
}

void Directory::remove(){
   rdir.remove();
   Node::remove();
}

std::auto_ptr<Directory> Directory::create(FsDb& dbs){
   auto_ptr<Directory> n(new Directory(dbs,Node::create(dbs)->getid()));
   n->setattr<mode_t>("offlinefs.mode",S_IFDIR);
   n->addchild(".",*n);
   return n;
}

auto_ptr<Directory> Directory::create(FsDb& dbs,string path){
   auto_ptr<Directory> n=Directory::create(dbs);
   try{
      Path p(dbs,path);
      p.parent->addchild(p.leaf,*n);
      n->addchild("..",*p.parent);
   }catch(...){
      n->remove();
      throw;
   }
   return n;
}

Directory::Path::Path(FsDb& dbs,string path){
   std::string parentpath;
   string::size_type slashpos=0;
   string::size_type notslash=path.find_last_not_of("/");
   if(notslash!=string::npos){
      slashpos=path.rfind("/",notslash);
      leaf=path.substr(slashpos+1,notslash-slashpos);
      parentpath=path.substr(0,slashpos);
   }
   parent.reset((Directory*)Node::getnode(dbs,parentpath).release());
   if(!dynamic_cast<Directory*>(parent.get()))
      throw ENotDir();
}

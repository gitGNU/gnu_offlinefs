#include "file.hxx"
#include "directory.hxx"
#include "../medium.hxx"

using std::auto_ptr;
using std::string;

auto_ptr<File> File::create(FsDb& dbs){
   auto_ptr<File> n(new File(dbs,Node::create(dbs)->getid()));
   n->setattr<mode_t>("offlinefs.mode",S_IFREG);
   return n;
}

std::auto_ptr<File> File::create(FsDb& dbs,std::string path){
   return Node::create_<File>(dbs,path);
}

void File::remove(){
   try{
      Medium::getmedium(dbs,getattr<uint32_t>("offlinefs.mediumid"))->delfile(*this);
   }catch(...){}
   Node::remove();
}

auto_ptr<Medium> File::getmedium(std::string phid){
   auto_ptr<Medium> m;
   try{
      m=Medium::getmedium(dbs,getattr<uint32_t>("offlinefs.mediumid"));
   }catch(EAttrNotFound& e){
      m=Medium::defaultmedium(dbs);
      m->addfile(*this,phid);
   }
   return m;
}

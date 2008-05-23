#include "fsdb.hxx"
#include "fsnodes.hxx"
#include "media.hxx"

using std::auto_ptr;

FsDb::FsDb(std::string dbroot):Environment(dbroot),nodes(*this,"nodes"), directories(*this,"directories"),media(*this,"media") {}
   
FsDb::~FsDb(){
   try{
      close();
   }catch(...){}
}

void FsDb::open(){
   nodes.open();
   directories.open();
   media.open();
}

void FsDb::close(){
   nodes.close();
   directories.close();
   media.close();
}

void FsDb::rebuild(){
   nodes.rebuild();
   directories.rebuild();
   media.rebuild();

   // Initialize the root directory (it will get 0 as id)
   auto_ptr<Directory> root=Directory::create(*this);
   root->setattr<mode_t>("offlinefs.mode",S_IFDIR|0755);
   root->setattr<uid_t>("offlinefs.uid",getuid());
   root->setattr<gid_t>("offlinefs.gid",getgid());

   root->addchild("..",*root);
}

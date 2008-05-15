#include "fsdb.hxx"
#include "fsnodes.hxx"
#include "medium.hxx"

using std::auto_ptr;

FsDb::FsDb(std::string dbroot):Environment(dbroot),nodes(*this,"nodes"), directories(*this,"directories"),media(*this,"media"){
   try{
      nodes.open();
      directories.open();
      media.open();
   }catch(...){
      nodes.rebuild();
      directories.rebuild();
      media.rebuild();
      auto_ptr<Directory> root=Directory::create(*this);
      root->setattr<mode_t>("offlinefs.mode",S_IFDIR|0755);
      root->setattr<uid_t>("offlinefs.uid",getuid());
      root->setattr<gid_t>("offlinefs.gid",getgid());

      root->addchild("..",*root);
      Medium_directory::create(*this,"/home/curro/tmp/");
//      Medium_mount::create(*this,"/home/curro/tmp/","etiqueta","true");
   }
}
   

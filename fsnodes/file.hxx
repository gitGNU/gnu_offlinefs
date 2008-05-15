#ifndef FSNODES_FILE_HXX
#define FSNODES_FILE_HXX

#include "node.hxx"

class Medium;

class File:public Node{
   public:
      File(FsDb& dbs,uint64_t id):Node(dbs,id) {}

      static std::auto_ptr<File> create(FsDb& dbs);
      static std::auto_ptr<File> create(FsDb& dbs,std::string path);

      virtual void remove();

      std::auto_ptr<Medium> getmedium(std::string phid);
};

#endif

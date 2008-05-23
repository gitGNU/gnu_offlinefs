#ifndef MEDIA_INSERT_HXX
#define MEDIA_INSERT_HXX

#include "directory.hxx"

// Similar to Medium_directory, but it will run a specified shell script before
// accessing a file.
// Attributes:
//      checkcmd: shell command that will be run before any file operation:
//                insertscript will be run if it returns non zero.
//                It is supposed to check if the medium is still present.
//      insertscript: script that will get executed 
//                (with: sh -c insertscript "label" "directory") when the medium
//                is not present. It is supposed to ask the user and then mount
//                the medium. The operation will be aborted if it returns non zero.
//      label: symbolic name that will be passed as a parameter to insertscript
class Medium_insert:public Medium_directory{
   private:
      bool check();
      void insert();
   public:
      Medium_insert(FsDb& dbs,uint32_t id):Medium_directory(dbs,id) {}
      static std::auto_ptr<Medium_insert> create(FsDb& dbs);
      virtual std::auto_ptr<Source> getsource(File& f,int mode);
      virtual int truncate(File& f,off_t length);
      virtual Stats getstats();
      virtual void addfile(File& f,std::string phid);
      virtual void delfile(File& f);
};

#endif

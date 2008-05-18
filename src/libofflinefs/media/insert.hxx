#ifndef MEDIA_INSERT_HXX
#define MEDIA_INSERT_HXX

#include "directory.hxx"

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

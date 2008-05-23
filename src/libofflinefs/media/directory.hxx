#ifndef MEDIA_DIRECTORY_HXX
#define MEDIA_DIRECTORY_HXX

#include "medium.hxx"

// This medium stores the associated files inside a directory,
// somewhere else in the filesystem.
// It takes into account the following medium attributes:
//      directory: used when trying to access a file: the real path is constructed
//                 by concatenating directory+"/"+phid
//      unlink_files: if it's equal to "true", the real backend files will
//                 be removed when not needed.
class Medium_directory:public Medium{
      std::string realpath(File& f);
   public:
      Medium_directory(FsDb& dbs,uint32_t id):Medium(dbs,id) {}
      static std::auto_ptr<Medium_directory> create(FsDb& dbs);
      virtual std::auto_ptr<Source> getsource(File& f,int mode);
      virtual int truncate(File& f,off_t length);
      virtual Stats getstats();
      virtual void addfile(File& f,std::string phid);
      virtual void delfile(File& f);
};

#endif

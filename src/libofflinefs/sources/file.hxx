#ifndef SOURCES_FILE_HXX
#define SOURCES_FILE_HXX

#include "source.hxx"

class Source_file:public Source{
      int fd;
   public:
      Source_file(File& f,std::string path,int mode);
      virtual ~Source_file();
      virtual int read(char* buf, size_t nbyte, off_t offset);
      virtual int write(const char* buf, size_t nbyte, off_t offset);
      virtual int flush();
      virtual int fsync(int datasync);
};

#endif

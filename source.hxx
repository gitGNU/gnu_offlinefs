#ifndef SOURCE_HXX
#define SOURCE_HXX

#include "common.hxx"
#include "fsnodes.hxx"

class Source{
   protected:
      File f;
      off_t size;
      int mode;
   public:
      Source(File& f,int mode);
      virtual ~Source() {}
      File& getfile() {return f;}
      virtual int read(char* buf, size_t nbyte, off_t offset)=0;
      virtual int write(const char* buf, size_t nbyte, off_t offset)=0;
      virtual int flush()=0;
      virtual int fsync(int datasync)=0;
};

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

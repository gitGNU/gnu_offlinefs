#ifndef SOURCES_SOURCE_HXX
#define SOURCES_SOURCE_HXX

#include <common.hxx>
#include <fsnodes.hxx>

// Class that would implement every possible operation over an open file
class Source{
   protected:
      File f;
      off_t size;
      int mode;
   public:
      Source(File& f,int mode);
      virtual ~Source() {}
      File& getfile() {return f;}

      // Analogous to the standard unix calls
      virtual int read(char* buf, size_t nbyte, off_t offset)=0;
      virtual int write(const char* buf, size_t nbyte, off_t offset)=0;
      virtual int flush()=0;
      virtual int fsync(int datasync)=0;
};

#endif

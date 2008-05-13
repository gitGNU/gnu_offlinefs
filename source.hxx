#ifndef SOURCE_HXX
#define SOURCE_HXX

class Source{
   public:
      virtual ~Source()=0;
      virtual int read(char* buf, size_t nbyte, off_t offset)=0;
      virtual int write(char* buf, size_t nbyte, off_t offset)=0;
      virtual int flush()=0;
      virtual int fsync(int datasync)=0;
};

#endif

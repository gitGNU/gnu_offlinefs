#ifndef MEDIA_MEDIUM_HXX
#define MEDIA_MEDIUM_HXX

#include <common.hxx>
#include <fsnodes.hxx>
#include <fsdb.hxx>
#include <sources.hxx>

class Medium:public Database<uint32_t>::Register{
      FsDb& dbs;
   public:
      Medium(FsDb& dbs,uint32_t id):Register(dbs.media,id),dbs(dbs) {}
      class EInUse:public std::runtime_error{
	 public:
	    EInUse();
      };
      class ENotFound:public std::runtime_error{
	 public:
	    ENotFound();
      };
      class Stats{
	 public:
	    unsigned long blocks;
	    unsigned long freeblocks;
      };

      static std::auto_ptr<Medium> defaultmedium(FsDb& dbs);
      static std::auto_ptr<Medium> getmedium(FsDb& dbs, uint32_t id);
      static std::auto_ptr<Medium> create(FsDb& dbs, std::string type);

      virtual void remove();

      virtual std::auto_ptr<Source> getsource(File& f,int mode)=0;
      virtual int truncate(File& f,off_t length)=0;


      virtual Stats getstats()=0;
      static Stats collectstats(FsDb& dbs);

      virtual void addfile(File& f,std::string phid);
      virtual void delfile(File& f);

};

#endif

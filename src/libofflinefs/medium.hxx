#ifndef MEDIUM_HXX
#define MEDIUM_HXX

#include "common.hxx"
#include "fsnodes.hxx"
#include "fsdb.hxx"
#include "source.hxx"

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

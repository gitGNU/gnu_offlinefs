#ifndef MEDIA_MEDIUM_HXX
#define MEDIA_MEDIUM_HXX

#include <common.hxx>
#include <fsnodes.hxx>
#include <fsdb.hxx>
#include <sources.hxx>

// Class defining the interface of an abstract object that would implement
// all the specific operations over the data contained in a file
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
	    // Stored in multiples of 4096B
	    unsigned long blocks;
	    unsigned long freeblocks;
      };

      // Default medium associated to a file when it gets created
      static std::auto_ptr<Medium> defaultmedium(FsDb& dbs);
      // Instances a medium derived object (depending on the stored medium type)
      // It can throw EAttrNotFound if the medium does not exist or ENotFound if
      // the stored medium type is not implemented.
      static std::auto_ptr<Medium> getmedium(FsDb& dbs, uint32_t id);
      // Initialize a medium of the specified type. It throws ENotFound if 
      // the type is not implemented.
      static std::auto_ptr<Medium> create(FsDb& dbs, std::string type);

      // It throws EInUse if it's associated with any file
      virtual void remove();

      virtual std::auto_ptr<Source> getsource(File& f,int mode)=0;
      virtual int truncate(File& f,off_t length)=0;


      virtual Stats getstats()=0; 
      // Returns the global filesystem statistics for the media stored in the database
      static Stats collectstats(FsDb& dbs);

      // Link file f with this medium, phid should be a string that will be used
      // to locate the file in the medium
      virtual void addfile(File& f,std::string phid);
      virtual void delfile(File& f);

};

#endif

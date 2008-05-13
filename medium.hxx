#ifndef MEDIUM_HXX
#define MEDIUM_HXX

#include "fsnodes.hxx"
#include <memory>

class Medium{
      Database& db;
   public:
      class Stats{
	 public:
	    unsigned long blocks;
	    unsigned long freeblocks;
      };

      static std::auto_ptr<Medium> defaultmedium(Database& db);
      static std::auto_ptr<Medium> getmedium(Database& db, uint32_t id);
      virtual void remove()=0;

      virtual std::auto_ptr<Source> getsource(File& f)=0;
      virtual int truncate(File& f)=0;


};

#endif

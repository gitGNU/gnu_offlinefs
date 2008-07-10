//     Copyright (C) 2008 Francisco Jerez
//     This file is part of offlinefs.

//     offlinefs is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.

//     offlinefs is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.

//     You should have received a copy of the GNU General Public License
//     along with offlinefs.  If not, see <http://www.gnu.org/licenses/>.

#ifndef MEDIA_MEDIUM_HXX
#define MEDIA_MEDIUM_HXX

#include <common.hxx>
#include <fsnodes.hxx>
#include <fsdb.hxx>
#include <sources.hxx>

// Class defining the interface of an abstract object that would implement
// all the specific operations over the data contained in a file
class Medium:public Database<uint32_t>::Register{
   protected:
      FsTxn& txns;
   public:
      Medium(FsTxn& txns,uint32_t id):Register(txns.media,id),txns(txns) {}
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
	    Stats():blocks(0),freeblocks(0) {}
	    // Stored in multiples of 4096B
	    unsigned long blocks;
	    unsigned long freeblocks;
      };

      // Instances a medium derived object (depending on the stored medium type)
      // It can throw EAttrNotFound if the medium does not exist or ENotFound if
      // the stored medium type is not implemented.
      static std::auto_ptr<Medium> getmedium(FsTxn& txns, uint32_t id);
      // Initialize a medium of the specified type. It throws ENotFound if 
      // the type is not implemented.
      static std::auto_ptr<Medium> create(FsTxn& txns, std::string type);

      // It throws EInUse if it's associated with any file
      virtual void remove();

      virtual std::auto_ptr<Source> getsource(File& f,int mode)=0;
      virtual int truncate(File& f,off_t length)=0;


      virtual Stats getstats()=0; 
      // Returns the global filesystem statistics for the media stored in the database
      static Stats collectstats(FsTxn& txns);

      // Link file f with this medium, phid should be a string that will be used
      // to locate the file in the medium
      virtual void addfile(File& f,std::string phid);
      virtual void delfile(File& f);

};

#endif

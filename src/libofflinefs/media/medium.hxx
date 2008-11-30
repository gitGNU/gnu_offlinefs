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
#include <nodes.hxx>
#include <fsdb.hxx>
#include <chunks/chunk.hxx>

// Interface to an object implementing the file data operations
class Medium:public Database<uint32_t>::Register{
   public:
      class ENotImplemented:public std::runtime_error{
	 public:
	    ENotImplemented(const std::string& s): runtime_error(s) {}
      };
      class ENotFound:public std::runtime_error{
	 public:
	    ENotFound(const std::string& s): runtime_error(s) {}
      };

      // Filesystem statistics
      class Stats{
	 public:
	    Stats():total(0),free(0) {}

	    uint64_t total;
	    uint64_t free;
      };

      Medium(FsTxn& txns,uint32_t id):Register(txns.media,id) {}

      // Instances a medium derived object (depending on the stored
      // medium type) It can throw ENotFound if the medium does
      // not exist or ENotImplemented if the stored medium type is not
      // implemented.
      static std::auto_ptr<Medium> getmedium(FsTxn& txns, uint32_t id);
      // The same as above, but looks up the medium by label (very slow)
      static std::auto_ptr<Medium> getmedium(FsTxn& txns, std::string label);
      // Initialize a medium of the specified type with some
      // reasonable default attributes. It throws ENotImplemented if
      // the type is not implemented.
      static std::auto_ptr<Medium> create(FsTxn& txns, std::string type);

      // Instantiate a Chunk-derived class that gives access to the
      // specified phid inside this medium
      virtual std::auto_ptr<Chunk> getchunk(std::string phid, int mode)=0;

      // Return filesystem statistics
      virtual Stats getstats()=0; 

      // Link file f with this medium, phid should be a string that
      // will be used to identify the file inside the medium
      virtual void addfile(std::string phid)=0;
      virtual void delfile(std::string phid)=0;
};

#endif

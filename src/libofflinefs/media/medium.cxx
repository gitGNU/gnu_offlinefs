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

#include "medium.hxx"
#include "directory.hxx"
#include "insert.hxx"
#include <stdlib.h>

using std::auto_ptr;
using std::string;
using std::list;

std::auto_ptr<Medium> Medium::getmedium(FsTxn& txns, uint32_t id){
   string mediumtype;
   Register r(txns.media,id);

   try{
      mediumtype = r.getattrv("type");
   }catch(EAttrNotFound& e){
      std::ostringstream os;
      os << "Medium::getmedium: Unknown medium ID " << id << ".";
      throw ENotFound(os.str());
   }

   if(mediumtype=="directory")
      return std::auto_ptr<Medium>(new Medium_directory(txns,id));
   else if(mediumtype=="insert")
      return std::auto_ptr<Medium>(new Medium_insert(txns,id));

   throw ENotImplemented(string("Medium::getmedium: Unknown medium type \"") + mediumtype + string("\"."));
}

std::auto_ptr<Medium> Medium::getmedium(FsTxn& txns, string label){
   std::list<uint32_t> media;
   txns.dbs.media.getregisters(txns.media,media);

   for(std::list<uint32_t>::iterator it = media.begin();
       it != media.end(); it++){
      try{
	 if(string(Register(txns.media,*it).getattrv("label")) == label)
	    return Medium::getmedium(txns,*it);
      }catch(EAttrNotFound& e){}
   }

   throw ENotFound("Medium::getmedium: Unknown medium \""+label+"\".");
}

std::auto_ptr<Medium> Medium::create(FsTxn& txns, string type){
   if(type=="directory")
      return auto_ptr<Medium>(Medium_directory::create(txns));
   else if(type=="insert")
      return auto_ptr<Medium>(Medium_insert::create(txns));

   throw ENotImplemented(string("Medium::getmedium: Unknown medium type \"") + type + string("\"."));
}

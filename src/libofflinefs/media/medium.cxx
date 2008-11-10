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

using std::string;
using std::list;
using std::tr1::shared_ptr;

std::auto_ptr<Medium> Medium::getmedium(libconfig::Setting& conf){
   string mediumtype;

   if(!conf.lookupValue("type",mediumtype)){
      std::ostringstream os;
      os << "Medium::getmedium: Error parsing config file after line " 
	 << conf.getSourceLine() << ": \"type\" parameter required.";
      throw std::runtime_error(os.str());
   }

   if(mediumtype=="directory")
      return std::auto_ptr<Medium>(new Medium_directory(conf));
   else if(mediumtype=="insert")
      return std::auto_ptr<Medium>(new Medium_insert(conf));

   throw std::runtime_error(string("Medium::getmedium: Unknown medium type \"") + mediumtype + string("\"."));
}

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

#include "source.hxx"
#include "single.hxx"
#include "types.hxx"

using std::string;
using std::auto_ptr;
namespace off = offlinefs;

Source::Source(File& f,int mode):dbs(f.txns.dbs),fileid(f.getid()),size(0),mode(mode) {
   size=f.getattr<off::off_t>("offlinefs.size");
}

auto_ptr<Source> Source::getsource(File& f,int mode){
   string source = f.getattrv("offlinefs.source");

   if(source == "single")
      return auto_ptr<Source>(new Source_single(f,mode));
   else
      throw std::runtime_error(string("Source::getsource: Unknown source type \"")+ source + "\".");
}

void Source::remove(File& f){
   string source = f.getattrv("offlinefs.source");

   if(source == "single")
      Source_single::remove(f);
   else
      throw std::runtime_error(string("Source::remove: Unknown source type \"")+ source + "\".");
}

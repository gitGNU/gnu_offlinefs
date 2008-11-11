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

#include "file.hxx"
#include "directory.hxx"
#include <sources/source.hxx>

using std::auto_ptr;
using std::string;

auto_ptr<File> File::create(FsTxn& txns){
   auto_ptr<File> n(new File(txns,Node::create(txns)->getid()));
   n->setattr<mode_t>("offlinefs.mode",S_IFREG);
   return n;
}

void File::remove(){
   try{
      Source::remove(*this);
   }catch(...){}
   Node::remove();
}

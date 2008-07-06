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

#include "symlink.hxx"
#include "directory.hxx"

using std::auto_ptr;
using std::string;

Symlink::Symlink(FsTxn& txns,uint64_t id):Node(txns,id) {
}


auto_ptr<Symlink> Symlink::create(FsTxn& txns){
   auto_ptr<Symlink> n(new Symlink(txns,Node::create(txns)->getid()));
   n->setattr<mode_t>("offlinefs.mode",S_IFLNK);
   n->setattrv("offlinefs.symlink",Buffer());
   return n;
}

std::auto_ptr<Symlink> Symlink::create(FsTxn& txns,const SContext& sctx,std::string path){
   return Node::create_<Symlink>(txns,sctx,path);
}

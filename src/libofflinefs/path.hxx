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


#ifndef PATH_HXX
#define PATH_HXX

#include <common.hxx>
#include <fsdb.hxx>
#include <scontext.hxx>
#include <pathcache.hxx>
#include <nodes.hxx>

template<typename T>
class Path{
   public:
      std::auto_ptr<Directory> parent;
      std::string leaf;

      //Parse the path, copy the leaf name (the last token of the path) to leaf,
      // even if it doesn't exist, and instance a Directory object representing
      // leaf's parent
      Path(FsTxn& txns, const SContext& sctx, PathCache& pch, std::string path){
	 std::string parentpath;
	 std::string::size_type slashpos=0;
	 std::string::size_type notslash=path.find_last_not_of("/");
	 if(notslash!=std::string::npos){
	    slashpos=path.rfind("/",notslash);
	    leaf=path.substr(slashpos+1,notslash-slashpos);
	    parentpath=path.substr(0,slashpos);
	 }

	 parent=Node::cast<Directory>(pch.getnode(txns,sctx,parentpath));
      }


      // Create a new node named leaf at parent 
      std::auto_ptr<T> create(FsTxn& txns, const SContext& sctx){
	 parent->access(sctx, W_OK|X_OK);

	 std::auto_ptr<T> n=T::create(txns);
	 try{
	    parent->addchild(leaf,*n);
	 }catch(...){
	    n->remove();
	    throw;
	 }

	 return n;
      }
};

#endif

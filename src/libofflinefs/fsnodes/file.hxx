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

#ifndef FSNODES_FILE_HXX
#define FSNODES_FILE_HXX

#include "node.hxx"

class Medium;

class File:public Node{
   public:
      File(FsTxn& txns,uint64_t id):Node(txns,id) {}

      static std::auto_ptr<File> create(FsTxn& txns);
      static std::auto_ptr<File> create(FsTxn& txns,const SContext& sctx,std::string path);

      virtual void remove();

      std::auto_ptr<Medium> getmedium(std::string phid);
};

#endif

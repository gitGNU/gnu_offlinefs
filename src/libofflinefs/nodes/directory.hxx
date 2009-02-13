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

#ifndef NODES_DIRECTORY_HXX
#define NODES_DIRECTORY_HXX

#include "node.hxx"

class Directory:public Node{
   protected:
      Database<uint64_t>::Register rdir;
   public:
      class EExists:public std::runtime_error{
	 public:
	    EExists();
      };

      Directory(FsTxn& txns,uint64_t id);
      virtual ~Directory() {}

      // Link n at the specified name, it throws EExists if there is already a child
      // with the same name
      void addchild(std::string name, Node& n);
      void delchild(std::string name); // It throws ENotFound if the file doesn't exist
      std::auto_ptr<Node> getchild(std::string name); // It can throw ENotFound
      template<typename ConT> void getchildren(ConT& children) { rdir.getattrs(children); }
      int countchildren() { return rdir.countattrs(); }

      virtual void remove();

      //Initialize an empty directory
      static std::auto_ptr<Directory> create(FsTxn& txns);
};

#endif

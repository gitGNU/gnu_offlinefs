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

#include "fsdb.hxx"
#include "nodes.hxx"

using std::auto_ptr;

FsDb::FsDb(std::string dbroot):Environment(dbroot),nodes(*this,"nodes"), directories(*this,"directories"),media(*this,"media") {}

FsDb::~FsDb(){
   try{
      close();
   }catch(...){}
}

void FsDb::open(){
   nodes.open();
   directories.open();
   media.open();
}

void FsDb::close(){
   nodes.close();
   directories.close();
   media.close();
}

void FsDb::rebuild(){
   nodes.rebuild();
   directories.rebuild();
   media.rebuild();

   // Initialize the root directory (it will get 0 as id)
   FsTxn txns(*this);
   auto_ptr<Directory> root=Directory::create(txns);
   root->setattr<mode_t>("offlinefs.mode",S_IFDIR|0755);
   root->setattr<uid_t>("offlinefs.uid",getuid());
   root->setattr<gid_t>("offlinefs.gid",getgid());

   root->addchild("..",*root);
}

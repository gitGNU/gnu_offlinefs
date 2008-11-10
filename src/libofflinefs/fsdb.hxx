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

#ifndef FSDB_HXX
#define FSDB_HXX

#include "common.hxx"
#include "database.hxx"
#include "mediacache.hxx"

class FsDb:public Environment{
   public:
      FsDb(std::string dbroot, std::string config);
      ~FsDb();

      void open();
      void close();
      // Reinitialize the databases, erasing all its contents
      void rebuild();

      // Database storing node attributes
      Database<uint64_t> nodes;

      // Database storing node subdirectories
      // Each register represents a directory, with its children
      // as attributes, mapping a child name into its node id.
      Database<uint64_t> directories;

      // Media cache
      MediaCache mcache;
};

//Automatically begin/commit a transaction in each database
class FsTxn{
   public:
      FsDb& dbs;
      Database<uint64_t>::Txn nodes;
      Database<uint64_t>::Txn directories;
 
      FsTxn(FsDb& dbs):dbs(dbs),nodes(dbs.nodes),directories(dbs.directories){}
      void commit() {nodes.commit(); directories.commit();}
      void abort() {nodes.abort(); directories.abort();}
};

#endif

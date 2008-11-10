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

#ifndef MEDIA_INSERT_HXX
#define MEDIA_INSERT_HXX

#include "directory.hxx"

// Similar to Medium_directory, but it will run a specified shell script before
// accessing a file.
// Attributes:
//      checkcmd: shell command that will be run before any file operation:
//                insertscript will be run if it returns non zero.
//                It is supposed to check if the medium is still present.
//      insertcmd: shell command that will get executed  when the medium
//                is not present. It is supposed to ask the user and then mount
//                the medium. The operation will be aborted if it returns non zero.
class Medium_insert:public Medium_directory{
   private:
      bool check();
      void insert();

      std::string checkcmd;
      std::string insertcmd;
   public:
      Medium_insert(libconfig::Setting& conf);

      virtual std::auto_ptr<Source> getsource(File& f,int mode);
      virtual int truncate(File& f,off_t length);
      virtual Stats getstats();
      virtual void addfile(File& f,std::string phid);
      virtual void delfile(File& f);
};

#endif

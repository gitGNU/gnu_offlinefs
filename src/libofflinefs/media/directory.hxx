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

#ifndef MEDIA_DIRECTORY_HXX
#define MEDIA_DIRECTORY_HXX

#include "medium.hxx"

// This medium stores the associated files inside a directory,
// somewhere else in the filesystem.
// It takes into account the following medium attributes:
//      directory: used when trying to access a file: the real path is constructed
//                 by concatenating directory+"/"+phid
//      unlink_files: if it's equal to "true", the real backend files will
//                 be removed when not needed.
class Medium_directory:public Medium{
   protected:
      std::string label;
      std::string directory;
      bool unlink_files;

      std::string realpath(File& f);
   public:
      Medium_directory(libconfig::Setting& conf);

      virtual std::auto_ptr<Source> getsource(File& f,int mode);
      virtual int truncate(File& f,off_t length);
      virtual Stats getstats();
      virtual void addfile(File& f,std::string phid);
      virtual void delfile(File& f);
};

#endif

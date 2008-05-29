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

#ifndef SCONTEXT_HXX
#define SCONTEXT_HXX

#include <common.hxx>
#include <set>

class SContext{
   public:
      SContext(uid_t uid,gid_t gid);
      uid_t uid;
      gid_t gid;
      std::set<gid_t> groups; // Unix groups the user belongs to
};

#endif

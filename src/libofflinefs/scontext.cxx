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

#include "scontext.hxx"
#include <grp.h>
#include <pwd.h>

SContext::SContext(uid_t uid, gid_t gid):uid(uid), gid(gid){
   int ngroups=0;
   struct passwd* pw=getpwuid(uid);
   if(!pw)
      throw std::runtime_error("SContext::SContext: getpwuid failed.");
   
   getgrouplist(pw->pw_name,gid,NULL,&ngroups);
   gid_t* pgroups=new gid_t[ngroups];
   if(getgrouplist(pw->pw_name,gid,pgroups,&ngroups)<0){
      delete[] pgroups;
      throw std::runtime_error("SContext::SContext: getgrouplist failed.");
   }
   for(gid_t* pgroup=pgroups;pgroup<pgroups+ngroups;pgroup++)
      groups.insert(*pgroup);
   delete[] pgroups;
}

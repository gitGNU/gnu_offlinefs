#include "scontext.hxx"
#include <grp.h>
#include <pwd.h>

SContext::SContext(uid_t uid, gid_t gid):uid(uid), gid(gid){
   int ngroups;
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

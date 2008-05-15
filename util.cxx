#include "util.hxx"
#include <grp.h>
#include <pwd.h>

bool ingroup(uid_t uid,gid_t gid){
   struct passwd* pw=getpwuid(uid);
   if(!pw)
      throw std::runtime_error("ingroup: getpwuid failed.");
   std::string uname(pw->pw_name);
   struct group* gr=getgrgid(gid);
   if(!gr)
      throw std::runtime_error("ingroup: getgrgid failed.");
   for(char** pmem=gr->gr_mem; *pmem!=NULL; pmem++)
      if(uname==*pmem)
	 return true;
   return false;
}


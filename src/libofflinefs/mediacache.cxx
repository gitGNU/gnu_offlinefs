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

#include <mediacache.hxx>
#include <media/medium.hxx>

using std::string;
using std::pair;
using std::tr1::shared_ptr;

MediaCache::MediaCache(std::string config):config(config), checktime(0) {
   uptodate();
}

void MediaCache::uptodate(){
   struct stat st;
   if(stat(config.c_str(), &st) || st.st_mtime>checktime){
      checktime = time(NULL);
      reload();
   }
}

void MediaCache::reload(){
   cache.clear();

   try{
      libconfig::Config conf;
      conf.readFile(config.c_str());

      if(conf.exists("media")){
	 libconfig::Setting& media_setting = conf.lookup("media");

	 for(int i=0; i<media_setting.getLength(); i++){
	    string label;

	    if(!media_setting[i].lookupValue("label",label)){
	       std::ostringstream os;
	       os << "MediaCache::reload: Error parsing config file after line " 
		  << media_setting[i].getSourceLine() << ": \"label\" parameter required.";
	       throw std::runtime_error(os.str());
	    }

	    cache.insert(pair<string,CElem>(label,shared_ptr<Medium>(Medium::getmedium(media_setting[i]).release())));
	 }
      }
   }catch(libconfig::FileIOException& e){
   }catch(libconfig::ParseException& e){
      std::ostringstream os;
      os << "MediaCache::reload: Error parsing the configuration file (line " 
	 << e.getLine() << "): " << e.getError() << "\n";
      throw std::runtime_error(os.str());
   }
}

shared_ptr<Medium> MediaCache::getmedium(std::string id){
   uptodate();

   Cache::iterator it = cache.find(id);
   if(it==cache.end())
      throw std::runtime_error(string("MediaCache::getmedium: Medium \"") + id + string("\" not found."));
   return it->second;
}

std::list<shared_ptr<Medium> > MediaCache::list(){
   std::list<shared_ptr<Medium> > ms;
   uptodate();

   for(Cache::iterator it = cache.begin();
       it != cache.end(); it++)
      ms.push_back(it->second);

   return ms;
}

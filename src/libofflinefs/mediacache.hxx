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

#ifndef MEDIACACHE_HXX
#define MEDIACACHE_HXX

#include <libconfig.h++>
#include <tr1/unordered_map>
#include <tr1/memory>

#include <common.hxx>

class Medium;

class MediaCache{
   protected:
      typedef std::tr1::shared_ptr<Medium> CElem;
      typedef std::tr1::unordered_map<std::string, CElem> Cache;

      Cache cache;
      std::string config;
      time_t checktime;

      void uptodate();
      void reload();
   public:
      MediaCache(std::string config);
      
      std::tr1::shared_ptr<Medium> getmedium(std::string id);
      std::list<std::tr1::shared_ptr<Medium> > list();
};

#endif

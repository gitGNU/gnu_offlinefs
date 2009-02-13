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

#ifndef TOKEN_HXX
#define TOKEN_HXX

#include <common.hxx>

typedef std::pair<std::string::const_iterator,
		  std::string::const_iterator> Token;

template<typename ConT>
void tokenize(std::string ss, ConT& result,
	      const std::string& path){
   std::string::size_type n0=0, n1=0, n=path.size();

   while(n0 < n){
      if((n1=path.find_first_of(ss,n0)) == std::string::npos)
	 n1=n;

      if(n1>n0)
	 result.push_back(Token(path.begin()+n0,
				path.begin()+n1));

      n0 = n1+1;
   }
}

template<typename ItT>
void join(std::string s, std::string& result, ItT tok0, ItT tok1){
   if(tok0 != tok1){
      result.assign(tok0->first,tok0->second);
      tok0++;
   }

   while(tok0 != tok1){
      result.append(s).append(tok0->first, tok0->second);
      tok0++;
   }
}

#endif

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

#include <string>
#include <list>
#include <stdexcept>
#include <utility>
#include <tr1/unordered_map>

template<typename T> class FParser;

// Format string token specification. If fmapparams is non-NULL, it
// represents a "parametrized" format, that is, it requires a
// parameter in the format string like: "%tok{parameter}" and it can
// be repeated several times in the format string: each match will be
// inserted in the unordered_map pointed by fmapparams, with the
// parameter name as key.  Otherwise, it will match "%tok", and the
// string pointed by fmapval will be set to the matched text
template<typename T>
class FSpec{
   public:
      FSpec():fmapval(NULL),fmapparams(NULL) {}
      FSpec(std::string tok,std::string (T::*fmapval)):tok(tok),fmapval(fmapval),fmapparams(NULL) {}
      FSpec(std::string tok,std::tr1::unordered_map<std::string,std::string> (T::*fmapparams)):tok(tok),fmapval(NULL),fmapparams(fmapparams) {}
      std::string tok;
      std::string (T::*fmapval);
      std::tr1::unordered_map<std::string,std::string> (T::*fmapparams);
};

// Formatted string parser
template<typename T>
class Format{
      friend class FParser<T>;
      struct DTok{
	    std::string delimiter;
	    FSpec<T> fspec;
	    std::string param;
      };
      std::list<DTok> toks;
   public:
      // Fill in fmap from the specified string
      void match(T& fmap,std::string text);
};

// Formatted string parser generator. It has to be filled with a list
// of valid token specifications
template<typename T>
class FParser{
      std::list<FSpec<T> > toks;
   public:
      FParser& add(const FSpec<T>& fspec) {toks.push_back(fspec); return *this;}

      // Generate a formatted string parser from a format string 
      Format<T> parse(std::string formatstr);
};


template<typename T>
void Format<T>::match(T& fmap,std::string text){
   std::string::size_type pos0=0;
   for(typename std::list<DTok>::iterator it=toks.begin();it!=toks.end();++it){
      std::string::size_type pos1=text.find(it->delimiter,pos0);
      if(pos1==std::string::npos)
	 throw std::runtime_error("Error parsing input: It doesn't match the format string.");

      if(it->fspec.fmapparams)
	 (fmap.*(it->fspec.fmapparams)).insert(std::pair<std::string,std::string>(it->param,text.substr(pos0,pos1-pos0)));
      if(it->fspec.fmapval)
	 fmap.*(it->fspec.fmapval)=text.substr(pos0,pos1-pos0);

      pos0=pos1+it->delimiter.size();
   }
}

template<typename T>
Format<T> FParser<T>::parse(std::string formatstr){
   Format<T> format;
   typename Format<T>::DTok dtok;
   std::string::size_type pos=0;

   while(true){
      std::string::size_type nextf=formatstr.find("%",pos);

      dtok.delimiter+=formatstr.substr(pos,nextf-pos);
      if(nextf==std::string::npos)
	 break;
      pos=nextf+1;

      // Interpret "%%" as "%"
      if(formatstr[pos]=='%'){
	 dtok.delimiter+="%";
	 pos++;
	 continue;
      }

      // Push in the last token
      format.toks.push_back(dtok);
      dtok=typename Format<T>::DTok();
 
      // Compare each known token with the rest of the string
      typename std::list<FSpec<T> >::iterator it;
      for(it=toks.begin();it!=toks.end();++it){
	 std::string::size_type ssize=it->tok.size();
	 if(formatstr.substr(pos,ssize)==it->tok){
	    // Read the parameter, if it is needed
	    if(it->fmapparams){
	       if(formatstr[pos+ssize]!='{')
		  throw std::runtime_error("Error: format \""+it->tok+"\" requires a parameter.");
	       std::string::size_type cpos=formatstr.find("}",pos+ssize+1);
	       if(cpos==std::string::npos)
		  throw std::runtime_error("Error: '{' found without matching '}'");
	       dtok.param=formatstr.substr(pos+ssize+1,cpos-pos-ssize-1);
	       ssize=cpos+1-pos;
	    }else
	       dtok.param.clear();

	    dtok.fspec=*it;
	    pos+=ssize;
	    break;
	 }
      }
      if(it==toks.end())
	 throw std::runtime_error("Error: unknown format near \""+formatstr.substr(pos-1,4)+"...\".");
   }
   format.toks.push_back(dtok);
   
   return format;
}

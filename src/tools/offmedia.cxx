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

#include <common.hxx>

#include <iostream>
#include <boost/program_options.hpp>

#include <fsdb.hxx>
#include <media/medium.hxx>

using std::string;
using std::vector;
using std::runtime_error;
using std::auto_ptr;
using std::list;
namespace po=boost::program_options;

// Instantiate a Medium object depending on the options collected from
// the command line
auto_ptr<Medium> getmedium(FsTxn& txns, po::variables_map& vm){
   if(vm.count("id"))
      return Medium::getmedium(txns,vm["id"].as<uint32_t>());
   else if(vm.count("label"))
      return Medium::getmedium(txns,vm["label"].as<string>());

   throw runtime_error("Error: No medium specified.\n"
		       "Use --help for more information.");
}

// Replace some special characters in s0 with escape sequences
string escape(string s0){
   string s1;

   for(string::iterator it = s0.begin();
       it != s0.end(); it++){
      if(*it == '\\')
	 s1+="\\\\";
      else if(*it == '"')
	 s1+="\\\"";
      else if(*it == '\n')
	 s1+="\\n";
      else
	 s1+=*it;
   }

   return s1;
}

int main(int argc, char** argv){
   string dbroot;

   // Command line parsing
   po::options_description options("Options");
   options.add_options()
      ("help,h","Print help")
      ("version,V","Print version")
      ("add,a",po::value<string>(),"Create a new medium of type <arg>")
      ("set,s","Set a medium attribute")
      ("unset,u","Delete a medium attribute")
      ("remove,r","Remove a medium")
      ("list,L","Show media attributes")
      ("label,l",po::value<string>(),"Select medium by label")
      ("id,i",po::value<uint32_t>(),"Select medium by ID")
      ("verbose,v","Verbose mode")
      ("dbroot,b",po::value<string>(),"Database root")
      ("no-escape","Don't escape the special characters from --list");

   po::options_description hidden_options;
   hidden_options.add_options()
      ("attribute",po::value<string>())
      ("value",po::value<string>());

   po::options_description all_options;
   all_options.add(options);
   all_options.add(hidden_options);

   po::positional_options_description pdesc;
   pdesc.add("attribute",1);
   pdesc.add("value",1);

   po::variables_map vm;
   try{
      po::store(po::command_line_parser(argc,argv).options(all_options).positional(pdesc).run(),vm);
   }catch(std::exception& e){
      std::cerr << "Error parsing command line: " << e.what() << std::endl;
      std::cerr << "Use --help for more information." << std::endl;
      return 1;
   }
   po::notify(vm);

   // Process the parsed options
   if(vm.count("dbroot"))
      dbroot=vm["dbroot"].as<string>();
   else{
      char* home=getenv("HOME");
      if(home){
	 dbroot = string(home)+"/.offlinefs/";
      }

      if(dbroot.empty()){
	 std::cerr << "Error: Database root required." << std::endl;
	 std::cerr << "Use --help for more information." << std::endl;
	 return 1;
      }
   }

   if(vm.count("help")){
      std::cerr << "Usage:\n"
		<< "  "<< argv[0] << " --label <medium label> --add <medium type> \n"
		<< "  "<< argv[0] << " --label <medium label> --set <attribute> <value>\n"
		<< "  "<< argv[0] << " --label <medium label> --unset <attribute>\n"
		<< "  "<< argv[0] << " --label <medium label> --remove\n"
		<< "  "<< argv[0] << " --list [ --label <medium label> [attribute] ]\n";
      std::cerr << options;
      return 0;
   }

   if(vm.count("version")){
      std::cerr << "offlinefs version: " << VERSION << "\n"		\
	 "Copyright (C) 2008 Francisco Jerez\n"				\
	 "This program is free software: you can redistribute it and/or modify\n" \
	 "it under the terms of the GNU General Public License as published by\n" \
	 "the Free Software Foundation, either version 3 of the License, or\n" \
	 "(at your option) any later version.\n"			\
	 "\n"								\
	 "This program is distributed in the hope that it will be useful,\n" \
	 "but WITHOUT ANY WARRANTY; without even the implied warranty of\n" \
	 "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n" \
	 "GNU General Public License for more details.\n"		\
	 "\n"								\
	 "You should have received a copy of the GNU General Public License\n" \
	 "along with offlinefs.  If not, see <http://www.gnu.org/licenses/>.\n";
      std::cerr << std::endl;
      return 0;
   }
   
   // Open the database
   FsDb dbs(dbroot);
   dbs.open();

   FsTxn txns(dbs);

   try{
      if(vm.count("add")){
	 // Option requirements
	 if(vm.count("attribute") || vm.count("value"))
	    throw runtime_error("Error: Too many options.\n"
				"Use --help for more information.");
	 if(!vm.count("label"))
	    throw runtime_error("Error: No medium label specified.\n"
				"Use --help for more information.");

	 // Create the medium
	 auto_ptr<Medium> m=Medium::create(txns,vm["add"].as<string>());
	 m->setattrv("label",vm["label"].as<string>());

      }else if(vm.count("set")){
	 // Option requirements
	 if(!vm.count("attribute") || !vm.count("value"))
	    throw runtime_error("Error: Not enough arguments.\n"
				"Use --help for more information.");

	 // Set the attribute
	 auto_ptr<Medium> m=getmedium(txns,vm);
	 m->setattrv(vm["attribute"].as<string>(),vm["value"].as<string>());

      }else if(vm.count("unset")){
	 // Option requirements
	 if(!vm.count("attribute"))
	    throw runtime_error("Error: Not enough arguments.\n"
				"Use --help for more information.");
	 if(vm.count("value"))
	    throw runtime_error("Error: Too many options.\n"
				"Use --help for more information.");

	 // Remove the attribute
	 auto_ptr<Medium> m=getmedium(txns,vm);
	 m->delattr(vm["attribute"].as<string>());

      }else if(vm.count("remove")){
	 // Option requirements
	 if(vm.count("attribute") || vm.count("value"))
	    throw runtime_error("Error: Too many options.\n"
				"Use --help for more information.");

	 // Remove the medium
	 auto_ptr<Medium> m=getmedium(txns,vm);
	 m->remove();

      }else if(vm.count("list")){
	 // Option requirements
	 if((!vm.count("label") && vm.count("attribute")) || vm.count("value"))
	    throw runtime_error("Error: Too many options.\n"
				"Use --help for more information.");

	 // Show the medium list
	 list<uint32_t> media = dbs.media.listregisters(txns.media);
	 
	 for(list<uint32_t>::iterator mid = media.begin();
	     mid != media.end(); mid++){
	    auto_ptr<Medium> m = Medium::getmedium(txns,*mid);

	    // Filter by label/id
	    if((vm.count("label") && string(m->getattrv("label")) != vm["label"].as<string>()) ||
	       (vm.count("id") && *mid != vm["id"].as<uint32_t>()))
	       continue;

	    if(vm.count("verbose"))
	       std::cout << "[medium " << *mid << "]\n";
	    else
	       std::cout << "[medium]\n";

	    list<string> attributes = m->getattrs();

	    for(list<string>::iterator attribute = attributes.begin();
		attribute != attributes.end(); attribute++){
	       // Filter by attribute
	       if(vm.count("attribute") && *attribute != vm["attribute"].as<string>())
		  continue;

	       if(vm.count("no-escape"))
		  std::cout << *attribute << " = \"" << string(m->getattrv(*attribute)) << "\"\n";
	       else
		  std::cout << *attribute << " = \"" << escape(m->getattrv(*attribute)) << "\"\n";
	    }

	    std::cout << "\n";
	 }

      }else{
	 throw runtime_error("Error: No operation specified.\n"
			     "Use --help for more information.");
      }
   }catch(std::exception& e){
      txns.abort();
      std::cerr << e.what() << "\n";
      return 1;
   }

   return 0;
}

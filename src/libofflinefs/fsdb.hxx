#ifndef FSDB_HXX
#define FSDB_HXX

#include "common.hxx"
#include "database.hxx"

class FsDb:public Environment{
   public:
      FsDb(std::string dbroot);
      ~FsDb();

      void open();
      void close();
      // Reinitialize the databases, erasing all its contents
      void rebuild();

      // Database storing node attributes
      Database<uint64_t> nodes;

      // Database storing node subdirectories
      // Each register represents a directory, with its children
      // as attributes, mapping a child name into its node id.
      Database<uint64_t> directories;

      // Database storing medium attributes
      Database<uint32_t> media;
};
#endif

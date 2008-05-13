#ifndef DATABASE_HXX
#define DATABASE_HXX

#include <db4.5/db_cxx.h>

class Database{
      DbEnv* dbenv;
      void close();
      void open();
      void rebuild();
   public:
      Database(std::string path);
      ~Database();
      Db* nodes;
      Db* directories;
      Db* media;
};

#endif

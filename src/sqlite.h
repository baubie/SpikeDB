#ifndef SQLITE_H
#define SQLITE_H

#include <string>
#include <vector>
#include <map>
#include <sqlite3.h>

class SQLite
{
    public:
        SQLite(const char* filename);
        ~SQLite();

        bool open(const char* filename);
        std::vector< std::map<std::string,std::string> > query(const char* query);
        void close();

    private:
        sqlite3 *db;
};

#endif 

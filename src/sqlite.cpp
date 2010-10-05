#include "sqlite.h"
#include <iostream>


SQLite::SQLite(const char* filename)
{
    db = NULL;
    open(filename);
};

SQLite::~SQLite()
{
}

bool SQLite::open(const char* filename)
{
    return (sqlite3_open(filename, &db) == SQLITE_OK);
}

std::vector< std::map<std::string,std::string> > SQLite::query(const char* query)
{
    sqlite3_stmt *stmt;
    std::vector< std::map<std::string,std::string> > results;

    if(sqlite3_prepare_v2(db, query, -1, &stmt, 0) == SQLITE_OK)
    {
        int cols = sqlite3_column_count(stmt);
        int result = 0;
        while(true)
        {
            result = sqlite3_step(stmt);
            if(result == SQLITE_ROW)
            {
                std::map<std::string,std::string> values;
                for (int col = 0; col < cols; ++col)
                {
                    std::string val;
                    char *ptr = (char*)sqlite3_column_text(stmt,col);
                    if (ptr)
                    {
                        val = ptr;
                    }
                    values[(char*)sqlite3_column_name(stmt,col)] = val;
                }
                results.push_back(values);
            } else {
                break;
            }
        }
        sqlite3_finalize(stmt);
    }
    std::string error = sqlite3_errmsg(db);
//  if (error != "not an error") std::cerr << "ERROR: " << error << std::endl;
    return results;
}


void SQLite::close()
{
    sqlite3_close(db);
}

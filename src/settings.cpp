#include "stdafx.h"


#include "settings.h"


Settings::Settings()
{

#ifdef __APPLE__
	mkdir("/Library/Application Support/SpikeDB", 0755);
	Glib::ustring filename = "/Library/Application Support/SpikeDB/spikedb.settings";
#else
#ifdef linux
	struct passwd *p  = getpwuid(getuid());
	char *home = p->pw_dir;
	Glib::ustring filename(home);
	filename += "/.spikedb";
	mkdir(filename.c_str(), 0755);
	filename += "/spikedb.settings";
#else
#ifdef WIN32
	Glib::ustring filename = "spikedb.settings";
#endif
	Glib::ustring filename = "spikedb.settings";
#endif
#endif

    if (sqlite3_open(filename.c_str(), &db) != SQLITE_OK) {
        std::cerr << "CRITICAL ERROR: Unable to open settings file." << std::endl;
    }

    sqlite3_stmt *stmt=0;
    const char query[] = "CREATE TABLE IF NOT EXISTS settings (key TEXT PRIMARY KEY, value TEXT)";
    sqlite3_prepare_v2(db,query,strlen(query), &stmt, NULL);
    sqlite3_step(stmt);
	sqlite3_finalize(stmt);
}

Settings::~Settings()
{
	sqlite3_close(db);
}   

bool Settings::get_value(std::string key, std::string &value)
{
    sqlite3_stmt *stmt=0;
    const char query[] = "SELECT value FROM settings WHERE key=?";
    sqlite3_prepare_v2(db,query,strlen(query), &stmt, NULL);
    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_ROW)
	{
		value = (char*)sqlite3_column_text(stmt,0);
        sqlite3_finalize(stmt);
		return true;
	}
	sqlite3_finalize(stmt);
	return false;
}

bool Settings::get_bool(std::string key)
{
	return get_bool(key,false);
}

bool Settings::get_bool(std::string key, bool def)
{
	std::string value;
	if (get_value(key, value)) return value=="1";
	return def;
}
void Settings::set(std::string key, bool value)
{
    sqlite3_stmt *stmt=0;
    const char query[] = "INSERT OR REPLACE INTO settings (key,value) VALUES(?,?)";
    sqlite3_prepare_v2(db,query,strlen(query), &stmt, NULL);
    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, value?1:0);
    sqlite3_step(stmt);
	sqlite3_finalize(stmt);
}

std::string Settings::get_string(std::string key)
{
	return get_string(key,"");
}

std::string Settings::get_string(std::string key, std::string def)
{
	std::string value;
	if (get_value(key, value)) return value;
	return def;
}
void Settings::set(std::string key, std::string value)
{
    sqlite3_stmt *stmt=0;
    const char query[] = "INSERT OR REPLACE INTO settings (key,value) VALUES(?,?)";
    sqlite3_prepare_v2(db,query,strlen(query), &stmt, NULL);
    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, value.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
	sqlite3_finalize(stmt);
}

int Settings::get_int(std::string key)
{
	return get_int(key,0);
}
int Settings::get_int(std::string key, int def)
{
	std::string value;
	if (get_value(key, value)) return atoi(value.c_str());
	return def;
}
void Settings::set(std::string key, int value)
{
    sqlite3_stmt *stmt=0;
    const char query[] = "INSERT OR REPLACE INTO settings (key,value) VALUES(?,?)";
    sqlite3_prepare_v2(db,query,strlen(query), &stmt, NULL);
    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, value);
    sqlite3_step(stmt);
	sqlite3_finalize(stmt);
}

double Settings::get_double(std::string key)
{
	return get_double(key,0);
}
double Settings::get_double(std::string key, double def)
{
	std::string value;
	if (get_value(key, value)) return atof(value.c_str());
	return def;
}
void Settings::set(std::string key, double value)
{
    sqlite3_stmt *stmt=0;
    const char query[] = "INSERT OR REPLACE INTO settings (key,value) VALUES(?,?)";
    sqlite3_prepare_v2(db,query,strlen(query), &stmt, NULL);
    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 2, value);
    sqlite3_step(stmt);
	sqlite3_finalize(stmt);
}

float Settings::get_float(std::string key)
{
	return get_float(key,0);
}
	
float Settings::get_float(std::string key, float def)
{
	std::string value;
	if (get_value(key, value)) return (float)atof(value.c_str());
	return def;
}
void Settings::set(std::string key, float value)
{
    sqlite3_stmt *stmt=0;
    const char query[] = "INSERT OR REPLACE INTO settings (key,value) VALUES(?,?)";
    sqlite3_prepare_v2(db,query,strlen(query), &stmt, NULL);
    sqlite3_bind_text(stmt, 1, key.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 2, value);
    sqlite3_step(stmt);
	sqlite3_finalize(stmt);
}


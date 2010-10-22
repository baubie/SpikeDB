#include "settings.h"


Settings::Settings()
{
    if (sqlite3_open("spikedb.settings", &db) != SQLITE_OK) {
        std::cerr << "CRITICAL ERROR: Unable to open settings file." << std::endl;
    }
}

std::string get_value(std::string key)
{

}

bool Settings::get_bool(std::string key)
{
	return get_bool(key,false);
}

bool Settings::get_bool(std::string key, bool default)
{
}

std::string Settings::get_string(std::string key)
{
	return get_string(key,"");
}

std::string Settings::get_string(std::string key, std::string default)
{
}

int Settings::get_int(std::string key)
{
	return get_int(key,0);
}
int Settings::get_int(std::string key, int default)
{
}

double Settings::get_double(std::string key)
{
	return get_double(key,0);
}
double Settings::get_double(std::string key, double default)
{
}

float Settings::get_float(std::string key)
{
	return get_float(key,0);
}
	
float Settings::get_float(std::string key, float default)
{

}


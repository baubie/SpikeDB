#ifndef SETTINGS_H
#define SETTINGS_H

#include <sqlite3.h>
#include <string>


class Settings {

	public:

    	Settings();

		bool get_bool(std::string key);
		bool get_bool(std::string key, bool default);

		std::string get_string(std::string key);
		std::string get_string(std::string key, std::string default);

		int get_int(std::string key);
		int get_int(std::string key, int default);

		double get_double(std::string key);
		double get_double(std::string key, double default);

		float get_float(std::string key);
		float get_float(std::string key, float default);

	private:
        // SQLite Database
        sqlite3 *db;
		std::string get_value(std::string key);

};

#endif

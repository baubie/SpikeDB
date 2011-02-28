#ifndef SETTINGS_H
#define SETTINGS_H

#include <sqlite3.h>
#include <string>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <gtkmm.h>

#ifdef __APPLE__
#include <sys/stat.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

class Settings {

	public:

    	Settings();
		~Settings();

		bool get_bool(std::string key);
		bool get_bool(std::string key, bool def);
		void set(std::string key, bool value);

		std::string get_string(std::string key);
		std::string get_string(std::string key, std::string def);
		void set(std::string key, std::string value);

		int get_int(std::string key);
		int get_int(std::string key, int def);
		void set(std::string key, int value);

		double get_double(std::string key);
		double get_double(std::string key, double def);
		void set(std::string key, double value);

		float get_float(std::string key);
		float get_float(std::string key, float def);
		void set(std::string key, float value);

	private:
        // SQLite Database
        sqlite3 *db;
		bool get_value(std::string key, std::string &value);
};

#endif

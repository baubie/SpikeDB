/*
Copyright (c) 2011-2012, Brandon Aubie
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef SETTINGS_H
#define SETTINGS_H

#include "sqlite3.h"
#include <string>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <gtkmm.h>

#ifdef __APPLE__
#include <sys/stat.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

#ifdef linux
#include <sys/stat.h>
#include <pwd.h>
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

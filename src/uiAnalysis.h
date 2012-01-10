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

#ifndef UIANALYSIS_H
#define UIANALYSIS_H

#ifdef WIN32
#include <Windows.h>
#endif


#include "pySpikeDB.h"
#include <gtkmm.h>
#include <sqlite3.h>
#include <vector>
#include <map>
#include <algorithm>
#include "spikedata.h"
#include "uiFileDetailsTreeView.h"
#include "easyplotmm/easyplotmm.h"
#include "settings.h"
#include "animalColumns.h"
#include <dirent.h>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

#ifdef linux
#include <pwd.h>
#endif

class uiAnalysis : public Gtk::VBox {

	public:
		uiAnalysis(sqlite3 **db, Gtk::Notebook* notebook, uiFileDetailsTreeView* fileDetailsTree, Gtk::TreeView* animalTree, AnimalColumns* animalColumns, Gtk::Statusbar* statusbar, bool compact, Settings *settings, Gtk::Window* parent);
		virtual ~uiAnalysis();

		EasyPlotmm* getPlot();
		void runPlugin();
		void forceSpikesAbs(double begin, double end);

		/**
		 * Python print for window
		 */
		void print(const std::string& s);

		std::vector<std::pair<Glib::ustring, Glib::ustring> > plugins;

	protected:

		/**
		 * Member variables
		 */
		sqlite3 **db;
		uiFileDetailsTreeView* mp_FileDetailsTree;
		Gtk::Statusbar* mp_StatusBar;
		Gtk::TreeView* mp_AnimalTree;
		Gtk::Notebook* mp_Notebook;
		AnimalColumns* mp_AnimalColumns;
		Gtk::Window* mp_parent;
		Glib::ustring m_filename;
		bool compact;
		Settings *settings;
		float forceAbsBegin, forceAbsEnd;
		std::map<std::pair<std::string,std::string>, double> savedNumberOptions;
		std::map<std::pair<std::string,std::string>, bool> savedCheckboxOptions;

		/**
		 * Child widgets
		 */  
		Gtk::TextView *tvOutput;
		Glib::RefPtr<Gtk::TextBuffer> mrp_tbOutput;
		Gtk::ToolButton *tbOpen;
		Gtk::ToolButton *tbRun;
		Gtk::ComboBoxText *tbPlugins;
		Gtk::ToolButton *tbOptions;
        EasyPlotmm* mp_plot;

		/**
		 * Signal handlers
		 */
		void on_open_clicked();
		void on_run_clicked();
		void on_plugin_changed();
		void on_showerr_clicked();
		void on_options_clicked();
		void on_data_point_clicked(const double &x, const double &y, const std::string &name, const std::string &data);
		void on_hovered_on_point(const double &x, const double &y, const std::string &name, const std::string &data);
		void on_moved_off_point();

		/**
		 * Helper functions
		 */
		void addOutput(Glib::ustring t);
		void runScript(bool showAdvanced, const Glib::ustring &plugin = "");
		void initPlugins();

	private:
		static bool setupPython;

};


#endif

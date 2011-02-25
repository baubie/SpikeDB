#ifndef UIANALYSIS_H
#define UIANALYSIS_H

#include "pySpikeDB.h"
#include <gtkmm.h>
#include <sqlite3.h>
#include <vector>
#include "spikedata.h"
#include "uiFileDetailsTreeView.h"
#include "easyplotmm/easyplotmm.h"
#include "settings.h"

class uiAnalysis : public Gtk::VBox {

	public:
		uiAnalysis(sqlite3 **db, uiFileDetailsTreeView* fileDetailsTree, bool compact, Settings *settings, Gtk::Window* parent=NULL);
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
		Gtk::Window* m_parent;
		Glib::ustring m_filename;
		bool compact;
		Settings *settings;
		float forceAbsBegin, forceAbsEnd;

		/**
		 * Child widgets
		 */  
		Gtk::TextView *tvOutput;
		Glib::RefPtr<Gtk::TextBuffer> mrp_tbOutput;
		Gtk::ToolButton *tbOpen;
		Gtk::ToolButton *tbRun;
		Gtk::ComboBoxText *tbPlugins;
		Gtk::CheckButton *tbShowErr; 
        EasyPlotmm* mp_plot;

		/**
		 * Signal handlers
		 */
		void on_open_clicked();
		void on_run_clicked();
		void on_plugin_changed();
		void on_showerr_clicked();

		/**
		 * Helper functions
		 */
		void addOutput(Glib::ustring t);
		void runScript(const Glib::ustring &plugin = "");
		void initPlugins();

	private:

};

#endif

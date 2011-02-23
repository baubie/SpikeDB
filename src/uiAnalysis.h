#ifndef UIANALYSIS_H
#define UIANALYSIS_H

#include "pySpikeDB.h"
#include <gtkmm.h>
#include <sqlite3.h>
#include "spikedata.h"
#include "uiFileDetailsTreeView.h"
#include "easyplotmm/easyplotmm.h"

class uiAnalysis : public Gtk::VBox {

	public:
		uiAnalysis(sqlite3 **db, uiFileDetailsTreeView* fileDetailsTree, bool compact, Gtk::Window* parent=NULL);
		virtual ~uiAnalysis();

	protected:

		/**
		 * Member variables
		 */
		sqlite3 **db;
		uiFileDetailsTreeView* mp_FileDetailsTree;
		Gtk::Window* m_parent;
		Glib::ustring m_filename;

		/**
		 * Child widgets
		 */  
		Gtk::TextView *tvOutput;
		Glib::RefPtr<Gtk::TextBuffer> mrp_tbOutput;
		Gtk::ToolButton *tbOpen;
		Gtk::ToolButton *tbRun;
        EasyPlotmm* mp_plot;

		/**
		 * Signal handlers
		 */
		void on_open_clicked();
		void on_run_clicked();

		/**
		 * Helper functions
		 */
		void runScript();
		void addOutput(Glib::ustring t);

	private:

};

#endif

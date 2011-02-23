#ifndef UIANALYSIS_H
#define UIANALYSIS_H

#ifdef __APPLE__
#include <Python/Python.h>
#else
#include <Python.h>
#endif

#include <set>
#include <vector>
#include <gtkmm.h>
#include <sqlite3.h>
#include "spikedata.h"
#include "uiFileDetailsTreeView.h"
#include "easyplotmm/easyplotmm.h"


/**
 * Python setup functions
 */
PyObject* buildCellList();
PyObject* buildFileList(PyObject *self, PyObject *args);


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
        EasyPlotmm* mp_Plot;

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



		/**
		 * Used to find unique cells.
		 */
		struct CellID {
			Glib::ustring animalID;
			int cellID;

			// Overload the < operator for easy comparison
			friend bool operator<(CellID const& a, CellID const& b)
			{
				if (a.animalID.compare(b.animalID) == 0)
					return a.cellID < b.cellID;
				else
					return a.animalID.compare(b.animalID) < 0;
			}
		};

	private:

};

#endif

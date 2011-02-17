#ifndef UIANALYSIS_H
#define UIANALYSIS_H

#ifdef __APPLE__
#include <Python/Python.h>
#else
#include <Python.h>
#endif

#include <set>
#include <string.h>
#include <gtkmm.h>
#include "uiFileDetailsTreeView.h"

class uiAnalysis : public Gtk::VBox {

	public:
		uiAnalysis(uiFileDetailsTreeView* fileDetailsTree, Gtk::Window* parent=NULL);
		virtual ~uiAnalysis();

	protected:

		/**
		 * Member variables
		 */
		uiFileDetailsTreeView* mp_FileDetailsTree;
		Gtk::Window* m_parent;
		Glib::ustring m_filename;

		/**
		 * Child widgets
		 */
		Glib::RefPtr<Gtk::TextBuffer> mrp_tbOutput;
		Gtk::ToolButton *tbRun;

		/**
		 * Signal handlers
		 */
		void on_open_clicked();
		void on_run_clicked();

		/**
		 * Helper functions
		 */
		void runScript();


		/**
		 * Python setup functions
		 */
		PyObject* buildCellList();

		/**
		 * Used to find unique cells.
		 */
		struct CellID {
			Glib::ustring animalID;
			int cellID;

			// Overload the < operator for easy comparison
			friend bool operator<(CellID const& a, CellID const& b)
			{
				if (strcmp(a.animalID.c_str(), b.animalID.c_str()) == 0)
					return a.cellID < b.cellID;
				else
					return strcmp(a.animalID.c_str(), b.animalID.c_str()) < 0;
			}
		};
};

#endif

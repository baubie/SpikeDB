#ifndef UIANALYSIS_H
#define UIANALYSIS_H

#include <Python.h>
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
};

#endif

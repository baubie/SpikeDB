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

#ifndef GUI_H
#define GUI_H

#define CURRENT_DB_VERSION 1.5
#define CURRENT_VERSION 1.5


#include "uiMenuBar.h"
#include "uiAnalysis.h"
#include "uiFileDetailsTreeView.h"
#include "uiFilterFrame.h"
#include "uiPropTable.h"
#include "uiTags.h"
#include "easyplotmm/easyplotmm.h"
#include "spikedata.h"
#include "animalColumns.h"

#include <gtkmm.h>
#include <giomm.h>
#include <string>
#include <vector>
#include <dirent.h>
#
#include <sqlite3.h>
#include "settings.h"
#include "tokenize.h"
#include <dirent.h>
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif


/*
 * Notes on Memory Management
 * If we are creating something that we don't need to access
 * use Gtk::Manage() to make it and not bother with member variables.
 * If you can, just use a member variable.
 */



struct CellID
{
	Glib::ustring animalID;
	int cellID;
};

struct FileID
{
	Glib::ustring animalID;
	int cellID;
	int fileID;
};


class GUI : public Gtk::Window
{
    public:
        GUI();
        virtual ~GUI();

        sqlite3 *db; /**< Pointer to out SQLite3 database. */
		Settings settings; /**< Settings object. */

    protected:

		/**
		 * Member variables
		 */
		bool uiReady; /**< When FALSE, block UI updates. */
        std::string curXVariable;
		Glib::ustring m_curAnimalID;
		int m_curCellID;
		int m_curFileNum;


		/**
		 * Initialization
		 */
		void init_gui();
		void init_menu();
		void version_check();
		double database_version();

		/**
		 * Widgets
		 */
		uiFilterFrame* mp_uiFilterFrame; /**< Filter widgets in top left corner. */
		Gtk::TreeView* mp_AnimalsTree;
		uiMenuBar* mp_MenuBar;
		Gtk::MenuItem m_Menu_Import;

		Gtk::HPaned *hpRight;
		uiPropTable<Glib::ustring> m_uiAnimalDetails; /**< Animal details property table. */
		uiTags m_AnimalTags;
		uiPropTable<CellID> m_uiCellDetails; /**< Cell details property table. */
		uiTags m_CellTags;
		Gtk::CheckButton* mp_BadCell;
		uiPropTable<FileID> m_uiFileDetails; /**< Animal details property table. */
		uiTags m_FileTags;

		Gtk::ImageMenuItem* mp_MenuImportFolder; /**< Import menu item. Require access to enable/disable it. */
		uiAnalysis* mp_Analysis;
        uiFileDetailsTreeView* mp_FileDetailsTree;
		Gtk::Statusbar* mp_Statusbar;
        Gtk::HBox* mp_HBoxPlots;
        EasyPlotmm* mp_PlotSpikes;
		uiAnalysis* mp_QuickAnalysis;
		Gtk::Statusbar* mp_statusbar;
		Gtk::Notebook *mp_Notebook;

        Glib::RefPtr<Gtk::TreeStore> mrp_AnimalTree;
        Glib::RefPtr<Gtk::TreeSelection> mrp_AnimalSelection;
        Glib::RefPtr<Gtk::ListStore> mrp_CellDetailsList;
        Glib::RefPtr<Gtk::ListStore> mrp_MeanType;

		Glib::RefPtr<Gdk::Pixbuf> mrp_pbIcon;


		/**
		 * Helper functions
		 */
		void updateTagCompletion();
		bool openDatabase(std::string filename);
	    void importSpikeFile(std::string filename, char* d_name);
        void populateAnimalTree();
        void populateDetailsList(const Glib::ustring animalID, const int cellID);
        void populateAnimalDetailsList(const Glib::ustring animalID);
        void populateCellDetailsList(const Glib::ustring animalID, const int cellID);
        void populateFileDetailsList(const Glib::ustring animalID, const int cellID, const int fileID);
        void changeAnimalSelection();
        void addFileToPlot(const Gtk::TreeModel::iterator& iter);
		void updateSideLists(const Gtk::TreeModel::iterator& iter);
		void getFilesStatement(sqlite3_stmt **stmt, const Glib::ustring animalID, const int cellID);




		/**
		 * Signal handling
		 */
		void on_bad_cell_toggle();
		void on_plotspikes_zoom_changed(double begin, double end);
        void on_menuNewDatabase_activate(); /**< Handle the New Database menu item. */
        void on_menuOpenDatabase_activate(); /**< Handle the Open Database menu item. */
        void on_menuImportFolder_activate(); /**< Handle the Import Folder menu item. */
        void on_menuQuit_activate(); /**< Handle the Quit menu item. */
		void on_menuAbout_activate();
		void on_filter_changed(); /**< Handle when the filter changes in any way. */
        void on_filedetails_selection_changed();
		void on_animal_tag_deleted(Glib::ustring tag);
		void on_cell_tag_deleted(Glib::ustring tag);
		void on_file_tag_deleted(Glib::ustring tag);
		bool on_animal_tag_added(Glib::ustring tag);
		bool on_cell_tag_added(Glib::ustring tag);
		bool on_file_tag_added(Glib::ustring tag);
		void on_filedetails_set_hidden(bool hidden);
        int on_animal_sort(const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b);
		void on_animaldetails_edited(
				Glib::ustring ID,
				Glib::ustring name,
				Glib::ustring oldvalue,
				Glib::ustring newvalue,
				uiPropTableRowType type);
		void on_celldetails_edited(
				CellID ID,
				Glib::ustring name,
				Glib::ustring oldvalue,
				Glib::ustring newvalue,
				uiPropTableRowType type);

		void on_filedetails_edited(
				FileID ID,
				Glib::ustring name,
				Glib::ustring oldvalue,
				Glib::ustring newvalue,
				uiPropTableRowType type);


		AnimalColumns m_AnimalColumns;	


};


#endif

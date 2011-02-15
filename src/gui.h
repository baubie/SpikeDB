#ifndef GUI_H
#define GUI_H

#define CURRENT_DB_VERSION 1.2
#define CURRENT_VERSION 1.2

#include <gtkmm.h>
#include <giomm.h>
#include <string>
#include <vector>
#include <dirent.h>
#include <sqlite3.h>
#include "easyplotmm/easyplotmm.h"
#include "spikedata.h"
#include "settings.h"
#include "tokenize.h"

#include "uiFilterFrame.h"
#include "uiPropTable.h"
#include "uiTags.h"


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


class GUI : public Gtk::Window
{
    public:
        GUI(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
        virtual ~GUI();

        sqlite3 *db; /**< Pointer to out SQLite3 database. */


    protected:

		bool uiReady; /**< When FALSE, block UI updates. */

		void init_toolbar(); /**< Initialize the toolbar. */

		void updateTagCompletion();

        void on_menuNewDatabase_activate(); /**< Handle the New Database menu item. */
        void on_menuOpenDatabase_activate(); /**< Handle the Open Database menu item. */
        void on_menuImportFolder_activate(); /**< Handle the Import Folder menu item. */
        void on_menuQuit_activate(); /**< Handle the Quit menu item. */
		void on_analyze_changed(); /**< Handle when the analyze parameters change. */
		void on_meantype_changed(); /**< Handle when the type of mean plot changes. */ 
		void on_filter_changed(); /**< Handle when the filter changes in any way. */
		void on_animal_tag_deleted(Glib::ustring tag);
		void on_cell_tag_deleted(Glib::ustring tag);
		bool on_animal_tag_added(Glib::ustring tag);
		bool on_cell_tag_added(Glib::ustring tag);

		/** 
		 * Handle when a row in the animal details property table is edited. 
		 * */
		void on_animaldetails_edited(
				Glib::ustring ID,
				Glib::ustring name,
				Glib::ustring oldvalue,
				Glib::ustring newvalue,
				uiPropTableRowType type);

		/** 
		 * Handle when a row in the cell details property table is edited. 
		 * */
		void on_celldetails_edited(
				CellID ID,
				Glib::ustring name,
				Glib::ustring oldvalue,
				Glib::ustring newvalue,
				uiPropTableRowType type);

        Glib::RefPtr<Gtk::Builder> mrp_Glade; /**< Reference to Glade file. */ 
		Settings settings; /**< Settings object. */

		uiFilterFrame m_uiFilterFrame; /**< Filter widgets in top left corner. */
		uiPropTable<Glib::ustring> m_uiAnimalDetails; /**< Animal details property table. */
		uiTags m_AnimalTags;
		uiPropTable<CellID> m_uiCellDetails; /**< Cell details property table. */
		uiTags m_CellTags;


		Gtk::ImageMenuItem* mp_MenuImportFolder; /**< Import menu item. Require access to enable/disable it. */


        Gtk::TreeView* mp_AnimalsTree;
        Gtk::TreeView* mp_FilesDetailsTree;
		Gtk::Statusbar* mp_Statusbar;
        Gtk::HBox* mp_HBoxPlots;
        Gtk::VBox* mp_VBoxAnalyze;
		Gtk::ComboBox* mp_MeanType;
		Gtk::ComboBox* mp_DataSource;
		Gtk::ComboBox* mp_XVar;
		Gtk::ComboBox* mp_YVar;


        // Child Widgets (created in c++)
        Glib::RefPtr<Gtk::TreeStore> mrp_AnimalTree;
        Glib::RefPtr<Gtk::ListStore> mrp_DetailsList;
        Glib::RefPtr<Gtk::TreeSelection> mrp_AnimalSelection;
        Glib::RefPtr<Gtk::TreeSelection> mrp_DetailsSelection;
        Glib::RefPtr<Gtk::ListStore> mrp_CellDetailsList;
        Glib::RefPtr<Gtk::ListStore> mrp_DataSource;
        Glib::RefPtr<Gtk::ListStore> mrp_XVar;
        Glib::RefPtr<Gtk::ListStore> mrp_YVar;
        Glib::RefPtr<Gtk::ListStore> mrp_MeanType;
        Glib::RefPtr<Gtk::ListStore> mrp_TypeFilter;

        // Plots
        EasyPlotmm* mp_PlotSpikes;
        EasyPlotmm* mp_PlotMeans;
        EasyPlotmm* mp_PlotAnalyze;


		// Models for the Data Source combobox
		class AnalyzeColumns : public Gtk::TreeModel::ColumnRecord
		{
        	public:
				AnalyzeColumns()
				{ add(m_col_name); }
                Gtk::TreeModelColumn<Glib::ustring> m_col_name;
		};

        // Animal Tree Model Columns
        class AnimalColumns : public Gtk::TreeModel::ColumnRecord
        {
            public:
                AnimalColumns()
                { add(m_col_name); }
                Gtk::TreeModelColumn<Glib::ustring> m_col_name;
        };

        // Files Details Tree Model Columns
        class FilesDetailsColumns : public Gtk::TreeModel::ColumnRecord
        {
            public:
                FilesDetailsColumns()
                { add(m_col_time); 
				  add(m_col_animalID); add(m_col_cellID); add(m_col_filenum); add(m_col_xaxis); 
                  add(m_col_type); add(m_col_freq); add(m_col_trials); add(m_col_onset); 
				  add(m_col_dur); add(m_col_atten); add(m_col_tags); 
                }
				Gtk::TreeModelColumn<glong> m_col_time;
                Gtk::TreeModelColumn<Glib::ustring> m_col_animalID;
                Gtk::TreeModelColumn<int> m_col_cellID;
                Gtk::TreeModelColumn<int> m_col_filenum;
                Gtk::TreeModelColumn<Glib::ustring> m_col_xaxis;
                Gtk::TreeModelColumn<Glib::ustring> m_col_type;
                Gtk::TreeModelColumn<Glib::ustring> m_col_freq;
                Gtk::TreeModelColumn<int> m_col_trials;
                Gtk::TreeModelColumn<Glib::ustring> m_col_dur;
                Gtk::TreeModelColumn<Glib::ustring> m_col_onset;
                Gtk::TreeModelColumn<Glib::ustring> m_col_atten;
                Gtk::TreeModelColumn<Glib::ustring> m_col_tags;
        };
        FilesDetailsColumns m_FilesDetailsColumns;

        AnimalColumns m_AnimalColumns;
		AnalyzeColumns m_DataSourceColumns;
		AnalyzeColumns m_XVarColumns;
		AnalyzeColumns m_YVarColumns;
		AnalyzeColumns m_MeanTypeColumns;

		bool openDatabase(std::string filename);
	    void importSpikeFile(std::string filename, char* d_name);
        void populateAnimalTree();
        void populateDetailsList(const Glib::ustring animalID, const int cellID);
        void populateAnimalDetailsList(const Glib::ustring animalID);
        void populateCellDetailsList(const Glib::ustring animalID, const int cellID);
        void changeAnimalSelection();
        void changeDetailsSelection();
        void addFileToPlot(const Gtk::TreeModel::iterator& iter);
		void updateSideLists(const Gtk::TreeModel::iterator& iter);
		void updateAnalyzePlot();

		void getFilesStatement(sqlite3_stmt **stmt, const Glib::ustring animalID, const int cellID);
		void getCellsStatement(sqlite3_stmt **stmt, const Glib::ustring animalID, const int cellID);
        
        // Helper to sort by string for parent and number by child
        int on_animal_sort(const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b);

        std::string curXVariable;

		Glib::ustring m_curAnimalID;
		int m_curCellID;
		int m_curFileNum;
};


#endif

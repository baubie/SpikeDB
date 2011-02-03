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

class GUI : public Gtk::Window
{
    public:
        GUI(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
        virtual ~GUI();

        // SQLite Database
        sqlite3 *db;


    protected:


        /**
		 * Signal handlers
		 */
        void on_menuNewDatabase_activate();
        void on_menuOpenDatabase_activate();
        void on_menuImportFolder_activate();
        void on_menuQuit_activate();
		void on_analyze_changed();
		void on_meantype_changed();


		/**
		 * Reference to Glade file
		 */
        Glib::RefPtr<Gtk::Builder> m_refGlade;


		/** 
		 * Settings object
		 */
		Settings settings;


		/**
		 * uiReady will not update the UI until the 
		 * construtor is done.
		 */
		bool uiReady;


		/**
		 * uiFilterFrame
		 * Top left corner frame with filter widgets
		 */
		uiFilterFrame m_uiFilterFrame; 


		/**
		 * uiPropTable
		 */
		uiPropTable m_uiAnimalDetails;
		Gtk::Alignment* mp_AlignAnimalDetails;

		 

        Gtk::ImageMenuItem* mp_MenuImportFolder;
        Gtk::ImageMenuItem* mp_MenuQuit;
        Gtk::ImageMenuItem* mp_MenuOpenDatabase;
        Gtk::ImageMenuItem* mp_MenuNewDatabase;
        Gtk::TreeView* mp_AnimalTree;
        Gtk::TreeView* mp_DetailsList;
        Gtk::HBox* mp_HBoxPlots;
		Gtk::Statusbar* mp_Statusbar;
        Gtk::VBox* mp_VBoxAnalyze;
		Gtk::ComboBox* mp_MeanType;
		Gtk::ComboBox* mp_DataSource;
		Gtk::ComboBox* mp_XVar;
		Gtk::ComboBox* mp_YVar;


        // Child Widgets (created in c++)
        Glib::RefPtr<Gtk::TreeStore> m_refAnimalTree;
        Glib::RefPtr<Gtk::ListStore> m_refDetailsList;
        Glib::RefPtr<Gtk::TreeSelection> m_refAnimalSelection;
        Glib::RefPtr<Gtk::TreeSelection> m_refDetailsSelection;
        Glib::RefPtr<Gtk::ListStore> m_refCellDetailsList;
        Glib::RefPtr<Gtk::ListStore> m_refDataSource;
        Glib::RefPtr<Gtk::ListStore> m_refXVar;
        Glib::RefPtr<Gtk::ListStore> m_refYVar;
        Glib::RefPtr<Gtk::ListStore> m_refMeanType;
        Glib::RefPtr<Gtk::ListStore> m_refTypeFilter;

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

        // Details Tree Model Columns
        class DetailsColumns : public Gtk::TreeModel::ColumnRecord
        {
            public:
                DetailsColumns()
                { add(m_col_animalID); add(m_col_cellID); add(m_col_filenum); add(m_col_xaxis); 
                  add(m_col_type); add(m_col_freq); add(m_col_trials); add(m_col_onset); add(m_col_dur); add(m_col_atten); 
                  add(m_col_tags); 
                }
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
        Gtk::CellRendererText m_rend_filetags;
        Gtk::TreeViewColumn m_tvcol_filetags;
        void on_filetags_edited(const Glib::ustring& path_string, const Glib::ustring& new_text);
        void filetags_cell_data(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);


        AnimalColumns m_AnimalColumns;
        DetailsColumns m_DetailsColumns;

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
		void updateFilter();
		void updateAnalyzePlot();

		void getFilesStatement(sqlite3_stmt **stmt, const Glib::ustring animalID, const int cellID);
		void getCellsStatement(sqlite3_stmt **stmt, const Glib::ustring animalID, const int cellID);
        
        // Helper to sort by string for parent and number by child
        int on_animal_sort(const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b);

    private:
        std::string curXVariable;
};


#endif

#ifndef GUI_H
#define GUI_H

#include <gtkmm.h>
#include <string>
#include <dirent.h>
#include <sqlite3.h>
#include "easyplotmm/easyplotmm.h"
#include "spikedata.h"

class GUI : public Gtk::Window
{
    public:
        GUI(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
        virtual ~GUI();

    protected:

        // SQLite Database
        sqlite3 *db;

        // Signal handlers
        void on_menuQuit_activate();
        void on_menuImportFolder_activate();

        // Child Widgets (from glade file)
        Glib::RefPtr<Gtk::Builder> m_refGlade;
        Gtk::ImageMenuItem* m_pMenuImportFolder;
        Gtk::ImageMenuItem* m_pMenuQuit;
        Gtk::TreeView* m_pAnimalTree;
        Gtk::TreeView* m_pDetailsList;
        Gtk::TreeView* m_pAnimalDetailsList;
        Gtk::TreeView* m_pCellDetailsList;
        Gtk::HBox* m_pHBoxPlots;

        // Child Widgets (created in c++)
        Glib::RefPtr<Gtk::TreeStore> m_refAnimalTree;
        Glib::RefPtr<Gtk::ListStore> m_refDetailsList;
        Glib::RefPtr<Gtk::TreeSelection> m_refAnimalSelection;
        Glib::RefPtr<Gtk::TreeSelection> m_refDetailsSelection;
        Glib::RefPtr<Gtk::ListStore> m_refAnimalDetailsList;
        Glib::RefPtr<Gtk::ListStore> m_refCellDetailsList;
        EasyPlotmm* m_pPlotSpikes;
        EasyPlotmm* m_pPlotMeans;

        // Network Tree Model Columns
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
                  add(m_col_type); add(m_col_freq); add(m_col_dur); add(m_col_atten); add(m_col_tags); 
                }
                Gtk::TreeModelColumn<Glib::ustring> m_col_animalID;
                Gtk::TreeModelColumn<int> m_col_cellID;
                Gtk::TreeModelColumn<int> m_col_filenum;
                Gtk::TreeModelColumn<Glib::ustring> m_col_xaxis;
                Gtk::TreeModelColumn<Glib::ustring> m_col_type;
                Gtk::TreeModelColumn<Glib::ustring> m_col_freq;
                Gtk::TreeModelColumn<Glib::ustring> m_col_dur;
                Gtk::TreeModelColumn<Glib::ustring> m_col_atten;
                Gtk::TreeModelColumn<Glib::ustring> m_col_tags;
        };
        Gtk::CellRendererText m_rend_filetags;
        Gtk::TreeViewColumn m_tvcol_filetags;
        void on_filetags_edited(const Glib::ustring& path_string, const Glib::ustring& new_text);
        void filetags_cell_data(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);

        class AnimalDetailsColumns : public Gtk::TreeModel::ColumnRecord
        {
            public:
                AnimalDetailsColumns()
                { add(m_col_animalID); add(m_col_name); add(m_col_value); }
                Gtk::TreeModelColumn<Glib::ustring> m_col_animalID;
                Gtk::TreeModelColumn<Glib::ustring> m_col_name;
                Gtk::TreeModelColumn<Glib::ustring> m_col_value;
        };
        Gtk::CellRendererText m_rend_animalvalue;
        Gtk::TreeViewColumn m_tvcol_animalvalue;
        void on_animalvalue_edited(const Glib::ustring& path_string, const Glib::ustring& new_text);
        void animalvalue_cell_data(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);

        class CellDetailsColumns : public Gtk::TreeModel::ColumnRecord
        {
            public:
                CellDetailsColumns()
                { add(m_col_animalID); add(m_col_cellID); add(m_col_name); add(m_col_value); }
                Gtk::TreeModelColumn<Glib::ustring> m_col_animalID;
                Gtk::TreeModelColumn<int> m_col_cellID;
                Gtk::TreeModelColumn<Glib::ustring> m_col_name;
                Gtk::TreeModelColumn<Glib::ustring> m_col_value;
        };
        Gtk::CellRendererText m_rend_cellvalue;
        Gtk::TreeViewColumn m_tvcol_cellvalue;
        void on_cellvalue_edited(const Glib::ustring& path_string, const Glib::ustring& new_text);
        void cellvalue_cell_data(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);

        AnimalColumns m_AnimalColumns;
        DetailsColumns m_DetailsColumns;
        AnimalDetailsColumns m_AnimalDetailsColumns;
        CellDetailsColumns m_CellDetailsColumns;

        void populateAnimalTree();
        void populateDetailsList(const Glib::ustring animalID, const int cellID);
        void populateAnimalDetailsList(const Glib::ustring animalID);
        void populateCellDetailsList(const Glib::ustring animalID, const int cellID);
        void changeAnimalSelection();
        void changeDetailsSelection();
        void addFileToPlot(const Gtk::TreeModel::iterator& iter);
	void updateSideLists(const Gtk::TreeModel::iterator& iter);
        
        // Helper to sort by string for parent and number by child
        int on_animal_sort(const Gtk::TreeModel::iterator& a, const Gtk::TreeModel::iterator& b);

    private:
        std::string curXVariable;
};


#endif

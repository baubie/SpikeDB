#ifndef GUI_H
#define GUI_H

#include <gtkmm.h>
#include <plotmm/plot.h>
#include <plotmm/scalediv.h>
#include <plotmm/errorcurve.h>
#include <plotmm/symbol.h>
#include <plotmm/paint.h>
#include "spikedata.h"

class GUI : public Gtk::Window
{
    public:
        GUI(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade);
        virtual ~GUI();

    protected:

        // Signal handlers
        void on_menuQuit_activate();

        // Child Widgets (from glade file)
        Glib::RefPtr<Gtk::Builder> m_refGlade;
        Gtk::ImageMenuItem* m_pMenuQuit;
        Gtk::TreeView* m_pAnimalTree;
        Gtk::TreeView* m_pDetailsList;

        // Child Widgets (created in c++)
        std::vector<PlotMM::Plot*> m_pPlot;
        Glib::RefPtr<Gtk::TreeStore> m_refAnimalTree;
        Glib::RefPtr<Gtk::ListStore> m_refDetailsList;
        Glib::RefPtr<Gtk::TreeSelection> m_refAnimalSelection;
        Glib::RefPtr<Gtk::TreeSelection> m_refDetailsSelection;

        // Network Tree Model Columns
        class AnimalColumns : public Gtk::TreeModel::ColumnRecord
        {
            public:
                AnimalColumns()
                { add(m_col_name); }
                Gtk::TreeModelColumn<Glib::ustring> m_col_name;
        };

        class DetailsColumns : public Gtk::TreeModel::ColumnRecord
        {
            public:
                DetailsColumns()
                { add(m_col_filenum); add(m_col_CF); add(m_col_depth); add(m_col_xaxis); 
                  add(m_col_tags); 
                }
                Gtk::TreeModelColumn<int> m_col_filenum;
                Gtk::TreeModelColumn<int> m_col_CF;
                Gtk::TreeModelColumn<int> m_col_depth;
                Gtk::TreeModelColumn<Glib::ustring> m_col_xaxis;
                Gtk::TreeModelColumn<Glib::ustring> m_col_tags;
        };

        AnimalColumns m_AnimalColumns;
        DetailsColumns m_DetailsColumns;

    private:
        void deletePlots();
        void populateAnimalTree();
        void populateDetailsList();
};


#endif

#ifndef UIFILEDETAILSTREEVIEW_H
#define UIFILEDETAILSTREEVIEW_H

#include <gtkmm.h>
#include <vector>
#include <sqlite3.h>
#include "spikedata.h"
#include "uiTags.h"

class uiFileDetailsTreeView : public Gtk::TreeView {
	
	public:
		uiFileDetailsTreeView(sqlite3** db, Gtk::Window *parent);
        virtual ~uiFileDetailsTreeView();
		Glib::RefPtr<Gtk::TreeSelection> treeSelection();
		Gtk::TreeModel::Row	newrow();

		Glib::ustring animalID(const Gtk::TreeModel::iterator& iter);
		int cellID(const Gtk::TreeModel::iterator& iter);
		int fileID(const Gtk::TreeModel::iterator& iter);

		void clear();

        // Files Details Tree Model Columns
        class Columns : public Gtk::TreeModel::ColumnRecord
        {
            public:
                Columns()
                { 
				  add(m_col_hidden); add(m_col_time); add(m_col_props);
				  add(m_col_animalID); add(m_col_cellID); add(m_col_filenum); add(m_col_xaxis); 
                  add(m_col_type); add(m_col_freq); add(m_col_trials); add(m_col_onset); 
				  add(m_col_dur); add(m_col_atten); add(m_col_speakertype); add(m_col_azimuth); add(m_col_elevation); 
				  add(m_col_carfreq); add(m_col_threshold); add(m_col_threshold_attn); add(m_col_depth);
				  add(m_col_location); add(m_col_bad);
                }

                Gtk::TreeModelColumn<bool> m_col_hidden;
                Gtk::TreeModelColumn<Glib::ustring> m_col_props;
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

				// Some hidden properties
                Gtk::TreeModelColumn<Glib::ustring> m_col_speakertype;
                Gtk::TreeModelColumn<Glib::ustring> m_col_azimuth;
                Gtk::TreeModelColumn<Glib::ustring> m_col_elevation;

				// Cell details
                Gtk::TreeModelColumn<int> m_col_carfreq;
                Gtk::TreeModelColumn<int> m_col_threshold;
                Gtk::TreeModelColumn<int> m_col_threshold_attn;
                Gtk::TreeModelColumn<int> m_col_depth;
                Gtk::TreeModelColumn<Glib::ustring> m_col_location;
				Gtk::TreeModelColumn<int> m_col_bad;
        };
        Columns m_Columns;

        Glib::RefPtr<Gtk::ListStore> mrp_ListStore;

		/**
		 * Signal that a file was hidden.
		 */
		typedef sigc::signal<void,bool> type_signal_file_set_hidden;
		type_signal_file_set_hidden signal_file_set_hidden();

		typedef sigc::signal<void,Glib::ustring> type_signal_tag_deleted;
		typedef sigc::signal<bool,Glib::ustring> type_signal_tag_added;
		type_signal_tag_deleted signal_tag_deleted();
		type_signal_tag_added signal_tag_added();


	protected:

		Gtk::Window* m_parent;
        sqlite3 **db; /**< Pointer to out SQLite3 database. */

		type_signal_file_set_hidden m_signal_file_set_hidden;
		type_signal_tag_deleted m_signal_tag_deleted;
		type_signal_tag_added m_signal_tag_added;

        Glib::RefPtr<Gtk::TreeSelection> mrp_Selection;
		Gtk::Menu m_Menu_FileDetails;

		void on_view_file_details();
		void show_file_details(const Gtk::TreeModel::iterator& iter);
		void on_file_details_button_press_event(GdkEventButton* event);
		void on_tag_deleted(Glib::ustring tag);
		bool on_tag_added(Glib::ustring tag);
};

#endif

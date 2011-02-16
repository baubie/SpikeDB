#ifndef UIFILEDETAILSTREEVIEW_H
#define UIFILEDETAILSTREEVIEW_H

#include <gtkmm.h>
#include <vector>

class uiFileDetailsTreeView : public Gtk::TreeView {
	
	public:
		uiFileDetailsTreeView(Gtk::Window *parent);
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
                { add(m_col_hidden); add(m_col_time); add(m_col_props);
				  add(m_col_animalID); add(m_col_cellID); add(m_col_filenum); add(m_col_xaxis); 
                  add(m_col_type); add(m_col_freq); add(m_col_trials); add(m_col_onset); 
				  add(m_col_dur); add(m_col_atten); add(m_col_tags); 
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
                Gtk::TreeModelColumn<Glib::ustring> m_col_tags;
        };
        Columns m_Columns;

		/**
		 * Signal that a file was hidden.
		 */
		typedef sigc::signal<void,bool> type_signal_file_set_hidden;
		type_signal_file_set_hidden signal_file_set_hidden();

	protected:

		Gtk::Window* m_parent;

		type_signal_file_set_hidden m_signal_file_set_hidden;

        Glib::RefPtr<Gtk::ListStore> mrp_ListStore;
        Glib::RefPtr<Gtk::TreeSelection> mrp_Selection;
		Gtk::Menu* mp_Menu_FileDetails;

		void on_view_file_details();
		void show_file_details(const Gtk::TreeModel::iterator& iter);
		void on_file_details_button_press_event(GdkEventButton* event);
};

#endif

#ifndef UIFILTERFRAME_H
#define UIFILTERFRAME_H

#include <gtkmm.h>
#include <vector>


class uiFilterFrame : public Gtk::Frame {

	public:
		uiFilterFrame();
        virtual ~uiFilterFrame();

		int minFiles();
		void minFiles(int set);

		Glib::ustring tag();
		void updateTagCompletion(std::vector<Glib::ustring> tags);

		bool showHidden();
		void showHidden(bool set);

		int XVar();

		typedef sigc::signal<void> type_signal_changed;
		type_signal_changed signal_changed();



	protected:
		type_signal_changed m_signal_changed;

		/**
		 * Parent window
		 */
		Gtk::Object *parent;

		/**
		 * Tree model columns for the EntryCompletion's filter model.
		 */
		class ModelColumns : public Gtk::TreeModel::ColumnRecord
		{
			public:
				ModelColumns()
				{ add(m_col_name); }
				Gtk::TreeModelColumn<Glib::ustring> m_col_name;
		} m_tagColumns;
		Glib::RefPtr<Gtk::ListStore> mrp_CompletionModel;
		void on_tag_changed();
		bool queue_change_signal;
		bool check_change_queue();
		Glib::Timer m_timer;


		/**
		 * Minimum number of files spin box
		 */
		Gtk::SpinButton* mp_sbMinFiles;
		Gtk::Adjustment m_adjMinFiles;
		void on_adjMinFiles_changed();

		/**
		 * X-Variable Type Filter ComboBox
		 */
		Gtk::ComboBoxText m_XVar;
		void on_XVar_changed();

		/**
		 * Tag filter
		 */
		Gtk::Entry m_tag;

		/**
		 * Hidden file checkbox
		 */
		Gtk::CheckButton *mp_cbHidden;
		void on_hidden_toggled();
};
					  
#endif

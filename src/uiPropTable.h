#ifndef UIPROPTABLE_H
#define UIPROPTABLE_H

#include <gtkmm.h>

/** Properties table widget with static, editable, and special tags row types. */
class uiPropTable : public Gtk::TreeView {

	public:

		/**
		 * Define the types of rows that are allowed in the table.
		 */
        enum RowType {
        	Static, /**< Single line of static text. */
			Editable, /**< Single line of editable text. */
			StaticLong, /**< Three lines of static text. */
			EditableLong, /**< Three lines of editable text. */
			Tags /**< Special tags format with a list of tags you can add or remove. */
		};


		/**
		 * Constructor
		 */
		uiPropTable();


		/**
		 * Destructor
		 */
        virtual ~uiPropTable();


		/**
		 * Clear all of the rows from the table.
		 */
        void clear();


        /**
		 * Adds row to the properties table.
		 * @param[in] ID Hidden identification string.
		 * @param[in] name Text for the name cell.
		 * @param[in] value Text for the value cell.
		 * @param[in] type Define the behaviour and display style of the value cell.
		 */
		void addRow(Glib::ustring ID, 
					Glib::ustring name, 
					Glib::ustring value, 
                    uiPropTable::RowType type);

		typedef sigc::signal<void,
							 Glib::ustring, 
						     Glib::ustring, 
							 Glib::ustring, 
							 Glib::ustring, 
							 uiPropTable::RowType
						 	 > type_signal_rowedited;

        /**
		 * Signals that a row has been edited.
		 */
		type_signal_rowedited signal_rowedited();

	protected:

		type_signal_rowedited m_signal_rowedited;

        class Columns : public Gtk::TreeModel::ColumnRecord
        {
            public:
                Columns()
                { add(m_col_type); add(m_col_ID); add(m_col_name); add(m_col_value); }
                Gtk::TreeModelColumn<RowType> m_col_type;
                Gtk::TreeModelColumn<Glib::ustring> m_col_ID;
                Gtk::TreeModelColumn<Glib::ustring> m_col_name;
                Gtk::TreeModelColumn<Glib::ustring> m_col_value;
        } m_Columns;


		/**
		 * Pointer to the ListStore (Model) behind the table/
		 */
        Glib::RefPtr<Gtk::ListStore> m_refList;

        Gtk::TreeViewColumn m_tvcol_value;
        Gtk::CellRendererText m_rend_value;
        void on_value_edited(const Glib::ustring& path_string, const Glib::ustring& new_text);
        void value_cell_data(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);

};

#endif

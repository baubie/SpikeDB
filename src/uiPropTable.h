#ifndef UIPROPTABLE_H
#define UIPROPTABLE_H

#include <gtkmm.h>

class uiPropTable : public Gtk::TreeView {

	public:

        enum RowType {
        	Static, /**< Single line of static text. */
			Editable, /**< Single line of editable text. */
			StaticLong, /**< Three lines of static text. */
			EditableLong, /**< Three lines of editable text. */
			Tags /**< Special tags format with a list of tags you can add or remove. */
		};

		uiPropTable();
        virtual ~uiPropTable();

        void clear();
		void addRow(Glib::ustring ID, /**< [in] Hidden identification string. */
					Glib::ustring name, /**< [in] Name column text. */
					Glib::ustring value, /**< [in] Value column text. */
                    uiPropTable::RowType type /**< [in] Type of row to add. */
					);

	private:

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

        Glib::RefPtr<Gtk::ListStore> m_refList;

        Gtk::CellRendererText m_rend_value;
        Gtk::TreeViewColumn m_tvcol_value;
        void on_value_edited(const Glib::ustring& path_string, const Glib::ustring& new_text);
        void value_cell_data(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);

};

#endif

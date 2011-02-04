#ifndef UIPROPTABLE_H
#define UIPROPTABLE_H

#include <gtkmm.h>

/**
 * Define the types of rows that are allowed in the table.
 */
enum uiPropTableRowType {
	Static, /**< Single line of static text. */
	Editable, /**< Single line of editable text. */
	StaticLong, /**< Three lines of static text. */
	EditableLong, /**< Three lines of editable text. */
	Tags /**< Special tags format with a list of tags you can add or remove. */
};

/** Properties table widget with static, editable, and special tags row types. */
template <class T>
class uiPropTable : public Gtk::TreeView {

	public:

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
		void addRow(T ID, 
					Glib::ustring name, 
					Glib::ustring value, 
                    uiPropTableRowType type);

        /**
		 * Adds row to the properties table.
		 * @param[in] ID Hidden identification string.
		 * @param[in] name Text for the name cell.
		 * @param[in] value Integer for the value cell.
		 * @param[in] type Define the behaviour and display style of the value cell.
		 */
		void addRow(T ID, 
					Glib::ustring name, 
					int value, 
                    uiPropTableRowType type);

		typedef sigc::signal<void,
						     T,
						     Glib::ustring, 
							 Glib::ustring, 
							 Glib::ustring, 
							 uiPropTableRowType
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
                Gtk::TreeModelColumn<uiPropTableRowType> m_col_type;
                Gtk::TreeModelColumn<T> m_col_ID;
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

template <class T>
uiPropTable<T>::uiPropTable()
{
	
	// Setup the ListStore and add it to our tree
	m_refList = Gtk::ListStore::create(m_Columns);
	this->set_model(m_refList);

	 // Add the Name column to the tree
	this->append_column("Name", m_Columns.m_col_name);


	// Add the Value column to the tree
	//  Requires some custom setup to give us more control
	m_tvcol_value.set_title("Value");
	m_rend_value.property_editable() = true;
	m_tvcol_value.pack_start(m_rend_value);
	this->append_column(m_tvcol_value);
	m_rend_value.signal_edited().connect(sigc::mem_fun(*this, &uiPropTable::on_value_edited));
	m_tvcol_value.set_cell_data_func(m_rend_value, sigc::mem_fun(*this, &uiPropTable::value_cell_data)); 

}

template <class T>
uiPropTable<T>::~uiPropTable() {}


template <class T>
void uiPropTable<T>::clear()
{
	m_refList->clear();
}

template <class T>
void uiPropTable<T>::addRow(T ID, Glib::ustring name, Glib::ustring value, uiPropTableRowType type)
{
	Gtk::TreeModel::Row row = *(m_refList->append());
	row[m_Columns.m_col_type] = type;
	row[m_Columns.m_col_ID] = ID;
	row[m_Columns.m_col_name] = name;
	row[m_Columns.m_col_value] = value;
}

template <class T>
void uiPropTable<T>::addRow(T ID, Glib::ustring name, int value, uiPropTableRowType type)
{
	char buf[G_ASCII_DTOSTR_BUF_SIZE];
	Gtk::TreeModel::Row row = *(m_refList->append());
	row[m_Columns.m_col_type] = type;
	row[m_Columns.m_col_ID] = ID;
	row[m_Columns.m_col_name] = name;
	row[m_Columns.m_col_value] = g_ascii_dtostr(buf,sizeof(buf),value);
}


template <class T>
typename uiPropTable<T>::type_signal_rowedited uiPropTable<T>::signal_rowedited()
{
	return m_signal_rowedited;
}

template <class T>
void uiPropTable<T>::on_value_edited(const Glib::ustring& path_string, const Glib::ustring& new_text)
{
	Gtk::TreePath path(path_string);
	Gtk::TreeModel::iterator iter = m_refList->get_iter(path);
	if (iter) {
		Gtk::TreeModel::Row row = *iter;

		// Only emit changed signal when the value changes.
		if (row.get_value(m_Columns.m_col_value) != new_text)
		{
			m_signal_rowedited.emit(
				row.get_value(m_Columns.m_col_ID),
				row.get_value(m_Columns.m_col_name),
				row.get_value(m_Columns.m_col_value),
				new_text,
				row.get_value(m_Columns.m_col_type));
		}

	}
}

template <class T>
void uiPropTable<T>::value_cell_data(Gtk::CellRenderer* /*renderer*/, const Gtk::TreeModel::iterator& iter)
{
	if (iter) {
		Gtk::TreeModel::Row row = *iter;
		m_rend_value.property_text() = row[m_Columns.m_col_value];
		if (row[m_Columns.m_col_type] == Editable) {
			m_rend_value.property_editable() = true;
			m_rend_value.property_cell_background() = "#DDEEFF";
		} else if (row[m_Columns.m_col_type] == EditableLong) {
			m_rend_value.property_editable() = true;
			m_rend_value.property_cell_background() = "#DDEEFF";
		} else if (row[m_Columns.m_col_type] == Static) {
			m_rend_value.property_editable() = false;
			m_rend_value.property_cell_background() = "#FFDDDD";
		}
	}
}

#endif

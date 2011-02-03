#include "uiPropTable.h"

uiPropTable::uiPropTable()
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

uiPropTable::~uiPropTable() {}


void uiPropTable::clear()
{
	m_refList->clear();
}

void uiPropTable::addRow(Glib::ustring ID, Glib::ustring name, Glib::ustring value, uiPropTable::RowType type)
{
	Gtk::TreeModel::Row row = *(m_refList->append());
	row[m_Columns.m_col_type] = type;
	row[m_Columns.m_col_ID] = ID;
	row[m_Columns.m_col_name] = name;
	row[m_Columns.m_col_value] = value;
}


uiPropTable::type_signal_rowedited uiPropTable::signal_rowedited()
{
	return m_signal_rowedited;
}



void uiPropTable::on_value_edited(const Glib::ustring& path_string, const Glib::ustring& new_text)
{
	Gtk::TreePath path(path_string);
	Gtk::TreeModel::iterator iter = m_refList->get_iter(path);
	if (iter) {
		Gtk::TreeModel::Row row = *iter;

		// Only emit changed signal when the value changes.
		if (row.get_value(m_Columns.m_col_value) != new_text)
		{
			m_signal_rowedited.emit(
				row.get_value(m_Columns.m_col_ID).c_str(),
				row.get_value(m_Columns.m_col_name).c_str(),
				row.get_value(m_Columns.m_col_value).c_str(),
				new_text,
				row.get_value(m_Columns.m_col_type));
		}

	}
}

void uiPropTable::value_cell_data(Gtk::CellRenderer* /*renderer*/, const Gtk::TreeModel::iterator& iter)
{
	if (iter) {
		Gtk::TreeModel::Row row = *iter;
		m_rend_value.property_text() = row[m_Columns.m_col_value];
		if (row[m_Columns.m_col_type] == uiPropTable::Editable) {
			m_rend_value.property_editable() = true;
			m_rend_value.property_cell_background() = "#DDEEFF";
		} else if (row[m_Columns.m_col_type] == uiPropTable::EditableLong) {
			m_rend_value.property_editable() = true;
			m_rend_value.property_cell_background() = "#DDEEFF";
		} else if (row[m_Columns.m_col_type] == uiPropTable::Static) {
			m_rend_value.property_editable() = false;
			m_rend_value.property_cell_background() = "#FFDDDD";
		}
	}
}

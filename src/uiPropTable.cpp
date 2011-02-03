#include "uiPropTable.h"

uiPropTable::uiPropTable()
{
	
	/**
	 * Setup the ListStore and add it to our tree
	 */
	m_refList = Gtk::ListStore::create(m_Columns);
	this->set_model(m_refList);


	/**
	 * Add the Name column to the tree
	 */
	this->append_column("Name", m_Columns.m_col_name);


	/**
	 * Add the Value column to the tree
	 * Requires some custom setup to give us more control
	 */
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


void uiPropTable::on_value_edited(const Glib::ustring& path_string, const Glib::ustring& new_text)
{

	Gtk::TreePath path(path_string);

	Gtk::TreeModel::iterator iter = m_refList->get_iter(path);

	if (iter) {
		/*
		// Update tree model
		Gtk::TreeModel::Row row = *iter;

		// Update sqlite database
		Glib::ustring query;
		if (row.get_value(m_AnimalDetailsColumns.m_col_name) == "Tags") {
			query = "UPDATE animals SET tags=? WHERE ID=?";
		}
		if (row.get_value(m_AnimalDetailsColumns.m_col_name) == "Species") {
			query = "UPDATE animals SET species=? WHERE ID=?";
		}
		if (row.get_value(m_AnimalDetailsColumns.m_col_name) == "Sex") {
			query = "UPDATE animals SET sex=? WHERE ID=?";
		}
		if (row.get_value(m_AnimalDetailsColumns.m_col_name) == "Weight (g)") {
			query = "UPDATE animals SET weight=? WHERE ID=?";
		}
		if (row.get_value(m_AnimalDetailsColumns.m_col_name) == "Age") {
			query = "UPDATE animals SET age=? WHERE ID=?";
		}
		if (row.get_value(m_AnimalDetailsColumns.m_col_name) == "Notes") {
			query = "UPDATE animals SET notes=? WHERE ID=?";
		}

		const char* q = query.c_str();
		sqlite3_stmt *stmt = 0;
		sqlite3_prepare_v2(db, q, strlen(q), &stmt, NULL);
		sqlite3_bind_text(stmt, 1, new_text.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_text(stmt, 2, row.get_value(m_AnimalDetailsColumns.m_col_animalID).c_str(), -1, SQLITE_TRANSIENT);
		int r = sqlite3_step(stmt);
		if (r == SQLITE_DONE) {
			row[m_AnimalDetailsColumns.m_col_value] = new_text;
		} else{
			std::cerr << "ERROR: Could not update animal. " << sqlite3_errmsg(db) << std::endl;
		}
		sqlite3_finalize(stmt);
		*/
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

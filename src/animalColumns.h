#ifndef ANIMALCOLUMNS_H
#define ANIMALCOLUMNS_H

// Animal Tree Model Columns
class AnimalColumns : public Gtk::TreeModel::ColumnRecord
{
	public:
		AnimalColumns()
		{ add(m_col_name); }
		Gtk::TreeModelColumn<Glib::ustring> m_col_name;
};

#endif

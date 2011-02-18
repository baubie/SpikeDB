#include "uiFileDetailsTreeView.h"

uiFileDetailsTreeView::uiFileDetailsTreeView(sqlite3 **db, Gtk::Window *parent)
{

	m_parent = parent;
	this->db = db;


	mrp_ListStore = Gtk::ListStore::create(m_Columns);
	mrp_ListStore->set_sort_column(m_Columns.m_col_time, Gtk::SORT_ASCENDING);
	this->set_model(mrp_ListStore);
	this->append_column("", m_Columns.m_col_props);
	this->append_column("AnimalID", m_Columns.m_col_animalID);
	this->append_column("CellID", m_Columns.m_col_cellID);
	this->append_column("#", m_Columns.m_col_filenum);
	this->append_column("X-Var", m_Columns.m_col_xaxis);
	this->append_column("Type", m_Columns.m_col_type);
	this->append_column("Trials", m_Columns.m_col_trials);
	this->append_column("CarFreq (Hz)", m_Columns.m_col_freq);
	this->append_column("Dur (ms)", m_Columns.m_col_dur);
	this->append_column("Onset (ms)", m_Columns.m_col_onset);
	this->append_column("Atten (db)", m_Columns.m_col_atten);
	mrp_Selection = this->get_selection();
	mrp_Selection->set_mode(Gtk::SELECTION_MULTIPLE);
	this->set_rules_hint(true);

	// Setup right click handling
	this->signal_button_press_event().connect_notify(
				sigc::mem_fun(*this, &uiFileDetailsTreeView::on_file_details_button_press_event)
			);
	mp_Menu_FileDetails = Gtk::manage( new Gtk::Menu());
	//Fill menu
	{
		Gtk::Menu::MenuList& menulist = mp_Menu_FileDetails->items();

		menulist.push_back( Gtk::Menu_Helpers::MenuElem("_View Details",
					sigc::mem_fun(*this, &uiFileDetailsTreeView::on_view_file_details)));
	}


}

uiFileDetailsTreeView::~uiFileDetailsTreeView() {}

Glib::RefPtr<Gtk::TreeSelection> uiFileDetailsTreeView::treeSelection()
{
	return mrp_Selection;
}

void uiFileDetailsTreeView::clear()
{
	mrp_ListStore->clear();
}

Gtk::TreeModel::Row uiFileDetailsTreeView::newrow()
{
	return *(mrp_ListStore->append());
}

Glib::ustring uiFileDetailsTreeView::animalID(const Gtk::TreeModel::iterator& iter)
{
	Gtk::TreeModel::Row row = *iter;
	return row.get_value(m_Columns.m_col_animalID);
}

int uiFileDetailsTreeView::cellID(const Gtk::TreeModel::iterator& iter)
{
	Gtk::TreeModel::Row row = *iter;
	return row.get_value(m_Columns.m_col_cellID);
}

int uiFileDetailsTreeView::fileID(const Gtk::TreeModel::iterator& iter)
{
	Gtk::TreeModel::Row row = *iter;
	return row.get_value(m_Columns.m_col_filenum);
}

void uiFileDetailsTreeView::on_file_details_button_press_event(GdkEventButton* event)
{
	if ( (event->type == GDK_BUTTON_PRESS) && (event->button == 3) )
	{
		Gtk::Menu::MenuList& menulist = mp_Menu_FileDetails->items();
		if (mrp_Selection->count_selected_rows() > 1)
		{
			menulist[0].set_sensitive(false);
		} else {
			menulist[0].set_sensitive(true);
		}
		mp_Menu_FileDetails->popup(event->button, event->time);
	}
}

void uiFileDetailsTreeView::show_file_details(const Gtk::TreeModel::iterator& iter)
{
	Gtk::TreeModel::Row row = *iter;


	std::cout << db << std::endl;

	sqlite3_stmt *stmt = 0;
	const char query[] = "SELECT header FROM files WHERE animalID=? AND cellID=? AND fileID=?";
	sqlite3_prepare_v2(*db, query, strlen(query), &stmt, NULL);
	sqlite3_bind_text(stmt, 1, row.get_value(m_Columns.m_col_animalID).c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 2, row.get_value(m_Columns.m_col_cellID));
	sqlite3_bind_int(stmt, 3, row.get_value(m_Columns.m_col_filenum));

	int r = sqlite3_step(stmt);

	if (r == SQLITE_ROW) {

		SpikeData sd;
		void *header = (void*)sqlite3_column_blob(stmt, 0);
		sd.setHeader(header);

		Gtk::Dialog dialog("File Details", true);
		dialog.set_transient_for(*m_parent);
		dialog.set_resizable(false);
		dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
		dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

		Gtk::HBox hbTime;
		Gtk::Label lblTimeName("Recording Time: ");
		Gtk::Label lblTime(sd.iso8601(sd.m_head.cDateTime));
		hbTime.pack_start(lblTimeName);
		hbTime.pack_start(lblTime);
		dialog.get_vbox()->pack_start(hbTime);

		Gtk::CheckButton cbHidden("Hide file in file list");
		cbHidden.set_active(row.get_value(m_Columns.m_col_hidden));
		dialog.get_vbox()->pack_start(cbHidden);
		dialog.show_all_children();
		int result = dialog.run();


		switch (result) {
			case (Gtk::RESPONSE_OK):
				m_signal_file_set_hidden.emit(cbHidden.get_active());
				break;
		}

	} else {
		Gtk::MessageDialog dialog(*m_parent, "Error loading file from database.", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
		dialog.set_secondary_text(sqlite3_errmsg(*db));
		dialog.run();
	}
	sqlite3_finalize(stmt);

}

void uiFileDetailsTreeView::on_view_file_details()
{

	std::vector<Gtk::TreeModel::Path> sel = mrp_Selection->get_selected_rows();
	if (sel.size() == 1) 
	{
		show_file_details(mrp_ListStore->get_iter(sel[0]));
	}

}

uiFileDetailsTreeView::type_signal_file_set_hidden uiFileDetailsTreeView::signal_file_set_hidden()
{
	return m_signal_file_set_hidden;
}
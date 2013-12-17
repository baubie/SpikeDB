/*
Copyright (c) 2011-2012, Brandon Aubie
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "stdafx.h"
#include "uiFileDetailsTreeView.h"

uiFileDetailsTreeView::uiFileDetailsTreeView(sqlite3 **db, Gtk::Window *parent)
{
	m_parent = parent;
	this->db = db;
	mrp_ListStore = Gtk::ListStore::create(m_Columns);
	this->linkModel();
	this->append_column("", m_Columns.m_col_props);
	this->append_column("AnimalID", m_Columns.m_col_animalID);
	this->append_column("CellID", m_Columns.m_col_cellID);
	this->append_column("#", m_Columns.m_col_filenum);
	this->append_column("X-Var", m_Columns.m_col_xaxis);
	this->append_column("Type", m_Columns.m_col_type);
	this->append_column("Trials", m_Columns.m_col_trials);
	this->append_column("CarFreq (Hz)", m_Columns.m_col_freq);
	this->append_column("FreqDev (Hz)", m_Columns.m_col_freqdev);
	this->append_column("Dur (ms)", m_Columns.m_col_dur);
	this->append_column("Onset (ms)", m_Columns.m_col_onset);
	this->append_column("Atten (db)", m_Columns.m_col_atten);
	mrp_Selection = this->get_selection();
	mrp_Selection->set_mode(Gtk::SELECTION_MULTIPLE);
	this->set_rules_hint(false);

	// Setup right click handling
	this->signal_button_press_event().connect_notify(
				sigc::mem_fun(*this, &uiFileDetailsTreeView::on_file_details_button_press_event)
			);
	//Fill menu
	{
		Gtk::Menu::MenuList& menulist = m_Menu_FileDetails.items();

		menulist.push_back( Gtk::Menu_Helpers::MenuElem("_View Details",
					sigc::mem_fun(*this, &uiFileDetailsTreeView::on_view_file_details)));
	}


	hasRows = false;

}

uiFileDetailsTreeView::~uiFileDetailsTreeView() {}

Glib::RefPtr<Gtk::TreeSelection> uiFileDetailsTreeView::treeSelection()
{
	return mrp_Selection;
}

void uiFileDetailsTreeView::linkModel()
{
	this->set_model(mrp_ListStore);
}

void uiFileDetailsTreeView::clear()
{
	mrp_ListStore->clear();
	this->unset_model();
	hasRows = false;
}

Gtk::TreeModel::Row uiFileDetailsTreeView::newrow()
{
	if (!hasRows) {
		m_lastInsertedRow = mrp_ListStore->append();
		hasRows = true;
	} else {
		m_lastInsertedRow = mrp_ListStore->insert_after(m_lastInsertedRow);
	}
	return *m_lastInsertedRow;
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
		m_signal_menu_will_show.emit();
		Gtk::Menu::MenuList& menulist = m_Menu_FileDetails.items();
		if (mrp_Selection->count_selected_rows() > 1)
		{
			menulist[0].set_sensitive(false);
		} else {
			menulist[0].set_sensitive(true);
		}
		m_Menu_FileDetails.popup(event->button, event->time);
	}
}

void uiFileDetailsTreeView::show_file_details(const Gtk::TreeModel::iterator& iter)
{
	Gtk::TreeModel::Row row = *iter;

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

		sqlite3_stmt *stmt2 = 0;
		const char query2[] = "SELECT tag FROM tags WHERE animalID=? AND cellID=? AND fileID=?";
		sqlite3_prepare_v2(*db, query2, -1, &stmt2, 0);
		sqlite3_bind_text(stmt2, 1, row.get_value(m_Columns.m_col_animalID).c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_int(stmt2, 2, row.get_value(m_Columns.m_col_cellID));
		sqlite3_bind_int(stmt2, 3, row.get_value(m_Columns.m_col_filenum));
		std::vector<Glib::ustring> tags;
		while (sqlite3_step(stmt2) == SQLITE_ROW) {
			Glib::ustring t = (char*)sqlite3_column_text(stmt2, 0);
			tags.push_back(t);
		}
		sqlite3_finalize(stmt2);
		Gtk::Frame frameTags("File Tags");
		uiTags tagsFile(m_parent);
		tagsFile.tags(tags);
		tagsFile.delete_assist = true;
		frameTags.add(tagsFile);
		dialog.get_vbox()->pack_start(frameTags);
		tagsFile.signal_deleted().connect(sigc::mem_fun(*this, &uiFileDetailsTreeView::on_tag_deleted));
		tagsFile.signal_added().connect(sigc::mem_fun(*this, &uiFileDetailsTreeView::on_tag_added));
		dialog.show_all_children();
		int result = dialog.run();


		switch (result) {
			case (Gtk::RESPONSE_OK):
				if (row.get_value(m_Columns.m_col_hidden) != cbHidden.get_active())
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

void uiFileDetailsTreeView::on_tag_deleted(Glib::ustring tag)
{
	m_signal_tag_deleted.emit(tag);
}

bool uiFileDetailsTreeView::on_tag_added(Glib::ustring tag)
{
	return m_signal_tag_added.emit(tag);
}

uiFileDetailsTreeView::type_signal_file_set_hidden uiFileDetailsTreeView::signal_file_set_hidden()
{
	return m_signal_file_set_hidden;
}

uiFileDetailsTreeView::type_signal_tag_added uiFileDetailsTreeView::signal_tag_added()
{
	return m_signal_tag_added;
}

uiFileDetailsTreeView::type_signal_tag_deleted uiFileDetailsTreeView::signal_tag_deleted()
{
	return m_signal_tag_deleted;
}

uiFileDetailsTreeView::type_signal_menu_will_show uiFileDetailsTreeView::signal_menu_will_show()
{
	return m_signal_menu_will_show;
}

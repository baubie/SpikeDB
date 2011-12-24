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
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "stdafx.h"

#include "uiFilterFrame.h"


uiFilterFrame::uiFilterFrame()
	: m_adjMinFiles(1, 1, 20, 1, 3, 0)
{

	/*
	 * Default initialization
	 */
    queue_change_signal = false;


	Gtk::VBox *vbFilter = Gtk::manage( new Gtk::VBox());

	/*
	 * Spin button to select minimum number of files.
	 */
	vbFilter->pack_start(*Gtk::manage(new Gtk::Label("Required # of Files")));
	mp_sbMinFiles = Gtk::manage( new Gtk::SpinButton() );
	vbFilter->pack_start(*mp_sbMinFiles);
	mp_sbMinFiles->set_adjustment(m_adjMinFiles);
	m_adjMinFiles.signal_value_changed().connect(
			sigc::mem_fun(*this, &uiFilterFrame::on_adjMinFiles_changed)
	);
	

	/*
	 * ComboBox to select the X-Variable 
	 */
	vbFilter->pack_start(*Gtk::manage(new Gtk::Label("X-Variable")));
	m_XVar.append_text("All");
	m_XVar.append_text("Freq");
	m_XVar.append_text("Dur");
	m_XVar.append_text("Onset");
	m_XVar.append_text("Atten");
	m_XVar.set_active(0);
	m_XVar.signal_changed().connect(
		sigc::mem_fun(*this, &uiFilterFrame::on_XVar_changed)
	);
	vbFilter->pack_start(m_XVar, false, false, 0);

	/*
	 * Tag filter
	 */
	Gtk::Label *tagLabel = Gtk::manage(new Gtk::Label("Tag Filter"));
	vbFilter->pack_start(*tagLabel, false, false, 0);
	vbFilter->pack_start(m_tag, false, false, 0);
	Glib::RefPtr<Gtk::EntryCompletion> mrp_tagCompletion = Gtk::EntryCompletion::create();
	m_tag.set_completion(mrp_tagCompletion);
	mrp_CompletionModel = Gtk::ListStore::create(m_tagColumns);
	mrp_tagCompletion->set_model(mrp_CompletionModel);
	mrp_tagCompletion->set_text_column(m_tagColumns.m_col_name);

	m_tag.signal_changed().connect(
			sigc::mem_fun(*this, &uiFilterFrame::on_tag_changed)
	);

	m_timer.start();
	Glib::signal_timeout().connect(sigc::mem_fun(*this, &uiFilterFrame::check_change_queue), 250);

	/*
	 * Hidden file checkbox
	 */
	mp_cbHidden = Gtk::manage(new Gtk::CheckButton("Show hidden files"));
	mp_cbHidden->set_active(false);
	vbFilter->pack_start(*mp_cbHidden, false, false, 0);
	mp_cbHidden->signal_toggled().connect(
		sigc::mem_fun(*this, &uiFilterFrame::on_hidden_toggled)
			);

	this->add(*vbFilter);
}

uiFilterFrame::~uiFilterFrame() {}


/*
 * Changed Signal
 */
uiFilterFrame::type_signal_changed uiFilterFrame::signal_changed()
{
	return m_signal_changed;
}

void uiFilterFrame::on_hidden_toggled()
{
	m_signal_changed.emit();
}

bool uiFilterFrame::showHidden()
{
	return mp_cbHidden->get_active();
}

void uiFilterFrame::showHidden(bool set)
{
	mp_cbHidden->set_active(set);
}

void uiFilterFrame::on_XVar_changed()
{
		m_signal_changed.emit();
}

void uiFilterFrame::on_adjMinFiles_changed()
{
	m_signal_changed.emit();
}

void uiFilterFrame::on_tag_changed()
{
	if (m_timer.elapsed() >= 1) 
	{
		m_signal_changed.emit();
		m_timer.reset();
	} else {
		queue_change_signal = true;
	}
}

bool uiFilterFrame::check_change_queue()
{
	if (queue_change_signal) {
		m_signal_changed.emit();
		queue_change_signal = false;
	}
	return true;
}

void uiFilterFrame::updateTagCompletion(std::vector<Glib::ustring> tags)
{
	Gtk::TreeModel::Row row;
	mrp_CompletionModel->clear();

	for (unsigned int i = 0; i < tags.size(); i++) 
	{
		row = *(mrp_CompletionModel->append());
		row[m_tagColumns.m_col_name] = tags[i];
	}
}

Glib::ustring uiFilterFrame::tag()
{
	return m_tag.get_text();
}

/*
 * Return the currently selected minimum number of files
 */
int uiFilterFrame::minFiles()
{
	return (int)m_adjMinFiles.get_value();
}
void uiFilterFrame::minFiles(int set)
{
	m_adjMinFiles.set_value(set);
}

int uiFilterFrame::XVar()
{
	return m_XVar.get_active_row_number();
}

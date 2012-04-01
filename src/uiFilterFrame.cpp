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

	Gtk::VBox *vbFilter = Gtk::manage( new Gtk::VBox() );

	Gtk::HBox *hbTopName = Gtk::manage( new Gtk::HBox() );
	Gtk::HBox *hbTopWidget = Gtk::manage( new Gtk::HBox() );
	hbTopName->pack_start(*Gtk::manage(new Gtk::Label("Min Files")));
	hbTopName->pack_start(*Gtk::manage(new Gtk::Label("X-Var")));
	vbFilter->pack_start( *hbTopName );

	/*
	 * Spin button to select minimum number of files.
	 */
	mp_sbMinFiles = Gtk::manage( new Gtk::SpinButton() );
	hbTopWidget->pack_start(*mp_sbMinFiles);
	mp_sbMinFiles->set_adjustment(m_adjMinFiles);
	m_adjMinFiles.signal_value_changed().connect(sigc::mem_fun(*this, &uiFilterFrame::on_adjMinFiles_changed));

	/*
	 * ComboBox to select the X-Variable 
	 */
	m_XVar.append_text("All");
	m_XVar.append_text("Freq");
	m_XVar.append_text("Dur");
	m_XVar.append_text("Onset");
	m_XVar.append_text("Atten");
	m_XVar.set_active(0);
	m_XVar.signal_changed().connect(sigc::mem_fun(*this, &uiFilterFrame::on_XVar_changed));
	hbTopWidget->pack_start(m_XVar, false, false, 0);
	vbFilter->pack_start( *hbTopWidget );


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
	m_tag.set_activates_default(false);
	m_tag.signal_activate().connect(sigc::mem_fun(*this, &uiFilterFrame::on_tag_changed));

	/*
	 * Hidden file checkbox
	 */
	mp_cbHidden = Gtk::manage(new Gtk::CheckButton("Show hidden files"));
	mp_cbHidden->set_active(false);
	vbFilter->pack_start(*mp_cbHidden, false, false, 0);
	mp_cbHidden->signal_toggled().connect(sigc::mem_fun(*this, &uiFilterFrame::on_hidden_toggled));

	/*
	 * Spike Filter
	 */
	Gtk::HSeparator *sep  = Gtk::manage(new Gtk::HSeparator());
	vbFilter->pack_start( *sep );

	Gtk::HBox *spikeFilterName = Gtk::manage( new Gtk::HBox() );
	Gtk::Label *spikeLabel = Gtk::manage(new Gtk::Label("Spike Filter"));
	Gtk::Label *minSpikeLabel = Gtk::manage(new Gtk::Label("Min"));
	Gtk::Label *maxSpikeLabel = Gtk::manage(new Gtk::Label("Max"));
	vbFilter->pack_start( *spikeLabel );
	spikeFilterName->pack_start( *minSpikeLabel );
	spikeFilterName->pack_start( *maxSpikeLabel );
	vbFilter->pack_start( *spikeFilterName );
	Gtk::HBox *spikeFilterEntryBox = Gtk::manage( new Gtk::HBox() );
	spikeFilterEntryBox->pack_start( m_minSpikes );
	spikeFilterEntryBox->pack_start( m_maxSpikes );
	m_minSpikes.set_width_chars(4);
	m_maxSpikes.set_width_chars(4);
	vbFilter->pack_start( *spikeFilterEntryBox, false, false, 0);
	mp_cbSpikeFilterAbsolute = Gtk::manage(new Gtk::CheckButton("Use Absolute Time"));
	vbFilter->pack_start( *mp_cbSpikeFilterAbsolute, false, false, 0);

	m_minSpikes.set_activates_default(false);
	m_minSpikes.signal_activate().connect(sigc::mem_fun(*this, &uiFilterFrame::on_spikeTime_changed));
	m_minSpikes.signal_focus_out_event().connect(sigc::mem_fun(*this, &uiFilterFrame::on_spikeTimeFocusLost));
	m_maxSpikes.set_activates_default(false);
	m_maxSpikes.signal_activate().connect(sigc::mem_fun(*this, &uiFilterFrame::on_spikeTime_changed));
	m_maxSpikes.signal_focus_out_event().connect(sigc::mem_fun(*this, &uiFilterFrame::on_spikeTimeFocusLost));
	mp_cbSpikeFilterAbsolute->signal_toggled().connect(sigc::mem_fun(*this, &uiFilterFrame::on_spikeTime_changed));

	this->add(*vbFilter);
}

uiFilterFrame::~uiFilterFrame() {}


/*
 * Changed Signal
 */
uiFilterFrame::type_signal_fileFilter_changed uiFilterFrame::signal_fileFilter_changed()
{
	return m_signal_fileFilter_changed;
}
uiFilterFrame::type_signal_spikeFilter_changed uiFilterFrame::signal_spikeFilter_changed()
{
	return m_signal_spikeFilter_changed;
}

void uiFilterFrame::on_hidden_toggled()
{
	m_signal_fileFilter_changed.emit();
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
	m_signal_fileFilter_changed.emit();
}

void uiFilterFrame::on_adjMinFiles_changed()
{
	m_signal_fileFilter_changed.emit();
}

void uiFilterFrame::on_tag_changed()
{
	m_signal_fileFilter_changed.emit();
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

double uiFilterFrame::minSpikeTime()
{
	if (m_minSpikes.get_text() == "") return -1*DBL_MAX;
	double val =  Glib::Ascii::strtod(m_minSpikes.get_text());
	return val;
}
double uiFilterFrame::maxSpikeTime()
{
	if (m_maxSpikes.get_text() == "") return DBL_MAX;
	double val = Glib::Ascii::strtod(m_maxSpikes.get_text());
	return val;
}
bool uiFilterFrame::absoluteSpikeTime()
{
	return mp_cbSpikeFilterAbsolute->get_active();
}
void uiFilterFrame::on_spikeTime_changed()
{
	m_signal_spikeFilter_changed.emit();
}


bool uiFilterFrame::on_spikeTimeFocusLost(GdkEventFocus *e)
{
	on_spikeTime_changed();
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

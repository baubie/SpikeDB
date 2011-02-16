#include "uiFilterFrame.h"


uiFilterFrame::uiFilterFrame(const Glib::RefPtr<Gtk::Builder>& refGlade)
	: m_refGlade(refGlade),
	  m_adjMinFiles(5, 1, 20, 1, 3, 0)
{
	/*
	 * Load the VBox
	 */
	m_refGlade->get_widget("VBoxFilter", mp_VBoxFilter);


	/*
	 * Spin button to select minimum number of files.
	 */
	m_refGlade->get_widget("sbMinFiles", mp_sbMinFiles);
	mp_sbMinFiles->set_adjustment(m_adjMinFiles);
	m_adjMinFiles.signal_value_changed().connect(
			sigc::mem_fun(*this, &uiFilterFrame::on_adjMinFiles_changed)
	);
	

	/*
	 * ComboBox to select the X-Variable 
	 */
	m_XVar.append_text("All");
	m_XVar.append_text("Freq");
	m_XVar.append_text("Dur");
	m_XVar.append_text("Onset");
	m_XVar.append_text("Atten");
	m_XVar.set_active(0);
	m_XVar.signal_changed().connect(
		sigc::mem_fun(*this, &uiFilterFrame::on_XVar_changed)
	);
	mp_VBoxFilter->pack_start(m_XVar, false, false, 0);

	/*
	 * Tag filter
	 */
	Gtk::Label *tagLabel = Gtk::manage(new Gtk::Label("Tag Filter"));
	mp_VBoxFilter->pack_start(*tagLabel, false, false, 0);
	mp_VBoxFilter->pack_start(m_tag, false, false, 0);
	Glib::RefPtr<Gtk::EntryCompletion> mrp_tagCompletion = Gtk::EntryCompletion::create();
	m_tag.set_completion(mrp_tagCompletion);
	mrp_CompletionModel = Gtk::ListStore::create(m_tagColumns);
	mrp_tagCompletion->set_model(mrp_CompletionModel);
	mrp_tagCompletion->set_text_column(m_tagColumns.m_col_name);

	m_tag.signal_changed().connect(
			sigc::mem_fun(*this, &uiFilterFrame::on_tag_changed)
	);

	m_timer.start();
	Glib::signal_timeout().connect(sigc::mem_fun(*this, &uiFilterFrame::check_change_queue), 5);

	/*
	 * Hidden file checkbox
	 */
	mp_cbHidden = Gtk::manage(new Gtk::CheckButton("Show hidden files"));
	mp_cbHidden->set_active(false);
	mp_VBoxFilter->pack_start(*mp_cbHidden, false, false, 0);
	mp_cbHidden->signal_toggled().connect(
		sigc::mem_fun(*this, &uiFilterFrame::on_hidden_toggled)
			);
}

uiFilterFrame::~uiFilterFrame()
{
	delete mp_sbMinFiles;
	delete mp_VBoxFilter;
}


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

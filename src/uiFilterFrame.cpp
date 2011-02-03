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


void uiFilterFrame::on_XVar_changed()
{
	m_signal_changed.emit();
}

void uiFilterFrame::on_adjMinFiles_changed()
{
	m_signal_changed.emit();
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

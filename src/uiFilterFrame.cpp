#include "uiFilterFrame.h"


uiFilterFrame::uiFilterFrame(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade) {

	// Setup the filter widgets
	m_refGlade->get_widget("sbMinFiles", m_pMinFiles);
	m_pMinFiles->set_adjustment(m_adjMinFiles);
	m_adjMinFiles.signal_value_changed().connect(sigc::mem_fun(*this, &GUI::updateFilter));
        // Filter
	m_refGlade->get_widget("cbTypeFilter", m_pTypeFilter);
        m_refTypeFilter = Gtk::ListStore::create(m_MeanTypeColumns);
        m_pTypeFilter->set_model(m_refTypeFilter);
        m_pTypeFilter->pack_start(m_MeanTypeColumns.m_col_name);
        m_pTypeFilter->signal_changed().connect( sigc::mem_fun(*this, &GUI::updateFilter) );
        row = *(m_refTypeFilter->append());
        row[m_MeanTypeColumns.m_col_name] = "All";
        row = *(m_refTypeFilter->append());
        row[m_MeanTypeColumns.m_col_name] = "Freq";
        row = *(m_refTypeFilter->append());
        row[m_MeanTypeColumns.m_col_name] = "Dur";
        row = *(m_refTypeFilter->append());
        row[m_MeanTypeColumns.m_col_name] = "Onset";
        row = *(m_refTypeFilter->append());
        row[m_MeanTypeColumns.m_col_name] = "Atten";
        m_pTypeFilter->set_active(0);
}

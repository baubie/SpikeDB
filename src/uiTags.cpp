#include "uiTags.h"

uiTags::uiTags() 
{
    this->signal_expose_event().connect(sigc::mem_fun(*this, &uiTags::on_uiTags_expose_event) );
    m_AddNew.signal_clicked().connect(sigc::mem_fun(*this, &uiTags::on_addnew_clicked) );
	m_AddNew.set_label("+");
	this->put(m_AddNew, 5, 10);
	m_AddNew.set_sensitive(false);
}

uiTags::~uiTags() {}

std::vector<Glib::ustring> uiTags::tags()
{
	return m_tags;
}

void uiTags::tags(std::vector<Glib::ustring> tags)
{
	m_tags = tags;
	m_AddNew.set_sensitive(true);

	for(unsigned int i = 0; i < m_tag_widgets.size(); i++) 
		delete m_tag_widgets[i];

	m_tag_widgets.clear();

	for(unsigned int i = 0; i < m_tags.size(); i++) 
		m_tag_widgets.push_back(new Tag(m_tags[i]));

	for(unsigned int i = 0; i < m_tag_widgets.size(); i++) 
		m_tag_widgets[i]->signal_deleted().connect(sigc::mem_fun(*this, &uiTags::on_tag_deleted) );

	needput = true;
	active = true;
	redraw();
}

uiTags::type_signal_added uiTags::signal_added()
{
	return m_signal_added;
}

uiTags::type_signal_deleted uiTags::signal_deleted()
{
	return m_signal_deleted;
}

void uiTags::on_tag_deleted(Glib::ustring tag)
{
	m_signal_deleted.emit(tag);
}

void uiTags::on_addnew_clicked() 
{
	Glib::ustring t = "NEWTAG";
	if (m_signal_added.emit(t)) {
		m_tags.push_back(t);
		tags(m_tags);
		redraw();
	}
}

bool uiTags::on_uiTags_expose_event(GdkEventExpose* /*e*/)
{
	redraw();
	return false;
}

void uiTags::redraw()
{
	if (active) m_AddNew.show();

	if (m_tags.empty()) return;
	const int spacing = 5;
	int x=spacing;
	int y=spacing;
	int w,h;
	int width = this->get_width();
	Gtk::Requisition rec = m_AddNew.size_request();
	x += rec.width;
	for(unsigned int i = 0; i < m_tag_widgets.size(); i++) 
	{
		m_tag_widgets[i]->show();
		rec = m_tag_widgets[i]->size_request();
		w = rec.width;
		h = rec.height;
		if (x > 0 && x+w > width) { x = spacing; y += h+spacing; }
		if (needput) this->put(*m_tag_widgets[i],x,y);
		else this->move(*m_tag_widgets[i],x,y);
		x += w+spacing;
	}
	needput = false;
}

uiTags::Tag::Tag(Glib::ustring tag)
{
	this->add(m_frame);
	m_Value.set_text(tag);
	m_Close.set_label("X");
	m_Close.set_relief(Gtk::RELIEF_NONE);
    m_Close.signal_clicked().connect(sigc::mem_fun(*this, &uiTags::Tag::on_delete_clicked) );

	m_hbox.pack_start(m_Close, false, false);
	m_hbox.pack_start(m_Value, false, false);
	m_frame.add(m_hbox);
	m_frame.set_shadow_type(Gtk::SHADOW_NONE);
	m_frame.set_border_width(2);
	show_all_children();
}

void uiTags::Tag::on_delete_clicked()
{
	m_signal_deleted.emit(m_Value.get_text());
}

uiTags::Tag::type_signal_deleted uiTags::Tag::signal_deleted()
{
	return m_signal_deleted;
}

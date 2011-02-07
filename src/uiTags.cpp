
#include "uiTags.h"

#include <iostream>


uiTags::uiTags() 
{

}

uiTags::~uiTags() {}

std::vector<Glib::ustring> uiTags::tags()
{
	return m_tags;
}

void uiTags::tags(std::vector<Glib::ustring> tags)
{
	m_tags = tags;
	refresh();
}

void uiTags::refresh()
{
	for(unsigned int i = 0; i < m_tag_widgets.size(); i++) 
		delete m_tag_widgets[i];

	m_tag_widgets.clear();

	for(unsigned int i = 0; i < m_tags.size(); i++) 
		m_tag_widgets.push_back(new Tag(m_tags[i]));

	int x=0;
	int y=0;
	int w,h;
	int width = this->get_width();
	for(unsigned int i = 0; i < m_tag_widgets.size(); i++) 
	{
		//TODO: Figure out how to get actual dimensions of tag.
		//      get_width() and get_allocation() returning 1.
		Gdk::Rectangle rec = m_tag_widgets[i]->get_allocation();
		w = 100;
		h = 40;
		this->put(*m_tag_widgets[i], x, y);
		if (x > 0 && x+w > width) { x = 0; y += h; }
		x += w;
	}

	show_all_children();
}

uiTags::Tag::Tag(Glib::ustring tag)
{
	m_Value.set_text(tag);
	m_Close.set_label("X");
	m_Close.set_relief(Gtk::RELIEF_NONE);
	m_hbox.pack_start(m_Close, false, false);
	m_hbox.pack_start(m_Value, false, false);
	this->add(m_hbox);
	this->set_shadow_type(Gtk::SHADOW_OUT);
	this->set_border_width(2);
}

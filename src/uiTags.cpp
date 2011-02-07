
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

	for(unsigned int i = 0; i < m_tag_widgets.size(); i++) 
		delete m_tag_widgets[i];

	m_tag_widgets.clear();

	for(unsigned int i = 0; i < m_tags.size(); i++) 
		m_tag_widgets.push_back(new Tag(m_tags[i]));

	for(unsigned int i = 0; i < m_tag_widgets.size(); i++)
	{
		this->put(*m_tag_widgets[i],50,0);
		m_tag_widgets[i]->show();
	}
	redraw();
}

void uiTags::redraw()
{
	int x=0;
	int y=0;
	int w,h;
	int width = this->get_width();
	m_AddNew.set_label("+ Add Tag");
	Gtk::Requisition rec = m_AddNew.size_request();
	x += rec.width;
	this->put(m_AddNew, 0,0);
	for(unsigned int i = 0; i < m_tag_widgets.size(); i++) 
	{
		//TODO: Figure out how to get actual dimensions of tag.
		//      get_width() and get_allocation() returning 1.
		rec = m_tag_widgets[i]->size_request();
		w = rec.width;
		h = rec.height;
		if (x > 0 && x+w > width) { x = 0; y += h; }
		this->move(*m_tag_widgets[i],x,y);
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
	this->set_shadow_type(Gtk::SHADOW_NONE);
	this->set_border_width(2);
	show_all_children();
}

#ifndef UIMENUBAR_H
#define UIMENUBAR_H

#include <gtkmm.h>

class uiMenuBar : public Gtk::MenuBar 
{
	public:
		uiMenuBar();
		virtual ~uiMenuBar();
		Gtk::Menu m_Menu_File;
		Gtk::Menu m_Menu_Help;
};

#endif

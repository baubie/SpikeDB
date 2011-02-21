#include "uiMenuBar.h"

uiMenuBar::uiMenuBar()
{
	this->items().push_back( Gtk::Menu_Helpers::MenuElem("_File", m_Menu_File) );
	this->items().push_back( Gtk::Menu_Helpers::MenuElem("_Help", m_Menu_Help) );
}

uiMenuBar::~uiMenuBar() {}

// Allow importing files
//mp_MenuImportFolder->set_sensitive(true);

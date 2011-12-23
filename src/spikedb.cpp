#include "stdafx.h"

#include "gui.h"

int main(int argc, char** argv)
{
    Gtk::Main kit(argc, argv);

    GUI* pWindow = new GUI();
	kit.run(*pWindow);
	delete pWindow; // Windows aren't managed so we have to delete them.

    return 0;
}

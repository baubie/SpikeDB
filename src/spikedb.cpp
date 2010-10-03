
#include <iostream>
#include "gui.h"

int main(int argc, char** argv)
{
    Gtk::Main kit(argc, argv);

    // Load the GtkBuilder file and instantiate its widgets
    Glib::RefPtr<Gtk::Builder> refBuilder = Gtk::Builder::create();
    try
    {
        refBuilder->add_from_file("spikedb.glade");
    }
    catch(const Glib::FileError& ex)
    {
        std::cerr << "FileError: " << ex.what() << std::endl;
        return 1;
    }
    catch(const Gtk::BuilderError& ex)
    {
        std::cerr << "BuilderError: " << ex.what() << std::endl;
        return 1;
    }


    GUI* pWindow = 0;
    refBuilder->get_widget_derived("mainWindow", pWindow);
    if (pWindow)
    {
        kit.run(*pWindow);
    }

    delete pWindow;

    return 0;
}

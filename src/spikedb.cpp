
#include "gui.h"
#include <iostream>

int main(int argc, char** argv)
{
    Gtk::Main kit(argc, argv);

    // Load the GtkBuilder file and instantiate its widgets
    Glib::RefPtr<Gtk::Builder> refBuilder = Gtk::Builder::create();
    try
    {
        refBuilder->add_from_file("ui/spikedb.glade");
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

	// Additional Cleanup
	Glib::Error::register_cleanup(); 
	Glib::wrap_register_cleanup();  

    return 0;
}

#include "gui.h"


GUI::GUI(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade)
: Gtk::Window(cobject),
  m_refGlade(refGlade),
  m_pMenuQuit(0),
  m_pAnimalTree(0),
  m_pDetailsList(0)
{
    set_title("Spike Database");

    // Setup the toolbar
    m_refGlade->get_widget("menuImportFolder", m_pMenuImportFolder);
    if (m_pMenuImportFolder)
        m_pMenuImportFolder->signal_activate().connect(sigc::mem_fun(*this, &GUI::on_menuImportFolder_activate));
    m_refGlade->get_widget("menuQuit", m_pMenuQuit);
    if (m_pMenuQuit)
        m_pMenuQuit->signal_activate().connect(sigc::mem_fun(*this, &GUI::on_menuQuit_activate));

    Gtk::TreeModelColumn<int> m_col_filenum;
    Gtk::TreeModelColumn<int> m_col_CF;
    Gtk::TreeModelColumn<int> m_col_depth;
    Gtk::TreeModelColumn<Glib::ustring> m_col_xaxis;
    Gtk::TreeModelColumn<Glib::ustring> m_col_tags;

    // Create Animals TreeView
    m_refGlade->get_widget("tvAnimals", m_pAnimalTree);
    m_refAnimalTree = Gtk::TreeStore::create(m_AnimalColumns);
    m_pAnimalTree->set_model(m_refAnimalTree);
    m_pAnimalTree->append_column("Animal/Cell ID", m_AnimalColumns.m_col_name);

    // Create Details TreeView
    m_refGlade->get_widget("tvDetails", m_pDetailsList);
    m_refDetailsList = Gtk::ListStore::create(m_DetailsColumns);
    m_pDetailsList->set_model(m_refDetailsList);
    m_pDetailsList->append_column("#", m_DetailsColumns.m_col_filenum);
    m_pDetailsList->append_column("CF (kHz)", m_DetailsColumns.m_col_CF);
    m_pDetailsList->append_column("Depth (um)", m_DetailsColumns.m_col_depth);
    m_pDetailsList->append_column("Variable", m_DetailsColumns.m_col_xaxis);
    m_pDetailsList->append_column("Tags", m_DetailsColumns.m_col_tags);

    show_all_children();
}

GUI::~GUI()
{
}

void GUI::deletePlots()
{

}


void GUI::populateAnimalTree()
{

}


void GUI::populateDetailsList()
{

}



// Signal Handlers
void GUI::on_menuImportFolder_activate()
{
    Gtk::FileChooserDialog dialog("Select a Folder Containing Spike Data Files",
        Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
    dialog.set_transient_for(*this);

    // Add response buttons to the dialog:
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

    // Show the dialog
    int result = dialog.run();

    switch(result)
    {
        case(Gtk::RESPONSE_OK):
            struct dirent *dptr;
            DIR *dirp;
            std::string filename = dialog.get_filename();
            if ((dirp=opendir(filename.c_str()))==NULL)
            {
                std::cerr << "ERROR: Unable to open " << filename << std::endl;
            } else {
                while ((dptr=readdir(dirp)))
                {
                    SpikeData sd;
                    if (dptr->d_type == 0x8)
                    {
                        std::string fullfile(filename);
                        fullfile += "/";
                        fullfile += dptr->d_name;
                        std::cout << "Filename: " << fullfile << std::endl;
                        if (sd.parse(fullfile.c_str()))
                            std::cout << "\t SPIKE FILE!" << std::endl;
                        else
                            std::cout << "\t lame file!" << std::endl;
                    }
                }
            }
            closedir(dirp);
            break;
    }
}
void GUI::on_menuQuit_activate()
{
    delete m_pMenuQuit;
    delete m_pAnimalTree;
    delete m_pDetailsList;
    hide();
}

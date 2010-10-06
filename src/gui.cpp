#include "gui.h"

void Tokenize(const std::string& str,
                      std::vector<std::string>& tokens,
                      const std::string& delimiters = " ")
{
    // Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (std::string::npos != pos || std::string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}


GUI::GUI(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade)
: Gtk::Window(cobject),
  m_refGlade(refGlade),
  m_pMenuQuit(0),
  m_pAnimalTree(0),
  m_pDetailsList(0),
  db("spikedb.db")
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
    m_refAnimalTree->set_sort_column(m_AnimalColumns.m_col_name,Gtk::SORT_ASCENDING);
    m_refAnimalTree->set_sort_func(0, sigc::mem_fun(*this,&GUI::on_animal_sort));
    m_refAnimalSelection = m_pAnimalTree->get_selection();
    m_refAnimalSelection->signal_changed().connect(
        sigc::mem_fun(*this, &GUI::changeAnimalSelection)
    );

    // Create Details TreeView
    m_refGlade->get_widget("tvDetails", m_pDetailsList);
    m_refDetailsList = Gtk::ListStore::create(m_DetailsColumns);
    m_pDetailsList->set_model(m_refDetailsList);
    m_pDetailsList->append_column("AnimalID", m_DetailsColumns.m_col_animalID);
    m_pDetailsList->append_column("CellID", m_DetailsColumns.m_col_cellID);
    m_pDetailsList->append_column("#", m_DetailsColumns.m_col_filenum);
    m_pDetailsList->append_column("CF (kHz)", m_DetailsColumns.m_col_CF);
    m_pDetailsList->append_column("Depth (um)", m_DetailsColumns.m_col_depth);
    m_pDetailsList->append_column("Variable", m_DetailsColumns.m_col_xaxis);
    m_pDetailsList->append_column("Tags", m_DetailsColumns.m_col_tags);
    m_refDetailsSelection = m_pDetailsList->get_selection();
    m_refDetailsSelection->set_mode(Gtk::SELECTION_MULTIPLE);
    m_refDetailsSelection->signal_changed().connect(
        sigc::mem_fun(*this, &GUI::changeDetailsSelection)
    );

    // Create Animal Details TreeView
    m_refGlade->get_widget("tvAnimalDetails", m_pAnimalDetailsList);
    m_refAnimalDetailsList = Gtk::ListStore::create(m_AnimalDetailsColumns);
    m_pAnimalDetailsList->set_model(m_refAnimalDetailsList);
    m_pAnimalDetailsList->append_column("Variable", m_AnimalDetailsColumns.m_col_name);
    m_pAnimalDetailsList->append_column_editable("Value", m_AnimalDetailsColumns.m_col_value);

    // Create Cell Details TreeView
    m_refGlade->get_widget("tvCellDetails", m_pCellDetailsList);
    m_refCellDetailsList = Gtk::ListStore::create(m_CellDetailsColumns);
    m_pCellDetailsList->set_model(m_refCellDetailsList);
    m_pCellDetailsList->append_column("Variable", m_CellDetailsColumns.m_col_name);
    m_pCellDetailsList->append_column_editable("Value", m_CellDetailsColumns.m_col_value);

    populateAnimalTree();

    show_all_children();
}

GUI::~GUI()
{
}

void GUI::deletePlots()
{

}

int GUI::on_animal_sort(const Gtk::TreeModel::iterator& a_, const Gtk::TreeModel::iterator& b_)
{
    const Gtk::TreeModel::Row a__ = *a_;
    const Gtk::TreeModel::Row b__ = *b_;

    Glib::ustring a = a__.get_value(m_AnimalColumns.m_col_name);
    Glib::ustring b = b__.get_value(m_AnimalColumns.m_col_name);
    if (a__.children().size() == 0 && a__.parent() != 0)
    {
       // Cell ID's.  Just sort by number.
       return (atoi(a.c_str()) > atoi(b.c_str()));
    }
    return a.compare(b);
}

void GUI::changeDetailsSelection()
{

}

void GUI::changeAnimalSelection()
{
    Gtk::TreeModel::iterator iter = m_refAnimalSelection->get_selected();
    if(iter) 
    {
        Gtk::TreeModel::Row row = *iter;
        if (row->parent() == 0) {
            // Root
            populateDetailsList("","");
        } else if (row->parent()->parent() == 0) {
            // First Level
            populateDetailsList(row->get_value(m_AnimalColumns.m_col_name),"");
        } else if (row->parent()->parent()->parent() == 0) {
           // Second Level 
            populateDetailsList(row->parent()->get_value(m_AnimalColumns.m_col_name),row->get_value(m_AnimalColumns.m_col_name));
        }
    }
}

void GUI::populateAnimalDetailsList(const Glib::ustring animalID)
{

}

void GUI::populateCellDetailsList(const Glib::ustring animalID, const Glib::ustring cellID)
{

}

void GUI::populateAnimalTree()
{
    std::string query("SELECT * FROM animals"); 
    std::vector< std::map<std::string,std::string> > r = db.query(query.c_str());

    m_refAnimalTree->clear();
    Gtk::TreeModel::Row base;
    Gtk::TreeModel::Row row;
    Gtk::TreeModel::Row childrow;
    base = *(m_refAnimalTree->append());
    base[m_AnimalColumns.m_col_name] = "Animals";
    for (unsigned int a = 0; a < r.size(); ++a)
    {
        row = *(m_refAnimalTree->append(base.children()));
        row[m_AnimalColumns.m_col_name] = r[a]["ID"];

        query = "SELECT * FROM cells WHERE animalID=\"";
        query += r[a]["ID"];
        query += "\"";
        std::vector< std::map<std::string,std::string> > rc = db.query(query.c_str());
        for (unsigned int c = 0; c < rc.size(); ++c)
        {
            childrow = *(m_refAnimalTree->append(row.children()));
            childrow[m_AnimalColumns.m_col_name] = rc[c]["cellID"];
        }
    }
}


void GUI::populateDetailsList(const Glib::ustring animalID, const Glib::ustring cellID)
{
    std::string query;
    std::cout << animalID << " " << cellID << std::endl;
    if (animalID != "" && cellID != "") {
       query = "SELECT * FROM files WHERE animalID=\"";
       query += animalID;
       query += "\" AND cellID=";
       query += cellID;
    } else if (animalID != "" && cellID == "") {
       query = "SELECT * FROM files WHERE animalID=\"";
       query += animalID;
       query += "\"";
    } else if (animalID == "" && cellID == "") {
       query = "SELECT * FROM files";
    }
    query += " ORDER BY animalID ASC, cellID ASC, fileID ASC";
    m_refDetailsList->clear();
    std::vector<std::map<std::string,std::string> > r = db.query(query.c_str());
    Gtk::TreeModel::Row row;
    for (unsigned int a = 0; a < r.size(); ++a)
    {
        row = *(m_refDetailsList->append());
        row[m_DetailsColumns.m_col_animalID] = r[a]["animalID"];
        row[m_DetailsColumns.m_col_cellID] = atoi(r[a]["cellID"].c_str());
        row[m_DetailsColumns.m_col_filenum] = atoi(r[a]["fileID"].c_str());
    }
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
                        if (sd.parse(fullfile.c_str()))
                        {
                            std::vector<std::string> fileTokens;
                            std::string shortfilename(dptr->d_name);
                            Tokenize(shortfilename, fileTokens, ".");

                            // Insert animal 
                            std::string query("INSERT INTO animals (ID) VALUES(\""); 
                            query += fileTokens[0];
                            query += "\")";
                            db.query(query.c_str());

                            // Insert Cell
                            query = "INSERT INTO cells (animalID,cellID) VALUES(\"";
                            query += fileTokens[0];
                            query += "\",";
                            query += fileTokens[1];
                            query += ")";
                            db.query(query.c_str());

                            // Insert File
                            query = "INSERT INTO file (animalID,cellID,fileID) VALUES(\"";
                            query += fileTokens[0];
                            query += "\",";
                            query += fileTokens[1];
                            query += ",";
                            query += fileTokens[2];
                            query += ")";
                            db.query(query.c_str());
                        }
                    }
                }
            }
            closedir(dirp);
            break;
    }
    populateAnimalTree();
}

void GUI::on_menuQuit_activate()
{
    delete m_pMenuQuit;
    delete m_pAnimalTree;
    delete m_pDetailsList;
    db.close();
    hide();
}

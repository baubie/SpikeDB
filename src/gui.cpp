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
  m_pDetailsList(0)
{
    set_title("Spike Database");

    if (sqlite3_open("spikedb.db", &db) != SQLITE_OK) {
        std::cerr << "CRITICAL ERROR: Unable to open database file." << std::endl;
        return;
    }

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
    m_pDetailsList->append_column_editable("CF (kHz)", m_DetailsColumns.m_col_CF);
    m_pDetailsList->append_column_editable("Depth (um)", m_DetailsColumns.m_col_depth);
    m_pDetailsList->append_column_editable("X-Var", m_DetailsColumns.m_col_xaxis);
    m_pDetailsList->append_column_editable("Tags", m_DetailsColumns.m_col_tags);
    m_refDetailsSelection = m_pDetailsList->get_selection();
    m_refDetailsSelection->set_mode(Gtk::SELECTION_MULTIPLE);
    m_refDetailsSelection->signal_changed().connect(
        sigc::mem_fun(*this, &GUI::changeDetailsSelection)
    );
    m_rend_CF.signal_edited().connect(sigc::mem_fun(*this, &GUI::on_detailscolumn_edited));

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

    m_refGlade->get_widget("hpanedPlots", m_pHPanedPlots);
    m_pPlotSpikes = new EasyPlotmm();
    m_pPlotMeans = new EasyPlotmm();
    m_pHPanedPlots->add1(*m_pPlotSpikes);
    m_pHPanedPlots->add2(*m_pPlotMeans);
    show_all_children();
}

GUI::~GUI()
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

void GUI::on_detailscolumn_edited(const Glib::ustring& path_string, const Glib::ustring& new_text)
{
    Gtk::TreePath path(path_string);
}

void GUI::changeDetailsSelection()
{
    m_refDetailsSelection->selected_foreach_iter(
        sigc::mem_fun(*this, &GUI::addFileToPlot)
        );
}

void GUI::addFileToPlot(const Gtk::TreeModel::iterator& iter)
{
    Gtk::TreeModel::Row row = *iter;
    sqlite3_stmt *stmt=0;
    const char query[] = "SELECT header,spikes FROM files WHERE animalID=? AND cellID=? AND fileID=?";
    sqlite3_prepare_v2(db,query,strlen(query), &stmt, NULL);
    sqlite3_bind_text(stmt, 1, row.get_value(m_DetailsColumns.m_col_animalID).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, row.get_value(m_DetailsColumns.m_col_cellID));
    sqlite3_bind_int(stmt, 3, row.get_value(m_DetailsColumns.m_col_filenum));
    int r;
    r = sqlite3_step(stmt);
    SpikeData sd;
    if (r == SQLITE_ROW)
    {
        void *header = (void*)sqlite3_column_blob(stmt,0);
        const HEADER *h = new HEADER(*static_cast<HEADER*>(header));
        sd.m_head = *h;
        
        SPIKESTRUCT *spikes = (SPIKESTRUCT*)sqlite3_column_blob(stmt,1);
        int spikes_length = sqlite3_column_bytes(stmt,1);
        int numSpikes = spikes_length/sizeof(SPIKESTRUCT);
        sd.m_spikeArray.assign(spikes,spikes+numSpikes);

        sd.printfile();
    } 
    else { std::cerr << "ERROR: Failed to read file from database. " << sqlite3_errmsg(db) << std::endl; }
    sqlite3_finalize(stmt);
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
    char query_animals[] = "SELECT ID FROM animals";
    sqlite3_stmt *stmt_animals, *stmt_cells;
    sqlite3_prepare_v2(db, query_animals, -1, &stmt_animals, 0);
    m_refAnimalTree->clear();
    Gtk::TreeModel::Row base;
    Gtk::TreeModel::Row row;
    Gtk::TreeModel::Row childrow;
    base = *(m_refAnimalTree->append());
    base[m_AnimalColumns.m_col_name] = "Animals";
    int r_animals, r_cells;
    while (true)
    {
        r_animals = sqlite3_step(stmt_animals);
        if (r_animals == SQLITE_ROW)
        {
            row = *(m_refAnimalTree->append(base.children()));
            char* animalID = (char*)sqlite3_column_text(stmt_animals,0);
            row[m_AnimalColumns.m_col_name] = animalID;
            char query_cells[] = "SELECT cellID FROM cells WHERE animalID=?";
            sqlite3_prepare_v2(db, query_cells, -1, &stmt_cells, 0);
            sqlite3_bind_text(stmt_cells, 1, animalID, -1, SQLITE_TRANSIENT);
            while (true)
            {
                r_cells = sqlite3_step(stmt_cells);
                if (r_cells == SQLITE_ROW)
                {
                    char* cellID = (char*)sqlite3_column_text(stmt_cells,0);
                    childrow = *(m_refAnimalTree->append(row.children()));
                    childrow[m_AnimalColumns.m_col_name] = cellID;
                } else { break; }
            }
        } else { break; }
    }
}


void GUI::populateDetailsList(const Glib::ustring animalID, const Glib::ustring cellID)
{
    sqlite3_stmt *stmt=0;
    if (animalID != "" && cellID != "") {
        const char query[] = "SELECT animalID, cellID, fileID FROM files WHERE animalID=? AND cellID=? ORDER BY animalID, cellID, fileID";
        sqlite3_prepare_v2(db,query,strlen(query), &stmt, NULL);
        sqlite3_bind_text(stmt, 1, animalID.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, cellID.c_str(), -1, SQLITE_TRANSIENT);
    } else if (animalID != "" && cellID == "") {
        const char query[] = "SELECT animalID, cellID, fileID FROM files WHERE animalID=? ORDER BY animalID, cellID, fileID";
        sqlite3_prepare_v2(db,query,strlen(query), &stmt, NULL);
        sqlite3_bind_text(stmt, 1, animalID.c_str(), -1, SQLITE_TRANSIENT);
    } else if (animalID == "" && cellID == "") {
        const char query[] = "SELECT animalID, cellID, fileID FROM files ORDER BY animalID, cellID, fileID";
        sqlite3_prepare_v2(db,query,strlen(query), &stmt, NULL);
    }

    m_refDetailsList->clear();
    Gtk::TreeModel::Row row;
    while(true)
    {
        int r = sqlite3_step(stmt);
        if (r == SQLITE_ROW)
        {
            row = *(m_refDetailsList->append());
            row[m_DetailsColumns.m_col_animalID] = (char*)sqlite3_column_text(stmt,0);
            row[m_DetailsColumns.m_col_cellID] = sqlite3_column_int(stmt,1);
            row[m_DetailsColumns.m_col_filenum] = sqlite3_column_int(stmt,2);
        } else { break; }
    }
    sqlite3_finalize(stmt);
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
                            const char q_animal[] = "INSERT INTO animals (ID) VALUES(?)";
                            sqlite3_stmt *stmt_animal=0;
                            sqlite3_prepare_v2(db,q_animal,strlen(q_animal),&stmt_animal,NULL);
                            sqlite3_bind_text(stmt_animal,1,fileTokens[0].c_str(),-1,SQLITE_TRANSIENT);
                            sqlite3_step(stmt_animal);
                            sqlite3_finalize(stmt_animal);

                            // Insert Cell
                            const char q_cell[] = "INSERT INTO cells (animalID,cellID) VALUES(?,?)";
                            sqlite3_stmt *stmt_cell=0;
                            sqlite3_prepare_v2(db,q_cell,strlen(q_cell), &stmt_cell, NULL);
                            sqlite3_bind_text(stmt_cell,1,fileTokens[0].c_str(),-1,SQLITE_TRANSIENT);
                            sqlite3_bind_int(stmt_cell,2,atoi(fileTokens[1].c_str()));
                            sqlite3_step(stmt_cell);
                            sqlite3_finalize(stmt_cell);

                            // Insert File
                            const char q_file[] = "INSERT INTO files (animalID,cellID,fileID,header,spikes) VALUES(?,?,?,?,?)";
                            sqlite3_stmt *stmt_file=0;
                            sqlite3_prepare_v2(db,q_file,strlen(q_file), &stmt_file, NULL);
                            sqlite3_bind_text(stmt_file, 1, fileTokens[0].c_str(), -1, SQLITE_TRANSIENT);
                            sqlite3_bind_int(stmt_file, 2, atoi(fileTokens[1].c_str()));
                            sqlite3_bind_int(stmt_file, 3, atoi(fileTokens[2].c_str()));
                            sqlite3_bind_blob(stmt_file, 4, (void*)&sd.m_head, sizeof(sd.m_head), SQLITE_TRANSIENT);
                            sqlite3_bind_blob(stmt_file, 5, (void*)&sd.m_spikeArray[0], sizeof(SPIKESTRUCT)*sd.m_spikeArray.size(), SQLITE_TRANSIENT);
                            sqlite3_step(stmt_file);
                            sqlite3_finalize(stmt_file);
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
    delete m_pMenuImportFolder;
    delete m_pAnimalTree;
    delete m_pDetailsList;
    delete m_pHPanedPlots;
    delete m_pCellDetailsList;
    delete m_pAnimalDetailsList;
    delete m_pPlotSpikes;
    delete m_pPlotMeans;
    sqlite3_close(db);
    hide();
}

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
  m_adjMinFiles(5,1,10,1,3,0)
{

    set_title("Spike Database - No database open");

    // Setup the toolbar
    m_refGlade->get_widget("menuOpenDatabase", m_pMenuOpenDatabase);
    if (m_pMenuOpenDatabase)
        m_pMenuOpenDatabase->signal_activate().connect(sigc::mem_fun(*this, &GUI::on_menuOpenDatabase_activate));
    m_refGlade->get_widget("menuImportFolder", m_pMenuImportFolder);
    if (m_pMenuImportFolder)
        m_pMenuImportFolder->signal_activate().connect(sigc::mem_fun(*this, &GUI::on_menuImportFolder_activate));
    m_refGlade->get_widget("menuQuit", m_pMenuQuit);
    if (m_pMenuQuit)
        m_pMenuQuit->signal_activate().connect(sigc::mem_fun(*this, &GUI::on_menuQuit_activate));

	// Setup the statusbar
	m_refGlade->get_widget("statusbar", m_pStatusbar);

	// Setup the filter widgets
	m_refGlade->get_widget("sbMinFiles", m_pMinFiles);
	m_pMinFiles->set_adjustment(m_adjMinFiles);
	m_adjMinFiles.signal_value_changed().connect(sigc::mem_fun(*this,&GUI::updateFilter));

	// Setup the analyze widgets
	m_refGlade->get_widget("vboxAnalyze", m_pVBoxAnalyze);
	m_refGlade->get_widget("cbDataSource", m_pDataSource);
	m_refGlade->get_widget("cbXVar", m_pYVar);
	m_refGlade->get_widget("cbYVar", m_pXVar);
	m_refDataSource = Gtk::ListStore::create(m_DataSourceColumns);
	m_refXVar = Gtk::ListStore::create(m_XVarColumns);
	m_refYVar = Gtk::ListStore::create(m_YVarColumns);
    m_pDataSource->set_model(m_refDataSource); 
    m_pXVar->set_model(m_refXVar); 
    m_pYVar->set_model(m_refYVar); 
	m_pDataSource->pack_start(m_DataSourceColumns.m_col_name);
	m_pXVar->pack_start(m_XVarColumns.m_col_name);
	m_pYVar->pack_start(m_YVarColumns.m_col_name);
	m_pDataSource->signal_changed().connect( sigc::mem_fun(*this, &GUI::on_analyze_changed));
	m_pXVar->signal_changed().connect( sigc::mem_fun(*this, &GUI::on_analyze_changed));
	m_pYVar->signal_changed().connect( sigc::mem_fun(*this, &GUI::on_analyze_changed));
	Gtk::TreeModel::Row row = *(m_refDataSource->append());
	row[m_DataSourceColumns.m_col_name] = "Animals";
	row = *(m_refDataSource->append());
	row[m_DataSourceColumns.m_col_name] = "Cells";
	m_pDataSource->set_active(0);
	on_analyze_changed();

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
    m_pDetailsList->append_column("X-Var", m_DetailsColumns.m_col_xaxis);
    m_pDetailsList->append_column("Type", m_DetailsColumns.m_col_type);
    m_pDetailsList->append_column("Trials", m_DetailsColumns.m_col_trials);
    m_pDetailsList->append_column("CarFreq (Hz)", m_DetailsColumns.m_col_freq);
    m_pDetailsList->append_column("Dur (ms)", m_DetailsColumns.m_col_dur);
    m_pDetailsList->append_column("Onset (ms)", m_DetailsColumns.m_col_onset);
    m_pDetailsList->append_column("Atten (db)", m_DetailsColumns.m_col_atten);

    /*
    // For the time being, we won't tag individual files
    m_tvcol_filetags.set_title("Tags");
    m_rend_filetags.property_editable() = true;
    m_tvcol_filetags.pack_start(m_rend_filetags);
    m_pDetailsList->append_column(m_tvcol_filetags);
    m_rend_filetags.signal_edited().connect(sigc::mem_fun(*this, &GUI::on_filetags_edited));
    m_tvcol_filetags.set_cell_data_func(m_rend_filetags,sigc::mem_fun(*this, &GUI::filetags_cell_data));
    */

    m_refDetailsSelection = m_pDetailsList->get_selection();
    m_refDetailsSelection->set_mode(Gtk::SELECTION_MULTIPLE);
    m_refDetailsSelection->signal_changed().connect(
        sigc::mem_fun(*this, &GUI::changeDetailsSelection)
    );

    // Create Animal Details TreeView
    m_refGlade->get_widget("tvAnimalDetails", m_pAnimalDetailsList);
    m_refAnimalDetailsList = Gtk::ListStore::create(m_AnimalDetailsColumns);
    m_pAnimalDetailsList->set_model(m_refAnimalDetailsList);
    m_pAnimalDetailsList->append_column("Name", m_AnimalDetailsColumns.m_col_name);
    
    m_tvcol_animalvalue.set_title("Value");
    m_rend_animalvalue.property_editable() = true;
    m_tvcol_animalvalue.pack_start(m_rend_animalvalue);
    m_pAnimalDetailsList->append_column(m_tvcol_animalvalue);
    m_rend_animalvalue.signal_edited().connect(sigc::mem_fun(*this, &GUI::on_animalvalue_edited));
    m_tvcol_animalvalue.set_cell_data_func(m_rend_animalvalue,sigc::mem_fun(*this, &GUI::animalvalue_cell_data));

    // Create Cell Details TreeView
    m_refGlade->get_widget("tvCellDetails", m_pCellDetailsList);
    m_refCellDetailsList = Gtk::ListStore::create(m_CellDetailsColumns);
    m_pCellDetailsList->set_model(m_refCellDetailsList);
    m_pCellDetailsList->append_column("Name", m_CellDetailsColumns.m_col_name);

    m_tvcol_cellvalue.set_title("Value");
    m_rend_cellvalue.property_editable() = true;
    m_tvcol_cellvalue.pack_start(m_rend_cellvalue);
    m_pCellDetailsList->append_column(m_tvcol_cellvalue);
    m_rend_cellvalue.signal_edited().connect(sigc::mem_fun(*this, &GUI::on_cellvalue_edited));
    m_tvcol_cellvalue.set_cell_data_func(m_rend_cellvalue,sigc::mem_fun(*this, &GUI::cellvalue_cell_data));


    m_refGlade->get_widget("hboxPlots", m_pHBoxPlots);
    m_pPlotSpikes = new EasyPlotmm();
    m_pPlotMeans = new EasyPlotmm();
    m_pPlotAnalyze = new EasyPlotmm();
    m_pHBoxPlots->pack_start(*m_pPlotSpikes);
    m_pHBoxPlots->pack_start(*m_pPlotMeans);
	m_pVBoxAnalyze->pack_start(*m_pPlotAnalyze);
    show_all_children();
}

GUI::~GUI()
{
}

void GUI::on_menuOpenDatabase_activate()
{

    Gtk::FileChooserDialog dialog("Select a Folder Containing Spike Data Files",
        Gtk::FILE_CHOOSER_ACTION_OPEN);
    dialog.set_transient_for(*this);

    // Add response buttons to the dialog:
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

    // Show the dialog
    int result = dialog.run();

    switch(result)
    {
        case(Gtk::RESPONSE_OK):
            std::string filename = dialog.get_filename();
			openDatabase(filename);
    		break;
	}
}

bool GUI::openDatabase(std::string filename)
{
    if (sqlite3_open(filename.c_str(), &db) != SQLITE_OK) {
        std::cerr << "CRITICAL ERROR: Unable to open database file." << std::endl;
        return false;
    }
	set_title("Spike Database - " + filename);

    // Do we have to update the database?
    sqlite3_stmt *stmt=0;
    const char query[] = "SELECT value FROM properties WHERE variable=?";
    sqlite3_prepare_v2(db,query,strlen(query), &stmt, NULL);
    sqlite3_bind_text(stmt, 1, "version", -1, SQLITE_TRANSIENT);
    int r = sqlite3_step(stmt);
	float version = 0;
    if (r == SQLITE_ROW)
	{
		version = (float)sqlite3_column_double(stmt,0);
	}
	int count = 0;
	while (version < CURRENT_DB_VERSION)
	{
		++count;
		if (count > 1000) {
			Gtk::MessageDialog dialog(*this, "Unable to update database file.", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
			dialog.set_secondary_text("This could be due to database corruption or a version too old to get updated.  Contact Brandon Aubie at brandon@aubie.ca for more information.");
			dialog.run();
			return false;
		}

		if (version == 1.0)
		{
			sqlite3_stmt *stmt_fix=0;
			const char query[] = "ALTER TABLE cells ADD COLUMN recordedby TEXT";
			sqlite3_prepare_v2(db,query,strlen(query), &stmt_fix, NULL);
			int r_fix = sqlite3_step(stmt_fix);
			if (r_fix == SQLITE_DONE)
			{
				version = 1.1;
				const char query[] = "UPDATE properties SET value=1.1 WHERE variable=?";
				sqlite3_prepare_v2(db,query,strlen(query), &stmt_fix, NULL);
				sqlite3_bind_text(stmt_fix, 1, "version", -1, SQLITE_TRANSIENT);
				r_fix = sqlite3_step(stmt_fix);
			Gtk::MessageDialog dialog(*this, "Database successfully upgraded from version 1.0 to version 1.1.", false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK);
			dialog.run();
			}
		}
	}

    populateAnimalTree();
	return true;
}

void GUI::updateFilter()
{
	// Pretend we clicked on an animal to repopulate details list
	changeAnimalSelection();
}

void GUI::on_analyze_changed()
{
	// Update the analyze plot
	Gtk::TreeModel::Row row;
	static int dataSource = -1;

	if (m_pDataSource->get_active_row_number() != dataSource)
	{
		dataSource = m_pDataSource->get_active_row_number();
		if (m_pDataSource->get_active_row_number() == 0)
		{
			m_refXVar->clear();
			m_refYVar->clear();
			row = *(m_refXVar->append());
			row[m_XVarColumns.m_col_name] = "Weight";

			row = *(m_refYVar->append());
			row[m_YVarColumns.m_col_name] = "Weight";
		}
		if (m_pDataSource->get_active_row_number() == 1)
		{
			m_refXVar->clear();
			m_refYVar->clear();
			row = *(m_refXVar->append());
			row[m_XVarColumns.m_col_name] = "CarFreq (Hz)";
			row = *(m_refXVar->append());
			row[m_XVarColumns.m_col_name] = "Threshold (dB SPL)";
			row = *(m_refXVar->append());
			row[m_XVarColumns.m_col_name] = "Depth (um)";

			row = *(m_refYVar->append());
			row[m_YVarColumns.m_col_name] = "CarFreq (Hz)";
			row = *(m_refYVar->append());
			row[m_YVarColumns.m_col_name] = "Threshold (dB SPL)";
			row = *(m_refYVar->append());
			row[m_YVarColumns.m_col_name] = "Depth (um)";
		}
		m_pXVar->set_active(0);
		m_pYVar->set_active(0);
	}
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

void GUI::on_filetags_edited(const Glib::ustring& path_string, const Glib::ustring& new_text)
{
    Gtk::TreePath path(path_string);

    Gtk::TreeModel::iterator iter = m_refDetailsList->get_iter(path);
    if (iter)
    {
        // Update tree model
        Gtk::TreeModel::Row row = *iter;
        row[m_DetailsColumns.m_col_tags] = new_text;

        // Update sqlite database
        const char q[] = "UPDATE files SET tags=? WHERE animalID=? AND cellID=? AND fileID=?";
        sqlite3_stmt *stmt=0;
        sqlite3_prepare_v2(db,q,strlen(q),&stmt,NULL);
        sqlite3_bind_text(stmt,1,new_text.c_str(),-1,SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt,2,row.get_value(m_DetailsColumns.m_col_animalID).c_str(),-1,SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt,3,row[m_DetailsColumns.m_col_cellID]);
        sqlite3_bind_int(stmt,4,row[m_DetailsColumns.m_col_filenum]);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }
}

void GUI::filetags_cell_data(Gtk::CellRenderer* /*renderer*/, const Gtk::TreeModel::iterator& iter)
{
    if (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        m_rend_filetags.property_text() = row[m_DetailsColumns.m_col_tags];
    }
}

void GUI::on_animalvalue_edited(const Glib::ustring& path_string, const Glib::ustring& new_text)
{
    Gtk::TreePath path(path_string);

    Gtk::TreeModel::iterator iter = m_refAnimalDetailsList->get_iter(path);
    if (iter)
    {
        // Update tree model
        Gtk::TreeModel::Row row = *iter;

        // Update sqlite database
        Glib::ustring query;
        if (row.get_value(m_AnimalDetailsColumns.m_col_name) == "Tags")
        {
            query = "UPDATE animals SET tags=? WHERE ID=?";
        }
        if (row.get_value(m_AnimalDetailsColumns.m_col_name) == "Species")
        {
            query = "UPDATE animals SET species=? WHERE ID=?";
        }
        if (row.get_value(m_AnimalDetailsColumns.m_col_name) == "Sex")
        {
            query = "UPDATE animals SET sex=? WHERE ID=?";
        }
        if (row.get_value(m_AnimalDetailsColumns.m_col_name) == "Weight (g)")
        {
            query = "UPDATE animals SET weight=? WHERE ID=?";
        }
        if (row.get_value(m_AnimalDetailsColumns.m_col_name) == "Age")
        {
            query = "UPDATE animals SET age=? WHERE ID=?";
        }
        if (row.get_value(m_AnimalDetailsColumns.m_col_name) == "Notes")
        {
            query = "UPDATE animals SET notes=? WHERE ID=?";
        }

        const char* q = query.c_str();
        sqlite3_stmt *stmt=0;
        sqlite3_prepare_v2(db,q,strlen(q),&stmt,NULL);
        sqlite3_bind_text(stmt,1,new_text.c_str(),-1,SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt,2,row.get_value(m_AnimalDetailsColumns.m_col_animalID).c_str(),-1,SQLITE_TRANSIENT);
        int r = sqlite3_step(stmt);
        if (r == SQLITE_DONE)
        {
            row[m_AnimalDetailsColumns.m_col_value] = new_text;
        } else {
            std::cerr << "ERROR: Could not update animal. " << sqlite3_errmsg(db) << std::endl; 
        }
        sqlite3_finalize(stmt);
    }
}

void GUI::animalvalue_cell_data(Gtk::CellRenderer* /*renderer*/, const Gtk::TreeModel::iterator& iter)
{
    if (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        m_rend_animalvalue.property_text() = row[m_AnimalDetailsColumns.m_col_value];
        if (row[m_AnimalDetailsColumns.m_col_name] != "ID")
        {
            m_rend_animalvalue.property_editable() = true;
            m_rend_animalvalue.property_cell_background() = "#DDEEFF";
        } else {
            m_rend_animalvalue.property_editable() = false;
            m_rend_animalvalue.property_cell_background() = "#FFDDDD";
        }
    }
}

void GUI::on_cellvalue_edited(const Glib::ustring& path_string, const Glib::ustring& new_text)
{
    Gtk::TreePath path(path_string);

    Gtk::TreeModel::iterator iter = m_refCellDetailsList->get_iter(path);
    if (iter)
    {
        // Update tree model
        Gtk::TreeModel::Row row = *iter;

        // Update sqlite database
        Glib::ustring query;
        if (row.get_value(m_CellDetailsColumns.m_col_name) == "CarFreq (Hz)")
        {
            query = "UPDATE cells SET freq=? WHERE animalID=? AND cellID=?";
        }
        if (row.get_value(m_CellDetailsColumns.m_col_name) == "Depth (um)")
        {
            query = "UPDATE cells SET depth=? WHERE animalID=? AND cellID=?";
        }
        if (row.get_value(m_CellDetailsColumns.m_col_name) == "Threshold (dB SPL)")
        {
            query = "UPDATE cells SET threshold=? WHERE animalID=? AND cellID=?";
        }
        if (row.get_value(m_CellDetailsColumns.m_col_name) == "Tags")
        {
            query = "UPDATE cells SET tags=? WHERE animalID=? AND cellID=?";
        }
        if (row.get_value(m_CellDetailsColumns.m_col_name) == "Notes")
        {
            query = "UPDATE cells SET notes=? WHERE animalID=? AND cellID=?";
        }
        const char* q = query.c_str();
        sqlite3_stmt *stmt=0;
        sqlite3_prepare_v2(db,q,strlen(q),&stmt,NULL);
        sqlite3_bind_text(stmt,1,new_text.c_str(),-1,SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt,2,row.get_value(m_CellDetailsColumns.m_col_animalID).c_str(),-1,SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt,3,row.get_value(m_CellDetailsColumns.m_col_cellID));
        int r = sqlite3_step(stmt);
        if (r == SQLITE_DONE)
        {
            row[m_CellDetailsColumns.m_col_value] = new_text;
        } else {
            std::cerr << "ERROR: Could not update cell. " << sqlite3_errmsg(db) << std::endl; 
        }
        sqlite3_finalize(stmt);
    }
}

void GUI::cellvalue_cell_data(Gtk::CellRenderer* /*renderer*/, const Gtk::TreeModel::iterator& iter)
{
    if (iter)
    {
        Gtk::TreeModel::Row row = *iter;
        m_rend_cellvalue.property_text() = row[m_CellDetailsColumns.m_col_value];
        if (row[m_CellDetailsColumns.m_col_name] != "Cell ID")
        {
            m_rend_cellvalue.property_editable() = true;
            m_rend_cellvalue.property_cell_background() = "#DDEEFF";
        } else {
            m_rend_cellvalue.property_editable() = false;
            m_rend_cellvalue.property_cell_background() = "#FFDDDD";
        }
    }
}

void GUI::changeDetailsSelection()
{
    m_pPlotMeans->clear();
    m_pPlotSpikes->clear();
    curXVariable = "";
    m_refDetailsSelection->selected_foreach_iter(
        sigc::mem_fun(*this, &GUI::addFileToPlot)
        );
    m_refDetailsSelection->selected_foreach_iter(
        sigc::mem_fun(*this, &GUI::updateSideLists)
        );
}

void GUI::updateSideLists(const Gtk::TreeModel::iterator& iter)
{
	Gtk::TreeModel::Row row = *iter;
	populateAnimalDetailsList(row.get_value(m_DetailsColumns.m_col_animalID));
	populateCellDetailsList(row.get_value(m_DetailsColumns.m_col_animalID),
							row.get_value(m_DetailsColumns.m_col_cellID));
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

        std::string xVariable = sd.xVariable();
        if (curXVariable == "") curXVariable = xVariable;
        else if(curXVariable != xVariable)
        {
            std::cerr << "ERROR: Plots have different X-Variables." << std::endl;
            return;
        }
        SPIKESTRUCT *spikes = (SPIKESTRUCT*)sqlite3_column_blob(stmt,1);
        int spikes_length = sqlite3_column_bytes(stmt,1);
        int numSpikes = spikes_length/sizeof(SPIKESTRUCT);
        sd.m_spikeArray.assign(spikes,spikes+numSpikes);

        std::vector<double> x(sd.m_head.nSweeps,0);
        std::vector<double> y(sd.m_head.nSweeps,0);
        std::vector<double> err(sd.m_head.nSweeps,0);
        std::vector<double> N(sd.m_head.nSweeps,0);

        std::vector<double> x_spikes;
        std::vector<double> y_spikes; 

        EasyPlotmm::Pen spikesPen;
        spikesPen.linewidth = 0.0;
        spikesPen.shape = EasyPlotmm::POINT;
        spikesPen.pointsize = 2;
        spikesPen.filled = true;

        double dy = sd.delta()/(sd.m_head.nPasses*1.25);

		// Calculate the means and get the spike times 
        for (int i = 0; i < sd.m_head.nSweeps; ++i)
        {
            x.at(i) = sd.xvalue(i);
            for (int p = 0; p < sd.m_head.nPasses; ++p)
            {
                for (unsigned int s = 0; s < sd.m_spikeArray.size(); ++s)
                {
					// Spike sweeps are 1 based but here we are 0 based
                    if (sd.m_spikeArray[s].nSweep == i+1 && sd.m_spikeArray[s].nPass == p)
                    {
                        if (sd.m_head.nPasses > 0)
                        {
                            y.at(i) += 1.0f / sd.m_head.nPasses;
                        }
                        x_spikes.push_back(sd.m_spikeArray[s].fTime);
                        y_spikes.push_back(sd.xvalue(i)+dy*p);
                    }
                }
            }
        }
        m_pPlotSpikes->axes(0,sd.m_head.nRepInt, sd.xvalue(0)-2*dy, sd.xvalue(sd.m_head.nSweeps-1)+sd.m_head.nPasses*dy);
        m_pPlotSpikes->plot(x_spikes,y_spikes,spikesPen);

		// Calculate the SD of the means
        for (int i = 0; i < sd.m_head.nSweeps; ++i)
        {
            for (int p = 0; p < sd.m_head.nPasses; ++p)
            {
				int numSpikes = 0;
                for (unsigned int s = 0; s < sd.m_spikeArray.size(); ++s)
                {
					// Spike sweeps are 1 based but here we are 0 based
                    if (sd.m_spikeArray[s].nSweep == i+1 && sd.m_spikeArray[s].nPass == p)
                    {
						numSpikes++;
                    }
                }
				if (numSpikes > 0) ++(N.at(i));
				err.at(i) += (numSpikes-y.at(i))*(numSpikes-y.at(i));
            }
        }
		for (int i = 0; i < sd.m_head.nSweeps; ++i)
		{
			if (N.at(i) > 1)
				// Calculate Standard Error from the Standard Deviation
				err.at(i) = sqrt(err.at(i)/(N.at(i)-1))/sqrt(N.at(i)); // N-1 as we have a sample of points
			else
				err.at(i) = 0;
		}
        m_pPlotMeans->plot(x,y,err);

        // Add stimuli to spikes plot
        EasyPlotmm::Pen ch1Pen;
        ch1Pen.linewidth = 2.0;
        ch1Pen.shape = EasyPlotmm::NONE;
        ch1Pen.color.r = 1;
        ch1Pen.color.g = 0;
        ch1Pen.color.b = 0;
        ch1Pen.color.a = 1;
        EasyPlotmm::Pen ch2Pen;
        ch2Pen.linewidth = 2.0;
        ch2Pen.shape = EasyPlotmm::NONE;
        ch2Pen.color.r = 0;
        ch2Pen.color.g = 0;
        ch2Pen.color.b = 1;
        ch2Pen.color.a = 1;
        for (int i = 0; i < sd.m_head.nSweeps; ++i)
        {
            std::vector<double> stimX;
            std::vector<double> stimY;
            stimX.push_back(sd.m_head.stimFirstCh1.fBegin+sd.m_head.deltaCh1.fBegin*i);
            stimY.push_back(sd.xvalue(i));
            stimX.push_back(sd.m_head.stimFirstCh1.fBegin+sd.m_head.stimFirstCh1.fDur+sd.m_head.deltaCh1.fBegin*i+sd.m_head.deltaCh1.fDur*i);
            stimY.push_back(sd.xvalue(i));
            m_pPlotSpikes->plot(stimX,stimY,ch1Pen);

            if (sd.m_head.nOnCh2 == 1)
            {
                stimX.clear();
                stimY.clear();
                stimX.push_back(sd.m_head.stimFirstCh2.fBegin+sd.m_head.deltaCh2.fBegin*i);
                stimY.push_back(sd.xvalue(i));
                stimX.push_back(sd.m_head.stimFirstCh2.fBegin+sd.m_head.stimFirstCh2.fDur+sd.m_head.deltaCh2.fBegin*i+sd.m_head.deltaCh2.fDur*i);
                stimY.push_back(sd.xvalue(i));
                m_pPlotSpikes->plot(stimX,stimY,ch2Pen);
            }
        }
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
            populateDetailsList("",-1);
        } else if (row->parent()->parent() == 0) {
            // First Level
            populateDetailsList(row->get_value(m_AnimalColumns.m_col_name),-1);
			populateAnimalDetailsList(row->get_value(m_AnimalColumns.m_col_name));
        } else if (row->parent()->parent()->parent() == 0) {
           // Second Level 
            populateDetailsList(row->parent()->get_value(m_AnimalColumns.m_col_name),
							atoi(row->get_value(m_AnimalColumns.m_col_name).c_str()));
			populateAnimalDetailsList(row->parent()->get_value(m_AnimalColumns.m_col_name));
			populateCellDetailsList(row->parent()->get_value(m_AnimalColumns.m_col_name),
							atoi(row->get_value(m_AnimalColumns.m_col_name).c_str()));
        }
    }
}

void GUI::populateAnimalDetailsList(const Glib::ustring animalID)
{
    m_refAnimalDetailsList->clear();
	char query[] = "SELECT species, sex, weight, age, tags, notes FROM animals WHERE ID=?";
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(db, query, -1, &stmt, 0);
	sqlite3_bind_text(stmt,1, animalID.c_str(), -1, SQLITE_TRANSIENT);
	int r;
	r = sqlite3_step(stmt);

	Gtk::TreeModel::Row row;

	if (r == SQLITE_ROW)
	{
    	row = *(m_refAnimalDetailsList->append());
		row[m_AnimalDetailsColumns.m_col_name] = "ID";
		row[m_AnimalDetailsColumns.m_col_value] = animalID;
		row[m_AnimalDetailsColumns.m_col_animalID] = animalID;

    	row = *(m_refAnimalDetailsList->append());
		row[m_AnimalDetailsColumns.m_col_name] = "Species";
		row[m_AnimalDetailsColumns.m_col_animalID] = animalID;
        if ((char*)sqlite3_column_text(stmt,0) != NULL)
            row[m_AnimalDetailsColumns.m_col_value] = (char*)sqlite3_column_text(stmt,0);

    	row = *(m_refAnimalDetailsList->append());
		row[m_AnimalDetailsColumns.m_col_name] = "Sex";
		row[m_AnimalDetailsColumns.m_col_animalID] = animalID;
        if ((char*)sqlite3_column_text(stmt,1) != NULL)
            row[m_AnimalDetailsColumns.m_col_value] = (char*)sqlite3_column_text(stmt,1);

    	row = *(m_refAnimalDetailsList->append());
		row[m_AnimalDetailsColumns.m_col_name] = "Weight (g)";
		row[m_AnimalDetailsColumns.m_col_animalID] = animalID;
        if ((char*)sqlite3_column_text(stmt,2) != NULL)
            row[m_AnimalDetailsColumns.m_col_value] = (char*)sqlite3_column_text(stmt,2);
        
    	row = *(m_refAnimalDetailsList->append());
		row[m_AnimalDetailsColumns.m_col_name] = "Age";
		row[m_AnimalDetailsColumns.m_col_animalID] = animalID;
        if ((char*)sqlite3_column_text(stmt,3) != NULL)
            row[m_AnimalDetailsColumns.m_col_value] = (char*)sqlite3_column_text(stmt,3);
        
    	row = *(m_refAnimalDetailsList->append());
		row[m_AnimalDetailsColumns.m_col_name] = "Tags";
		row[m_AnimalDetailsColumns.m_col_animalID] = animalID;
        if ((char*)sqlite3_column_text(stmt,4) != NULL)
            row[m_AnimalDetailsColumns.m_col_value] = (char*)sqlite3_column_text(stmt,4);

    	row = *(m_refAnimalDetailsList->append());
		row[m_AnimalDetailsColumns.m_col_name] = "Notes";
		row[m_AnimalDetailsColumns.m_col_animalID] = animalID;
        if ((char*)sqlite3_column_text(stmt,5) != NULL)
            row[m_AnimalDetailsColumns.m_col_value] = (char*)sqlite3_column_text(stmt,5);
	}
	sqlite3_finalize(stmt);
}

void GUI::populateCellDetailsList(const Glib::ustring animalID, const int cellID)
{
    m_refCellDetailsList->clear();
	char query[] = "SELECT depth, freq, tags, notes, threshold FROM cells WHERE animalID=? AND cellID=?";
	sqlite3_stmt *stmt;
	sqlite3_prepare_v2(db, query, -1, &stmt, 0);
	sqlite3_bind_text(stmt,1, animalID.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt,2, cellID);
	int r;
	r = sqlite3_step(stmt);

	Gtk::TreeModel::Row row;

	if (r == SQLITE_ROW)
	{
    	row = *(m_refCellDetailsList->append());
		row[m_CellDetailsColumns.m_col_name] = "Cell ID";
		row[m_CellDetailsColumns.m_col_animalID] = animalID;
		row[m_CellDetailsColumns.m_col_cellID] = cellID;
        char buffer[4];
        sprintf(buffer,"%d", cellID);
		row[m_CellDetailsColumns.m_col_value] = buffer;

    	row = *(m_refCellDetailsList->append());
		row[m_CellDetailsColumns.m_col_name] = "CarFreq (Hz)";
		row[m_CellDetailsColumns.m_col_animalID] = animalID;
		row[m_CellDetailsColumns.m_col_cellID] = cellID;
        if ((char*)sqlite3_column_text(stmt,1) != NULL)
            row[m_CellDetailsColumns.m_col_value] = (char*)sqlite3_column_text(stmt,1);

    	row = *(m_refCellDetailsList->append());
		row[m_CellDetailsColumns.m_col_name] = "Threshold (dB SPL)";
		row[m_CellDetailsColumns.m_col_animalID] = animalID;
		row[m_CellDetailsColumns.m_col_cellID] = cellID;
        if ((char*)sqlite3_column_text(stmt,4) != NULL)
            row[m_CellDetailsColumns.m_col_value] = (char*)sqlite3_column_text(stmt,4);

    	row = *(m_refCellDetailsList->append());
		row[m_CellDetailsColumns.m_col_name] = "Depth (um)";
		row[m_CellDetailsColumns.m_col_animalID] = animalID;
		row[m_CellDetailsColumns.m_col_cellID] = cellID;
        if ((char*)sqlite3_column_text(stmt,0) != NULL)
            row[m_CellDetailsColumns.m_col_value] = (char*)sqlite3_column_text(stmt,0);

    	row = *(m_refCellDetailsList->append());
		row[m_CellDetailsColumns.m_col_name] = "Tags";
		row[m_CellDetailsColumns.m_col_animalID] = animalID;
		row[m_CellDetailsColumns.m_col_cellID] = cellID;
        if ((char*)sqlite3_column_text(stmt,2) != NULL)
            row[m_CellDetailsColumns.m_col_value] = (char*)sqlite3_column_text(stmt,2);
        
    	row = *(m_refCellDetailsList->append());
		row[m_CellDetailsColumns.m_col_name] = "Notes";
		row[m_CellDetailsColumns.m_col_animalID] = animalID;
		row[m_CellDetailsColumns.m_col_cellID] = cellID;
        if ((char*)sqlite3_column_text(stmt,3) != NULL)
            row[m_CellDetailsColumns.m_col_value] = (char*)sqlite3_column_text(stmt,3);

	}
	sqlite3_finalize(stmt);
}

void GUI::populateAnimalTree()
{
    char query_animals[] = "SELECT ID FROM animals ORDER BY ID";
    sqlite3_stmt *stmt_animals, *stmt_cells;
    sqlite3_prepare_v2(db, query_animals, -1, &stmt_animals, 0);
    m_refAnimalTree->clear();
    Gtk::TreeModel::Row base;
    Gtk::TreeModel::Row row;
    Gtk::TreeModel::Row childrow;
    base = *(m_refAnimalTree->append());
    base[m_AnimalColumns.m_col_name] = "All Animals";
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


void GUI::populateDetailsList(const Glib::ustring animalID, const int cellID)
{
    sqlite3_stmt *stmt=0;
	int minFiles = (int)m_adjMinFiles.get_value();
    if (animalID != "" && cellID != -1) {
        const char query[] = "SELECT animalID, cellID, fileID, header,tags FROM files "
							 "JOIN(SELECT COUNT(*) AS file_count, animalID, cellID FROM files GROUP BY animalID, cellID) "
							 "USING(animalID, cellID) "
                             "WHERE animalID=? AND cellID=? AND file_count >= ? "
                             "ORDER BY animalID, cellID, fileID";
        sqlite3_prepare_v2(db,query,strlen(query), &stmt, NULL);
        sqlite3_bind_text(stmt, 1, animalID.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, cellID);
        sqlite3_bind_int(stmt, 3, minFiles);
    } else if (animalID != "" && cellID == -1) {
        const char query[] = "SELECT animalID, cellID, fileID, header,tags FROM files "
							 "JOIN(SELECT COUNT(*) AS file_count, animalID, cellID FROM files GROUP BY animalID, cellID) "
							 "USING(animalID, cellID) "
                             "WHERE animalID=? AND file_count >= ? "
                             "ORDER BY animalID, cellID, fileID";
        sqlite3_prepare_v2(db,query,strlen(query), &stmt, NULL);
        sqlite3_bind_text(stmt, 1, animalID.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int(stmt, 2, minFiles);
    } else if (animalID == "" && cellID == -1) {
        const char query[] = "SELECT animalID, cellID, fileID, header,tags FROM files "
							 "JOIN(SELECT COUNT(*) AS file_count, animalID, cellID FROM files GROUP BY animalID, cellID) "
							 "USING(animalID, cellID) "
                             "WHERE file_count >= ? "
                             "ORDER BY animalID, cellID, fileID";
        sqlite3_prepare_v2(db,query,strlen(query), &stmt, NULL);
        sqlite3_bind_int(stmt, 1, minFiles);
    }

    m_refDetailsList->clear();
    Gtk::TreeModel::Row row;
    while(true)
    {
        int r = sqlite3_step(stmt);
        if (r == SQLITE_ROW)
        {
            SpikeData sd;
            row = *(m_refDetailsList->append());
            row[m_DetailsColumns.m_col_animalID] = (char*)sqlite3_column_text(stmt,0);
            row[m_DetailsColumns.m_col_cellID] = sqlite3_column_int(stmt,1);
            row[m_DetailsColumns.m_col_filenum] = sqlite3_column_int(stmt,2);
            if (sqlite3_column_text(stmt,4) != NULL)
            {
                row[m_DetailsColumns.m_col_tags] = (char*)sqlite3_column_text(stmt,4);
            }

            void *header = (void*)sqlite3_column_blob(stmt,3);
            const HEADER *h = new HEADER(*static_cast<HEADER*>(header));
            sd.m_head = *h;
            row[m_DetailsColumns.m_col_xaxis] = sd.xVariable();
            row[m_DetailsColumns.m_col_type] = sd.type();
            row[m_DetailsColumns.m_col_trials] = sd.trials();

            char bufferCh1[20];
            char bufferCh2[20];
            char buffer[50];

            if (sd.m_head.nOnCh1 == 0) strcpy(bufferCh1,"-");
            else 
            {
                if (sd.frequency(1,0) == sd.frequency(1,1)) sprintf(bufferCh1, "%d", (int)sd.frequency(1,0));
                else strcpy(bufferCh1,"Var");
            }
            if (sd.m_head.nOnCh2 == 0) strcpy(bufferCh2,"-");
            else 
            {
                if (sd.frequency(2,0) == sd.frequency(2,1)) sprintf(bufferCh2, "%d", (int)sd.frequency(2,0));
                else strcpy(bufferCh2,"Var");
            }
            sprintf(buffer, "%s/%s", bufferCh1,bufferCh2);
            row[m_DetailsColumns.m_col_freq] = buffer;
            bufferCh1[0] = '\0';
            bufferCh2[0] = '\0';
            buffer[0] = '\0';


            if (sd.m_head.nOnCh1 == 0) strcpy(bufferCh1,"-");
            else 
            {
                if (sd.attenuation(1,0) == sd.attenuation(1,1)) sprintf(bufferCh1, "%d", (int)sd.attenuation(1,0));
                else strcpy(bufferCh1,"Var");
            }
            if (sd.m_head.nOnCh2 == 0) strcpy(bufferCh2,"-");
            else 
            {
                if (sd.attenuation(2,0) == sd.attenuation(2,1)) sprintf(bufferCh2, "%d", (int)sd.attenuation(2,0));
                else strcpy(bufferCh2,"Var");
            }
            sprintf(buffer, "%s/%s", bufferCh1,bufferCh2);
            row[m_DetailsColumns.m_col_atten] = buffer;
            bufferCh1[0] = '\0';
            bufferCh2[0] = '\0';
            buffer[0] = '\0';

            if (sd.m_head.nOnCh1 == 0) strcpy(bufferCh1,"-");
            else 
            {
                if (sd.duration(1,0) == sd.duration(1,1)) sprintf(bufferCh1, "%d", (int)sd.duration(1,0));
                else strcpy(bufferCh1,"Var");
            }
            if (sd.m_head.nOnCh2 == 0) strcpy(bufferCh2,"-");
            else 
            {
                if (sd.duration(2,0) == sd.duration(2,1)) sprintf(bufferCh2, "%d", (int)sd.duration(2,0));
                else strcpy(bufferCh2,"Var");
            }
            sprintf(buffer, "%s/%s", bufferCh1,bufferCh2);
            row[m_DetailsColumns.m_col_dur] = buffer;
            bufferCh1[0] = '\0';
            bufferCh2[0] = '\0';
            buffer[0] = '\0';

            if (sd.m_head.nOnCh1 == 0) strcpy(bufferCh1,"-");
            else 
            {
                if (sd.begin(1,0) == sd.begin(1,1)) sprintf(bufferCh1, "%d", (int)sd.begin(1,0));
                else strcpy(bufferCh1,"Var");
            }
            if (sd.m_head.nOnCh2 == 0) strcpy(bufferCh2,"-");
            else 
            {
                if (sd.begin(2,0) == sd.begin(2,1)) sprintf(bufferCh2, "%d", (int)sd.begin(2,0));
                else strcpy(bufferCh2,"Var");
            }
            sprintf(buffer, "%s/%s", bufferCh1,bufferCh2);
            row[m_DetailsColumns.m_col_onset] = buffer;
            bufferCh1[0] = '\0';
            bufferCh2[0] = '\0';
            buffer[0] = '\0';

        } else { break; }
    }
    sqlite3_finalize(stmt);
}



// Signal Handlers
void GUI::on_menuImportFolder_activate()
{
    Gtk::FileChooserDialog dialog("Select a Folder Containing Spike Data Files",
        Gtk::FILE_CHOOSER_ACTION_SELECT_FOLDER);
    dialog.set_select_multiple(true);
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
            std::vector<std::string> filenames = dialog.get_filenames();
            std::vector<std::string>::iterator it;
            std::string filename;

            for (it = filenames.begin(); it != filenames.end(); it++)
            {
                filename = *it;
                if ((dirp=opendir(filename.c_str()))==NULL)
                {
                    std::cerr << "ERROR: Unable to open " << filename << std::endl;
                } 
                else 
                {
                    while ((dptr=readdir(dirp)))
                    {
                        if (dptr->d_type == DT_REG)
                        {
							importSpikeFile(filename, dptr->d_name);
                        }
                    }
                }
                closedir(dirp);
            }
            break;
    }
    populateAnimalTree();
}

void GUI::importSpikeFile(std::string filename, char* d_name)
{
	SpikeData sd;
	std::string fullfile(filename);
	fullfile += "/";
	fullfile += d_name;
	if (sd.parse(fullfile.c_str()))
	{
		std::vector<std::string> fileTokens;
		std::string shortfilename(d_name);
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


void GUI::on_menuQuit_activate()
{
    delete m_pMenuQuit;
    delete m_pMenuImportFolder;
    delete m_pAnimalTree;
    delete m_pDetailsList;
    delete m_pHBoxPlots;
    delete m_pCellDetailsList;
    delete m_pAnimalDetailsList;
	delete m_pDataSource;
	delete m_pXVar;
	delete m_pYVar;
    delete m_pPlotSpikes;
    delete m_pPlotMeans;
    delete m_pPlotAnalyze;
    sqlite3_close(db);
    hide();
}

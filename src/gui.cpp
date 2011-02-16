#include "gui.h"

/*
CREATE TABLE animals (ID TEXT, species TEXT, sex TEXT, weight TEXT, age TEXT, notes TEXT, PRIMARY KEY(ID));
CREATE TABLE cells (
    "animalID" TEXT,
    "cellID" INTEGER,
    "notes" TEXT,
    "threshold" TEXT,
    "depth" TEXT,
    "freq" TEXT,
    "recordedby" TEXT
    );
CREATE TABLE "files" (animalID TEXT, cellID INTEGER, fileID INTEGER, notes TEXT, header BLOB, spikes BLOB, PRIMARY KEY(animalID, cellID, fileID));
CREATE TABLE properties (variable TEXT, value TEXT);
CREATE TABLE "tags" (
    "ID" INTEGER PRIMARY KEY AUTOINCREMENT,
    "animalID" INTEGER,
    "cellID" INTEGER,
    "fileID" INTEGER,
    "tag" TEXT
);
*/


GUI::GUI(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& refGlade)
	: Gtk::Window(cobject),
	mrp_Glade(refGlade),
	m_uiFilterFrame(refGlade),
	mp_AnimalsTree(0)
{
	uiReady = false;
	db = NULL;

	set_title("Spike Database - No database open");

	// Initialize the toolbar
	this->init_toolbar();

	// Initalize the statusbar
	mrp_Glade->get_widget("statusbar", mp_Statusbar);

	// Setup the filter frame
	m_uiFilterFrame.signal_changed().connect(sigc::mem_fun(*this, &GUI::on_filter_changed));
	if (settings.get_int("filterMinFiles") != 0) {
		m_uiFilterFrame.minFiles(settings.get_double("filterMinFiles"));
	} else { 
		settings.set("filterMinFiles", m_uiFilterFrame.minFiles());
	}

	// Create the plots
	mp_PlotSpikes = Gtk::manage(new EasyPlotmm());
	mp_PlotMeans = Gtk::manage(new EasyPlotmm());
	mp_PlotAnalyze = Gtk::manage(new EasyPlotmm());

	// Setup the animal details table
	Gtk::VBox* mp_AnimalDetailsVBox = Gtk::manage(new Gtk::VBox());
	Gtk::Alignment* mp_AlignAnimalDetails; /**< Container for the animal details property table. */
	mrp_Glade->get_widget("alignAnimalDetails", mp_AlignAnimalDetails);
	mp_AlignAnimalDetails->add(*mp_AnimalDetailsVBox);
	mp_AnimalDetailsVBox->pack_start(m_uiAnimalDetails);
	mp_AnimalDetailsVBox->pack_start(m_AnimalTags);
	m_AnimalTags.set_parent(this);
	m_uiAnimalDetails.signal_rowedited().connect(sigc::mem_fun(*this, &GUI::on_animaldetails_edited));
	m_AnimalTags.signal_deleted().connect(sigc::mem_fun(*this, &GUI::on_animal_tag_deleted));
	m_AnimalTags.signal_added().connect(sigc::mem_fun(*this, &GUI::on_animal_tag_added));
	
	// Setup the cell details table
	Gtk::VBox* mp_CellDetailsVBox = Gtk::manage(new Gtk::VBox());
	Gtk::ScrolledWindow* mp_ScrolledCellDetails = Gtk::manage(new Gtk::ScrolledWindow); /**< Container for the cell details property table. */
	mp_ScrolledCellDetails->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
	Gtk::Alignment* mp_AlignCellDetails; /**< Container for the animal details property table. */
	mrp_Glade->get_widget("alignCellDetails", mp_AlignCellDetails);
	mp_AlignCellDetails->add(*mp_ScrolledCellDetails);
	mp_ScrolledCellDetails->add(*mp_CellDetailsVBox);
	mp_CellDetailsVBox->pack_start(m_uiCellDetails);
	mp_CellDetailsVBox->pack_start(m_CellTags);
	m_CellTags.set_parent(this);
	m_uiCellDetails.signal_rowedited().connect(sigc::mem_fun(*this, &GUI::on_celldetails_edited));
	m_CellTags.signal_deleted().connect(sigc::mem_fun(*this, &GUI::on_cell_tag_deleted));
	m_CellTags.signal_added().connect(sigc::mem_fun(*this, &GUI::on_cell_tag_added));

	// Animals treeview 
	// Shown on left side under filter frame
	mrp_Glade->get_widget("tvAnimals", mp_AnimalsTree);
	mrp_AnimalTree = Gtk::TreeStore::create(m_AnimalColumns);
	mp_AnimalsTree->set_model(mrp_AnimalTree);
	mp_AnimalsTree->append_column("Animal/Cell ID", m_AnimalColumns.m_col_name);
	mrp_AnimalTree->set_sort_column(m_AnimalColumns.m_col_name, Gtk::SORT_ASCENDING);
	mrp_AnimalTree->set_sort_func(0, sigc::mem_fun(*this, &GUI::on_animal_sort));
	mrp_AnimalSelection = mp_AnimalsTree->get_selection();
	mrp_AnimalSelection->signal_changed().connect(sigc::mem_fun(*this, &GUI::changeAnimalSelection));

	// Create Files Details TreeView
	mp_FileDetailsTree = Gtk::manage( new uiFileDetailsTreeView(this) );
	Gtk::ScrolledWindow* p_swFileDetails; 
	mrp_Glade->get_widget("swFileDetails", p_swFileDetails);
	p_swFileDetails->add(*mp_FileDetailsTree);
	mp_FileDetailsTree->treeSelection()->signal_changed().connect(sigc::mem_fun(*this, &GUI::on_filedetails_selection_changed));
	mp_FileDetailsTree->signal_file_set_hidden().connect(sigc::mem_fun(*this, &GUI::on_filedetails_set_hidden));

	// Setup the analyze widgets
	mrp_Glade->get_widget("vboxAnalyze", mp_VBoxAnalyze);
	mrp_Glade->get_widget("cbDataSource", mp_DataSource);
	mrp_Glade->get_widget("cbXVar", mp_XVar);
	mrp_Glade->get_widget("cbYVar", mp_YVar);
	mrp_DataSource = Gtk::ListStore::create(m_DataSourceColumns);
	mrp_XVar = Gtk::ListStore::create(m_XVarColumns);
	mrp_YVar = Gtk::ListStore::create(m_YVarColumns);
	mp_DataSource->set_model(mrp_DataSource);
	mp_XVar->set_model(mrp_XVar);
	mp_YVar->set_model(mrp_YVar);
	mp_DataSource->pack_start(m_DataSourceColumns.m_col_name);
	mp_XVar->pack_start(m_XVarColumns.m_col_name);
	mp_YVar->pack_start(m_YVarColumns.m_col_name);
	mp_DataSource->signal_changed().connect( sigc::mem_fun(*this, &GUI::on_analyze_changed));
	mp_XVar->signal_changed().connect( sigc::mem_fun(*this, &GUI::on_analyze_changed));
	mp_YVar->signal_changed().connect( sigc::mem_fun(*this, &GUI::on_analyze_changed));
	Gtk::TreeModel::Row row = *(mrp_DataSource->append());
	row[m_DataSourceColumns.m_col_name] = "Animals";
	row = *(mrp_DataSource->append());
	row[m_DataSourceColumns.m_col_name] = "Cells";
	mp_DataSource->set_active(0);
	on_analyze_changed();


	mrp_Glade->get_widget("hboxPlots", mp_HBoxPlots);
	mrp_Glade->get_widget("cbMeanType", mp_MeanType);
	mrp_MeanType = Gtk::ListStore::create(m_MeanTypeColumns);
	mp_MeanType->set_model(mrp_MeanType);
	mp_MeanType->pack_start(m_MeanTypeColumns.m_col_name);
	mp_MeanType->signal_changed().connect( sigc::mem_fun(*this, &GUI::on_meantype_changed));
	row = *(mrp_MeanType->append());
	row[m_MeanTypeColumns.m_col_name] = "Mean Spikes per Trial";
	row = *(mrp_MeanType->append());
	row[m_MeanTypeColumns.m_col_name] = "Percentage of trials with at least 1 spike";
	row = *(mrp_MeanType->append());
	row[m_MeanTypeColumns.m_col_name] = "Mean First-Spike Latency";
	mp_MeanType->set_active(0);

	mp_HBoxPlots->pack_start(*mp_PlotSpikes);
	mp_HBoxPlots->pack_start(*mp_PlotMeans);
	mp_VBoxAnalyze->pack_start(*mp_PlotAnalyze);
	show_all_children();


	// Attempt to use the previously opened database.
	if (settings.get_string("lastDatabase") != "") openDatabase(settings.get_string("lastDatabase"));

	// Attempt to restore the window size and position
	if (settings.get_int("winWidth") != 0 && settings.get_int("winHeight") != 0) 
		resize(settings.get_int("winWidth"), settings.get_int("winHeight"));
	if (settings.get_int("winX") != 0 && settings.get_int("winY") != 0) 
		move(settings.get_int("winX"), settings.get_int("winY"));

	uiReady = true;
	updateTagCompletion();
	this->on_filter_changed();
}

GUI::~GUI()
{
}

void GUI::init_toolbar()
{
	Gtk::ImageMenuItem* mp_MenuNewDatabase;
	Gtk::ImageMenuItem* mp_MenuOpenDatabase;
	Gtk::ImageMenuItem* mp_MenuQuit;
	mrp_Glade->get_widget("menuNewDatabase", mp_MenuNewDatabase);
	if (mp_MenuNewDatabase) {
		mp_MenuNewDatabase->signal_activate().connect(sigc::mem_fun(*this, &GUI::on_menuNewDatabase_activate));
	}
	mrp_Glade->get_widget("menuOpenDatabase", mp_MenuOpenDatabase);
	if (mp_MenuOpenDatabase) {
		mp_MenuOpenDatabase->signal_activate().connect(sigc::mem_fun(*this, &GUI::on_menuOpenDatabase_activate));
	}
	mrp_Glade->get_widget("menuImportFolder", mp_MenuImportFolder);
	if (mp_MenuImportFolder) {
		mp_MenuImportFolder->set_sensitive(false);
		mp_MenuImportFolder->signal_activate().connect(sigc::mem_fun(*this, &GUI::on_menuImportFolder_activate));
	}
	mrp_Glade->get_widget("menuQuit", mp_MenuQuit);
	if (mp_MenuQuit) {
		mp_MenuQuit->signal_activate().connect(sigc::mem_fun(*this, &GUI::on_menuQuit_activate));
	}
}


void GUI::updateTagCompletion()
{
	sqlite3_stmt *stmt = 0;
	const char query[] = "SELECT DISTINCT tag FROM tags";
	sqlite3_prepare_v2(db, query, -1, &stmt, 0);
	std::vector<Glib::ustring> tags;
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		Glib::ustring t = (char*)sqlite3_column_text(stmt, 0);
		tags.push_back(t);
	}
	sqlite3_finalize(stmt);
	m_uiFilterFrame.updateTagCompletion(tags);
}


void GUI::on_menuNewDatabase_activate()
{
	Gtk::FileChooserDialog dialog("Select the SpikeDB Database File",
				      Gtk::FILE_CHOOSER_ACTION_SAVE);

	dialog.set_transient_for(*this);

	// Add response buttons to the dialog:
	dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

	// Show the dialog
	int result = dialog.run();

	switch (result) {
	case (Gtk::RESPONSE_OK):

		// First create an empty file
		Gio::init();
		std::string filename = dialog.get_filename();
		// TODO: Add error checking that the file doesn't exist and is valid
		Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(filename);
		Glib::RefPtr<Gio::FileOutputStream> stream = file->create_file();
		stream->close();
		stream.reset();

		// Load it as an sqlite3 database and save the tables
		if (sqlite3_open(filename.c_str(), &db) != SQLITE_OK) {
			std::cerr << "CRITICAL ERROR: Unable to open database file." << std::endl;
			return;
		}
		set_title("Spike Database - " + filename);

		sqlite3_stmt *stmt = 0;
		std::vector<std::string> query;
		query.push_back("CREATE TABLE animals (ID TEXT, species TEXT, sex TEXT, weight TEXT, age TEXT, notes TEXT, PRIMARY KEY(ID))");
		query.push_back("CREATE TABLE cells (animalID TEXT, cellID INTEGER, notes TEXT, threshold TEXT, depth TEXT, freq TEXT, recordedby TEXT, PRIMARY KEY(animalID, cellID))");
		query.push_back("CREATE TABLE files (animalID TEXT, cellID INTEGER, fileID INTEGER, notes TEXT, header BLOB, spikes BLOB,PRIMARY KEY(animalID, cellID, fileID))");
		query.push_back( "CREATE TABLE properties (variable TEXT, value TEXT)");
		query.push_back("INSERT INTO properties (variable, value) VALUES('version', '1.2')");

		for (unsigned int i = 0; i < query.size(); ++i) {
			sqlite3_prepare_v2(db, query[i].c_str(), query[i].length(), &stmt, NULL);
			sqlite3_bind_text(stmt, 1, "version", -1, SQLITE_TRANSIENT);
			sqlite3_step(stmt);
		}
		sqlite3_finalize(stmt);
		openDatabase(filename);
		break;
	}
}

void GUI::on_menuOpenDatabase_activate()
{

	Gtk::FileChooserDialog dialog("Select the SpikeDB Database File",
				      Gtk::FILE_CHOOSER_ACTION_OPEN);

	dialog.set_transient_for(*this);

	// Add response buttons to the dialog:
	dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

	// Show the dialog
	int result = dialog.run();

	switch (result) {
	case (Gtk::RESPONSE_OK):
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
	sqlite3_stmt *stmt = 0;
	const char query[] = "SELECT value FROM properties WHERE variable=?";
	sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
	sqlite3_bind_text(stmt, 1, "version", -1, SQLITE_TRANSIENT);
	int r = sqlite3_step(stmt);
	float version = 0;
	if (r == SQLITE_ROW) {
		version = (float)sqlite3_column_double(stmt, 0);
	}
	int count = 0;
	while (version < CURRENT_DB_VERSION) {
		++count;
		if (count > 1000) {
			Gtk::MessageDialog dialog(*this, "Unable to update database file.", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
			dialog.set_secondary_text("This could be due to database corruption or a version too old to get updated.  Contact Brandon Aubie at brandon@aubie.ca for more information.");
			dialog.run();
			return false;
		}
	}

	// Remember this database for next time
	settings.set("lastDatabase", filename);

	// Allow importing files
	mp_MenuImportFolder->set_sensitive(true);

	// Show the animals and cells from the database
	populateAnimalTree();
	sqlite3_finalize(stmt);
	return true;
}

void GUI::on_filter_changed()
{
	if (!uiReady) return;

	/*
	 * Save Settings
	 */
	settings.set("filterMinFiles", m_uiFilterFrame.minFiles());

	/*
	 * Update Animal Selection with Filter
	 */
	changeAnimalSelection();
}

void GUI::on_meantype_changed()
{
	mp_PlotMeans->clear();
	mp_PlotSpikes->clear();
	curXVariable = "";
	mp_FileDetailsTree->treeSelection()->selected_foreach_iter(
		sigc::mem_fun(*this, &GUI::addFileToPlot)
		);
}

void GUI::on_analyze_changed()
{          
	// Update the analyze plot
	Gtk::TreeModel::Row row;
	static int dataSource = -1;

	if (mp_DataSource->get_active_row_number() != dataSource) {
		dataSource = mp_DataSource->get_active_row_number();
		if (mp_DataSource->get_active_row_number() == 0) {
			mrp_XVar->clear();
			mrp_YVar->clear();
			row = *(mrp_XVar->append());
			row[m_XVarColumns.m_col_name] = "Weight";

			row = *(mrp_YVar->append());
			row[m_YVarColumns.m_col_name] = "Weight";
		}
		if (mp_DataSource->get_active_row_number() == 1) {
			mrp_XVar->clear();
			mrp_YVar->clear();
			row = *(mrp_XVar->append());
			row[m_XVarColumns.m_col_name] = "CarFreq (Hz)";
			row = *(mrp_XVar->append());
			row[m_XVarColumns.m_col_name] = "Threshold (dB SPL)";
			row = *(mrp_XVar->append());
			row[m_XVarColumns.m_col_name] = "Depth (um)";

			row = *(mrp_YVar->append());
			row[m_YVarColumns.m_col_name] = "CarFreq (Hz)";
			row = *(mrp_YVar->append());
			row[m_YVarColumns.m_col_name] = "Threshold (dB SPL)";
			row = *(mrp_YVar->append());
			row[m_YVarColumns.m_col_name] = "Depth (um)";
		}
		mp_XVar->set_active(0);
		mp_YVar->set_active(0);
	}
	updateAnalyzePlot();
}

void GUI::updateAnalyzePlot()
{
	/*
	 * This function requires a valid database.
	 */
	if (db == NULL) return;


	mp_PlotAnalyze->clear();
	EasyPlotmm::Pen pen;

	pen.linewidth = 0.0;
	pen.shape = EasyPlotmm::CIRCLE;
	pen.filled = true;

	std::vector<double> X, Y;

	sqlite3_stmt* stmt = 0;
	Glib::ustring animalID = "";
	int cellID = -1;
	Gtk::TreeModel::iterator iter = mrp_AnimalSelection->get_selected();
	if (iter) {
		Gtk::TreeModel::Row row = *iter;
		if (row->parent() == 0) {
			// Root
			animalID = "";
			cellID = -1;
		} else if (row->parent()->parent() == 0) {
			// First Level
			animalID = row->get_value(m_AnimalColumns.m_col_name);
		} else if (row->parent()->parent()->parent() == 0) {
			// Second Level
			animalID = row->parent()->get_value(m_AnimalColumns.m_col_name);
			cellID = atoi(row->get_value(m_AnimalColumns.m_col_name).c_str());
		}
	}
	getCellsStatement(&stmt, animalID, cellID);

	if (mp_DataSource->get_active_row_number() == 1) { // Cell Plots
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			bool hasX = false;
			// SELECT animalID, cellID, threshold, depth, freq FROM files
			if (mp_XVar->get_active_row_number() == 0) { // CarFreq
				if (sqlite3_column_type(stmt, 4) != SQLITE_NULL && sqlite3_column_double(stmt, 4) > 0) {
					X.push_back(sqlite3_column_double(stmt, 4));
					hasX = true;
					mp_PlotAnalyze->xname("Frequency (Hz)");
				}
			}
			if (mp_XVar->get_active_row_number() == 1) { // Threshold
				if (sqlite3_column_type(stmt, 2) != SQLITE_NULL) {
					X.push_back(sqlite3_column_double(stmt, 2));
					hasX = true;
					mp_PlotAnalyze->xname("Threshold (dB SPL)");
				}
			}
			if (mp_XVar->get_active_row_number() == 2) { // Depth
				if (sqlite3_column_type(stmt, 3) != SQLITE_NULL && sqlite3_column_double(stmt, 3) > 0) {
					X.push_back(sqlite3_column_double(stmt, 3));
					hasX = true;
					mp_PlotAnalyze->xname("Depth (um)");
				}
			}
			if (mp_YVar->get_active_row_number() == 0) { // CarFreq
				if (hasX) {
					if (sqlite3_column_type(stmt, 4) != SQLITE_NULL && sqlite3_column_double(stmt, 4) > 0) {
						Y.push_back(sqlite3_column_double(stmt, 4));
						mp_PlotAnalyze->yname("Frequency (Hz)");
					} else{ X.pop_back(); }
				}
			}
			if (mp_YVar->get_active_row_number() == 1) { // Threshold
				if (hasX) {
					if (sqlite3_column_type(stmt, 2) != SQLITE_NULL) {
						Y.push_back(sqlite3_column_double(stmt, 2));
						mp_PlotAnalyze->yname("Threshold (dB SPL)");
					}
				} else{ X.pop_back(); }
			}
			if (mp_YVar->get_active_row_number() == 2) { // Depth
				if (hasX) {
					if (sqlite3_column_type(stmt, 3) != SQLITE_NULL && sqlite3_column_double(stmt, 3) > 0) {
						Y.push_back(sqlite3_column_double(stmt, 3));
						mp_PlotAnalyze->yname("Depth (um)");
					}  else{ X.pop_back(); }
				}
			}
		}
	}
	sqlite3_finalize(stmt);

	double xmin = mp_PlotAnalyze->automatic();
	double xmax = mp_PlotAnalyze->automatic();
	double ymin = mp_PlotAnalyze->automatic();
	double ymax = mp_PlotAnalyze->automatic();

	if (mp_DataSource->get_active_row_number() == 1 && mp_XVar->get_active_row_number() == 2) {
		xmin = 0;
		xmax = 2000;
	}
	if (mp_DataSource->get_active_row_number() == 1 && mp_YVar->get_active_row_number() == 0) {
		ymin = 0;
		ymax = 80000;
	}
	if (mp_DataSource->get_active_row_number() == 1 && mp_YVar->get_active_row_number() == 2) {
		ymin = 0;
		ymax = 2000;
	}
	if (mp_DataSource->get_active_row_number() == 1 && mp_XVar->get_active_row_number() == 0) {
		xmin = 0;
		xmax = 80000;
	}
	mp_PlotAnalyze->axes(xmin, xmax, ymin, ymax);
	mp_PlotAnalyze->plot(X, Y, pen);
}

int GUI::on_animal_sort(const Gtk::TreeModel::iterator& a_, const Gtk::TreeModel::iterator& b_)
{
	const Gtk::TreeModel::Row a__ = *a_;
	const Gtk::TreeModel::Row b__ = *b_;

	Glib::ustring a = a__.get_value(m_AnimalColumns.m_col_name);
	Glib::ustring b = b__.get_value(m_AnimalColumns.m_col_name);

	if (a__.children().size() == 0 && a__.parent() != 0) {
		// Cell ID's.  Just sort by number.
		return atoi(a.c_str()) > atoi(b.c_str());
	}
	return a.compare(b);
}

void GUI::on_animaldetails_edited(
	Glib::ustring ID, Glib::ustring name, Glib::ustring /*oldvalue*/, Glib::ustring newvalue, uiPropTableRowType /*type*/)
{
	// Construct the SQL query for the relevant row.
	Glib::ustring query;
	if (name == "Species") query = "UPDATE animals SET species=? WHERE ID=?";
	if (name == "Sex") query = "UPDATE animals SET sex=? WHERE ID=?";
	if (name == "Weight (g)") query = "UPDATE animals SET weight=? WHERE ID=?";
	if (name == "Age") query = "UPDATE animals SET age=? WHERE ID=?";
	if (name == "Notes") query = "UPDATE animals SET notes=? WHERE ID=?";

	// Update the database.
	const char* q = query.c_str();
	sqlite3_stmt *stmt = 0;
	sqlite3_prepare_v2(db, q, strlen(q), &stmt, NULL);
	sqlite3_bind_text(stmt, 1, newvalue.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, ID.c_str(), -1, SQLITE_TRANSIENT);
	if (sqlite3_step(stmt) != SQLITE_DONE) std::cerr << "ERROR: Could not update animal. " << sqlite3_errmsg(db) << std::endl;
	sqlite3_finalize(stmt);

	// Repopulate the table with the database value to show, forsure, what is in the database now.
	populateAnimalDetailsList(ID);
}

void GUI::on_celldetails_edited(
	CellID ID, Glib::ustring name, Glib::ustring /*oldvalue*/, Glib::ustring newvalue, uiPropTableRowType /*type*/)
{
	// Construct the SQL query for the relevant row.
	Glib::ustring query;
	if (name == "CarFreq (Hz)")  query = "UPDATE cells SET freq=? WHERE animalID=? AND cellID=?";
	if (name == "Depth (um)")  query = "UPDATE cells SET depth=? WHERE animalID=? AND cellID=?";
	if (name == "Threshold (dB SPL)")  query = "UPDATE cells SET threshold=? WHERE animalID=? AND cellID=?";
	if (name == "Notes")  query = "UPDATE cells SET notes=? WHERE animalID=? AND cellID=?";

	// Update the database.
	const char* q = query.c_str();
	sqlite3_stmt *stmt = 0;
	sqlite3_prepare_v2(db, q, strlen(q), &stmt, NULL);
	sqlite3_bind_text(stmt, 1, newvalue.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, ID.animalID.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 3, ID.cellID);
	if (sqlite3_step(stmt) != SQLITE_DONE) std::cerr << "ERROR: Could not update cell. " << sqlite3_errmsg(db) << std::endl;
	sqlite3_finalize(stmt);

	// Repopulate the table with the database value to show, forsure, what is in the database now.
	populateCellDetailsList(ID.animalID,ID.cellID);
}

void GUI::on_filedetails_selection_changed()
{
	mp_PlotMeans->clear();
	mp_PlotSpikes->clear();
	curXVariable = "";
	mp_FileDetailsTree->treeSelection()->selected_foreach_iter(
		sigc::mem_fun(*this, &GUI::addFileToPlot)
		);
	mp_FileDetailsTree->treeSelection()->selected_foreach_iter(
		sigc::mem_fun(*this, &GUI::updateSideLists)
		);
}

void GUI::updateSideLists(const Gtk::TreeModel::iterator& iter)
{
	m_curAnimalID = mp_FileDetailsTree->animalID(iter);
	m_curCellID = mp_FileDetailsTree->cellID(iter);
	m_curFileNum = mp_FileDetailsTree->fileID(iter);

	populateAnimalDetailsList(m_curAnimalID);
	populateCellDetailsList(m_curAnimalID, m_curCellID);
}

void GUI::addFileToPlot(const Gtk::TreeModel::iterator& iter)
{
	/*
	 * This function requires a valid database.
	 */
	if (db == NULL) return;


	Gtk::TreeModel::Row row = *iter;
	sqlite3_stmt *stmt = 0;
	const char query[] = "SELECT header,spikes FROM files WHERE animalID=? AND cellID=? AND fileID=?";

	sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
	sqlite3_bind_text(stmt, 1, row.get_value(mp_FileDetailsTree->m_Columns.m_col_animalID).c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 2, row.get_value(mp_FileDetailsTree->m_Columns.m_col_cellID));
	sqlite3_bind_int(stmt, 3, row.get_value(mp_FileDetailsTree->m_Columns.m_col_filenum));
	int r;
	r = sqlite3_step(stmt);
	SpikeData sd;
	if (r == SQLITE_ROW) {
		void *header = (void*)sqlite3_column_blob(stmt, 0);
		sd.setHeader(header);
		std::string xVariable = sd.xVariable();
		if (curXVariable == "") {
			curXVariable = xVariable;
		} else if (curXVariable != xVariable) {
			std::cerr << "ERROR: Plots have different X-Variables." << std::endl;
			return;
		}
		SPIKESTRUCT *spikes = (SPIKESTRUCT*)sqlite3_column_blob(stmt, 1);
		int spikes_length = sqlite3_column_bytes(stmt, 1);
		int numSpikes = spikes_length / sizeof(SPIKESTRUCT);
		sd.m_spikeArray.assign(spikes, spikes + numSpikes);

		std::vector<double> x(sd.m_head.nSweeps, 0);
		std::vector<double> y_mean(sd.m_head.nSweeps, 0);
		std::vector<double> y_one(sd.m_head.nSweeps, 0);
		std::vector<double> y_fsl(sd.m_head.nSweeps, 0);
		std::vector<double> err(sd.m_head.nSweeps, 0);
		std::vector<double> N(sd.m_head.nSweeps, 0);

		std::vector<double> x_spikes;
		std::vector<double> y_spikes;

		EasyPlotmm::Pen spikesPen;
		spikesPen.linewidth = 0.0;
		spikesPen.shape = EasyPlotmm::POINT;
		spikesPen.pointsize = 2;
		spikesPen.filled = true;

		double dy = sd.delta() / (sd.m_head.nPasses * 1.25);

		// Calculate the means and get the spike times
		for (int i = 0; i < sd.m_head.nSweeps; ++i) {
			x.at(i) = sd.xvalue(i);
                        std::vector<double> first_spikes;
			for (int p = 0; p < sd.m_head.nPasses; ++p) {
				bool foundSpike = false;
                                double first_spike = 99999;
				for (unsigned int s = 0; s < sd.m_spikeArray.size(); ++s) {
					// Spike sweeps are 1 based but here we are 0 based
					if (sd.m_spikeArray[s].nSweep == i + 1 && sd.m_spikeArray[s].nPass == p+1) {
						if (!foundSpike) {
							y_one.at(i) += 1.0f / sd.m_head.nPasses;
							foundSpike = true;
						}
						if (sd.m_head.nPasses > 0) {
							y_mean.at(i) += 1.0f / sd.m_head.nPasses;
							if (first_spike > sd.m_spikeArray[s].fTime) {
								first_spike = sd.m_spikeArray[s].fTime;
							}
						}
						x_spikes.push_back(sd.m_spikeArray[s].fTime);
						y_spikes.push_back(sd.xvalue(i) + dy * p);
					}
				}
                                if (first_spike != 99999) {
                                    first_spikes.push_back(first_spike);
                                }
			}
                        for (unsigned int fsl_index = 0; fsl_index < first_spikes.size(); ++fsl_index) {
                            y_fsl.at(i) += first_spikes.at(fsl_index) / first_spikes.size();
                        }
		}
		double min_y = sd.xvalue(0) - 2 * dy;
		double max_y = sd.xvalue(sd.m_head.nSweeps - 1) + sd.m_head.nPasses * dy;
		if (min_y > max_y) {
			double t = min_y;
			min_y = max_y;
			max_y = t;
		}
		mp_PlotSpikes->axes(0, sd.m_head.nRepInt, min_y, max_y);
		mp_PlotSpikes->xname("Time (ms)");
		mp_PlotSpikes->yname(sd.xVariable());
		mp_PlotSpikes->plot(x_spikes, y_spikes, spikesPen);

		// Calculate the SD of the means
		for (int i = 0; i < sd.m_head.nSweeps; ++i) {
			for (int p = 0; p < sd.m_head.nPasses; ++p) {
				int numSpikes = 0;
				for (unsigned int s = 0; s < sd.m_spikeArray.size(); ++s) {
					// Spike sweeps are 1 based but here we are 0 based
					if (sd.m_spikeArray[s].nSweep == i + 1 && sd.m_spikeArray[s].nPass == p) {
						numSpikes++;
					}
				}
				if (numSpikes > 0) {
					++(N.at(i));
				}
				err.at(i) += (numSpikes - y_mean.at(i)) * (numSpikes - y_mean.at(i));
			}
		}
		for (int i = 0; i < sd.m_head.nSweeps; ++i) {
			if (N.at(i) > 1) {
				// Calculate Standard Error from the Standard Deviation
				err.at(i) = sqrt(err.at(i) / (N.at(i) - 1)) / sqrt(N.at(i));  // N-1 as we have a sample of points
			}else                                                                          {
				err.at(i) = 0;
			}
		}
		mp_PlotMeans->axes(mp_PlotMeans->automatic(), mp_PlotMeans->automatic(), 0, mp_PlotMeans->automatic());
		mp_PlotMeans->xname(sd.xVariable());

		if (mp_MeanType->get_active_row_number() == 0) {
			mp_PlotMeans->yname("Mean Spikes per Trial");
			mp_PlotMeans->plot(x, y_mean, err);
		}
		if (mp_MeanType->get_active_row_number() == 1) {
			mp_PlotMeans->yname("Percentage of Trials With Spikes");
			mp_PlotMeans->plot(x, y_one);
		}
		if (mp_MeanType->get_active_row_number() == 2) {
			mp_PlotMeans->yname("Mean First Spike Latency");
			mp_PlotMeans->plot(x, y_fsl);
		}

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
		for (int i = 0; i < sd.m_head.nSweeps; ++i) {
			if (sd.m_head.nOnCh1 == 1) {
				double start = sd.m_head.stimFirstCh1.fBegin + sd.m_head.deltaCh1.fBegin * i;
				double end = sd.m_head.stimFirstCh1.fBegin + sd.m_head.stimFirstCh1.fDur + sd.m_head.deltaCh1.fBegin * i + sd.m_head.deltaCh1.fDur * i;
				for (int j = 1; j <= sd.m_head.stimFirstCh1.nStimPerSweep; ++j) {
					std::vector<double> stimX;
					std::vector<double> stimY;
					stimX.push_back(start);
					stimY.push_back(sd.xvalue(i));
					stimX.push_back(end);
					stimY.push_back(sd.xvalue(i));
					mp_PlotSpikes->plot(stimX, stimY, ch1Pen, false);
					start = end + sd.m_head.stimFirstCh1.fStimInt + sd.m_head.deltaCh1.fStimInt * i;
					end = start + sd.m_head.stimFirstCh1.fDur + sd.m_head.deltaCh1.fDur * i;
				}
			}
			if (sd.m_head.nOnCh2 == 1) {
				double start = sd.m_head.stimFirstCh2.fBegin + sd.m_head.deltaCh2.fBegin * i;
				double end = sd.m_head.stimFirstCh2.fBegin + sd.m_head.stimFirstCh2.fDur + sd.m_head.deltaCh2.fBegin * i + sd.m_head.deltaCh2.fDur * i;
				for (int j = 1; j <= sd.m_head.stimFirstCh2.nStimPerSweep; ++j) {
					std::vector<double> stimX;
					std::vector<double> stimY;
					stimX.push_back(start);
					stimY.push_back(sd.xvalue(i));
					stimX.push_back(end);
					stimY.push_back(sd.xvalue(i));
					mp_PlotSpikes->plot(stimX, stimY, ch2Pen, false);
					start = end + sd.m_head.stimFirstCh2.fStimInt + sd.m_head.deltaCh2.fStimInt * i;
					end = start + sd.m_head.stimFirstCh2.fDur + sd.m_head.deltaCh2.fDur * i;
				}
			}
		}
	}else{ std::cerr << "ERROR: Failed to read file from database. " << sqlite3_errmsg(db) << std::endl; } 
	
	sqlite3_finalize(stmt);
}


void GUI::changeAnimalSelection()
{

	Gtk::TreeModel::iterator iter = mrp_AnimalSelection->get_selected();

	if (iter) {
		Gtk::TreeModel::Row row = *iter;
		if (row->parent() == 0) {
			// Root
			populateDetailsList("", -1);
			m_uiAnimalDetails.clear();
			m_AnimalTags.clear();
			m_uiCellDetails.clear();
			m_CellTags.clear();
		} else if (row->parent()->parent() == 0) {
			// First Level
			populateDetailsList(row->get_value(m_AnimalColumns.m_col_name), -1);
			populateAnimalDetailsList(row->get_value(m_AnimalColumns.m_col_name));
			m_uiCellDetails.clear();
			m_CellTags.clear();
		} else if (row->parent()->parent()->parent() == 0) {
			// Second Level
			populateDetailsList(row->parent()->get_value(m_AnimalColumns.m_col_name),
					    atoi(row->get_value(m_AnimalColumns.m_col_name).c_str()));
			populateAnimalDetailsList(row->parent()->get_value(m_AnimalColumns.m_col_name));
			populateCellDetailsList(row->parent()->get_value(m_AnimalColumns.m_col_name),
						atoi(row->get_value(m_AnimalColumns.m_col_name).c_str()));
		}
	} else{
		populateDetailsList("", -1);
	}
	updateAnalyzePlot();
}

void GUI::populateAnimalDetailsList(const Glib::ustring animalID)
{
	/*
	 * This function requires a valid database.
	 */
	if (db == NULL) return;

	sqlite3_stmt *stmt;
	int r;
	char query3[] = "SELECT COUNT(*) FROM cells WHERE animalID=?";
	sqlite3_prepare_v2(db, query3, -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, animalID.c_str(), -1, SQLITE_TRANSIENT);
	r = sqlite3_step(stmt);
	int numberOfCells = sqlite3_column_int(stmt,0);
	sqlite3_finalize(stmt);

	m_uiAnimalDetails.clear();
	const char query[] = "SELECT species, sex, weight, age, notes FROM animals WHERE ID=?";
	sqlite3_prepare_v2(db, query, -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, animalID.c_str(), -1, SQLITE_TRANSIENT);
	r = sqlite3_step(stmt);

	if (r == SQLITE_ROW) {
		m_uiAnimalDetails.addRow(animalID, "ID", animalID, Static);
		m_uiAnimalDetails.addRow(animalID, "Cells", numberOfCells, Static);

		m_uiAnimalDetails.addRow(animalID, "Species", 
			((char*)sqlite3_column_text(stmt, 0) == NULL) ? "" : (char*)sqlite3_column_text(stmt, 0),
			Editable
		);

		m_uiAnimalDetails.addRow(animalID, "Sex", 
			((char*)sqlite3_column_text(stmt, 1) == NULL) ? "" : (char*)sqlite3_column_text(stmt, 1),
			Editable
		);

		m_uiAnimalDetails.addRow(animalID, "Weight (g)", 
			((char*)sqlite3_column_text(stmt, 2) == NULL) ? "" : (char*)sqlite3_column_text(stmt, 2),
			Editable
		);

		m_uiAnimalDetails.addRow(animalID, "Age", 
			((char*)sqlite3_column_text(stmt, 3) == NULL) ? "" : (char*)sqlite3_column_text(stmt, 3),
			Editable
		);

		m_uiAnimalDetails.addRow(animalID, "Notes", 
			((char*)sqlite3_column_text(stmt, 4) == NULL) ? "" : (char*)sqlite3_column_text(stmt, 4),
			EditableLong
		);
	}
	sqlite3_finalize(stmt);

	const char query2[] = "SELECT tag FROM tags WHERE animalID=? AND cellID IS NULL AND fileID IS NULL";
	sqlite3_prepare_v2(db, query2, -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, animalID.c_str(), -1, SQLITE_TRANSIENT);
	std::vector<Glib::ustring> tags;
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		Glib::ustring t = (char*)sqlite3_column_text(stmt, 0);
		tags.push_back(t);
	}
	sqlite3_finalize(stmt);
	m_AnimalTags.tags(tags);
}

void GUI::on_animal_tag_deleted(Glib::ustring tag)
{
	sqlite3_stmt *stmt;
	const char query[] = "DELETE FROM tags WHERE tag=? AND animalID=? AND cellID IS NULL AND fileID IS NULL";
	sqlite3_prepare_v2(db, query, -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, tag.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, m_curAnimalID.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	populateAnimalDetailsList(m_curAnimalID);
	updateTagCompletion();
}

void GUI::on_cell_tag_deleted(Glib::ustring tag)
{
	sqlite3_stmt *stmt;
	const char query[] = "DELETE FROM tags WHERE tag=? AND animalID=? AND cellID=? AND fileID IS NULL";
	sqlite3_prepare_v2(db, query, -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, tag.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, m_curAnimalID.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 3, m_curCellID);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	populateCellDetailsList(m_curAnimalID, m_curCellID);
	updateTagCompletion();
}

void GUI::on_file_tag_deleted(Glib::ustring tag)
{
	sqlite3_stmt *stmt;
	const char query[] = "DELETE FROM tags WHERE tag=? AND animalID=? AND cellID=? AND fileID=?";
	sqlite3_prepare_v2(db, query, -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, tag.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, m_curAnimalID.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 3, m_curCellID);
	sqlite3_bind_int(stmt, 4, m_curFileNum);
	sqlite3_step(stmt);
	sqlite3_finalize(stmt);
	populateCellDetailsList(m_curAnimalID, m_curCellID);
	updateTagCompletion();
}

bool GUI::on_animal_tag_added(Glib::ustring tag)
{
	sqlite3_stmt *stmt;

	const char query2[] = "SELECT tag FROM tags WHERE tag=? AND animalID=? AND cellID IS NULL AND fileID IS NULL";
	sqlite3_prepare_v2(db, query2, -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, tag.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, m_curAnimalID.c_str(), -1, SQLITE_TRANSIENT);
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		sqlite3_finalize(stmt);
		return false;
	}
	sqlite3_finalize(stmt);

	const char query[] = "INSERT INTO tags (animalID, cellID, fileID, tag) VALUES(?,null,null,?)";
	sqlite3_prepare_v2(db, query, -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, m_curAnimalID.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, tag.c_str(), -1, SQLITE_TRANSIENT);
	if (sqlite3_step(stmt) != SQLITE_DONE) std::cerr << "ERROR: Could not insert tag. " << sqlite3_errmsg(db) << std::endl;
	sqlite3_finalize(stmt);
	updateTagCompletion();
	return true;
}

bool GUI::on_cell_tag_added(Glib::ustring tag)
{
	sqlite3_stmt *stmt;
	const char query2[] = "SELECT tag FROM tags WHERE tag=? AND animalID=? AND cellID=? AND fileID IS NULL";
	sqlite3_prepare_v2(db, query2, -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, tag.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, m_curAnimalID.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 3, m_curCellID);
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		sqlite3_finalize(stmt);
		return false;
	}
	sqlite3_finalize(stmt);

	const char query[] = "INSERT INTO tags (animalID, cellID, fileID, tag) VALUES(?,?,null,?)";
	sqlite3_prepare_v2(db, query, -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, m_curAnimalID.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 2, m_curCellID);
	sqlite3_bind_text(stmt, 3, tag.c_str(), -1, SQLITE_TRANSIENT);
	if (sqlite3_step(stmt) != SQLITE_DONE) std::cerr << "ERROR: Could not insert tag. " << sqlite3_errmsg(db) << std::endl;
	sqlite3_finalize(stmt);
	updateTagCompletion();
	return true;
}

bool GUI::on_file_tag_added(Glib::ustring tag)
{
	sqlite3_stmt *stmt;
	const char query2[] = "SELECT tag FROM tags WHERE tag=? AND animalID=? AND cellID=? AND fileID=?";
	sqlite3_prepare_v2(db, query2, -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, tag.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, m_curAnimalID.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 3, m_curCellID);
	sqlite3_bind_int(stmt, 4, m_curFileNum);
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		sqlite3_finalize(stmt);
		return false;
	}
	sqlite3_finalize(stmt);

	const char query[] = "INSERT INTO tags (animalID, cellID, fileID, tag) VALUES(?,?,?,?)";
	sqlite3_prepare_v2(db, query, -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, m_curAnimalID.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 2, m_curCellID);
	sqlite3_bind_int(stmt, 3, m_curFileNum);
	sqlite3_bind_text(stmt, 4, tag.c_str(), -1, SQLITE_TRANSIENT);
	if (sqlite3_step(stmt) != SQLITE_DONE) std::cerr << "ERROR: Could not insert tag. " << sqlite3_errmsg(db) << std::endl;
	sqlite3_finalize(stmt);
	updateTagCompletion();
	return true;
}

void GUI::on_filedetails_set_hidden(bool hidden)
{
	if (hidden) {
		on_file_tag_added("__HIDDEN__");
	} else {
		on_file_tag_deleted("__HIDDEN__");
	}
	this->on_filter_changed();
}

void GUI::populateCellDetailsList(const Glib::ustring animalID, const int cellID)
{
	/*
	 * This function requires a valid database.
	 */
	if (db == NULL) return;

	m_uiCellDetails.clear();
	
	int r;
	sqlite3_stmt *stmt;

	char query3[] = "SELECT COUNT(*) FROM files WHERE animalID=? AND cellID=?";
	sqlite3_prepare_v2(db, query3, -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, animalID.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 2, cellID);
	r = sqlite3_step(stmt);
	int numberOfFiles = sqlite3_column_int(stmt,0);
	sqlite3_finalize(stmt);

	char query[] = "SELECT depth, freq, notes, threshold FROM cells WHERE animalID=? AND cellID=?";
	sqlite3_prepare_v2(db, query, -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, animalID.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 2, cellID);
	r = sqlite3_step(stmt);

	if (r == SQLITE_ROW) {
		CellID ID;
		ID.animalID = animalID;
		ID.cellID = cellID;

		m_uiCellDetails.addRow(ID, "Cell ID", ID.cellID, Static);
		m_uiCellDetails.addRow(ID, "Files", numberOfFiles, Static);

		m_uiCellDetails.addRow(ID, "CarFreq (Hz)", 
			((char*)sqlite3_column_text(stmt, 1) == NULL) ? "" : (char*)sqlite3_column_text(stmt, 1),
			Editable
		);

		m_uiCellDetails.addRow(ID, "Threshold (dB SPL)", 
			((char*)sqlite3_column_text(stmt, 3) == NULL) ? "" : (char*)sqlite3_column_text(stmt, 3),
			Editable
		);

		m_uiCellDetails.addRow(ID, "Depth (um)", 
			((char*)sqlite3_column_text(stmt, 0) == NULL) ? "" : (char*)sqlite3_column_text(stmt, 0),
			Editable
		);

		m_uiCellDetails.addRow(ID, "Notes",
			((char*)sqlite3_column_text(stmt, 2) == NULL) ? "" : (char*)sqlite3_column_text(stmt, 2),
			Editable
		);
	} 
	sqlite3_finalize(stmt);

	const char query2[] = "SELECT tag FROM tags WHERE animalID=? AND cellID=? AND fileID IS NULL";
	sqlite3_prepare_v2(db, query2, -1, &stmt, 0);
	sqlite3_bind_text(stmt, 1, animalID.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 2, cellID);
	std::vector<Glib::ustring> tags;
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		tags.push_back((char*)sqlite3_column_text(stmt, 0));
	}
	sqlite3_finalize(stmt);
	m_CellTags.tags(tags);
}

void GUI::populateAnimalTree()
{
	char query_animals[] = "SELECT ID FROM animals ORDER BY ID";
	sqlite3_stmt *stmt_animals, *stmt_cells;

	sqlite3_prepare_v2(db, query_animals, -1, &stmt_animals, 0);
	mrp_AnimalTree->clear();
	Gtk::TreeModel::Row base;
	Gtk::TreeModel::Row row;
	Gtk::TreeModel::Row childrow;
	base = *(mrp_AnimalTree->append());
	base[m_AnimalColumns.m_col_name] = "All Animals";
	int r_animals, r_cells;
	while (true) {
		r_animals = sqlite3_step(stmt_animals);
		if (r_animals == SQLITE_ROW) {
			row = *(mrp_AnimalTree->append(base.children()));
			char* animalID = (char*)sqlite3_column_text(stmt_animals, 0);
			row[m_AnimalColumns.m_col_name] = animalID;
			char query_cells[] = "SELECT cellID FROM cells WHERE animalID=?";
			sqlite3_prepare_v2(db, query_cells, -1, &stmt_cells, 0);
			sqlite3_bind_text(stmt_cells, 1, animalID, -1, SQLITE_TRANSIENT);
			while (true) {
				r_cells = sqlite3_step(stmt_cells);
				if (r_cells == SQLITE_ROW) {
					char* cellID = (char*)sqlite3_column_text(stmt_cells, 0);
					childrow = *(mrp_AnimalTree->append(row.children()));
					childrow[m_AnimalColumns.m_col_name] = cellID;
				} else{ break; }
			}
		} else{ break; }
	}
	sqlite3_finalize(stmt_animals);
	sqlite3_finalize(stmt_cells);
}


void GUI::getFilesStatement(sqlite3_stmt **stmt, const Glib::ustring animalID, const int cellID)
{
	int minFiles = m_uiFilterFrame.minFiles();

	if (animalID != "" && cellID != -1) {
		const char query[] = "SELECT animalID, cellID, fileID, header FROM files "
				     "JOIN(SELECT COUNT(*) AS file_count, animalID, cellID FROM files GROUP BY animalID, cellID) "
				     "USING(animalID, cellID) "
				     "WHERE animalID=? AND cellID=? AND file_count >= ? "
				     "ORDER BY animalID, cellID, fileID";
		sqlite3_prepare_v2(db, query, strlen(query), stmt, NULL);
		sqlite3_bind_text(*stmt, 1, animalID.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_int(*stmt, 2, cellID);
		sqlite3_bind_int(*stmt, 3, minFiles);
	} else if (animalID != "" && cellID == -1) {
		const char query[] = "SELECT animalID, cellID, fileID, header FROM files "
				     "JOIN(SELECT COUNT(*) AS file_count, animalID, cellID FROM files GROUP BY animalID, cellID) "
				     "USING(animalID, cellID) "
				     "WHERE animalID=? AND file_count >= ? "
				     "ORDER BY animalID, cellID, fileID";
		sqlite3_prepare_v2(db, query, strlen(query), stmt, NULL);
		sqlite3_bind_text(*stmt, 1, animalID.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_int(*stmt, 2, minFiles);
	} else if (animalID == "" && cellID == -1) {
		const char query[] = "SELECT animalID, cellID, fileID, header FROM files "
				     "JOIN(SELECT COUNT(*) AS file_count, animalID, cellID FROM files GROUP BY animalID, cellID) "
				     "USING(animalID, cellID) "
				     "WHERE file_count >= ? "
				     "ORDER BY animalID, cellID, fileID";
		sqlite3_prepare_v2(db, query, strlen(query), stmt, NULL);
		sqlite3_bind_int(*stmt, 1, minFiles);
	}
}

void GUI::getCellsStatement(sqlite3_stmt **stmt, const Glib::ustring animalID, const int cellID)
{
	int minFiles = m_uiFilterFrame.minFiles();

	if (animalID != "" && cellID != -1) {
		const char query[] = "SELECT animalID, cellID, threshold, depth, freq FROM cells "
				     "JOIN(SELECT COUNT(*) AS file_count, animalID, cellID FROM files GROUP BY animalID, cellID) "
				     "USING(animalID, cellID) "
				     "WHERE animalID=? AND cellID=? AND file_count >= ? "
				     "ORDER BY animalID, cellID";
		sqlite3_prepare_v2(db, query, strlen(query), stmt, NULL);
		sqlite3_bind_text(*stmt, 1, animalID.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_int(*stmt, 2, cellID);
		sqlite3_bind_int(*stmt, 3, minFiles);
	} else if (animalID != "" && cellID == -1) {
		const char query[] = "SELECT animalID, cellID, threshold, depth, freq FROM cells "
				     "JOIN(SELECT COUNT(*) AS file_count, animalID, cellID FROM files GROUP BY animalID, cellID) "
				     "USING(animalID, cellID) "
				     "WHERE animalID=? AND file_count >= ? "
				     "ORDER BY animalID, cellID";
		sqlite3_prepare_v2(db, query, strlen(query), stmt, NULL);
		sqlite3_bind_text(*stmt, 1, animalID.c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_int(*stmt, 2, minFiles);
	} else if (animalID == "" && cellID == -1) {
		const char query[] = "SELECT animalID, cellID, threshold, depth, freq FROM cells "
				     "JOIN(SELECT COUNT(*) AS file_count, animalID, cellID FROM files GROUP BY animalID, cellID) "
				     "USING(animalID, cellID) "
				     "WHERE file_count >= ? "
				     "ORDER BY animalID, cellID";
		sqlite3_prepare_v2(db, query, strlen(query), stmt, NULL);
		sqlite3_bind_int(*stmt, 1, minFiles);
	}
}

void GUI::populateDetailsList(const Glib::ustring animalID, const int cellID)
{
	sqlite3_stmt *stmt = 0;

	getFilesStatement(&stmt, animalID, cellID);

	mp_FileDetailsTree->clear();
	Gtk::TreeModel::Row row;
	while (true) {
		int r = sqlite3_step(stmt);
		if (r == SQLITE_ROW) {

                        bool filtered = true;

                        SpikeData sd;
                        void *header = (void*)sqlite3_column_blob(stmt, 3);
                        sd.setHeader(header);

						int XVarFilter = m_uiFilterFrame.XVar();
                        if (XVarFilter == 1) {
                            if (sd.xVariable() != "Ch 1 Freq" && sd.xVariable() != "Ch 2 Freq") filtered = false;
                        }
                        if (XVarFilter == 2) {
                            if (sd.xVariable() != "Ch 1 Dur" && sd.xVariable() != "Ch 2 Dur") filtered = false;
                        }
                        if (XVarFilter == 3) {
                            if (sd.xVariable() != "Ch 1 Onset" && sd.xVariable() != "Ch 2 Onset") filtered = false;
                        }
                        if (XVarFilter == 4) {
                            if (sd.xVariable() != "Ch 1 Atten" && sd.xVariable() != "Ch 2 Atten") filtered = false;
                        }

						int r2;
						sqlite3_stmt *stmt2 = 0;
						if (m_uiFilterFrame.tag() != "")
						{
							char query_animal_tag[] = "SELECT COUNT(*) FROM tags WHERE tag=? AND animalID=? AND cellID IS NULL AND fileID IS NULL";
							sqlite3_prepare_v2(db, query_animal_tag, -1, &stmt2, 0);
							sqlite3_bind_text(stmt2, 1, m_uiFilterFrame.tag().c_str(), -1, SQLITE_TRANSIENT);
							sqlite3_bind_text(stmt2, 2, (char*)sqlite3_column_text(stmt, 0), -1, SQLITE_TRANSIENT);
							r2 = sqlite3_step(stmt2);
							bool allow_animal = sqlite3_column_int(stmt2,0) > 0;
							sqlite3_finalize(stmt2);

							char query_cell_tag[] = "SELECT COUNT(*) FROM tags WHERE tag=? AND animalID=? AND cellID=? AND fileID IS NULL";
							sqlite3_prepare_v2(db, query_cell_tag, -1, &stmt2, 0);
							sqlite3_bind_text(stmt2, 1, m_uiFilterFrame.tag().c_str(), -1, SQLITE_TRANSIENT);
							sqlite3_bind_text(stmt2, 2, (char*)sqlite3_column_text(stmt, 0), -1, SQLITE_TRANSIENT);
							sqlite3_bind_int(stmt2, 3, sqlite3_column_int(stmt, 1));
							r2 = sqlite3_step(stmt2);
							bool allow_cell = sqlite3_column_int(stmt2,0) > 0;
							sqlite3_finalize(stmt2);



							filtered = filtered && (allow_animal || allow_cell);
						}

						// Check for hidden files
						char query_cell_file[] = "SELECT COUNT(*) FROM tags WHERE tag=? AND animalID=? AND cellID=? AND fileID=?";
						sqlite3_prepare_v2(db, query_cell_file, -1, &stmt2, 0);
						sqlite3_bind_text(stmt2, 1, "__HIDDEN__", -1, SQLITE_TRANSIENT);
						sqlite3_bind_text(stmt2, 2, (char*)sqlite3_column_text(stmt, 0), -1, SQLITE_TRANSIENT);
						sqlite3_bind_int(stmt2, 3, sqlite3_column_int(stmt, 1));
						sqlite3_bind_int(stmt2, 4, sqlite3_column_int(stmt, 2));
						r2 = sqlite3_step(stmt2);
						bool hidden_file = (sqlite3_column_int(stmt2,0) > 0);
						sqlite3_finalize(stmt2);

						filtered = filtered && (!hidden_file || m_uiFilterFrame.showHidden());

                        if (filtered) {
                            row = mp_FileDetailsTree->newrow();
							row[mp_FileDetailsTree->m_Columns.m_col_hidden] = hidden_file;
							GTimeVal t;
							if (g_time_val_from_iso8601(sd.iso8601(sd.m_head.cDateTime).c_str(), &t))
							{
								row[mp_FileDetailsTree->m_Columns.m_col_time] = t.tv_usec;
							} else { 
								row[mp_FileDetailsTree->m_Columns.m_col_time] = -1;
							}
                            row[mp_FileDetailsTree->m_Columns.m_col_animalID] = (char*)sqlite3_column_text(stmt, 0);
                            row[mp_FileDetailsTree->m_Columns.m_col_cellID] = sqlite3_column_int(stmt, 1);
                            row[mp_FileDetailsTree->m_Columns.m_col_filenum] = sqlite3_column_int(stmt, 2);
                            if (sqlite3_column_text(stmt, 4) != NULL) {
                                    row[mp_FileDetailsTree->m_Columns.m_col_tags] = (char*)sqlite3_column_text(stmt, 4);
                            }

                            row[mp_FileDetailsTree->m_Columns.m_col_xaxis] = sd.xVariable();
                            row[mp_FileDetailsTree->m_Columns.m_col_trials] = sd.trials();

                            char bufferCh1[20];
                            char bufferCh2[20];
                            char buffer[50];

                            if (sd.m_head.nOnCh1 == 0) {
                                    strcpy(bufferCh1, "-");
                            } else{
                                    sprintf(bufferCh1, "%s", sd.type(1).c_str());
                            }
                            if (sd.m_head.nOnCh2 == 0) {
                                    strcpy(bufferCh2, "-");
                            } else{
                                    sprintf(bufferCh2, "%s", sd.type(2).c_str());
                            }
                            sprintf(buffer, "%s/%s", bufferCh1, bufferCh2);
                            row[mp_FileDetailsTree->m_Columns.m_col_type] = buffer;
                            bufferCh1[0] = '\0';
                            bufferCh2[0] = '\0';
                            buffer[0] = '\0';


                            if (sd.m_head.nOnCh1 == 0) {
                                    strcpy(bufferCh1, "-");
                            } else{
                                    if (sd.frequency(1, 0) == sd.frequency(1, 1)) {
                                            sprintf(bufferCh1, "%d", (int)sd.frequency(1, 0));
                                    } else{                              strcpy(bufferCh1, "Var"); }
                            }
                            if (sd.m_head.nOnCh2 == 0) {
                                    strcpy(bufferCh2, "-");
                            } else{
                                    if (sd.frequency(2, 0) == sd.frequency(2, 1)) {
                                            sprintf(bufferCh2, "%d", (int)sd.frequency(2, 0));
                                    } else{                              strcpy(bufferCh2, "Var"); }
                            }
                            sprintf(buffer, "%s/%s", bufferCh1, bufferCh2);
							row[mp_FileDetailsTree->m_Columns.m_col_freq] = buffer;
                            bufferCh1[0] = '\0';
                            bufferCh2[0] = '\0';
                            buffer[0] = '\0';


                            if (sd.m_head.nOnCh1 == 0) {
                                    strcpy(bufferCh1, "-");
                            } else{
                                    if (sd.attenuation(1, 0) == sd.attenuation(1, 1)) {
                                            sprintf(bufferCh1, "%d", (int)sd.attenuation(1, 0));
                                    } else{                              strcpy(bufferCh1, "Var"); }
                            }
                            if (sd.m_head.nOnCh2 == 0) {
                                    strcpy(bufferCh2, "-");
                            } else{
                                    if (sd.attenuation(2, 0) == sd.attenuation(2, 1)) {
                                            sprintf(bufferCh2, "%d", (int)sd.attenuation(2, 0));
                                    } else{                              strcpy(bufferCh2, "Var"); }
                            }
                            sprintf(buffer, "%s/%s", bufferCh1, bufferCh2);
                            row[mp_FileDetailsTree->m_Columns.m_col_atten] = buffer;
                            bufferCh1[0] = '\0';
                            bufferCh2[0] = '\0';
                            buffer[0] = '\0';

                            if (sd.m_head.nOnCh1 == 0) {
                                    strcpy(bufferCh1, "-");
                            } else{
                                    if (sd.duration(1, 0) == sd.duration(1, 1)) {
                                            sprintf(bufferCh1, "%d", (int)sd.duration(1, 0));
                                    } else{                              strcpy(bufferCh1, "Var"); }
                            }
                            if (sd.m_head.nOnCh2 == 0) {
                                    strcpy(bufferCh2, "-");
                            } else{
                                    if (sd.duration(2, 0) == sd.duration(2, 1)) {
                                            sprintf(bufferCh2, "%d", (int)sd.duration(2, 0));
                                    } else{                              strcpy(bufferCh2, "Var"); }
                            }
                            sprintf(buffer, "%s/%s", bufferCh1, bufferCh2);
                            row[mp_FileDetailsTree->m_Columns.m_col_dur] = buffer;
                            bufferCh1[0] = '\0';
                            bufferCh2[0] = '\0';
                            buffer[0] = '\0';

                            if (sd.m_head.nOnCh1 == 0) {
                                    strcpy(bufferCh1, "-");
                            } else{
                                    if (sd.begin(1, 0) == sd.begin(1, 1)) {
                                            sprintf(bufferCh1, "%d", (int)sd.begin(1, 0));
                                    } else{                              strcpy(bufferCh1, "Var"); }
                            }
                            if (sd.m_head.nOnCh2 == 0) {
                                    strcpy(bufferCh2, "-");
                            } else{
                                    if (sd.begin(2, 0) == sd.begin(2, 1)) {
                                            sprintf(bufferCh2, "%d", (int)sd.begin(2, 0));
                                    } else{                              strcpy(bufferCh2, "Var"); }
                            }
                            sprintf(buffer, "%s/%s", bufferCh1, bufferCh2);
                            row[mp_FileDetailsTree->m_Columns.m_col_onset] = buffer;
                            bufferCh1[0] = '\0';
                            bufferCh2[0] = '\0';
                            buffer[0] = '\0';
                        }

		} else{ 
			
			if (r == SQLITE_ERROR) {
				Gtk::MessageDialog dialog(*this, "Error loading file from database.", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK);
				dialog.set_secondary_text(sqlite3_errmsg(db));
				dialog.run();
			}
			
			break; }
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

	switch (result) {
	case (Gtk::RESPONSE_OK):
		struct dirent *dptr;
		DIR *dirp;
		std::vector<std::string> filenames = dialog.get_filenames();
		std::vector<std::string>::iterator it;
		std::string filename;

		for (it = filenames.begin(); it != filenames.end(); it++) {
			filename = *it;
			if ((dirp = opendir(filename.c_str())) == NULL) {
				std::cerr << "ERROR: Unable to open " << filename << std::endl;
			} else{
				while ((dptr = readdir(dirp))) {
					if (dptr->d_type == DT_REG) {
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
	if (sd.parse(fullfile.c_str())) {
		std::vector<std::string> fileTokens;
		std::string shortfilename(d_name);
		Tokenize(shortfilename, fileTokens, ".");

		// Insert animal
		const char q_animal[] = "INSERT INTO animals (ID) VALUES(?)";
		sqlite3_stmt *stmt_animal = 0;
		sqlite3_prepare_v2(db, q_animal, strlen(q_animal), &stmt_animal, NULL);
		sqlite3_bind_text(stmt_animal, 1, fileTokens[0].c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_step(stmt_animal);
		sqlite3_finalize(stmt_animal);

		// Insert Cell
		const char q_cell[] = "INSERT INTO cells (animalID,cellID) VALUES(?,?)";
		sqlite3_stmt *stmt_cell = 0;
		sqlite3_prepare_v2(db, q_cell, strlen(q_cell), &stmt_cell, NULL);
		sqlite3_bind_text(stmt_cell, 1, fileTokens[0].c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_int(stmt_cell, 2, atoi(fileTokens[1].c_str()));
		sqlite3_step(stmt_cell);
		sqlite3_finalize(stmt_cell);

		// Insert File
		const char q_file[] = "INSERT INTO files (animalID,cellID,fileID,header,spikes) VALUES(?,?,?,?,?)";
		sqlite3_stmt *stmt_file = 0;
		sqlite3_prepare_v2(db, q_file, strlen(q_file), &stmt_file, NULL);
		sqlite3_bind_text(stmt_file, 1, fileTokens[0].c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_int(stmt_file, 2, atoi(fileTokens[1].c_str()));
		sqlite3_bind_int(stmt_file, 3, atoi(fileTokens[2].c_str()));
		sqlite3_bind_blob(stmt_file, 4, (void*)&sd.m_head, sizeof(sd.m_head), SQLITE_TRANSIENT);
		sqlite3_bind_blob(stmt_file, 5, (void*)&sd.m_spikeArray[0], sizeof(SPIKESTRUCT) * sd.m_spikeArray.size(), SQLITE_TRANSIENT);
		sqlite3_step(stmt_file);
		sqlite3_finalize(stmt_file);
	}
}


void GUI::on_menuQuit_activate()
{
	// Save window properties
	int width, height, x, y;

	get_size(width, height);
	get_position(x, y);
	settings.set("winWidth", width);
	settings.set("winHeight", height);
	settings.set("winX", x);
	settings.set("winY", y);

	sqlite3_close(db);
	hide();
}

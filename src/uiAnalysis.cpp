#include "uiAnalysis.h"


uiAnalysis::uiAnalysis(sqlite3 *db, uiFileDetailsTreeView* fileDetailsTree, Gtk::Window* parent)
{
	this->db = db;
	mp_FileDetailsTree = fileDetailsTree;
	m_parent = parent;

	Gtk::Toolbar *toolbar = Gtk::manage( new Gtk::Toolbar() );
	Gtk::ToolButton *tbOpen = Gtk::manage( new Gtk::ToolButton(Gtk::Stock::OPEN) );
	tbRun = Gtk::manage( new Gtk::ToolButton(Gtk::Stock::EXECUTE) );
	tbRun->set_sensitive(false);
    toolbar->append(*tbOpen, sigc::mem_fun(*this, &uiAnalysis::on_open_clicked) );
    toolbar->append(*tbRun, sigc::mem_fun(*this, &uiAnalysis::on_run_clicked) );
	this->pack_start(*toolbar, false, false);

	Gtk::ScrolledWindow *swOutput = Gtk::manage( new Gtk::ScrolledWindow() );
	mrp_tbOutput = Gtk::TextBuffer::create();
	Gtk::TextView *tvOutput = Gtk::manage( new Gtk::TextView(mrp_tbOutput) );
	tvOutput->set_editable(false);
	swOutput->add(*tvOutput);
	this->pack_start(*swOutput);

}

uiAnalysis::~uiAnalysis() {}

void uiAnalysis::on_open_clicked()
{
	Gtk::FileChooserDialog dialog("Select the analysis script",
				      Gtk::FILE_CHOOSER_ACTION_OPEN);

	dialog.set_transient_for(*m_parent);

	// Add response buttons to the dialog:
	dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

	// Show the dialog
	int result = dialog.run();

	switch (result) {
	case (Gtk::RESPONSE_OK):
		std::string filename = dialog.get_filename();
		tbRun->set_sensitive(true);
		m_filename = filename;
		break;
	}
}   

void uiAnalysis::on_run_clicked()
{
	runScript();
}

void uiAnalysis::runScript()
{
	//TODO: Check if we have a valid filename
	if (m_filename == "") return;


	Py_Initialize();

	// Redirect python output
	Glib::ustring stdOut =
"class CatchOut:\n\
	def __init__(self):\n\
		self.value = ''\n\
	def write(self, txt):\n\
		self.value += txt\n\
catchOut = CatchOut()\n\
sys.stdout = catchOut\n\
sys.stderr = catchOut\n\
";
	PyObject *main_module = PyImport_ImportModule("__main__");
	PyObject *main_dict = PyModule_GetDict(main_module);

    PyObject *sys_module = PyImport_ImportModule("sys");
	PyDict_SetItemString(main_dict, "sys", sys_module);

	PyRun_SimpleString(stdOut.c_str());

    // Provide python with some useful data
	PyDict_SetItemString(main_dict, "Cells", buildCellList());
	PyDict_SetItemString(main_dict, "Files", buildFileList());

	// Open the file and run the code
	FILE *fp;
	fp = fopen(m_filename.c_str(), "r");
	PyRun_SimpleFile(fp, m_filename.c_str());
	fclose(fp);

	// Get the output and update the text buffer
	PyObject *catcher = PyObject_GetAttrString(main_module,"catchOut");
	PyObject *output = PyObject_GetAttrString(catcher,"value");
	Glib::ustring r(PyString_AsString(output));
	mrp_tbOutput->set_text(r);

	Py_Finalize();
}                                                          

PyObject* uiAnalysis::buildCellList()
{
	PyObject *list = PyList_New(0);

	/*
	Keys: AnimalID: string
		  CellID: number
		  CarFreq: number (in Hz)
		  Threshold: number (in dB SPL)
		  Depth: number (in um)
	*/

	std::set<CellID> uniqueCells;

	Gtk::TreeIter iter;
    for (iter = mp_FileDetailsTree->mrp_ListStore->children().begin(); 
	     iter != mp_FileDetailsTree->mrp_ListStore->children().end(); 
		 iter++)
	{
		Gtk::TreeModel::Row row = *iter;
		CellID tmp;
		tmp.animalID = row.get_value(mp_FileDetailsTree->m_Columns.m_col_animalID);
		tmp.cellID = row.get_value(mp_FileDetailsTree->m_Columns.m_col_cellID);
		if (uniqueCells.find(tmp) == uniqueCells.end())
		{
			uniqueCells.insert(tmp);
			PyObject *cell = Py_BuildValue("{s:s,s:i,s:i,s:i,s:i}", 
				"AnimalID", row.get_value(mp_FileDetailsTree->m_Columns.m_col_animalID).c_str(), 
				"CellID", row.get_value(mp_FileDetailsTree->m_Columns.m_col_cellID),
				"CarFreq", row.get_value(mp_FileDetailsTree->m_Columns.m_col_carfreq),
				"Threshold", row.get_value(mp_FileDetailsTree->m_Columns.m_col_threshold),
				"Depth", row.get_value(mp_FileDetailsTree->m_Columns.m_col_depth)
				);
			PyList_Append(list, cell);
		}
	}

	return list;
}

PyObject* uiAnalysis::buildFileList()
{
	PyObject *list = PyList_New(0);

	sqlite3_stmt *stmt = 0;
	const char query[] = "SELECT header FROM files WHERE animalID=? AND cellID=? AND fileID=?";

	Gtk::TreeIter iter;
    for (iter = mp_FileDetailsTree->mrp_ListStore->children().begin(); 
	     iter != mp_FileDetailsTree->mrp_ListStore->children().end(); 
		 iter++)
	{
		Gtk::TreeModel::Row row = *iter;
		sqlite3_prepare_v2(db, query, strlen(query), &stmt, NULL);
		sqlite3_bind_text(stmt, 1, row.get_value(mp_FileDetailsTree->m_Columns.m_col_animalID).c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_int(stmt, 2, row.get_value(mp_FileDetailsTree->m_Columns.m_col_cellID));
		sqlite3_bind_int(stmt, 3, row.get_value(mp_FileDetailsTree->m_Columns.m_col_filenum));

		int r = sqlite3_step(stmt);
		if (r == SQLITE_ROW) 
		{
			SpikeData sd;
			void *header = (void*)sqlite3_column_blob(stmt, 3);
			sd.setHeader(header);

			PyObject *file = Py_BuildValue("{s:s,s:i,s:i,s:i,s:i}", 
				"AnimalID", row.get_value(mp_FileDetailsTree->m_Columns.m_col_animalID).c_str(), 
				"CellID", row.get_value(mp_FileDetailsTree->m_Columns.m_col_cellID),
				"FileID", row.get_value(mp_FileDetailsTree->m_Columns.m_col_filenum)
				);
			PyList_Append(list, file);
		}
	}

	return list;
}

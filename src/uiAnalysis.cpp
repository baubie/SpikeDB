#include "uiAnalysis.h"


uiAnalysis::uiAnalysis(sqlite3 **db, uiFileDetailsTreeView* fileDetailsTree, Gtk::Window* parent)
{
	this->db = db;
	mp_FileDetailsTree = fileDetailsTree;
	m_parent = parent;

	Gtk::Toolbar *toolbar = Gtk::manage( new Gtk::Toolbar() );
	tbOpen = Gtk::manage( new Gtk::ToolButton(Gtk::Stock::OPEN) );
	tbRun = Gtk::manage( new Gtk::ToolButton(Gtk::Stock::EXECUTE) );
	tbRun->set_sensitive(false);
    toolbar->append(*tbOpen, sigc::mem_fun(*this, &uiAnalysis::on_open_clicked) );
    toolbar->append(*tbRun, sigc::mem_fun(*this, &uiAnalysis::on_run_clicked) );
	this->pack_start(*toolbar, false, false);

	Gtk::ScrolledWindow *swOutput = Gtk::manage( new Gtk::ScrolledWindow() );
	mrp_tbOutput = Gtk::TextBuffer::create();
	tvOutput = Gtk::manage( new Gtk::TextView(mrp_tbOutput) );
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

void uiAnalysis::addOutput(Glib::ustring t)
{
	mrp_tbOutput->insert(mrp_tbOutput->end(), t);
	while (Gtk::Main::events_pending()) {
    	Gtk::Main::iteration();
	}
}

void uiAnalysis::runScript()
{
	//TODO: Check if we have a valid filename
	if (m_filename == "") return;


	mrp_tbOutput->set_text("*** Initializing Analysis Plugin Library ***\n");
	addOutput("Using Script: ");
	addOutput(m_filename);
	addOutput("\n");

	Py_Initialize();

	// Redirect python output
	Glib::ustring stdOut =
"class CatchOut_:\n\
	def __init__(self):\n\
		self.value = ''\n\
	def write(self, txt):\n\
		self.value += txt\n\
catchOut = CatchOut_()\n\
sys.stdout = catchOut\n\
sys.stderr = catchOut\n\
";
	PyObject *main_module = PyImport_ImportModule("__main__");
	PyObject *main_dict = PyModule_GetDict(main_module);

    PyObject *sys_module = PyImport_ImportModule("sys");
	PyDict_SetItemString(main_dict, "sys", sys_module);
	Py_DECREF(sys_module);

	PyRun_SimpleString(stdOut.c_str());

    // Provide python with some useful data
	addOutput(" - Building Cells dictionary...");
	PyObject* pyCells = buildCellList();
	PyDict_SetItemString(main_dict, "Cells", pyCells);
	Py_DECREF(pyCells);
	addOutput(" [Done]\n");

	addOutput(" - Building Files dictionary...");
	PyObject* pyFiles = buildFileList();
    PyDict_SetItemString(main_dict, "Files", pyFiles);
	Py_DECREF(pyFiles);
	addOutput(" [Done]\n");

	addOutput("*** Running Analysis Plugin ***\n\n");

	// Open the file and run the code
	FILE *fp;
	fp = fopen(m_filename.c_str(), "r");
	PyRun_SimpleFile(fp, m_filename.c_str());
	fclose(fp);

	// Get the output and update the text buffer
	PyObject *catcher = PyObject_GetAttrString(main_module,"catchOut");
	PyObject *output = PyObject_GetAttrString(catcher,"value");
	Glib::ustring r(PyString_AsString(output));
	addOutput(r);

	addOutput("\n*** Analysis Plugin Completed ***");

	// Try to reclaim some memory
	PyRun_SimpleString("del Cells");
	PyRun_SimpleString("del Files");
	PyRun_SimpleString("import gc");
	PyRun_SimpleString("gc.collect()");


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
			Py_DECREF(cell);
		}
	}

	return list;
}

PyObject* uiAnalysis::buildFileList()
{
	PyObject *list = PyList_New(mp_FileDetailsTree->mrp_ListStore->children().size());

	sqlite3_stmt *stmt = 0;
	const char query[] = "SELECT header, spikes FROM files WHERE animalID=? AND cellID=? AND fileID=?";
	sqlite3_prepare_v2(*db, query, strlen(query), &stmt, NULL);

	Gtk::TreeIter iter;
	int listCount=0;
	int r;
    for (iter = mp_FileDetailsTree->mrp_ListStore->children().begin(); 
	     iter != mp_FileDetailsTree->mrp_ListStore->children().end(); 
		 iter++)
	{
		Gtk::TreeModel::Row row = *iter;
		sqlite3_bind_text(stmt, 1, row.get_value(mp_FileDetailsTree->m_Columns.m_col_animalID).c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_int(stmt, 2, row.get_value(mp_FileDetailsTree->m_Columns.m_col_cellID));
		sqlite3_bind_int(stmt, 3, row.get_value(mp_FileDetailsTree->m_Columns.m_col_filenum));

		r = sqlite3_step(stmt);
		if (r == SQLITE_ROW) 
		{
			SpikeData sd;
			void *header = (void*)sqlite3_column_blob(stmt, 0);
			sd.setHeader(header);

			SPIKESTRUCT *spikes = (SPIKESTRUCT*)sqlite3_column_blob(stmt, 1);
			int spikes_length = sqlite3_column_bytes(stmt, 1);
			int numSpikes = spikes_length / sizeof(SPIKESTRUCT);
			sd.m_spikeArray.assign(spikes, spikes + numSpikes);

			PyObject *file = Py_BuildValue("{s:s,s:i,s:i,s:s}", 
				"AnimalID", row.get_value(mp_FileDetailsTree->m_Columns.m_col_animalID).c_str(), 
				"CellID", row.get_value(mp_FileDetailsTree->m_Columns.m_col_cellID),
				"FileID", row.get_value(mp_FileDetailsTree->m_Columns.m_col_filenum),
				"datetime", sd.iso8601(sd.m_head.cDateTime).c_str()
				);                

			PyObject *trials = PyList_New(0);
			for (int i = 0; i < sd.m_head.nSweeps; ++i) {
				PyObject *trial = PyDict_New();
				PyDict_SetItemString(trial, "xvalue", PyLong_FromDouble(sd.xvalue(i)));
				PyObject *spikes = PyList_New(sd.m_head.nPasses);
				for (int p = 0; p < sd.m_head.nPasses; ++p) {
					PyObject *pass = PyList_New(0);
					for (unsigned int s = 0; s < sd.m_spikeArray.size(); ++s) {
						// Spike sweeps are 1 based but here we are 0 based
						if (sd.m_spikeArray[s].nSweep == i + 1 && sd.m_spikeArray[s].nPass == p+1) {
							PyObject* pyfTime = PyFloat_FromDouble((double)sd.m_spikeArray[s].fTime);
							PyList_Append(pass, pyfTime);
							Py_DECREF(pyfTime);
						}
					}
					PyList_SET_ITEM(spikes, p, pass);
				}
				PyDict_SetItemString(trial, "spikes", spikes);
				Py_DECREF(spikes);
				PyList_Append(trials, trial);
				Py_DECREF(trial);
			}
			PyDict_SetItemString(file, "trials", trials);
			Py_DECREF(trials);

			PyList_SET_ITEM(list, listCount, file);
			listCount++;
		}
	}
	sqlite3_finalize(stmt);

	return list;
}

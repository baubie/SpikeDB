#include "uiAnalysis.h"

namespace bp=boost::python;
using namespace bp;

uiAnalysis::uiAnalysis(sqlite3 **db, uiFileDetailsTreeView* fileDetailsTree, bool compact, Gtk::Window* parent)
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

	mp_plot = Gtk::manage( new EasyPlotmm() );
	Gtk::ScrolledWindow *swOutput = Gtk::manage( new Gtk::ScrolledWindow() );
	mrp_tbOutput = Gtk::TextBuffer::create();
	tvOutput = Gtk::manage( new Gtk::TextView(mrp_tbOutput) );
	tvOutput->set_editable(false);
	swOutput->add(*tvOutput);

	if (compact)
	{
		this->pack_start(*mp_plot);
	} else {
		Gtk::VPaned *vp = Gtk::manage( new Gtk::VPaned() );
		vp->pack1(*mp_plot);
		vp->pack2(*swOutput);
		this->pack_start(*vp);
	}


	if (compact)
	{
		toolbar->set_toolbar_style(Gtk::TOOLBAR_ICONS);
		toolbar->set_icon_size(Gtk::IconSize(Gtk::ICON_SIZE_SMALL_TOOLBAR));
	}

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

	bp::object main_module((
		bp::handle<>(bp::borrowed(PyImport_AddModule("__main__")))));
	bp::object main_namespace = main_module.attr("__dict__");


	main_namespace["pySpikeDB"] = class_<pySpikeDB>("pySpikeDB")
		.def("getCells", &pySpikeDB::getCells)
		.def("getFiles", &pySpikeDB::getFiles)
		.def("plotClear", &pySpikeDB::plotClear)
		.def("plotLine", &pySpikeDB::plotLine);

	pySpikeDB pySpikeDB(db, mp_FileDetailsTree, mp_plot);
	main_namespace["SpikeDB"] = bp::ptr(&pySpikeDB);

	addOutput("*** Running Analysis Plugin ***\n\n");

	// Handle stdout redirection
	// Stolen from: http://mail.python.org/pipermail/cplusplus-sig/2004-August/007679.html
	const Glib::ustring CatchOutput =
			 "class StdoutCatcher:\n"
			 "\tdef __init__(self):\n"
			 "\t\tself.data = ''\n"
			 "\tdef write(self, stuff):\n"
			 "\t\tself.data = self.data + stuff\n"
			 "\n"
			 "import sys\n"
			 "TheStdoutCatcher = StdoutCatcher()\n"
			 "sys.stdout = TheStdoutCatcher\n"
			 "sys.stderr = TheStdoutCatcher\n";
	PyRun_SimpleString(CatchOutput.c_str());


	// Open the file and run the code
	FILE *fp;
	fp = fopen(m_filename.c_str(), "r");
	PyRun_SimpleFile(fp, m_filename.c_str());
	fclose(fp);

	// Get the output and update the text buffer
	object Catcher (main_namespace ["TheStdoutCatcher"]);
	object CatcherData (borrowed (PyObject_GetAttrString (Catcher.ptr(),"data")));
	const std::string &S = extract<std::string>(CatcherData);
	addOutput(S);

	addOutput("\n*** Analysis Plugin Completed ***");
}                                                          


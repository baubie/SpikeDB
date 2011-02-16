#include "uiAnalysis.h"


uiAnalysis::uiAnalysis(Gtk::Window* parent)
{
	m_parent = parent;
	Gtk::Toolbar *toolbar = Gtk::manage( new Gtk::Toolbar() );
	Gtk::ToolButton *tbOpen = Gtk::manage( new Gtk::ToolButton(Gtk::Stock::OPEN) );
	tbRun = Gtk::manage( new Gtk::ToolButton(Gtk::Stock::EXECUTE) );
	tbRun->set_sensitive(false);
    toolbar->append(*tbOpen, sigc::mem_fun(*this, &uiAnalysis::on_open_clicked) );
    toolbar->append(*tbRun, sigc::mem_fun(*this, &uiAnalysis::on_run_clicked) );
	this->pack_start(*toolbar, false, false);

	mrp_tbOutput = Gtk::TextBuffer::create();
	Gtk::TextView *tvOutput = Gtk::manage( new Gtk::TextView(mrp_tbOutput) );
	tvOutput->set_editable(false);
	this->pack_start(*tvOutput);

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
"import sys\n\
class CatchOut:\n\
	def __init__(self):\n\
		self.value = ''\n\
	def write(self, txt):\n\
		self.value += txt\n\
catchOut = CatchOut()\n\
sys.stdout = catchOut\n\
sys.stderr = catchOut\n\
";

	PyObject *pModule = PyImport_AddModule("__main__");
	PyRun_SimpleString(stdOut.c_str());

	FILE *fp;
	fp = fopen(m_filename.c_str(), "r");
	PyRun_SimpleFile(fp, m_filename.c_str());
	fclose(fp);

	PyObject *catcher = PyObject_GetAttrString(pModule,"catchOut");
	PyObject *output = PyObject_GetAttrString(catcher,"value");
	Glib::ustring r(PyString_AsString(output));
	mrp_tbOutput->set_text(r);

	Py_Finalize();
}                                                          

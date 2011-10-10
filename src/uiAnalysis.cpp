#include "uiAnalysis.h"

namespace bp=boost::python;
using namespace bp;

uiAnalysis::uiAnalysis(sqlite3 **db, uiFileDetailsTreeView* fileDetailsTree, bool compact, Settings *settings, Gtk::Window* parent)
{
	this->db = db;
	this->compact = compact;
	forceAbsBegin = forceAbsEnd = -1;
	mp_FileDetailsTree = fileDetailsTree;
	m_parent = parent;
	this->settings = settings;

	initPlugins();

	Gtk::Toolbar *toolbar = Gtk::manage( new Gtk::Toolbar() );

	tbOpen = Gtk::manage( new Gtk::ToolButton(Gtk::Stock::OPEN) );
	tbRun = Gtk::manage( new Gtk::ToolButton(Gtk::Stock::EXECUTE) );
	tbRun->set_sensitive(false);

	if (!compact) {
		toolbar->append(*tbOpen, sigc::mem_fun(*this, &uiAnalysis::on_open_clicked) );
		toolbar->append(*tbRun, sigc::mem_fun(*this, &uiAnalysis::on_run_clicked) );
	}

	tbPlugins = Gtk::manage( new Gtk::ComboBoxText() );
	Gtk::ToolItem *tbPluginItem = Gtk::manage( new Gtk::ToolItem() );
	tbPluginItem->add(*tbPlugins);
	for (unsigned int i = 0; i < plugins.size(); ++i) {
		tbPlugins->append_text(plugins[i].second);
		if (plugins[i].second == "Mean Spike Count") tbPlugins->set_active_text("Mean Spike Count");
	}
	tbPlugins->signal_changed().connect(sigc::mem_fun(*this, &uiAnalysis::on_plugin_changed));
	tbPlugins->set_focus_on_click(false);
	toolbar->append(*tbPluginItem);

	tbShowErr = Gtk::manage( new Gtk::CheckButton() );
	tbShowErr->set_label("Show Error Bars");
	tbShowErr->set_active(false);
	Gtk::ToolItem *tbShowErrItem = Gtk::manage( new Gtk::ToolItem() );
	tbShowErr->signal_clicked().connect(sigc::mem_fun(*this, &uiAnalysis::on_showerr_clicked));
	tbShowErrItem->add(*tbShowErr);
	tbShowErr->set_focus_on_click(false);
	toolbar->append(*tbShowErrItem);

	this->pack_start(*toolbar, false, false);

	mp_plot = Gtk::manage( new EasyPlotmm() );
	Gtk::ScrolledWindow *swOutput = Gtk::manage( new Gtk::ScrolledWindow() );
	mrp_tbOutput = Gtk::TextBuffer::create();
	tvOutput = Gtk::manage( new Gtk::TextView(mrp_tbOutput) );
	tvOutput->set_editable(false);
	swOutput->add(*tvOutput);

	if (compact)
	{
		Gtk::VPaned *vp = Gtk::manage( new Gtk::VPaned() );
		tvOutput->set_size_request(-1,0);
		swOutput->set_size_request(-1,0);
		vp->pack1(*mp_plot);
		vp->pack2(*swOutput);
		vp->set_position(1000000); // Stupid high position to ensure it is closed by default
		swOutput->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
		this->pack_start(*vp);
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


void uiAnalysis::initPlugins()
{
	/**
	 * Setup built-in plugins
	 */

	plugins.push_back(std::pair<Glib::ustring,Glib::ustring>("", "Custom Analysis Script"));


	std::vector<std::string> search_paths;
#ifdef __APPLE__
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	CFURLRef resourcesURL = CFBundleCopyBundleURL(mainBundle);
	CFStringRef str = CFURLCopyFileSystemPath( resourcesURL, kCFURLPOSIXPathStyle );
	CFRelease(resourcesURL);
	char path[1024];
	CFStringGetCString( str, path, FILENAME_MAX, kCFStringEncodingASCII );
	CFRelease(str);
	Glib::ustring app_path(path);
	search_paths.push_back(app_path+"/../plugins/");
	search_paths.push_back("/Library/Application Support/SpikeDB/plugins/");
#endif
#ifdef linux
	struct passwd *p  = getpwuid(getuid());
	char *home = p->pw_dir;
	Glib::ustring homedir(home);
	homedir += "/.spikedb/plugins/";
	// Figure out where to search here
	search_paths.push_back("/usr/local/share/spikedb/plugins/");
	search_paths.push_back(homedir);
#endif


	struct dirent *dptr;
	DIR *dirp;
	std::vector<std::string>::iterator it;
	std::string filename;

	for (it = search_paths.begin(); it != search_paths.end(); it++) {
		filename = *it;
		if ((dirp = opendir(filename.c_str())) == NULL) {
			std::cerr << "ERROR: Unable to open " << filename << std::endl;
		} else{
			while ((dptr = readdir(dirp))) {
#ifdef _WIN32
                {
#else
				if (dptr->d_type == DT_REG) {
#endif
					std::string possibleFile = dptr->d_name;
					if (possibleFile.length() >= 4 && possibleFile.substr(possibleFile.length()-3,3) == ".py")
					{
						Glib::RefPtr<Gio::File> file=Gio::File::create_for_path(filename+possibleFile);
						Glib::RefPtr<Gio::DataInputStream> fin=Gio::DataInputStream::create(file->read());
						std::string line;
						std::string name;
						fin->read_line(line);
						if (line.substr(0,3) == "###")
						{
							plugins.push_back(std::pair<Glib::ustring,Glib::ustring>(
										filename+possibleFile,
										line.substr(4))
									);
						}
					}
				}
			}
			closedir(dirp);
		}
	}
}

void uiAnalysis::on_showerr_clicked()
{
	mp_plot->clear();
	runPlugin();
}

void uiAnalysis::on_open_clicked()
{
	Gtk::FileChooserDialog dialog("Select the analysis script",
				      Gtk::FILE_CHOOSER_ACTION_OPEN);

	dialog.set_transient_for(*m_parent);

	// Set default folder
	dialog.set_current_folder(settings->get_string("lastScriptFolder", "."));

	// Add response buttons to the dialog:
	dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
	dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);

	// Show the dialog
	int result = dialog.run();

	switch (result) {
	case (Gtk::RESPONSE_OK):
		std::string filename = dialog.get_filename();
		settings->set("lastScriptFolder", Glib::path_get_dirname(filename));
		tbRun->set_sensitive(true);
		m_filename = filename;
		tbPlugins->set_active_text("Custom Analysis Script");
		break;
	}
}

void uiAnalysis::on_run_clicked()
{
	mp_plot->clear();
	runScript();
}

void uiAnalysis::on_plugin_changed()
{
	mp_plot->clear();
	runPlugin();
}

void uiAnalysis::addOutput(Glib::ustring t)
{
	mrp_tbOutput->insert(mrp_tbOutput->end(), t);
	while (Gtk::Main::events_pending()) {
    	Gtk::Main::iteration();
	}
}

void uiAnalysis::forceSpikesAbs(double begin, double end)
{
	forceAbsBegin = begin;
	forceAbsEnd = end;
}

void uiAnalysis::runPlugin()
{
	runScript(plugins[tbPlugins->get_active_row_number()].first);
}

void uiAnalysis::runScript(const Glib::ustring &plugin)
{
	//TODO: Check if we have a valid filename
	if (m_filename == "" && plugin == "") return;

	if (!compact) {
		mrp_tbOutput->set_text("*** Initializing Analysis Plugin Library ***\n");
		addOutput("Using Script: ");
		addOutput(m_filename);
		addOutput("\n");
	} else {
		// Make sure we clear it
		mrp_tbOutput->set_text("");
	}

	Py_Initialize();

	bp::object main_module((
		bp::handle<>(bp::borrowed(PyImport_AddModule("__main__")))));
	bp::object main_namespace = main_module.attr("__dict__");

	main_namespace["pySpikeDB"] = class_<pySpikeDB>("pySpikeDB")
		.def("getCells", &pySpikeDB::getCells)
		.def("getFiles", &pySpikeDB::getFiles)
		.def("write", &pySpikeDB::print)
		.def("mean", &pySpikeDB::mean)
		.def("stddev", &pySpikeDB::stddev)
		.def("ttest", &pySpikeDB::ttest)
		.def("filterSpikesAbs", &pySpikeDB::filterSpikesAbs)
		.def("filterSpikesRel", &pySpikeDB::filterSpikesRel)
		.def("plotXLabel", &pySpikeDB::plotXLabel)
		.def("plotYLabel", &pySpikeDB::plotYLabel)
		.def("plotYMin", &pySpikeDB::plotYMin)
		.def("plotYMax", &pySpikeDB::plotYMax)
		.def("plotXMin", &pySpikeDB::plotXMin)
		.def("plotXMax", &pySpikeDB::plotXMax)
		.def("plotClear", &pySpikeDB::plotClear)
		.def("plotSetRGBA", &pySpikeDB::plotSetRGBA)
		.def("plotSetPointSize", &pySpikeDB::plotSetPointSize)
		.def("plotSetLineWidth", &pySpikeDB::plotSetLineWidth)
		.def("plotLine", &pySpikeDB::plotLine);
	pySpikeDB _pySpikeDB(db, mp_FileDetailsTree, mp_plot, mrp_tbOutput);
	_pySpikeDB.setShowErr(tbShowErr->get_active());
	_pySpikeDB.forceSpikesAbs(forceAbsBegin, forceAbsEnd);
	main_namespace["SpikeDB"] = bp::ptr(&_pySpikeDB);
	main_namespace["SpikeDB"].attr("__dict__")["VARYING"] = VARYING_STIMULUS;
	main_namespace["SpikeDB"].attr("__dict__")["NOPOINT"] = EasyPlotmm::NOPOINT;


	// Redirect stderr and stdout
	main_namespace["pySpikeDBWrite"] = class_<pySpikeDB>("pySpikeDBWrite")
		.def("write", &pySpikeDB::print);
	pySpikeDB _pySpikeDBWrite(db, mp_FileDetailsTree, mp_plot, mrp_tbOutput);
	main_namespace["SpikeDBWrite"] = bp::ptr(&_pySpikeDBWrite);
	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.stderr = SpikeDBWrite");
	PyRun_SimpleString("sys.stdout = SpikeDBWrite");


	if (!compact) addOutput("*** Running Analysis Plugin ***\n\n");

	// Open the file and run the code
	FILE *fp;
	if (plugin == "") {
		fp = fopen(m_filename.c_str(), "r");
	} else {
		fp = fopen(plugin.c_str(), "r");
	}
	PyRun_SimpleFile(fp, m_filename.c_str());
	fclose(fp);

	if (!compact) addOutput("\n*** Analysis Plugin Completed ***");
}

void uiAnalysis::print(const std::string &s)
{
	addOutput(s);
}

EasyPlotmm* uiAnalysis::getPlot()
{
	return mp_plot;
}

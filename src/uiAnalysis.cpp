/*
Copyright (c) 2011-2012, Brandon Aubie
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "stdafx.h"

#include "uiAnalysis.h"

bool uiAnalysis::setupPython = false;

namespace bp=boost::python;
using namespace bp;

uiAnalysis::uiAnalysis(sqlite3 **db, Gtk::Notebook* notebook, uiFileDetailsTreeView* fileDetailsTree, Gtk::TreeView* animalTree, AnimalColumns* animalColumns, Gtk::Statusbar* statusbar, bool compact, Settings *settings, Gtk::Window* parent=NULL)
{
	this->db = db;
	this->compact = compact;
	forceAbsBegin = forceAbsEnd = -1;
	forceRelBegin = forceRelEnd = -1;
	mp_Notebook = notebook;
	mp_FileDetailsTree = fileDetailsTree;
	mp_StatusBar = statusbar;
	mp_AnimalTree = animalTree;
	mp_AnimalColumns = animalColumns;
	mp_parent = parent;
	this->settings = settings;

	this->setupPython = false;

	initPlugins();

	Gtk::Toolbar *toolbar = Gtk::manage( new Gtk::Toolbar() );

	tbOpen = Gtk::manage( new Gtk::ToolButton(Gtk::Stock::OPEN) );
	tbRun = Gtk::manage( new Gtk::ToolButton(Gtk::Stock::EXECUTE) );
	tbOptions = Gtk::manage( new Gtk::ToolButton(Gtk::Stock::PREFERENCES) );
	tbAction = Gtk::manage( new Gtk::ToolButton(Gtk::Stock::ADD) );
	mp_pbStatus = Gtk::manage( new Gtk::ProgressBar() );
	tbRun->set_sensitive(false);

	Gtk::ToolItem *tiProgressBar = Gtk::manage( new Gtk::ToolItem() );
	tiProgressBar->add(*mp_pbStatus);

	if (!compact) {
		toolbar->append(*tbOpen, sigc::mem_fun(*this, &uiAnalysis::on_open_clicked) );
		toolbar->append(*tbRun, sigc::mem_fun(*this, &uiAnalysis::on_run_clicked) );
	}

	tbPlugins = Gtk::manage( new Gtk::ComboBoxText() );
	Gtk::ToolItem *tbPluginItem = Gtk::manage( new Gtk::ToolItem() );
	tbPluginItem->add(*tbPlugins);
	bool foundActive = false;
	std::vector<std::pair<Glib::ustring, Glib::ustring> >::iterator it;
	int count = 0;
	for (it = plugins.begin(); it != plugins.end(); it++) {
		tbPlugins->append((*it).second);
		if (plugins[count].second == "Spike Count Analysis") {
			tbPlugins->set_active(count);
			foundActive = true;
		}
		count++;
	}
	if (!foundActive) {
		tbPlugins->set_active(0);
	}
	tbPlugins->signal_changed().connect(sigc::mem_fun(*this, &uiAnalysis::on_plugin_changed));
	tbPlugins->set_focus_on_click(false);
	toolbar->append(*tbPluginItem);

	tbOptions->signal_clicked().connect(sigc::mem_fun(*this, &uiAnalysis::on_options_clicked));
	tbAction->signal_clicked().connect(sigc::mem_fun(*this, &uiAnalysis::on_action_clicked));
	tbAction->set_sensitive(false);
	Gtk::ToolItem *tbOptionsItem = Gtk::manage( new Gtk::ToolItem() );
	Gtk::ToolItem *tbActionItem = Gtk::manage( new Gtk::ToolItem() );
	tbOptionsItem->add(*tbOptions);
	tbActionItem->add(*tbAction);
	toolbar->append(*tbOptionsItem);
	toolbar->append(*tbActionItem);

	if (!compact) {
		toolbar->append(*tiProgressBar);
	}
	this->pack_start(*toolbar, false, false);

	mp_plot = Gtk::manage( new EasyPlotmm() );
	Gtk::ScrolledWindow *swOutput = Gtk::manage( new Gtk::ScrolledWindow() );
	mrp_tbOutput = Gtk::TextBuffer::create();
	tvOutput = Gtk::manage( new Gtk::TextView(mrp_tbOutput) );
	tvOutput->set_editable(false);
	tvOutput->set_cursor_visible(false);
	swOutput->add(*tvOutput);

	mp_plot->signal_clicked_point().connect(sigc::mem_fun(*this, &uiAnalysis::on_data_point_clicked));
	mp_plot->signal_hovered_on_point().connect(sigc::mem_fun(*this, &uiAnalysis::on_hovered_on_point));
	mp_plot->signal_moved_off_point().connect(sigc::mem_fun(*this, &uiAnalysis::on_moved_off_point));

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

	if (!compact) {
		plugins.push_back(std::pair<Glib::ustring,Glib::ustring>("", "Custom Analysis Script"));
	}

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
	search_paths.push_back("./plugins/");
#endif
#ifdef linux
	struct passwd *p  = getpwuid(getuid());
	char *home = p->pw_dir;
	Glib::ustring homedir(home);
	homedir += "/.spikedb/plugins/";
	// Figure out where to search here
	search_paths.push_back("/usr/local/share/spikedb/plugins/");
	search_paths.push_back(homedir);
	search_paths.push_back("./plugins/");
#endif
#ifdef WIN32
	search_paths.push_back(".\\plugins\\");
#endif

	struct dirent *dptr;
	DIR *dirp;
	std::vector<std::string>::iterator it;
	std::vector<std::string> addedPlugins;
	std::string filename;

	for (it = search_paths.begin(); it != search_paths.end(); it++) {
		filename = *it;
		if ((dirp = opendir(filename.c_str())) == NULL) {
//			std::cerr << "ERROR: Unable to open " << filename << std::endl;
		} else{
			while ((dptr = readdir(dirp))) {
				if (dptr->d_type == DT_REG) {
					std::string possibleFile = dptr->d_name;
					if (possibleFile.length() >= 4 && possibleFile.substr(possibleFile.length()-3,3) == ".py")
					{
						Glib::RefPtr<Gio::File> file=Gio::File::create_for_path(filename+possibleFile);
						Glib::RefPtr<Gio::DataInputStream> fin=Gio::DataInputStream::create(file->read());
#ifdef WIN32
						fin->set_newline_type(Gio::DATA_STREAM_NEWLINE_TYPE_CR_LF);
#endif
						std::string line;
						fin->read_line(line);
						if (line.substr(0,3) == "###")
						{
							if (std::find(addedPlugins.begin(), addedPlugins.end(), file->get_path()) == addedPlugins.end())
							{
								// Ensure we don't double up the same plugins
								plugins.push_back(std::pair<Glib::ustring,Glib::ustring>(
											filename+possibleFile,
											line.substr(4))
										);

								addedPlugins.push_back(file->get_path());
							}
						}
					}
				}
			}
			closedir(dirp);
		}
	}
}

void uiAnalysis::on_data_point_clicked(const double &/*x*/, const double &/*y*/, const std::string &/*name*/, const std::string &data)
{
	Gtk::TreeModel::Path path(data);
	mp_AnimalTree->expand_to_path(path);
	mp_AnimalTree->scroll_to_row(path);
	mp_AnimalTree->get_selection()->select(path);
	mp_Notebook->set_current_page(0);
}

void uiAnalysis::on_hovered_on_point(const double &x, const double &y, const std::string &name, const std::string &/*data*/)
{
	// Update status bar
	std::stringstream ss;
	ss << "Data Point: (" << x << "," << y << ") " << name;
	mp_StatusBar->push(ss.str());
}

void uiAnalysis::on_moved_off_point()
{
	mp_StatusBar->pop();
}

void uiAnalysis::on_options_clicked()
{
	mp_plot->clear();
	if (!plugins.empty()) {
		runScript(true,false,plugins[tbPlugins->get_active_row_number()].first);
	}
}

void uiAnalysis::on_action_clicked()
{
	mp_plot->clear();
	if (!plugins.empty()) {
		runScript(false,true,plugins[tbPlugins->get_active_row_number()].first);
	}
}

void uiAnalysis::on_open_clicked()
{
	Gtk::FileChooserDialog dialog("Select the analysis script",
				      Gtk::FILE_CHOOSER_ACTION_OPEN);

	dialog.set_transient_for(*mp_parent);

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
	runScript(false,false);
}

void uiAnalysis::on_plugin_changed()
{
	mp_plot->clear();
	if (plugins[tbPlugins->get_active_row_number()].first != "")
	{
		// Don't automatically run custom scripts
		runPlugin();
	}
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
	forceAbsBegin = (float)begin;
	forceAbsEnd = (float)end;
}

void uiAnalysis::forceSpikesRel(double begin, double end)
{
	forceRelBegin = (float)begin;
	forceRelEnd = (float)end;
}

void uiAnalysis::runPlugin()
{
	if (!plugins.empty()) {
		runScript(false,false,plugins[tbPlugins->get_active_row_number()].first);
	}
}

void uiAnalysis::runScript(bool showAdvanced, bool runAction, const Glib::ustring &plugin)
{
	if (m_filename == "" && plugin == "") return;


	tvOutput->add_modal_grab();
	while (Gtk::Main::events_pending()) {
    	Gtk::Main::iteration();
	}

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

	bp::object main_module(bp::handle<>(bp::borrowed(PyImport_AddModule("__main__"))));
	bp::object main_namespace = main_module.attr("__dict__");

if (!uiAnalysis::setupPython)
{
	main_namespace["pySpikeDB"] = class_<pySpikeDB>("pySpikeDB")
		.def("write", &pySpikeDB::print)
		.def("addOptionCheckbox", &pySpikeDB::addOptionCheckbox)
		.def("addOptionRadio", &pySpikeDB::addOptionRadio)
		.def("addOptionNumber", &pySpikeDB::addOptionNumber)
		.def("addRuler", &pySpikeDB::addRuler)
		.def("getCells", &pySpikeDB::getCells)
		.def("getFiles", &pySpikeDB::getFiles)
		.def("getOptions", &pySpikeDB::getOptions)
		.def("getFilesSingleCell", &pySpikeDB::getFilesSingleCell)
		.def("mean", &pySpikeDB::mean)
		.def("stddev", &pySpikeDB::stddev)
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
		.def("plotLine", &pySpikeDB::plotLine)
		.def("setPointNames", &pySpikeDB::setPointNames)
		.def("setPointData", &pySpikeDB::setPointData)
		.def("updateProgress", &pySpikeDB::updateProgress)
		.def("enableActionButton", &pySpikeDB::enableActionButton);
	uiAnalysis::setupPython = true;
}

	pySpikeDB _pySpikeDB(db, mp_FileDetailsTree, mp_AnimalTree,mp_AnimalColumns,mp_plot, mrp_tbOutput, mp_pbStatus);


	_pySpikeDB.forceSpikesAbs(forceAbsBegin, forceAbsEnd);
	_pySpikeDB.forceSpikesRel(forceRelBegin, forceRelEnd);

	main_namespace["SpikeDB"] = bp::ptr(&_pySpikeDB);
	main_namespace["SpikeDB"].attr("__dict__")["VARYING"] = VARYING_STIMULUS;
	main_namespace["SpikeDB"].attr("__dict__")["NOPOINT"] = EasyPlotmm::NOPOINT;

	PyRun_SimpleString("import sys");
	PyRun_SimpleString("sys.stdout = SpikeDB");
	PyRun_SimpleString("sys.stderr = SpikeDB");

	if (!compact) addOutput("*** Running Analysis Plugin ***\n\n");

	// Incase our new script doesn't have one of the functions, we'll give default blank ones.
	PyRun_SimpleString("def SpikeDBAdvanced(): return");
	PyRun_SimpleString("def SpikeDBRun(): return");
	PyRun_SimpleString("def SpikeDBAction(): return");

	// Open the file and run the code
	char *filename;
	char mode[] = "r";
	if (plugin == "") {
		filename = const_cast<char *>(m_filename.c_str());
	} else {
		filename = const_cast<char *>(plugin.c_str());
	}
	PyObject* PyFileObject = PyFile_FromString(filename, mode);
	PyRun_SimpleFile(PyFile_AsFile(PyFileObject), filename);

	// Run the SpikeDBAdvanced function
	PyRun_SimpleString("SpikeDBAdvanced()");
	_pySpikeDB.addOptionCheckbox("showErrorBars", "Show Error Bars", false);

	// If we have a previously saved settings, overide the defaults with them now
	std::vector<boost::shared_ptr<pySpikeDB::Option> >::iterator optIter;
	for (optIter = _pySpikeDB.options.begin(); optIter != _pySpikeDB.options.end(); optIter++) {
		if (savedNumberOptions.find(std::pair<std::string,std::string>(filename, (*optIter)->name)) != savedNumberOptions.end()) {
			if ((*optIter)->type == pySpikeDB::Option::NUMBER) {
				pySpikeDB::numberOption* opt = static_cast<pySpikeDB::numberOption *>((*optIter).get());
				opt->setValue(savedNumberOptions.find((std::pair<std::string,std::string>(filename, opt->name)))->second );
			}
		}
		if (savedCheckboxOptions.find(std::pair<std::string,std::string>(filename, (*optIter)->name)) != savedCheckboxOptions.end()) {
			if ((*optIter)->type == pySpikeDB::Option::CHECKBOX) {
				pySpikeDB::checkboxOption* opt = static_cast<pySpikeDB::checkboxOption *>((*optIter).get());
				opt->setValue( savedCheckboxOptions.find((std::pair<std::string,std::string>(filename, opt->name)))->second );
			}
		}
		if (savedRadioOptions.find(std::pair<std::string,std::string>(filename, (*optIter)->name)) != savedRadioOptions.end()) {
			if ((*optIter)->type == pySpikeDB::Option::RADIO) {
				pySpikeDB::radioOption* opt = static_cast<pySpikeDB::radioOption *>((*optIter).get());
				opt->setValue(savedRadioOptions.find((std::pair<std::string,std::string>(filename, opt->name)))->second );
			}
		}
	}

	// Enable the button if the script called SpikeDB().enableActionButton()
	tbAction->set_sensitive(_pySpikeDB.actionButton);

	if (showAdvanced) {

		Gtk::Dialog dialog("Script Options", *mp_parent, true);

		/**
		 * Create the custom widgets here
		 */
		Gtk::VBox vbox;

		Gtk::CheckButton *saveSettings = Gtk::manage( new Gtk::CheckButton("Save Settings For This Session") );
		saveSettings->set_active(true); // Save options by default
		Gtk::HSeparator *sep = Gtk::manage( new Gtk::HSeparator() );
		vbox.pack_end(*saveSettings);
		saveSettings->show();
		vbox.pack_end(*sep);
		sep->show();

		std::vector< std::pair<Glib::ustring,Gtk::Entry*> > numberOptions;
		std::vector< std::pair<Glib::ustring,std::vector<Gtk::RadioButton*> > > radioOptions;
		std::vector< std::pair<Glib::ustring,Gtk::CheckButton*> > cbOptions;

		for (optIter = _pySpikeDB.options.begin(); optIter != _pySpikeDB.options.end(); optIter++) {
			if ((*optIter)->type == pySpikeDB::Option::NUMBER) {
				pySpikeDB::numberOption* opt = static_cast<pySpikeDB::numberOption *>((*optIter).get());
				Gtk::HBox *hbox = Gtk::manage(new Gtk::HBox() );
				Gtk::Label *label = Gtk::manage(new Gtk::Label(opt->description) );
				hbox->pack_start(*label,false,true);
				Gtk::Entry* numberOption = Gtk::manage( new Gtk::Entry() );
				char numBuffer[10];
				sprintf(numBuffer, "%.3f", opt->getValue());
				numberOption->set_text(numBuffer);
				numberOption->set_width_chars(6);
				numberOptions.push_back(std::make_pair(opt->name,numberOption));
				hbox->pack_start(*numberOption, false, true);
				vbox.pack_start(*hbox, false, true);
			}
			if ((*optIter)->type == pySpikeDB::Option::CHECKBOX) {
				pySpikeDB::checkboxOption* opt = static_cast<pySpikeDB::checkboxOption *>((*optIter).get());
				Gtk::CheckButton* cbOption = Gtk::manage( new Gtk::CheckButton(opt->description) );
				cbOption->set_active(opt->getValue());
				cbOptions.push_back(std::make_pair(opt->name,cbOption));
				vbox.pack_start(*cbOption, false, true);
			}
			if ((*optIter)->type == pySpikeDB::Option::RADIO) {
				pySpikeDB::radioOption* opt = static_cast<pySpikeDB::radioOption *>((*optIter).get());
				Gtk::RadioButtonGroup group;
				unsigned int count = 0;
				std::vector<Gtk::RadioButton*> buttons;
				for (std::vector<std::string>::iterator it = opt->getItems()->begin(); it != opt->getItems()->end(); it++) {
					Gtk::RadioButton* radioOption = Gtk::manage( new Gtk::RadioButton(group, *it) );
					if (count == opt->getValue()) radioOption->set_active(true);
					buttons.push_back(radioOption);
					vbox.pack_start(*radioOption, false, true);
					count++;
				}
				radioOptions.push_back(make_pair(opt->name, buttons));
			}
			if ((*optIter)->type == pySpikeDB::Option::RULER) {
				vbox.pack_start(*Gtk::manage( new Gtk::HSeparator() ), false, true);
			}
		}

		dialog.get_vbox()->pack_start(vbox);
		dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
		dialog.add_button("Run Script", Gtk::RESPONSE_OK);
		dialog.show_all_children();

		if (dialog.run() == Gtk::RESPONSE_OK) {
			dialog.hide();
			// Update the option values
			for(unsigned int i = 0; i < cbOptions.size(); ++i) {
				for (optIter = _pySpikeDB.options.begin(); optIter != _pySpikeDB.options.end(); optIter++) {
					if ((*optIter)->name == cbOptions.at(i).first && (*optIter)->type == pySpikeDB::Option::CHECKBOX) {
						pySpikeDB::checkboxOption* opt = static_cast<pySpikeDB::checkboxOption *>((*optIter).get());
						opt->setValue(cbOptions.at(i).second->get_active());
						if (saveSettings->get_active()) {
							savedCheckboxOptions[std::pair<std::string,std::string>(filename,(*optIter)->name)] = opt->getValue();
						}
					}
				}
			}
			for(unsigned int i = 0; i < numberOptions.size(); ++i) {
				for (optIter = _pySpikeDB.options.begin(); optIter != _pySpikeDB.options.end(); optIter++) {
					if ((*optIter)->name == numberOptions.at(i).first && (*optIter)->type == pySpikeDB::Option::NUMBER) {
						pySpikeDB::numberOption* opt = static_cast<pySpikeDB::numberOption *>((*optIter).get());
						opt->setValue(Glib::Ascii::strtod(numberOptions.at(i).second->get_text()));
						if (saveSettings->get_active()) {
							savedNumberOptions[std::pair<std::string,std::string>(filename,(*optIter)->name)] = opt->getValue();
						}
					}
				}
			}
			for(unsigned int i = 0; i < radioOptions.size(); ++i) {
				for (optIter = _pySpikeDB.options.begin(); optIter != _pySpikeDB.options.end(); optIter++) {
					if ((*optIter)->name == radioOptions.at(i).first && (*optIter)->type == pySpikeDB::Option::RADIO) {
						pySpikeDB::radioOption* opt = static_cast<pySpikeDB::radioOption *>((*optIter).get());
						unsigned int value;
						for (value = 0; value < radioOptions.at(i).second.size(); ++value) {
							if (radioOptions.at(i).second.at(value)->get_active()) {
								break;
							}
						}
						opt->setValue(value);
						if (saveSettings->get_active()) {
							savedRadioOptions[std::pair<std::string,std::string>(filename,(*optIter)->name)] = value;
						}
					}
				}
			}

			if (!saveSettings->get_active()) {
				// Delete saved settings
				for (optIter = _pySpikeDB.options.begin(); optIter != _pySpikeDB.options.end(); optIter++) {
					if ((*optIter)->type == pySpikeDB::Option::NUMBER) {
						savedNumberOptions.erase((std::pair<std::string,std::string>(filename, (*optIter)->name)));
					}
					if ((*optIter)->type == pySpikeDB::Option::CHECKBOX) {
						savedCheckboxOptions.erase((std::pair<std::string,std::string>(filename, (*optIter)->name)));
					}
				}
			}
			PyRun_SimpleString("SpikeDBRun()");
		}

	} else {
		if (runAction) {
			// Run the action item
			PyRun_SimpleString("SpikeDBAction()");
		} else {
			// Run the SpikeDBRun function
			PyRun_SimpleString("SpikeDBRun()");
		}
	}

	//Py_Finalize();
	if (!compact) addOutput("\n*** Analysis Plugin Completed ***");
	tvOutput->remove_modal_grab();
}

void uiAnalysis::print(const std::string &s)
{
	addOutput(s);
}

EasyPlotmm* uiAnalysis::getPlot()
{
	return mp_plot;
}

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

#include "gui.h"
#include "pySpikeDB.h"

namespace bp=boost::python;
using namespace bp;

pySpikeDB::pySpikeDB() {}

pySpikeDB::pySpikeDB(sqlite3** db, uiFileDetailsTreeView* fileDetailsTree, Gtk::TreeView* animalTree, AnimalColumns* animalColumns, EasyPlotmm *plot, Glib::RefPtr<Gtk::TextBuffer> tbOutput, Gtk::ProgressBar* pbStatus)
{
	this->db = db;
	this->mp_FileDetailsTree = fileDetailsTree;
	this->mp_AnimalTree = animalTree;
	this->mp_AnimalColumns = animalColumns;
	this->mp_plot = plot;
	this->mrp_tbOutput = tbOutput;
	this->mp_pbStatus = pbStatus;
	this->reset();
}

void pySpikeDB::addOptionCheckbox(const std::string &name, const std::string &description, bool def)
{
	boost::shared_ptr<checkboxOption> opt(new checkboxOption(def));
	opt->name = name;
	opt->description = description;
	options.push_back(opt);
}

void pySpikeDB::addOptionRadio(const std::string &name, boost::python::list &items, const std::string &description, int def)
{
	boost::shared_ptr<radioOption> opt(new radioOption(def));
	opt->name = name;
	opt->description = description;
	opt->setItems(list2vec<std::string>(items));
	options.push_back(opt);
}

void pySpikeDB::addOptionNumber(const std::string &name, const std::string &description, double def)
{
	boost::shared_ptr<numberOption> opt(new numberOption(def));
	opt->name = name;
	opt->description = description;
	options.push_back(opt);
}

void pySpikeDB::addRuler()
{
	boost::shared_ptr<rulerOption> opt(new rulerOption());
	options.push_back(opt);
}

void pySpikeDB::updateProgress(const float &val)
{
	if (mp_pbStatus) {
		float limitedVal = val;
		if (val < 0) limitedVal = 0;
		if (val > 1) limitedVal = 1;
		Glib::ustring text;
		char buffer[5];
		sprintf(buffer, "%d%%", (int)(limitedVal*100));
		mp_pbStatus->set_fraction(limitedVal);
		mp_pbStatus->set_text(buffer);
		while (Gtk::Main::events_pending()) {
			Gtk::Main::iteration();
		}
	}
}

void pySpikeDB::forceSpikesAbs(const float &begin, const float &end)
{
	forceAbsBegin = begin;
	forceAbsEnd = end;
	filterAbsBegin = -1;
	filterAbsEnd = -1;
	filterRelBegin = -1;
	filterRelEnd = -1;
}

void pySpikeDB::forceSpikesRel(const float &begin, const float &end)
{
	forceRelBegin = begin;
	forceRelEnd = end;
	filterAbsBegin = -1;
	filterAbsEnd = -1;
	filterRelBegin = -1;
	filterRelEnd = -1;
}

void pySpikeDB::filterSpikesAbs(const float &begin, const float &end)
{
	filterAbsBegin = begin;
	filterAbsEnd = end;
	filterRelBegin = -1;
	filterRelEnd = -1;
}

void pySpikeDB::filterSpikesRel(const float &begin, const float &end)
{
	filterAbsBegin = -1;
	filterAbsEnd = -1;
	filterRelBegin = begin;
	filterRelEnd = end;
}

void pySpikeDB::print(const std::string &s)
{
	mrp_tbOutput->insert(mrp_tbOutput->end(), s);
	while (Gtk::Main::events_pending()) {
    	Gtk::Main::iteration();
	}
}

void pySpikeDB::reset()
{
	plotClear();
	filterAbsBegin=filterAbsEnd=-1;
	filterRelBegin=filterRelEnd=-1;
	forceAbsBegin=forceAbsEnd=-1;
	forceRelBegin=forceRelEnd=-1;
	xmin=xmax=ymin=ymax=EasyPlotmm::AUTOMATIC;
	options.clear();
	actionButton = false;
	mp_pbStatus->set_fraction(0);
	mp_pbStatus->set_text("");

}

void pySpikeDB::enableActionButton()
{
	actionButton = true;
}

bp::object pySpikeDB::getOptions()
{
	bp::dict opts;

	std::vector<boost::shared_ptr<pySpikeDB::Option> >::iterator optIter;
	for (optIter = options.begin(); optIter != options.end(); optIter++) {
		if ((*optIter)->type == pySpikeDB::Option::NUMBER) {
			pySpikeDB::numberOption* opt = static_cast<pySpikeDB::numberOption *>((*optIter).get());
			opts[opt->name] = opt->getValue();
		}
		if ((*optIter)->type == pySpikeDB::Option::CHECKBOX) {
			pySpikeDB::checkboxOption* opt = static_cast<pySpikeDB::checkboxOption *>((*optIter).get());
			opts[opt->name] = opt->getValue();
		}
		if ((*optIter)->type == pySpikeDB::Option::RADIO) {
			pySpikeDB::radioOption* opt = static_cast<pySpikeDB::radioOption *>((*optIter).get());
			opts[opt->name] = opt->getValue();
		}
	}
	return opts;
}
bp::object pySpikeDB::getCells()
{
	bp::list list;

	/*
	Keys: AnimalID: string
		  CellID: number
		  CarFreq: number (in Hz)
		  Threshold: number (in dB SPL)
		  Depth: number (in um)
	*/

	std::set<CellID> uniqueCells;

	Gtk::TreeIter iter;
	Gtk::TreeIter animalIter;
	Gtk::TreeIter cellIter;

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
			bp::dict cell;
			cell["AnimalID"] = row.get_value(mp_FileDetailsTree->m_Columns.m_col_animalID).c_str();
			cell["CellID"] = row.get_value(mp_FileDetailsTree->m_Columns.m_col_cellID);
			cell["CarFreq"] = row.get_value(mp_FileDetailsTree->m_Columns.m_col_carfreq);
			cell["Threshold"] = row.get_value(mp_FileDetailsTree->m_Columns.m_col_threshold);
			cell["ThresholdAttn"] = row.get_value(mp_FileDetailsTree->m_Columns.m_col_threshold_attn);
			cell["Depth"] = row.get_value(mp_FileDetailsTree->m_Columns.m_col_depth);
			cell["Location"] = row.get_value(mp_FileDetailsTree->m_Columns.m_col_location).c_str();
			cell["TreePath"] = "";

			/* Search the Animal Tree for this cell */
			/* We know the top level is "All" so start at the second level */
			for (animalIter = mp_AnimalTree->get_model()->children().begin()->children().begin();
				 animalIter != mp_AnimalTree->get_model()->children().begin()->children().end();
				 animalIter++)
			{
				if ((*animalIter).get_value(mp_AnimalColumns->m_col_name) == row.get_value(mp_FileDetailsTree->m_Columns.m_col_animalID))
				{
					for (cellIter = animalIter->children().begin();
						 cellIter != animalIter->children().end();
						 cellIter++)
					{
						if(Glib::Ascii::strtod((*cellIter).get_value(mp_AnimalColumns->m_col_name)) == (double)row.get_value(mp_FileDetailsTree->m_Columns.m_col_cellID))
						{
							cell["TreePath"] = mp_AnimalTree->get_model()->get_path(cellIter).to_string().c_str();
						}
					}
				}
			}
			/* End Search */

			// Get the tags
			bp::list tags;
			sqlite3_stmt *stmt = 0;
			const char query[] = "SELECT tag FROM tags WHERE animalID=? AND cellID=? AND fileID IS NULL";
			sqlite3_prepare_v2(*db, query, strlen(query), &stmt, NULL);
			sqlite3_bind_text(stmt, 1,row.get_value(mp_FileDetailsTree->m_Columns.m_col_animalID).c_str(), -1, SQLITE_TRANSIENT);
			sqlite3_bind_int(stmt, 2, row.get_value(mp_FileDetailsTree->m_Columns.m_col_cellID));
			while (sqlite3_step(stmt) == SQLITE_ROW) {
				tags.append(Glib::ustring((char*)sqlite3_column_text(stmt, 0)).c_str());
			}
			sqlite3_finalize(stmt);
			cell["tags"] = tags;

			list.append(cell);
		}
	}

	return list;
}

bp::object pySpikeDB::getFile(const Gtk::TreeModel::iterator& iter)
{

	// Ensure forced filter overrides anything set in script
	if (forceAbsBegin != -1) {
		filterAbsBegin = forceAbsBegin;
		filterAbsEnd = forceAbsEnd;
	}
	if (forceRelBegin != -1) {
		filterRelBegin = forceRelBegin;
		filterRelEnd = forceRelEnd;
	}

	bp::dict file;
	sqlite3_stmt *stmt = 0;
	const char query[] = "SELECT header, spikes, sweeporder FROM files WHERE animalID=? AND cellID=? AND fileID=?";
	sqlite3_prepare_v2(*db, query, strlen(query), &stmt, NULL);

	int r;
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

		file["AnimalID"] = row.get_value(mp_FileDetailsTree->m_Columns.m_col_animalID).c_str();
		file["CellID"] = row.get_value(mp_FileDetailsTree->m_Columns.m_col_cellID);
		file["FileID"] = row.get_value(mp_FileDetailsTree->m_Columns.m_col_filenum);
		file["datetime"] = sd.iso8601(sd.m_head.cDateTime).c_str();
		file["location"] = row.get_value(mp_FileDetailsTree->m_Columns.m_col_location).c_str();
		std::string strSweepOrder = ((char*)sqlite3_column_text(stmt,2) == NULL) ? "" : (char*)sqlite3_column_text(stmt, 2);
		bp::list SweepOrder;
		std::vector<std::string> sweeps;
		boost::split(sweeps, strSweepOrder, boost::is_any_of(","));
		for (std::vector<std::string>::iterator it = sweeps.begin(); it != sweeps.end(); it++) {
			SweepOrder.append(atof((*it).c_str()));
		}
		file["sweeporder"] = SweepOrder;

		// Get the tags
		bp::list tags;
		sqlite3_stmt *stmt = 0;
		const char query[] = "SELECT tag FROM tags WHERE animalID=? AND cellID=? AND fileID=?";
		sqlite3_prepare_v2(*db, query, strlen(query), &stmt, NULL);
		sqlite3_bind_text(stmt, 1,row.get_value(mp_FileDetailsTree->m_Columns.m_col_animalID).c_str(), -1, SQLITE_TRANSIENT);
		sqlite3_bind_int(stmt, 2, row.get_value(mp_FileDetailsTree->m_Columns.m_col_cellID));
		sqlite3_bind_int(stmt, 3, row.get_value(mp_FileDetailsTree->m_Columns.m_col_filenum));
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			tags.append(Glib::ustring((char*)sqlite3_column_text(stmt, 0)).c_str());
		}
		sqlite3_finalize(stmt);
		file["tags"] = tags;

		file["speakertype"] = row.get_value(mp_FileDetailsTree->m_Columns.m_col_speakertype).c_str();
		file["azimuth"] = row.get_value(mp_FileDetailsTree->m_Columns.m_col_azimuth).c_str();
		file["elevation"] = row.get_value(mp_FileDetailsTree->m_Columns.m_col_elevation).c_str();

		bp::dict type;
		if (sd.m_head.nOnCh1 == 0) { type[1] = "";} 
		else { type[1] = sd.type(1); }
		if (sd.m_head.nOnCh2 == 0) { type[2] = "";} 
		else { type[2] = sd.type(2); }
		file["type"] = type;

		bp::dict dur;
		if (sd.m_head.nOnCh1 == 0) { dur[1] = "";} 
		else { 
			if (sd.duration(1, 0) == sd.duration(1, 1)) {
				dur[1] = sd.duration(1,0);
			} else {
				dur[1] = VARYING_STIMULUS;
			}
		}
		if (sd.m_head.nOnCh2 == 0) { dur[2] = "";} 
		else { 
			if (sd.duration(2, 0) == sd.duration(2, 1)) {
				dur[2] = sd.duration(2,0);
			} else {
				dur[2] = VARYING_STIMULUS;
			}
		}
		file["duration"] = dur;

		bp::dict attn;
		if (sd.m_head.nOnCh1 == 0) { attn[1] = "";} 
		else { 
			if (sd.attenuation(1, 0) == sd.attenuation(1, 1)) {
				attn[1] = sd.attenuation(1,0);
			} else {
				attn[1] = VARYING_STIMULUS;
			}
		}
		if (sd.m_head.nOnCh2 == 0) { attn[2] = "";} 
		else { 
			if (sd.attenuation(2, 0) == sd.attenuation(2, 1)) {
				attn[2] = sd.attenuation(2,0);
			} else {
				attn[2] = VARYING_STIMULUS;
			}
		}
		file["attenuation"] = attn;

		bp::dict freq;
		if (sd.m_head.nOnCh1 == 0) { freq[1] = "";} 
		else { 
			if (sd.frequency(1, 0) == sd.frequency(1, 1)) {
				freq[1] = sd.frequency(1,0);
			} else {
				freq[1] = VARYING_STIMULUS;
			}
		}
		if (sd.m_head.nOnCh2 == 0) { freq[2] = "";} 
		else { 
			if (sd.frequency(2, 0) == sd.frequency(2, 1)) {
				freq[2] = sd.frequency(2,0);
			} else {
				freq[2] = VARYING_STIMULUS;
			}
		}
		file["frequency"] = freq;


        /* NEW IN 1.9 */
		bp::dict freqdev;
		if (sd.m_head.nOnCh1 == 0) { freqdev[1] = "";} 
		else { 
			if (sd.freqdev(1, 0) == sd.freqdev(1, 1)) {
				freqdev[1] = sd.freqdev(1,0);
			} else {
				freqdev[1] = VARYING_STIMULUS;
			}
		}
		if (sd.m_head.nOnCh2 == 0) { freqdev[2] = "";} 
		else { 
			if (sd.freqdev(2, 0) == sd.freqdev(2, 1)) {
				freqdev[2] = sd.freqdev(2,0);
			} else {
				freqdev[2] = VARYING_STIMULUS;
			}
		}
		file["freqdev"] = freqdev;


		bp::dict begin;
		if (sd.m_head.nOnCh1 == 0) { begin[1] = "";} 
		else { 
			if (sd.begin(1, 0) == sd.begin(1, 1)) {
				begin[1] = sd.begin(1,0);
			} else {
				begin[1] = VARYING_STIMULUS;
			}
		}
		if (sd.m_head.nOnCh2 == 0) { begin[2] = "";} 
		else { 
			if (sd.begin(2, 0) == sd.begin(2, 1)) {
				begin[2] = sd.begin(2,0);
			} else {
				begin[2] = VARYING_STIMULUS;
			}
		}
		file["begin"] = begin;

		file["xvar"] = sd.xVariable();

		bp::list trials;
		for (int i = 0; i < sd.m_head.nSweeps; ++i) {
			bp::dict trial;
			trial["xvalue"] = sd.xvalue(i);

			bp::dict tbegin;
			tbegin[1] = sd.begin(1,i);
			tbegin[2] = sd.begin(2,i);
			trial["begin"] = tbegin;

			bp::dict tduration;
			tduration[1] = sd.duration(1,i);
			tduration[2] = sd.duration(2,i);
			trial["duration"] = tduration;

			bp::dict tfrequency;
			tfrequency[1] = sd.frequency(1,i);
			tfrequency[2] = sd.frequency(2,i);
			trial["frequency"] = tfrequency;

			bp::dict tattenuation;
			tattenuation[1] = sd.attenuation(1,i);
			tattenuation[2] = sd.attenuation(2,i);
			trial["attenuation"] = tattenuation;

			bp::list spikes;
			for (int p = 0; p < sd.m_head.nPasses; ++p) {
				bp::list pass;
				for (unsigned int s = 0; s < sd.m_spikeArray.size(); ++s) {
					// Spike sweeps are 1 based but here we are 0 based
					if (sd.m_spikeArray[s].nSweep == i + 1 && sd.m_spikeArray[s].nPass == p+1) {

						if ( filterAbsBegin == -1 && filterRelBegin == -1 )
						{
							pass.append(sd.m_spikeArray[s].fTime);
						} else if (filterAbsBegin != -1 && filterAbsEnd > filterAbsBegin &&
								   sd.m_spikeArray[s].fTime >= filterAbsBegin &&
								   sd.m_spikeArray[s].fTime <= filterAbsEnd)
						{
							pass.append(sd.m_spikeArray[s].fTime);
						} else if (filterRelBegin != -1 && filterRelEnd > filterRelBegin)
						{
							float st = sd.m_spikeArray[s].fTime;
							if ((sd.m_head.nOnCh1 == 1 && st >= sd.begin(1,i)+filterRelBegin && st <= sd.begin(1,i)+sd.duration(1,i)+filterRelEnd) ||
								(sd.m_head.nOnCh2 == 1 && st >= sd.begin(2,i)+filterRelBegin && st <= sd.begin(2,i)+sd.duration(2,i)+filterRelEnd))
							{
								pass.append(st);
							}
						}
					}
				}
				spikes.append(pass);
			}
			trial["passes"] = spikes;
			trials.append(trial);
		}
		file["trials"] = trials;
	}
	sqlite3_finalize(stmt);
	return file;
}

bp::object pySpikeDB::getFiles(bool selOnly)
{
	mp_AnimalTree->set_sensitive(false);
	mp_FileDetailsTree->set_sensitive(false);

	bp::list list;
	if (selOnly) {
		std::vector<Gtk::TreeModel::Path> rows = mp_FileDetailsTree->treeSelection()->get_selected_rows();
		for(unsigned int i = 0; i < rows.size(); i++)
		{
			Gtk::TreeModel::iterator iter = mp_FileDetailsTree->get_model()->get_iter(rows[i]);
			list.append(getFile(iter));
		}

	} else {
		Gtk::TreeIter iter;
		print("Loading files...");
		unsigned int next = 10;
		float count = 0;
		unsigned int total = mp_FileDetailsTree->mrp_ListStore->children().size();
		for (iter = mp_FileDetailsTree->mrp_ListStore->children().begin(); 
			 iter != mp_FileDetailsTree->mrp_ListStore->children().end(); 
			 iter++)
		{
			if (iter->get_value(mp_FileDetailsTree->m_Columns.m_col_bad) == 0)
			{
				// Don't add bad cells
				list.append(getFile(iter));
			}
			count++;
			if (100*(count/total) >= next) 
			{
				std::stringstream ss;
				ss << "..." << next << "%";
				print(ss.str());
				next += 10;
			}
		}
		print("\n");
	}

	mp_AnimalTree->set_sensitive(true);
	mp_FileDetailsTree->set_sensitive(true);

	return list;
}

bp::object pySpikeDB::getFilesSingleCell(const std::string &animalID, const int &cellID)
{
	mp_AnimalTree->set_sensitive(false);
	mp_FileDetailsTree->set_sensitive(false);

	bp::list list;
	Gtk::TreeIter iter;
	for (iter = mp_FileDetailsTree->mrp_ListStore->children().begin(); 
		 iter != mp_FileDetailsTree->mrp_ListStore->children().end(); 
		 iter++)
	{
		Gtk::TreeModel::Row row = *iter;
		if (row.get_value(mp_FileDetailsTree->m_Columns.m_col_animalID) == animalID) {
			if (row.get_value(mp_FileDetailsTree->m_Columns.m_col_cellID) == cellID ) {
				if (iter->get_value(mp_FileDetailsTree->m_Columns.m_col_bad) == 0)
				{
					// Don't add bad cells
					list.append(getFile(iter));
				}
			}
		}
	}

	mp_AnimalTree->set_sensitive(true);
	mp_FileDetailsTree->set_sensitive(true);
	return list;
}

void pySpikeDB::plotSetRGBA(const float &r, const float &g, const float &b, const float &a)
{
	m_plotPen.color.r = r;
	m_plotPen.color.g = g;
	m_plotPen.color.b = b;
	m_plotPen.color.a = a;
}

void pySpikeDB::plotSetPointSize(const float &s)
{
	m_plotPen.pointsize = s;
}

void pySpikeDB::plotSetLineWidth(const float &s)
{
	m_plotPen.linewidth = s;
}

void pySpikeDB::plotClear()
{
	mp_plot->clear();
	m_plotPen.color.r = 0;
	m_plotPen.color.g = 0;
	m_plotPen.color.b = 0;
	m_plotPen.color.a = 1;
	m_plotPen.linewidth = 2.0;
	m_plotPen.filled = true;
	m_plotPen.pointsize = 8.0;
}

void pySpikeDB::plotXLabel(const std::string &s)
{
	mp_plot->xname(s);
}

void pySpikeDB::plotYLabel(const std::string &s)
{
	mp_plot->yname(s);
}

void pySpikeDB::plotYMin(const float &v)
{
	ymin = v;
	mp_plot->axes(xmin,xmax,v,ymax);
}

void pySpikeDB::plotYMax(const float &v)
{
	ymax = v;
	mp_plot->axes(xmin,xmax,ymin,v);
}

void pySpikeDB::plotXMin(const float &v)
{
	xmin = v;
	mp_plot->axes(v,xmax,ymin,ymax);
}

void pySpikeDB::plotXMax(const float &v)
{
	xmax = v;
	mp_plot->axes(xmin,v,ymin,ymax);
}

void pySpikeDB::plotHist(boost::python::list &/*x*/, boost::python::list &/*y*/, boost::python::list &/*err*/)
{
	/*
	std::vector<double> x, y, err;
	if (bp::len(pyX) != bp::len(pyY))
	{
		std::cerr << "Error: Plot X and Y do not match in length." << std::endl;
		return;
	}
	if (bp::len(pyX) == 0)
	{
		std::cerr << "Error: No plot data." << std::endl;
		return;
	}
	x = list2vec<double>(pyX);
	y = list2vec<double>(pyY);
	err = list2vec<double>(pyErr);

	if (bp::len(pyX) == bp::len(pyErr) && showErr)
	{
		mp_plot->plotHist(x, y, err, m_plotPen);
	}
	else
	{
		mp_plot->plotHist(x, y, m_plotPen);
	}
	*/
}

void pySpikeDB::plotLine(bp::list &pyX, bp::list &pyY, bp::list &pyErr)
{
	std::vector<double> x, y, err;
	if (bp::len(pyX) != bp::len(pyY))
	{
		std::cerr << "Error: Plot X and Y do not match in length." << std::endl;
		return;
	}
	if (bp::len(pyX) == 0)
	{
		std::cerr << "Error: No plot data." << std::endl;
		return;
	}
	x = list2vec<double>(pyX);
	y = list2vec<double>(pyY);
	err = list2vec<double>(pyErr);

	checkboxOption* showErrorBars = static_cast<checkboxOption*>(findOptionByName("showErrorBars"));
	if (bp::len(pyX) == bp::len(pyErr) && showErrorBars && showErrorBars->getValue())
	{
		mp_plot->plot(x, y, err, m_plotPen);
	}
	else
	{
		mp_plot->plot(x, y, m_plotPen);
	}
}

void pySpikeDB::setPointNames(bp::list &names)
{
	std::vector<std::string> n = list2vec<std::string>(names);
	mp_plot->setPointNames(n);
}

void pySpikeDB::setPointData(boost::python::list &data)
{
	std::vector<std::string> d = list2vec<std::string>(data);
	mp_plot->setPointData(d);
}

template<typename T>
std::vector<T> pySpikeDB::list2vec(bp::list &l)
{
	std::vector<T> r;
	for (int i = 0; i < bp::len(l); ++i)
	{
		r.push_back(extract<T>(l[i]));
	}
	return r;
}

double pySpikeDB::mean(boost::python::list &v)
{
	int N = bp::len(v);
	int rN = 0;
	double r = 0;
	for (int i = 0; i < N; ++i)
	{
		double e = extract<double>(v[i]);
		if (e != EasyPlotmm::NOPOINT)
		{
			r += e;
			rN++;
		}
	}
	if (rN > 0) r /= rN;
	return r;
}

double pySpikeDB::stddev(boost::python::list &v)
{
	int N = bp::len(v);
	if (N <= 1) return 0;
	int rN = 0;
	double m = mean(v);
	double r = 0;
	for (int i = 0; i < N; ++i)
	{
		double e = extract<double>(v[i])-m;
		if (e != EasyPlotmm::NOPOINT)
		{
			r += e*e;
			rN++;
		}
	}
	if (rN <= 1) return 0;
	return sqrt(r/(rN-1));
}

pySpikeDB::Option* pySpikeDB::findOptionByName(const Glib::ustring &name) {

	std::vector<boost::shared_ptr<Option> >::iterator it;
	for (it = options.begin(); it != options.end(); it++) {
		if ((*it).get()->name == name) {
			return (*it).get();
		}
	}
	return NULL;
}

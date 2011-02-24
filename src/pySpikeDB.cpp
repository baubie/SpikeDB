#include "pySpikeDB.h"

namespace bp=boost::python;
using namespace bp;

pySpikeDB::pySpikeDB() {}


pySpikeDB::pySpikeDB(sqlite3** db,uiFileDetailsTreeView* fileDetailsTree, EasyPlotmm* plot)
{
	this->db = db;
	this->mp_FileDetailsTree = fileDetailsTree;
	this->mp_plot = plot;
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
			cell["Depth"] = row.get_value(mp_FileDetailsTree->m_Columns.m_col_depth);
			list.append(cell);
		}
	}

	return list;
}

bp::object pySpikeDB::getFile(const Gtk::TreeModel::iterator& iter)
{
	bp::dict file;
	sqlite3_stmt *stmt = 0;
	const char query[] = "SELECT header, spikes FROM files WHERE animalID=? AND cellID=? AND fileID=?";
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
				dur[1] = "Var";
			}
		}
		if (sd.m_head.nOnCh2 == 0) { dur[2] = "";} 
		else { 
			if (sd.duration(2, 0) == sd.duration(2, 1)) {
				dur[2] = sd.duration(2,0);
			} else {
				dur[2] = "Var";
			}
		}
		file["duration"] = dur;

		bp::dict attn;
		if (sd.m_head.nOnCh1 == 0) { attn[1] = "";} 
		else { 
			if (sd.attenuation(1, 0) == sd.attenuation(1, 1)) {
				attn[1] = sd.attenuation(1,0);
			} else {
				attn[1] = "Var";
			}
		}
		if (sd.m_head.nOnCh2 == 0) { attn[2] = "";} 
		else { 
			if (sd.attenuation(2, 0) == sd.attenuation(2, 1)) {
				attn[2] = sd.attenuation(2,0);
			} else {
				attn[2] = "Var";
			}
		}
		file["attenuation"] = attn;

		bp::dict freq;
		if (sd.m_head.nOnCh1 == 0) { freq[1] = "";} 
		else { 
			if (sd.frequency(1, 0) == sd.frequency(1, 1)) {
				freq[1] = sd.frequency(1,0);
			} else {
				freq[1] = "Var";
			}
		}
		if (sd.m_head.nOnCh2 == 0) { freq[2] = "";} 
		else { 
			if (sd.frequency(2, 0) == sd.frequency(2, 1)) {
				freq[2] = sd.frequency(2,0);
			} else {
				freq[2] = "Var";
			}
		}
		file["frequency"] = freq;

		bp::dict begin;
		if (sd.m_head.nOnCh1 == 0) { begin[1] = "";} 
		else { 
			if (sd.begin(1, 0) == sd.begin(1, 1)) {
				begin[1] = sd.begin(1,0);
			} else {
				begin[1] = "Var";
			}
		}
		if (sd.m_head.nOnCh2 == 0) { begin[2] = "";} 
		else { 
			if (sd.begin(2, 0) == sd.begin(2, 1)) {
				begin[2] = sd.begin(2,0);
			} else {
				begin[2] = "Var";
			}
		}
		file["begin"] = begin;

		file["xVar"] = sd.xVariable();

		bp::list trials;
		for (int i = 0; i < sd.m_head.nSweeps; ++i) {
			bp::dict trial;
			trial["xvalue"] = sd.xvalue(i);
			bp::list spikes;
			for (int p = 0; p < sd.m_head.nPasses; ++p) {
				bp::list pass;
				for (unsigned int s = 0; s < sd.m_spikeArray.size(); ++s) {
					// Spike sweeps are 1 based but here we are 0 based
					if (sd.m_spikeArray[s].nSweep == i + 1 && sd.m_spikeArray[s].nPass == p+1) {
						pass.append(sd.m_spikeArray[s].fTime);
					}
				}
				spikes.append(pass);
			}
			trial["spikes"] = spikes;
			trials.append(trial);
		}
		file["trials"] = trials;
	}
	sqlite3_finalize(stmt);
	return file;
}

bp::object pySpikeDB::getFiles(bool selOnly)
{
	bp::list list;


	if (selOnly) {
		std::vector<Gtk::TreeModel::iterator> rows;
		mp_FileDetailsTree->treeSelection()->selected_foreach_iter(
			sigc::mem_fun(rows, &std::vector<Gtk::TreeModel::iterator>::push_back)
			);

		std::vector<Gtk::TreeModel::iterator>::iterator iter;
		for (iter = rows.begin(); 
			 iter != rows.end(); 
			 iter++)
		{
			list.append(getFile(*iter));
		}
	} else {
		Gtk::TreeIter iter;
		for (iter = mp_FileDetailsTree->mrp_ListStore->children().begin(); 
			 iter != mp_FileDetailsTree->mrp_ListStore->children().end(); 
			 iter++)
		{
			list.append(getFile(iter));
		}
	}

	return list;
}

void pySpikeDB::plotClear()
{
	mp_plot->clear();
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
	x = list2vec(pyX);
	y = list2vec(pyY);
	err = list2vec(pyErr);

	if (bp::len(pyX) == bp::len(pyErr))
	{
		mp_plot->plot(x, y, err);
	}
	else
	{
		mp_plot->plot(x, y);
	}
}

std::vector<double> pySpikeDB::list2vec(boost::python::list &l)
{
	std::vector<double> r;
	for (int i = 0; i < bp::len(l); ++i)
	{
		r.push_back(extract<double>(l[i]));
	}
	return r;
}

double pySpikeDB::mean(boost::python::list &v)
{
	int N = bp::len(v);
	double r = 0;
	for (int i = 0; i < N; ++i)
	{
		r += extract<double>(v[i]);
	}
	if (N > 0) r /= N;
	return r;
}

double pySpikeDB::stddev(boost::python::list &v)
{
	int N = bp::len(v);
	if (N <= 1) return 0;
	double m = mean(v);
	double r = 0;
	for (int i = 0; i < N; ++i)
	{
		double t = (extract<double>(v[i]) - m);
		r += t*t;
	}
	return r/(N-1);
}


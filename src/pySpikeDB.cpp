#include "pySpikeDB.h"

namespace bp=boost::python;
using namespace bp;

pySpikeDB::pySpikeDB() {}


pySpikeDB::pySpikeDB(sqlite3** db,uiFileDetailsTreeView* fileDetailsTree) 
{
	this->db = db;
	this->mp_FileDetailsTree = fileDetailsTree;
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

bp::object pySpikeDB::getFiles()
{
	bp::list list;

	sqlite3_stmt *stmt = 0;
	const char query[] = "SELECT header, spikes FROM files WHERE animalID=? AND cellID=? AND fileID=?";
	sqlite3_prepare_v2(*db, query, strlen(query), &stmt, NULL);

	Gtk::TreeIter iter;
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

			bp::dict file;
			file["AnimalID"] = row.get_value(mp_FileDetailsTree->m_Columns.m_col_animalID).c_str();
			file["CellID"] = row.get_value(mp_FileDetailsTree->m_Columns.m_col_cellID);
			file["FileID"] = row.get_value(mp_FileDetailsTree->m_Columns.m_col_filenum);
			file["datetime"] = sd.iso8601(sd.m_head.cDateTime).c_str();

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
			list.append(file);
		}
	}
	sqlite3_finalize(stmt);
	return list;
}

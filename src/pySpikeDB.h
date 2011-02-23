#ifndef PYSPIKEDB_H
#define PYSPIKEDB_H

#include <boost/python.hpp>
#include <gtkmm.h>
#include <sqlite3.h>
#include <set>
#include <vector>
#include "spikedata.h"
#include "uiFileDetailsTreeView.h"


class pySpikeDB {


	public:

		pySpikeDB();
		pySpikeDB(sqlite3** db,uiFileDetailsTreeView* fileDetailsTree);

		boost::python::object getFiles();
		boost::python::object getCells();

		sqlite3 **db;
		uiFileDetailsTreeView* mp_FileDetailsTree;
	private:


		/**
		 * Used to find unique cells.
		 */
		struct CellID {
			Glib::ustring animalID;
			int cellID;

			// Overload the < operator for easy comparison
			friend bool operator<(CellID const& a, CellID const& b)
			{
				if (a.animalID.compare(b.animalID) == 0)
					return a.cellID < b.cellID;
				else
					return a.animalID.compare(b.animalID) < 0;
			}
		};

};

#endif 

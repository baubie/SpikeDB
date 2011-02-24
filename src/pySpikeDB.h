#ifndef PYSPIKEDB_H
#define PYSPIKEDB_H

#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>
#include <gtkmm.h>
#include <sqlite3.h>
#include <set>
#include <vector>
#include <ostream>
#include <sstream>
#include "spikedata.h"
#include "uiFileDetailsTreeView.h"
#include "easyplotmm/easyplotmm.h"

#include <iostream>

#define VARYING_STIMULUS -99999999

class pySpikeDB {


	public:

		pySpikeDB();
		pySpikeDB(sqlite3** db,uiFileDetailsTreeView* fileDetailsTree, EasyPlotmm *plot, Glib::RefPtr<Gtk::TextBuffer> tbOutput);

		boost::python::object getFiles(bool selOnly);
		boost::python::object getCells();
		double stddev(boost::python::list &v);
		double mean(boost::python::list &v);

		void plotSetRGBA(float r, float g, float b, float a);
		void plotSetPointSize(float s);
		void plotSetLineWidth(float s);
		void plotLine(boost::python::list &x, boost::python::list &y, boost::python::list &err);
		void plotClear();

		void print(const std::string &s);

	private:

		sqlite3 **db;
		uiFileDetailsTreeView* mp_FileDetailsTree;
		EasyPlotmm *mp_plot;
		Glib::RefPtr<Gtk::TextBuffer> mrp_tbOutput;

		/**
		 * Current Plotting Values
		 */
		EasyPlotmm::Pen m_plotPen;

		/**
		 * Helper Functions
		 */
		boost::python::object getFile(const Gtk::TreeModel::iterator& iter);
		std::vector<double> list2vec(boost::python::list &l);

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

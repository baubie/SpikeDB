#ifndef PYSPIKEDB_H
#define PYSPIKEDB_H

#ifdef WIN32
#undef HAVE_UNISTD_H
#endif

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
#include "animalColumns.h"
#include <iostream>

#define VARYING_STIMULUS -99999999

class pySpikeDB {


	public:

		pySpikeDB();
		pySpikeDB(sqlite3** db, uiFileDetailsTreeView* fileDetailsTree, Gtk::TreeView* animalTree, AnimalColumns* animalColumns, EasyPlotmm *plot, Glib::RefPtr<Gtk::TextBuffer> tbOutput);

		void setShowErr(bool showErr);

		boost::python::object getFiles(bool selOnly);
		boost::python::object getFilesSingleCell(const std::string &animalID, const int &cellID);
		boost::python::object getCells();

		void reset();

		void forceSpikesAbs(const float &begin, const float &end);
		void filterSpikesAbs(const float &begin, const float &end);
		void filterSpikesRel(const float &begin, const float &end);

		void plotSetRGBA(const float &r, const float &g, const float &b, const float &a);
		void plotSetPointSize(const float &s);
		void plotSetLineWidth(const float &s);
		void plotXMin(const float &v);
		void plotXMax(const float &v);
		void plotYMin(const float &v);
		void plotYMax(const float &v);
		void plotLine(boost::python::list &x, boost::python::list &y, boost::python::list &err);
		void plotHist(boost::python::list &x, boost::python::list &y, boost::python::list &err);
		void plotClear();
		void plotXLabel(const std::string &s);
		void plotYLabel(const std::string &s);

		void setPointNames(boost::python::list &names);
		void setPointData(boost::python::list &data);

		double stddev(boost::python::list &v);
		double mean(boost::python::list &v);
		boost::python::object ttest(boost::python::list &a, boost::python::list &b, bool eqvar);

		void print(const std::string &s);

	private:

		sqlite3 **db;
		uiFileDetailsTreeView* mp_FileDetailsTree;
		Gtk::TreeView* mp_AnimalTree;
		AnimalColumns* mp_AnimalColumns;
		EasyPlotmm *mp_plot;
		Glib::RefPtr<Gtk::TextBuffer> mrp_tbOutput;

		/**
		 * Filtering Mode
		 */
		float filterAbsBegin, filterAbsEnd;
		float filterRelBegin, filterRelEnd;
		float forceAbsBegin, forceAbsEnd;

		/**
		 * Current Plotting Values
		 */
		EasyPlotmm::Pen m_plotPen;
		double xmin,xmax,ymin,ymax;
		bool showErr;

		/**
		 * Helper Functions
		 */
		boost::python::object getFile(const Gtk::TreeModel::iterator& iter);
		template<typename T> std::vector<T> list2vec(boost::python::list &l);

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

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

#ifndef PYSPIKEDB_H
#define PYSPIKEDB_H

#ifdef WIN32
#undef HAVE_UNISTD_H
#endif

#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>
#include <boost/shared_ptr.hpp>
#include <gtkmm.h>
#include <sqlite3.h>
#include <set>
#include <vector>
#include <ostream>
#include <sstream>
#include <iostream>

#include "spikedata.h"
#include "uiFileDetailsTreeView.h"
#include "easyplotmm/easyplotmm.h"
#include "animalColumns.h"

#define VARYING_STIMULUS -99999999

class pySpikeDB {


	public:

		pySpikeDB();
		pySpikeDB(sqlite3** db, uiFileDetailsTreeView* fileDetailsTree, Gtk::TreeView* animalTree, AnimalColumns* animalColumns, EasyPlotmm *plot, Glib::RefPtr<Gtk::TextBuffer> tbOutput, Gtk::ProgressBar* pbStatus);

		boost::python::object getFiles(bool selOnly);
		boost::python::object getFilesSingleCell(const std::string &animalID, const int &cellID);
		boost::python::object getCells();
		boost::python::object getOptions();

		void reset();


		void addOptionCheckbox(const std::string &name, const std::string &description, bool def);
		void addOptionRadio(const std::string &name, boost::python::list &items, const std::string &description, int def);
		void addOptionNumber(const std::string &name, const std::string &description, double def);
		void addRuler();

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

		void updateProgress(const float &val);

		void enableActionButton();


		/**
		 * Option arrays
		 * Public to be accessed from uiAnalysis
		 */
		class Option {
			public:
				enum OptionType { RULER, CHECKBOX, NUMBER, RADIO };
				Option(OptionType type) : type(type) {}
				std::string name;
				std::string description;
				OptionType type;
		};

		class rulerOption : public Option {
			public:
				rulerOption() : Option(RULER) { }
		};

		class checkboxOption : public Option {

			public:
				checkboxOption(double def) : Option(CHECKBOX)
					{ this->def = def; setValue(def); }
				bool def;
				bool getValue() { return value;  }
				void setValue(bool value) { this->value = value; }
			private:
				bool value;
		};

		class numberOption : public Option {

			public:
				numberOption(double def) : Option(NUMBER)
					{ this->def = def; setValue(def); }
				double def;
				double getValue() { return value;  }
				void setValue(double value) { this->value = value; }
			private:
				double value;
		};

		class radioOption : public Option {

			public:
				radioOption(int def) : Option(RADIO)
					{ this->def = def; setValue(def); }
				unsigned int def;
				unsigned int getValue() { return value; }
				Glib::ustring getStringValue()
				{ try { return items.at(value); }
				  catch(std::out_of_range e) { return ""; }
				}
				Glib::ustring getSelected();
				void setValue(unsigned int value) { this->value = value; }
				void setItems(const std::vector<std::string>& items) { this->items = items; }
				std::vector<std::string>* getItems() { return &this->items; }
			private:
				unsigned int value;
				std::vector<std::string> items;
		};

		std::vector <boost::shared_ptr<Option> > options;

		bool actionButton;


	private:

		sqlite3 **db;
		uiFileDetailsTreeView* mp_FileDetailsTree;
		Gtk::TreeView* mp_AnimalTree;
		AnimalColumns* mp_AnimalColumns;
		EasyPlotmm *mp_plot;
		Glib::RefPtr<Gtk::TextBuffer> mrp_tbOutput;
		Gtk::ProgressBar *mp_pbStatus;

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

		/**
		 * Helper Functions
		 */
		Option* findOptionByName(const Glib::ustring &name);
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

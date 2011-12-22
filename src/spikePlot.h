#ifndef SPIKEPLOT_H
#define SPIKEPLOT_H

#include "easyplotmm/easyplotmm.h"
#include <sstream>

class SpikePlot : public EasyPlotmm {

	public:
		void setStatusbar(Gtk::Statusbar **sb);

	protected:
		Gtk::Statusbar** mp_statusbar;

		virtual void dataPointClicked(const double x, const double y, const std::string name, const std::string data);
		virtual void on_cursor_over_point(const int setIndex, const int pointIndex, const double x, const double y, const std::string name, const std::string data);
};

#endif

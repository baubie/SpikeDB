#include "spikePlot.h"

void SpikePlot::setStatusbar(Gtk::Statusbar **sb)
{
	mp_statusbar = sb;
}


void SpikePlot::dataPointClicked(const double x, const double y, const std::string name, const std::string data)
{
	// Default action is no extra data
	// Therefore, we will show the (x,y) and name
	std::cout << "Clicked a point. Do stuff." << std::endl;
}

void SpikePlot::on_cursor_over_point(const int setIndex, const int pointIndex, const double x, const double y, const std::string name, const std::string data)
{
	// Call parent class
	EasyPlotmm::on_cursor_over_point(setIndex,pointIndex,x,y,name,data);

}

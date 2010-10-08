#include "easyplotmm.h"

const double EasyPlotmm::AUTOMATIC = ((double) -1);

EasyPlotmm::EasyPlotmm() :
    xmin(AUTOMATIC),
    xmax(AUTOMATIC),
    ymin(AUTOMATIC),
    ymax(AUTOMATIC)
{

}

EasyPlotmm::~EasyPlotmm()
{
}

void EasyPlotmm::axes(double xmin, double xmax, double ymin, double ymax)
{

}

void EasyPlotmm::clear()
{

}

void EasyPlotmm::plot(std::vector<double> x, std::vector<double> y, Pen p)
{

}

bool EasyPlotmm::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
    return true;
}

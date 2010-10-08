#ifndef EASYPLOTMM_H
#define EASYPLOTMM_H

#include <gtkmm/drawingarea.h>
#include <vector>

class EasyPlotmm : public Gtk::DrawingArea
{
    public:
        EasyPlotmm();
        virtual ~EasyPlotmm();

        struct Pen {
            float linewidth;
            float pointsize;
        };

        void axes(double xmin, double xmax, double ymin, double ymax);
        void clear();
        void plot(std::vector<double> x, std::vector<double> y, Pen p);

        // Constants 
        static const double AUTOMATIC;

    protected:
        // Override default signal handler
        virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);

        double xmin,xmax,ymin,ymax;
};

#endif

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
        void plot(std::vector<double> x, std::vector<double> y);
        void plot(std::vector<double> x, std::vector<double> y, Pen p);
        void redraw();

        // Constants 
        static const double AUTOMATIC;

        // Plot parameters
        double pad_left,pad_top,pad_right,pad_bottom;


    protected:
        // Override default signal handler
        virtual bool on_expose_event(GdkEventExpose* event);
        double m_xmin,m_xmax,m_ymin,m_ymax;
        std::vector< std::vector<double> > m_x;
        std::vector< std::vector<double> > m_y;
        std::vector< Pen > m_pen;
};

#endif

#ifndef EASYPLOTMM_H
#define EASYPLOTMM_H

#include <gtkmm/drawingarea.h>
#include <vector>

class EasyPlotmm : public Gtk::DrawingArea
{
    public:
        EasyPlotmm();
        virtual ~EasyPlotmm();

        enum Shape {
            CIRCLE,
            TRIANGLE,
            SQUARE,
            DIAMOND,
            FCIRCLE,
            FTRIANGLE,
            FSQUARE,
            FDIAMOND
        };

        struct Pen {
            Pen() {
                linewidth = 0.0;
                pointsize = 2;
            }
            float linewidth;
            float pointsize;
            Shape shape;
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
        void drawshape(double x, double y, double size, Shape shape);
        double m_xmin,m_xmax,m_ymin,m_ymax;
        std::vector< std::vector<double> > m_x;
        std::vector< std::vector<double> > m_y;
        std::vector< Pen > m_pens;
};

#endif

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
            DIAMOND
        };

        struct Pen {
            Pen() {
                linewidth = 1.0;
                pointsize = 10;
                filled = false;
                shape = SQUARE;
            }
            float linewidth;
            float pointsize;
            bool filled;
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
        void drawshape(Cairo::RefPtr<Cairo::Context> cr, double size, Shape shape, bool filled);
        double m_xmin,m_xmax,m_ymin,m_ymax;
        std::vector< std::vector<double> > m_x;
        std::vector< std::vector<double> > m_y;
        std::vector< Pen > m_pens;
};

#endif

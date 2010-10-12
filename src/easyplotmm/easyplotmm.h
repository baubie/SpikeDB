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
            UTRIANGLE,
            SQUARE,
            DIAMOND,
            POINT
        };

        struct RGBA {
            RGBA() {
                r = g = b = 0;
                a = 1;
            }
            double r;
            double g;
            double b;
            double a;
        };

        struct Pen {
            Pen() {
                linewidth = 1.0;
                pointsize = 8;
                filled = false;
                shape = CIRCLE;
            }
            float linewidth;
            float pointsize;
            bool filled;
            Shape shape;
            RGBA color;
        };

        void axes(double xmin, double xmax, double ymin, double ymax);
        void clear();
        void plot(std::vector<double> x, std::vector<double> y);
        void plot(std::vector<double> x, std::vector<double> y, Pen p);
        void set_bg_rgba(double r, double g, double b, double a);
        void redraw();
        Pen getPen();

        // Constants 
        static const double AUTOMATIC;

        // Plot parameters
        double pad_left,pad_top,pad_right,pad_bottom;


    protected:
        // Override default signal handler
        virtual bool on_expose_event(GdkEventExpose* event);
        void drawshape(Cairo::RefPtr<Cairo::Context> cr, double size, Shape shape, bool filled, RGBA col);
        void makeDefaultPens();
        RGBA bg;
        double m_xmin,m_xmax,m_ymin,m_ymax;
        std::vector< std::vector<double> > m_x;
        std::vector< std::vector<double> > m_y;
        std::vector< Pen > m_pens;

        std::vector< Pen > m_defPens;
        unsigned int curPen;
};

#endif

#ifndef EASYPLOTMM_H
#define EASYPLOTMM_H

#include <gtkmm/drawingarea.h>
#include <vector>
#include <float.h>

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
            POINT,
            NONE
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
            RGBA errcolor;
        };

        void axes(double xmin, double xmax, double ymin, double ymax);
        void clear();
        void plot(std::vector<double> x, std::vector<double> y);
        void plot(std::vector<double> x, std::vector<double> y, std::vector<double> err);
        void plot(std::vector<double> x, std::vector<double> y, std::vector<double> err, Pen p);
        void plot(std::vector<double> x, std::vector<double> y, Pen p);
        void redraw();
        Pen getPen();

        // Set properties
        void set_bg_rgba(double r, double g, double b, double a);

        // Constants 
        static const double AUTOMATIC;

        // Plot parameters
        double m_xmin,m_xmax,m_ymin,m_ymax; // Axes
        double pad_left,pad_top,pad_right,pad_bottom;
        double x_st_size,x_bt_size,y_st_size,y_bt_size;
        double label_pad;


    protected:
        // Override default signal handler
        virtual bool on_expose_event(GdkEventExpose* event);
        virtual bool on_event_button_press(GdkEventButton* event);
        virtual bool on_event_motion(GdkEventMotion* event);
        virtual bool on_event_button_release(GdkEventButton* event);
        void drawshape(Cairo::RefPtr<Cairo::Context> cr, double size, Shape shape, bool filled, RGBA col);
        void drawerr(Cairo::RefPtr<Cairo::Context> cr, double err, double scale, double size, RGBA col);
        void makeDefaultPens();

        // Store plots
        std::vector< std::vector<double> > m_x;
        std::vector< std::vector<double> > m_y;
        std::vector< std::vector<double> > m_err;
        std::vector< Pen > m_pens;

        RGBA bg; // Plot background color

        // Default pens
        std::vector< Pen > m_defPens;
        unsigned int curPen;

        // Store the passed values for safe keeping
        int ylab_width,lab_height;
        double m_ixmin,m_ixmax,m_iymin,m_iymax; // Axes
        int zoom_start, zoom_end;
        double zoom_start_scale, zoom_end_scale;
        bool in_zoom;
        bool has_plot;
};

#endif

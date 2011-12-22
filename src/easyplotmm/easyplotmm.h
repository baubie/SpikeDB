#ifndef EASYPLOTMM_H
#define EASYPLOTMM_H

#include <gtkmm.h>
#include <vector>
#include <float.h>
#include <giomm.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <sstream>
#include <iostream>

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

		double automatic();
        void axes(double xmin, double xmax, double ymin, double ymax);
		void xname(std::string name);
		void yname(std::string name);
        void clear();
        void plot(std::vector<double> x, std::vector<double> y);
        void plot(std::vector<double> x, std::vector<double> y, bool exportable);
        void plot(std::vector<double> x, std::vector<double> y, Pen p);
        void plot(std::vector<double> x, std::vector<double> y, Pen p, bool exportable);
        void plot(std::vector<double> x, std::vector<double> y, std::vector<double> err);
        void plot(std::vector<double> x, std::vector<double> y, std::vector<double> err, bool exportable);
        void plot(std::vector<double> x, std::vector<double> y, std::vector<double> err, Pen p);
        void plot(std::vector<double> x, std::vector<double> y, std::vector<double> err, Pen p, bool exportable);

		void setPointNames(std::vector<std::string> names);
		void setPointData(std::vector<std::string> data);

        void plotHist(std::vector<double> x, std::vector<double> y, Pen p);
        void plotHist(std::vector<double> x, std::vector<double> y, std::vector<double> err, Pen p);
        void redraw();
        Pen getPen();

        // Set properties
        void set_bg_rgba(double r, double g, double b, double a);

        // Constants 
        static const double AUTOMATIC;
        static const double NOPOINT;

        // Plot parameters
        double m_xmin,m_xmax,m_ymin,m_ymax; // Axes
        double pad_left,pad_top,pad_right,pad_bottom;
        double x_st_size,x_bt_size,y_st_size,y_bt_size;
        double label_pad;

		// Changed zoom
		typedef sigc::signal<void,double,double> type_signal_zoom_changed;
		type_signal_zoom_changed signal_zoom_changed();

		// Drew point under cursor 
		typedef sigc::signal<void,int,int,double,double,std::string,std::string> type_signal_drew_point_under_cursor;
		type_signal_drew_point_under_cursor signal_drew_point_under_cursor();


    protected:

		// To be overridden for custom actions
		virtual void dataPointClicked(const double /*x*/, const double /*y*/, const std::string /*name*/, const std::string /*data*/) {};
		virtual void on_cursor_over_point(const int setIndex, const int pointIndex, const double x, const double y, const std::string name, const std::string data);

		type_signal_zoom_changed m_signal_zoom_changed;
		type_signal_drew_point_under_cursor m_signal_drew_point_under_cursor;

        // Override default signal handler
        virtual bool on_expose_event(GdkEventExpose* event);
        virtual bool on_event_button_press(GdkEventButton* event);
        virtual bool on_event_motion(GdkEventMotion* event);
        virtual bool on_event_leave(GdkEventCrossing* event);
        virtual bool on_event_button_release(GdkEventButton* event);
        void drawshape(Cairo::RefPtr<Cairo::Context> cr, double size, Shape shape, bool filled, RGBA col);
        void drawerr(Cairo::RefPtr<Cairo::Context> cr, double err, double scale, double size, RGBA col);
        void makeDefaultPens();
		void showError(std::string err);
		bool checkMousePosition();

        // Context menu
        Gtk::Menu m_Menu_Popup;
        void export_data();
		Glib::ustring lastdir;

        // Store plots
        std::vector< std::vector<double> > m_x_hist;
        std::vector< std::vector<double> > m_y_hist;
        std::vector< std::vector<double> > m_x;
        std::vector< std::vector<double> > m_y;
        std::vector< std::vector<double> > m_err;
        std::vector< std::vector<std::string> > m_names;
        std::vector< std::vector<std::string> > m_data;
        std::vector< bool > m_exportable;
        std::vector< Pen > m_pens;

        RGBA bg; // Plot background color

        // Default pens
        std::vector< Pen > m_defPens;
        unsigned int curPen;

        // Store the passed values for safe keeping
        int ylab_width,lab_height,xname_width,xname_height,yname_width,yname_height;
		std::string m_xname, m_yname;
        double m_ixmin,m_ixmax,m_iymin,m_iymax; // Axes
        gdouble zoom_start, zoom_end;
        double zoom_start_scale, zoom_end_scale;
        bool in_zoom;
		bool in_crosshairs;
		gdouble ch_x, ch_y;
        bool has_plot;
		double mouse_x, mouse_y;
		sigc::connection motionID;
		bool checkForPointUnderCursor;
		int curPointUnderMouse;
		int curSetUnderMouse;
};

#endif

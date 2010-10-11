#include "easyplotmm.h"
#include <cairomm/context.h>
#include <iostream>

const double EasyPlotmm::AUTOMATIC = ((double) -1);

EasyPlotmm::EasyPlotmm() :
    m_xmin(AUTOMATIC),
    m_xmax(AUTOMATIC),
    m_ymin(AUTOMATIC),
    m_ymax(AUTOMATIC)
{
    pad_left = 50;
    pad_top = 5;
    pad_right = 5;
    pad_bottom = 25;
}

EasyPlotmm::~EasyPlotmm()
{
}

void EasyPlotmm::redraw()
{
    Glib::RefPtr<Gdk::Window> win = get_window();
    if (win)
    {
        Gdk::Rectangle r(0,0, get_allocation().get_width(),
                              get_allocation().get_height());
        win->invalidate_rect(r, false);
    }
}

void EasyPlotmm::axes(double xmin, double xmax, double ymin, double ymax)
{

}

void EasyPlotmm::clear()
{
    m_x.clear();
    m_y.clear();
    m_pens.clear();
}

void EasyPlotmm::plot(std::vector<double> x, std::vector<double> y)
{
    Pen p;
    plot(x,y,p);    
}

void EasyPlotmm::plot(std::vector<double> x, std::vector<double> y, Pen p)
{
    // Add plot to vectors
    m_x.push_back(x);
    m_y.push_back(y);
    m_pens.push_back(p);

    // Force Redraw
    redraw();
}

void EasyPlotmm::drawshape(Cairo::RefPtr<Cairo::Context> cr, double s, Shape shape, bool filled)
{
    if (s <= 0) return;

    switch(shape)
    {
        case SQUARE:
            cr->set_line_width(1.0);
            cr->rel_move_to(-s/2, s/2);
            cr->rel_line_to(s,0);
            cr->rel_line_to(0,-s);
            cr->rel_line_to(-s,0);
            cr->rel_line_to(0,s);
            // Move back to the middle
            cr->rel_move_to(s/2,-s/2);
            break;
    }
    if (filled)
    {
        cr->paint();
    }
}

bool EasyPlotmm::on_expose_event(GdkEventExpose* event)
{

    Glib::RefPtr<Gdk::Window> window = get_window();
    if (window)
    {
        Gtk::Allocation allocation = get_allocation();
        const int width = allocation.get_width();
        const int height = allocation.get_height();
        Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();

        const int xlength = width-pad_left-pad_right;
        const int ylength = height-pad_top-pad_bottom;

        cr->rectangle(event->area.x, event->area.y,
                      event->area.width, event->area.height);
        cr->clip();

        // Scale the window so the bottom left of the graph is
        // (0,0) on the screen

        cr->translate(pad_left, height-pad_bottom);

        // Draw background and axes
        cr->save();
        cr->set_source_rgb(1,1,1);
        cr->paint();
        cr->restore();
        cr->set_source_rgb(0,0,1.0);
        cr->set_line_width(1);
        cr->move_to(0,0);
        cr->line_to(xlength,0);
        cr->move_to(0,0);
        cr->line_to(0,-ylength);
        cr->stroke();

        // Stop here if there is no data to plot
        if (m_x.empty()) return true;
        if (m_x.size() != m_y.size())
        {
            std::cout << "ERROR: X and Y vectors are not the same size." << std::endl;
            return true;
        }
        if (m_x.size() != m_pens.size())
        {
            std::cout << "ERROR: Only " << m_pens.size() << " pens for " << m_x.size() << " plots." << std::endl;
            return true;
        }
        
        // Draw datasets
        // Determine axes
        double xmin,xmax,ymin,ymax;
        xmin = xmax = ymin = ymax = 0;
        for (unsigned int xs = 0; xs < m_x.size(); ++xs) {
            if (m_x[xs].size() != m_y[xs].size())
            {
                std::cout << "ERROR: Plot #" << xs << " do not have same X and Y vector sizes." << std::endl;
                return true;
            }
            for (unsigned int x = 0; x < m_x[xs].size(); ++x) {
                xmin = xmin > m_x[xs][x] ? m_x[xs][x] : xmin;
                xmax = xmax < m_x[xs][x] ? m_x[xs][x] : xmax;
            }
        }
        for (unsigned int ys = 0; ys < m_y.size(); ++ys) {
            for (unsigned int y = 0; y < m_y[ys].size(); ++y) {
                ymin = ymin > m_y[ys][y] ? m_y[ys][y] : ymin;
                ymax = ymax < m_y[ys][y] ? m_y[ys][y] : ymax;
            }
        }
        if (m_xmin != AUTOMATIC) xmin = m_xmin;
        if (m_xmax != AUTOMATIC) xmax = m_xmax;
        if (m_ymin != AUTOMATIC) ymin = m_ymin;
        if (m_ymax != AUTOMATIC) ymax = m_ymax;

        double xscale = xlength/(xmax-xmin);
        double yscale = -ylength/(ymax-ymin);

        // Plot data values
        cr->set_source_rgb(0,0,0);
        for (unsigned int i  = 0; i < m_x.size(); ++i) {
            cr->set_line_width(m_pens[i].linewidth);
            cr->move_to(m_x[i][0]*xscale,m_y[i][0]*yscale);
            drawshape(cr,m_pens[i].pointsize,m_pens[i].shape,m_pens[i].filled);
            for (unsigned int j = 1; j < m_x[i].size(); ++j) {
                cr->line_to(m_x[i][j]*xscale,m_y[i][j]*yscale);
                drawshape(cr,m_pens[i].pointsize,m_pens[i].shape,m_pens[i].filled);
            }
        }
        cr->stroke();

        // Add the labels
        int numX = xlength/50;
        int numY = ylength/50;
        double dx,dy; 
        dx = ((xmax-xmin)/numX);
        dy = ((ymax-ymin)/numY);
        if (dx > 0)
        {
            for (double x = xmin; x <= xmax; x+=dx)
            {
                Glib::RefPtr<Pango::Layout> pangoLayout = Pango::Layout::create (cr);
                cr->move_to(x*xscale,0);
                char buffer[5];
                sprintf(buffer,"%.f",x);
                pangoLayout->set_text(buffer);
                pangoLayout->update_from_cairo_context(cr);
                pangoLayout->add_to_cairo_context(cr);
                cr->stroke();
            }
        }
        if (dy > 0)
        {
            for (double y = ymin; y <= ymax; y+=dy)
            {
                Glib::RefPtr<Pango::Layout> pangoLayout = Pango::Layout::create (cr);
                cr->move_to(-12,y*yscale);
                char buffer[5];
                sprintf(buffer,"%f",y);
                pangoLayout->set_text(buffer);
                pangoLayout->update_from_cairo_context(cr);
                pangoLayout->add_to_cairo_context(cr);
                cr->stroke();
            }
        }

        cr->clip();
    }
    return true;
}

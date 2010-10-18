#include "easyplotmm.h"
#include <cairomm/context.h>
#include <iostream>

const double EasyPlotmm::AUTOMATIC = (DBL_MAX -1);

EasyPlotmm::EasyPlotmm() :
    m_xmin(AUTOMATIC),
    m_xmax(AUTOMATIC),
    m_ymin(AUTOMATIC),
    m_ymax(AUTOMATIC),
    pad_left(10),
    pad_top(10),
    pad_right(20),
    pad_bottom(10),
    x_st_size(5),
    x_bt_size(10),
    y_st_size(5),
    y_bt_size(10),
    label_pad(3),
    curPen(0)
{
    bg.r = 1;
    bg.g = 1;
    bg.b = 1;

    m_ixmin = m_xmin;
    m_ixmax = m_xmax;
    m_iymin = m_ymin;
    m_iymax = m_ymax;

    makeDefaultPens();
    this->add_events(Gdk::BUTTON_PRESS_MASK);
    this->add_events(Gdk::BUTTON_MOTION_MASK);
    this->add_events(Gdk::BUTTON_RELEASE_MASK);
    this->signal_button_press_event().connect(sigc::mem_fun(*this, &EasyPlotmm::on_event_button_press) );
    this->signal_motion_notify_event().connect(sigc::mem_fun(*this, &EasyPlotmm::on_event_motion) );
    this->signal_button_release_event().connect(sigc::mem_fun(*this, &EasyPlotmm::on_event_button_release) );
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

bool EasyPlotmm::on_event_button_press(GdkEventButton* event)
{
    switch(event->type)
    {
        case GDK_BUTTON_PRESS:
            if (event->button == 1)
            {
               in_zoom = true && has_plot;
               zoom_start = event->x;
               zoom_end = event->x;
               redraw();
            }
            break;
        case GDK_2BUTTON_PRESS:
            in_zoom = false;
            m_xmin = m_ixmin;
            m_xmax = m_ixmax;
            redraw();
            break;
        default:
            break;

    }
    return true;
}

bool EasyPlotmm::on_event_motion(GdkEventMotion* event)
{
    if (in_zoom)
    {
        zoom_end = event->x;
        redraw();
    }
    return true;
}

bool EasyPlotmm::on_event_button_release(GdkEventButton* event)
{
    if (event->button == 1)
    {
       in_zoom = false;
       if (zoom_start_scale != zoom_end_scale)
       {
           zoom_end = event->x;
           if (zoom_start_scale < zoom_end_scale)
           {
               m_xmin = zoom_start_scale;
               m_xmax = zoom_end_scale;
           }
           else
           {
               m_xmax = zoom_start_scale;
               m_xmin = zoom_end_scale;
           }

           redraw();
       }
    }
    return true;
}

void EasyPlotmm::axes(double xmin, double xmax, double ymin, double ymax)
{
    m_xmin = m_ixmin = xmin;
    m_xmax = m_ixmax = xmax;
    m_ymin = m_iymin = ymin;
    m_ymax = m_iymax = ymax;
}

void EasyPlotmm::clear()
{
    m_x.clear();
    m_y.clear();
	m_err.clear();
    m_pens.clear();
    curPen = 0;
    redraw();
    has_plot = false;
}

EasyPlotmm::Pen EasyPlotmm::getPen()
{
    Pen p = m_defPens[curPen];
    curPen++;
    if (curPen >= m_defPens.size()) curPen = 0;
    return p;
}

void EasyPlotmm::plot(std::vector<double> x, std::vector<double> y)
{
	std::vector<double> err(x.size(),0);
    plot(x,y,err,getPen());    
}

void EasyPlotmm::plot(std::vector<double> x, std::vector<double> y, Pen p)
{
    // Add plot to vectors
	std::vector<double> err(x.size(),0);
    plot(x,y,err,p);    
}

void EasyPlotmm::plot(std::vector<double> x, std::vector<double> y, std::vector<double> err)
{
    // Add plot to vectors
    plot(x,y,err,getPen());    
}

void EasyPlotmm::plot(std::vector<double> x, std::vector<double> y, std::vector<double> err, Pen p)
{
    // Add plot to vectors
    m_x.push_back(x);
    m_y.push_back(y);
	m_err.push_back(err);
    m_pens.push_back(p);

    // Force Redraw
    redraw();

    has_plot = true;
}

void EasyPlotmm::drawshape(Cairo::RefPtr<Cairo::Context> cr, double s, Shape shape, bool filled, RGBA col)
{
    if (s <= 0) return;
    double h;
    cr->set_line_width(1.0);
    cr->set_source_rgba(col.r,col.g,col.b,col.a);
    switch(shape)
    {
        case POINT:
            cr->set_line_width(s);
            cr->rel_move_to(-s/2,0);
            cr->rel_line_to(s,0);
            cr->rel_move_to(-s/2,0);
            break;

        case CIRCLE:
            double x,y;
            cr->get_current_point(x,y);
            cr->move_to(x+s/2,y);
            cr->arc(x,y,s/2,0.0,2.0*M_PI);
            cr->move_to(x,y);
            break;

        case SQUARE:
            cr->rel_move_to(-s/2, s/2);
            cr->rel_line_to(s,0);
            cr->rel_line_to(0,-s);
            cr->rel_line_to(-s,0);
            cr->rel_line_to(0,s);
            // Move back to the middle
            cr->rel_move_to(s/2,-s/2);
            break;

        case DIAMOND:
            h = sqrt(2)*s;
            cr->rel_move_to(0,-h/2);
            cr->rel_line_to(-h/2,h/2);
            cr->rel_line_to(h/2,h/2);
            cr->rel_line_to(h/2,-h/2);
            cr->rel_line_to(-h/2,-h/2);
            cr->rel_move_to(0,h/2);
            break;

        case TRIANGLE:
            h = sqrt(s*s-(0.25*s*s));
            cr->rel_move_to(-s/2,h/2);
            cr->rel_line_to(s,0);
            cr->rel_line_to(-s/2,-h);
            cr->rel_line_to(-s/2,h);
            // Move back to the middle
            cr->rel_move_to(s/2,-h/2);
            break; 

        case UTRIANGLE:
            h = sqrt(s*s-(0.25*s*s));
            cr->rel_move_to(-s/2,-h/2);
            cr->rel_line_to(s,0);
            cr->rel_line_to(-s/2,h);
            cr->rel_line_to(-s/2,-h);
            // Move back to the middle
            cr->rel_move_to(s/2,h/2);
            break; 
        case NONE:
        default:
            break;

    }
    if (shape != POINT && shape != NONE)
    {
        if (filled) cr->set_source_rgba(col.r,col.g,col.b,col.a);
        else cr->set_source_rgba(bg.r,bg.g,bg.b,bg.a);
        cr->fill_preserve();
        cr->set_source_rgba(col.r,col.g,col.b,col.a);
    }
}

void EasyPlotmm::drawerr(Cairo::RefPtr<Cairo::Context> cr, double err, double scale, double size, RGBA col)
{
	if (err <= 0) return;

    cr->set_line_width(1.0);
    cr->set_source_rgba(col.r,col.g,col.b,col.a);
	cr->rel_move_to(0, err*scale);
	cr->rel_line_to(0, -2*err*scale);
	cr->rel_move_to(0, err*scale);
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


        cr->rectangle(event->area.x, event->area.y,
                      event->area.width, event->area.height);
        cr->clip();


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
        if (m_x.size() != m_err.size())
        {
            std::cout << "ERROR: X and ERR vectors are not the same size." << std::endl;
            return true;
        }
        
        // Draw datasets
        // Determine axes
        double xmin,xmax,ymin,ymax;
        xmin = ymin = DBL_MAX;
        xmax = ymax = -DBL_MAX;
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

        double maxX = fabs(xmin)>fabs(xmax)?fabs(xmin):fabs(xmax);
        double maxY = fabs(ymin)>fabs(ymax)?fabs(ymin):fabs(ymax);
        double xorder = 1;
        double yorder = 1;
        if (maxX < 0) maxX = 1;
        if (maxY < 0) maxY = 1;

        if (maxX > 10) while (maxX / xorder > 10) { xorder *= 10; }
        if (maxX != 0 && maxX <= 10) while (maxX / xorder <= 1) { xorder /= 10; }
        if (maxY > 10) while (maxY / yorder > 10) { yorder *= 10; }
        if (maxY != 0 && maxY <= 10) while (maxY / yorder <= 1 ) { yorder /= 10; }

        double Xst, Xbt, Yst, Ybt;
        Xst = xorder/10.0;
        Yst = yorder/10.0;
        Xbt = Xst*5;
        Ybt = Yst*5;

        // Determine the first tick mark
        double xmin_st = xmin;
        double xmin_bt = xmin;
        double ymin_st = ymin;
        double ymin_bt = ymin;
        double xmax_st = xmax;
        double xmax_bt = xmax;
        double ymax_st = ymax;
        double ymax_bt = ymax;
        for (double x = 0; x <= xmin; x+=Xst) xmin_st = x+Xst;
        for (double x = 0; x <= xmin; x+=Xbt) xmin_bt = x+Xbt;
        for (double x = 0; x <= xmax; x+=Xst) xmax_st = x;
        for (double x = 0; x <= xmax; x+=Xbt) xmax_bt = x;
        for (double y = 0; y <= ymin; y+=Yst) ymin_st = y+Yst;
        for (double y = 0; y <= ymin; y+=Ybt) ymin_bt = y+Ybt;
        for (double y = 0; y <= ymax; y+=Yst) ymax_st = y;
        for (double y = 0; y <= ymax; y+=Ybt) ymax_bt = y;

        bool x_useint = !((int)(xmin) == (int)(xmin+Xbt));
        bool y_useint = !((int)(ymin) == (int)(ymin+Ybt));
        Glib::RefPtr<Pango::Layout> pangoLayout = Pango::Layout::create (cr);
        char buffer[10];
        if (y_useint) sprintf(buffer,"%i",(int)(ymin_bt));
        else sprintf(buffer,"%.1f",ymin_bt); 
        Pango::FontDescription label_font_descr("sans normal 8");
        pangoLayout->set_font_description(label_font_descr);
        pangoLayout->set_text(buffer);
        pangoLayout->update_from_cairo_context(cr);
        pangoLayout->get_size(ylab_width,lab_height);
        ylab_width /= Pango::SCALE;
        lab_height /= Pango::SCALE;

        const int xlength = width-pad_left-pad_right-ylab_width-y_bt_size-label_pad;
        const int ylength = height-pad_top-pad_bottom-lab_height-x_bt_size-label_pad;
        double xscale = xlength/(xmax-xmin);
        double yscale = -ylength/(ymax-ymin);

        // Translate the window so the bottom left of the graph is
        // (0,0) on the screen
        cr->translate(pad_left+ylab_width+y_bt_size+label_pad, height-pad_bottom-lab_height-x_bt_size-label_pad);

        // Draw background and axes
        cr->save();
        cr->set_source_rgb(bg.r,bg.g,bg.b);
        cr->paint();
        cr->restore();
        cr->set_source_rgba(0,0,0,1);
        cr->set_line_width(2);
        cr->move_to(0,0);
        cr->line_to(xlength,0);
        cr->move_to(0,0);
        cr->line_to(0,-ylength);
        cr->stroke();

        // Draw the ticks
        cr->save();
        cr->set_line_width(2.0);

        if (xmax != 0 && Xst != 0)
        {
            for (double x = fabs(xmin_st); x <= fabs(xmax_st); x+=Xst)
            {
                cr->move_to((x-xmin)*xscale, 0);
                cr->rel_line_to(0, x_st_size);
                cr->stroke();
            }
            for (double x = fabs(xmin_bt); x <= fabs(xmax_bt); x+=Xbt)
            {
                cr->move_to((x-xmin)*xscale, 0);
                cr->rel_line_to(0, x_bt_size);
                cr->stroke();
            }
        }
        if (ymax != 0 && Yst != 0)
        {
            for (double y = fabs(ymin_st); y <= fabs(ymax_st); y+=Yst)
            {
                cr->move_to(0,(y-ymin)*yscale);
                cr->rel_line_to(-y_st_size, 0);
                cr->stroke();
            }
            for (double y = fabs(ymin_bt); y <= fabs(ymax_bt); y+=Ybt)
            {
                cr->move_to(0,(y-ymin)*yscale);
                cr->rel_line_to(-y_bt_size, 0);
                cr->stroke();
            }
        }
        cr->restore();

        // Add the labels
        cr->set_source_rgba(0,0,0,1);
        cr->set_line_width(1.0);
        if (xmax != 0 && Xbt != 0)
        {
            for (double x = fabs(xmin_bt); x <= fabs(xmax_bt); x+=Xbt)
            {
                Glib::RefPtr<Pango::Layout> pangoLayout = Pango::Layout::create (cr);
                char buffer[10];
                if (x_useint) sprintf(buffer,"%i",(int)x);
                else sprintf(buffer,"%.1f",x); 
                pangoLayout->set_font_description(label_font_descr);
                pangoLayout->set_text(buffer);
                pangoLayout->update_from_cairo_context(cr);
                int w,h;
                pangoLayout->get_size(w,h);
                cr->move_to((x-xmin)*xscale-(w/Pango::SCALE)/2.0,x_bt_size+label_pad);
                pangoLayout->add_to_cairo_context(cr);
                cr->stroke();
            }
        }
        if (ymax > 0 && Ybt != 0)
        {
            for (double y = fabs(ymin_bt); y <= fabs(ymax_bt); y+=Ybt)
            {
                Glib::RefPtr<Pango::Layout> pangoLayout = Pango::Layout::create (cr);
                cr->move_to(-ylab_width-y_bt_size-label_pad,(y-ymin)*yscale-lab_height/2);
                char buffer[10];
                if (y_useint) sprintf(buffer,"%i",(int)y);
                else sprintf(buffer,"%.1f",y); 
                pangoLayout->set_font_description(label_font_descr);
                pangoLayout->set_alignment(Pango::ALIGN_RIGHT);
                pangoLayout->set_width(Pango::SCALE*(ylab_width));
                pangoLayout->set_text(buffer);
                pangoLayout->update_from_cairo_context(cr);
                pangoLayout->add_to_cairo_context(cr);
                cr->stroke();
            }
        }

        // Plot data values
        for (unsigned int i  = 0; i < m_x.size(); ++i) 
        {
            // Cull data to be within the x,y axes
            std::vector<double> cull_x, cull_y, cull_err;
            for (unsigned int j = 0; j < m_x[i].size(); ++j)
            {
                if (m_x[i][j] >= xmin && m_x[i][j] <= xmax && m_y[i][j] >= ymin && m_y[i][j] <= ymax)
                {
                    cull_x.push_back(m_x[i][j]);
                    cull_y.push_back(m_y[i][j]);
                    cull_err.push_back(m_err.at(i).at(j));
                }
            }

            if (cull_x.size() > 0)
            {
                cr->set_source_rgba(m_pens[i].color.r,m_pens[i].color.g,m_pens[i].color.b,m_pens[i].color.a);
                cr->set_line_width(m_pens[i].linewidth);
                cr->move_to((cull_x[0]-xmin)*xscale,(cull_y[0]-ymin)*yscale);
                drawshape(cr,m_pens[i].pointsize,m_pens[i].shape,m_pens[i].filled,m_pens[i].color);
				drawerr(cr, cull_err[0],yscale,m_pens[i].pointsize,m_pens[i].color); 
                for (unsigned int j = 1; j < cull_x.size(); ++j) 
                {
                    if (m_pens[i].linewidth > 0)
                    {
                        cr->line_to((cull_x[j]-xmin)*xscale,(cull_y[j]-ymin)*yscale);
                    }
                    else
					{
                        cr->move_to((cull_x[j]-xmin)*xscale,(cull_y[j]-ymin)*yscale);
					}
                    drawshape(cr,m_pens[i].pointsize,m_pens[i].shape,m_pens[i].filled,m_pens[i].color);
					drawerr(cr,cull_err[j],yscale,m_pens[i].pointsize,m_pens[i].color); 
                }
                cr->stroke();
            }
        }


        // Add zoom box
        if (in_zoom)
        {
            double start, end;
            start = (zoom_start - pad_left-ylab_width-y_bt_size-label_pad);
            end = (zoom_end - pad_left-ylab_width-y_bt_size-label_pad);

            if (start < 0) start = 0;
            if (end < 0) end = 0;
            if (start > xlength) start = xlength;
            if (end > xlength) end = xlength;
            zoom_start_scale = start/xscale+xmin;
            zoom_end_scale = end/xscale+xmin;

            cr->set_line_width(2.0);
            cr->set_source_rgba(0,0,0,0.5);

            cr->move_to(start,0);
            cr->rel_line_to(0,-ylength);
            cr->move_to(end, 0);
            cr->rel_line_to(0,-ylength);
            cr->stroke();

            cr->set_source_rgba(0.2, 0.4, 1.0, 0.5);
            cr->move_to(start,0);
            cr->line_to(end,0);
            cr->line_to(end,-ylength);
            cr->line_to(start,-ylength);
            cr->line_to(start,0);
            cr->fill();
        }

        cr->clip();
    }
    return true;
}

void EasyPlotmm::set_bg_rgba(double r, double g, double b, double a)
{
    bg.r = r;
    bg.g = g;
    bg.b = b;
    bg.a = a;
}


void EasyPlotmm::makeDefaultPens()
{
    Pen p;
    p.linewidth = 1.0;
    p.pointsize = 8.0;
    p.shape = CIRCLE;
    p.filled = true;
    m_defPens.push_back(p);
    p.shape = SQUARE;
    p.filled = false;
    m_defPens.push_back(p);
    p.shape = TRIANGLE;
    p.filled = true;
    m_defPens.push_back(p);
    p.shape = UTRIANGLE;
    p.filled = false;
    m_defPens.push_back(p);
    p.shape = DIAMOND;
    p.filled = true;
    m_defPens.push_back(p);
}

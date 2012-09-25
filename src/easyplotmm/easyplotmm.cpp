/*
Copyright (c) 2011-2012, Brandon Aubie
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "stdafx.h"

#include "easyplotmm.h"
#include <cairomm/context.h>

const double EasyPlotmm::AUTOMATIC = (DBL_MAX -1);
const double EasyPlotmm::NOPOINT = (DBL_MAX -2);

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

	hoveringOnPoint = false;

    m_ixmin = m_xmin;
    m_ixmax = m_xmax;
    m_iymin = m_ymin;
    m_iymax = m_ymax;

	in_zoom = false;
	in_crosshairs = false;

    makeDefaultPens();
    this->add_events(Gdk::BUTTON_PRESS_MASK);
    this->add_events(Gdk::BUTTON_MOTION_MASK);
    this->add_events(Gdk::BUTTON_RELEASE_MASK);
    this->add_events(Gdk::POINTER_MOTION_MASK);
    this->add_events(Gdk::LEAVE_NOTIFY_MASK);
    this->signal_button_press_event().connect(sigc::mem_fun(*this, &EasyPlotmm::on_event_button_press) );
    this->signal_motion_notify_event().connect(sigc::mem_fun(*this, &EasyPlotmm::on_event_motion) );
    this->signal_leave_notify_event().connect(sigc::mem_fun(*this, &EasyPlotmm::on_event_leave) );
    this->signal_button_release_event().connect(sigc::mem_fun(*this, &EasyPlotmm::on_event_button_release) );

    // Fill the context menu
    {
    Gtk::Menu::MenuList& menulist = m_Menu_Popup.items();

    menulist.push_back( Gtk::Menu_Helpers::MenuElem("_Export Data",
        sigc::mem_fun(*this, &EasyPlotmm::export_data ) ) );
    }
    m_Menu_Popup.accelerate(*this);

	this->set_size_request(-1, 40);

}

EasyPlotmm::~EasyPlotmm()
{
}


EasyPlotmm::type_signal_zoom_changed EasyPlotmm::signal_zoom_changed()
{
	return m_signal_zoom_changed;
}

EasyPlotmm::type_signal_hovered_on_point EasyPlotmm::signal_hovered_on_point()
{
	return m_signal_hovered_on_point;
}

EasyPlotmm::type_signal_moved_off_point EasyPlotmm::signal_moved_off_point()
{
	return m_signal_moved_off_point;
}

EasyPlotmm::type_signal_clicked_point EasyPlotmm::signal_clicked_point()
{
	return m_signal_clicked_point;
}


double EasyPlotmm::automatic()
{
	return AUTOMATIC;
}

void EasyPlotmm::export_data()
{
    Gtk::FileChooserDialog dialog("Choose a file to export the data to",
        Gtk::FILE_CHOOSER_ACTION_SAVE);
//    dialog.set_transient_for(this->get_parent_window());

    // Add response buttons to the dialog:
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::SAVE, Gtk::RESPONSE_OK);

	if (lastdir != "") dialog.set_current_folder(lastdir);

    // Show the dialog
    int result = dialog.run();

    switch(result)
    {
        case(Gtk::RESPONSE_OK):
            Gio::init();
            Glib::ustring filename = dialog.get_filename();
            lastdir = dialog.get_current_folder();
            int fileIndex = 0;
			int filesToExport = 0;
            for (unsigned int i = 0; i < m_x.size(); ++i)
            {
                if (m_exportable.at(i))
                {
					filesToExport++;
				}
			}
            for (unsigned int i = 0; i < m_x.size(); ++i)
            {
                if (m_exportable.at(i))
                {
                    ++fileIndex;
                    std::ostringstream ssIn;
					if (filesToExport > 1 ) {
						ssIn << filename << "_" << fileIndex;
					} else {
						ssIn << filename;
					}
                    Glib::ustring datafilename = ssIn.str();
                    try
                    {
                        Glib::RefPtr<Gio::File> file = Gio::File::create_for_path(datafilename);
                        if (!file)
                            std::cerr << "Gio::File::create_for_path() returned an empty RefPtr." << std::endl;

                        Glib::RefPtr<Gio::FileOutputStream> stream;

                        if(file->query_exists())
                            stream = file->replace();
                        else
                            stream = file->create_file();

                        if (!stream)
                            std::cerr << "Gio::File::create_file() returned an empty RefPtr." << std::endl;

                        std::ostringstream contents;
                        for (unsigned int point = 0; point < m_x.at(i).size(); ++point)
                        {
                            contents << m_x.at(i).at(point) << " ";
							if (m_y.at(i).at(point) != NOPOINT) {
								contents << m_y.at(i).at(point) << " ";
								contents << m_err.at(i).at(point) << std::endl;
							} else {
								contents << "-" << " ";
								contents << "-" << " " << std::endl;
							}
                        }
                        const int bytes_written = stream->write(contents.str());

                        if (bytes_written == -1) 
                        {
                            std::cerr << "Error: Unable to write to file " << datafilename << std::endl;
                        }

                        stream->close();
                        stream.reset();
                    }
                    catch(const Glib::Exception& ex)
                    {
                        std::cerr << "Exception caught: " << ex.what() << std::endl;
                    }
                }
            }
    		break;
	}
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
    // Only respond to button clicks when we have a graph
	if (m_x.empty() || m_y.empty() || m_err.empty()) return false;

    switch(event->type)
    {
        case GDK_BUTTON_PRESS:
			if (curPointUnderMouse != -1 )
			{
				try {
				m_signal_clicked_point.emit(m_x.at(curSetUnderMouse).at(curPointUnderMouse),
										    m_y.at(curSetUnderMouse).at(curPointUnderMouse),
										    m_names.at(curSetUnderMouse).at(curPointUnderMouse),
										    m_data.at(curSetUnderMouse).at(curPointUnderMouse));
				} catch (std::out_of_range& /*o*/) {
					m_signal_clicked_point.emit(0,0,"Error","");
				}
			}
            if ((event->state & GDK_MOD1_MASK) && event->button == 1)
            {
               in_crosshairs = has_plot;
			   ch_x = event->x;
			   ch_y = event->y;
               redraw();
            }
            else if (event->button == 1)
            {
               in_zoom = has_plot;
               zoom_start = event->x;
               zoom_end = event->x;
               redraw();
            }
            else if (event->button == 3)
            {
                // Show the context menu
                m_Menu_Popup.popup(event->button, event->time);
            }
            break;

        case GDK_2BUTTON_PRESS:
            in_zoom = false;
            m_xmin = m_ixmin;
            m_xmax = m_ixmax;
			m_signal_zoom_changed.emit(m_xmin,m_xmax);
            redraw();
            break;
        default:
            break;

    }
    return true;
}

bool EasyPlotmm::on_event_leave(GdkEventCrossing* /*event*/)
{
	motionID.disconnect();
	return true;
}
bool EasyPlotmm::on_event_motion(GdkEventMotion* event)
{
	if (hoveringOnPoint) {
		m_signal_moved_off_point.emit();
		hoveringOnPoint = false;
	}
	// Assume no point at first
	curPointUnderMouse = -1;
	mouse_x = event->x;
	mouse_y = event->y;
	motionID.disconnect();
	motionID = Glib::signal_timeout().connect(sigc::mem_fun(*this, &EasyPlotmm::checkMousePosition), 100);	
    if (in_zoom)
    {
        zoom_end = event->x;
        redraw();
    } else if (in_crosshairs)
	{
		ch_x = event->x;
		ch_y = event->y;
		redraw();
	}
    return true;
}

bool EasyPlotmm::checkMousePosition()
{
	checkForPointUnderCursor = true;
	redraw();
	// Cancel event
	return false;
}

void EasyPlotmm::cursorHoveredOverPoint(const int setIndex, const int pointIndex)
{
	checkForPointUnderCursor = false;
	curPointUnderMouse = pointIndex;
	curSetUnderMouse = setIndex;

	try {
	m_signal_hovered_on_point.emit(m_x.at(setIndex).at(pointIndex),
								   m_y.at(setIndex).at(pointIndex),
								   m_names.at(setIndex).at(pointIndex),
								   m_data.at(setIndex).at(pointIndex));
	} catch (std::out_of_range& /*o*/) {
		m_signal_hovered_on_point.emit(0, 0, "Error", "");
	}
	hoveringOnPoint = true;
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
		   m_signal_zoom_changed.emit(m_xmin,m_xmax);
           redraw();
       }
    }
	if (event->button == 3)
	{
    	in_crosshairs = false;
		redraw();
	}
    return true;
}

void EasyPlotmm::xname(std::string name)
{
	m_xname = name;
}

void EasyPlotmm::yname(std::string name)
{
	m_yname = name;
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

    for (unsigned int i=0; i < m_x.size(); ++i) {
        m_x.at(i).clear();
    }
    m_x.clear();

    for (unsigned int i=0; i < m_y.size(); ++i) {
        m_y.at(i).clear();
    }
	m_y.clear();

    for (unsigned int i=0; i < m_err.size(); ++i) {
        m_err.at(i).clear();
    }
	m_err.clear();

    for (unsigned int i=0; i < m_names.size(); ++i) {
        m_names.at(i).clear();
    }
	m_names.clear();

    for (unsigned int i=0; i < m_data.size(); ++i) {
        m_data.at(i).clear();
    }
	m_data.clear();

	m_pens.clear();
	m_xname = "";
	m_yname = "";
    curPen = 0;
	m_xmin=m_xmax=m_ymin=m_ymax=AUTOMATIC;
	xname_width = 0;
	xname_height = 0;
	yname_width = 0;
	yname_height = 0;
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
    plot(x,y,err,getPen(),true);
}

void EasyPlotmm::plot(std::vector<double> x, std::vector<double> y, bool exportable)
{
	std::vector<double> err(x.size(),0);
    plot(x,y,err,getPen(),exportable);    
}

void EasyPlotmm::plot(std::vector<double> x, std::vector<double> y, Pen p)
{
    // Add plot to vectors
	std::vector<double> err(x.size(),0);
    plot(x,y,err,p,true);    
}

void EasyPlotmm::plot(std::vector<double> x, std::vector<double> y, Pen p, bool exportable)
{
    // Add plot to vectors
	std::vector<double> err(x.size(),0);
    plot(x,y,err,p, exportable);    
}

void EasyPlotmm::plot(std::vector<double> x, std::vector<double> y, std::vector<double> err)
{
    // Add plot to vectors
    plot(x,y,err,getPen(),true);    
}

void EasyPlotmm::plot(std::vector<double> x, std::vector<double> y, std::vector<double> err, bool exportable)
{
    // Add plot to vectors
    plot(x,y,err,getPen(),exportable);    
}

void EasyPlotmm::plot(std::vector<double> x, std::vector<double> y, std::vector<double> err, Pen p)
{
    plot(x,y,err,p,true);
}

void EasyPlotmm::plot(std::vector<double> x, std::vector<double> y, std::vector<double> err, Pen p, bool exportable)
{
	if (x.empty() || y.empty() || err.empty()) return;

    // Add plot to vectors
    m_x.push_back(x);
    m_y.push_back(y);
	m_err.push_back(err);
    m_pens.push_back(p);
    m_exportable.push_back(exportable);

	// Add our blank names and data to be overwritten by a call to setPointNames or setPointData
	std::vector<std::string> blankStrings(x.size(),"");
	m_names.push_back(blankStrings);
	m_data.push_back(blankStrings);

    // Force Redraw
    redraw();

    has_plot = true;
}

void EasyPlotmm::setPointNames(std::vector<std::string> names)
{
	if (names.empty()) return;
	m_names.pop_back();
	m_names.push_back(names);
}

void EasyPlotmm::setPointData(std::vector<std::string> data)
{
	if (data.empty()) return;
	m_data.pop_back();
	m_data.push_back(data);
}


void EasyPlotmm::plotHist(std::vector<double> /*x*/, std::vector<double> /*y*/, std::vector<double> /*err*/, Pen /*p*/)
{

}


void EasyPlotmm::drawshape(Cairo::RefPtr<Cairo::Context> cr, double s, Shape shape, bool filled, RGBA col)
{
    if (s <= 0) return;
    double h;
	double x,y;
	cr->get_current_point(x,y);
	cr->begin_new_path();
    cr->move_to(x,y);
    cr->set_line_width(1.0);
    cr->set_source_rgba(col.r,col.g,col.b,col.a);
    switch(shape)
    {
        case POINT:
            cr->set_line_width(s);
            cr->rel_move_to(-s/2,0);
            cr->rel_line_to(s,0);
            cr->rel_move_to(-s/2,0);
			cr->stroke();
            break;

        case CIRCLE:
            cr->rel_move_to(s/2,0);
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
			cr->stroke();
            break;

        case DIAMOND:
            h = 1.41421356*s; // sqrt(2)*s
            cr->rel_move_to(0,-h/2);
            cr->rel_line_to(-h/2,h/2);
            cr->rel_line_to(h/2,h/2);
            cr->rel_line_to(h/2,-h/2);
            cr->rel_line_to(-h/2,-h/2);
            cr->rel_move_to(0,h/2);
			cr->stroke();
            break;

        case TRIANGLE:
            h = sqrt(s*s-(0.25*s*s));
            cr->rel_move_to(-s/2,h/2);
            cr->rel_line_to(s,0);
            cr->rel_line_to(-s/2,-h);
            cr->rel_line_to(-s/2,h);
            // Move back to the middle
            cr->rel_move_to(s/2,-h/2);
			cr->stroke();
            break; 

        case UTRIANGLE:
            h = sqrt(s*s-(0.25*s*s));
            cr->rel_move_to(-s/2,-h/2);
            cr->rel_line_to(s,0);
            cr->rel_line_to(-s/2,h);
            cr->rel_line_to(-s/2,-h);
            // Move back to the middle
            cr->rel_move_to(s/2,h/2);
			cr->stroke();
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

	double x, y;
	cr->get_current_point(x,y);
    cr->move_to(x,y);
    cr->set_line_width(1.0);
    cr->set_source_rgba(col.r,col.g,col.b,col.a);
	cr->rel_move_to(-size, err*scale);
	cr->rel_line_to(2*size, 0);
	cr->rel_move_to(-size, 0);
	cr->rel_line_to(0, -2*err*scale);
	cr->rel_move_to(-size, 0);
	cr->rel_line_to(2*size, 0);
	cr->rel_move_to(-size, err*scale);
}

void EasyPlotmm::showError(std::string err)
{
    Glib::RefPtr<Gdk::Window> window = get_window();
	Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();
    cr->set_line_width(0.5);
	Pango::FontDescription name_font_descr("sans normal 10");
	Glib::RefPtr<Pango::Layout> pangoLayout = Pango::Layout::create (cr);
	pangoLayout->set_font_description(name_font_descr);
	pangoLayout->set_text(err);
	pangoLayout->update_from_cairo_context(cr);
	cr->move_to(10,10);
	pangoLayout->add_to_cairo_context(cr);
	cr->stroke();
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
        if (m_x.size() != m_y.size())
        {
			showError("X and Y list lengths do not match.");
            return true;
        }
        if (m_x.size() != m_pens.size())
        {
            std::cout << "ERROR: Only " << m_pens.size() << " pens for " << m_x.size() << " plots." << std::endl;
            return true;
        }
        if (m_x.size() != m_err.size())
        {
			showError("X and ERR list lengths do not match.");
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
				showError("X and Y list lengths do not match.");
                return true;
            }
            for (unsigned int x = 0; x < m_x[xs].size(); ++x) {
                xmin = xmin > m_x[xs][x] ? m_x[xs][x] : xmin;
                xmax = xmax < m_x[xs][x] ? m_x[xs][x] : xmax;
            }
        }
        for (unsigned int ys = 0; ys < m_y.size(); ++ys) {
            for (unsigned int y = 0; y < m_y[ys].size(); ++y) {
                ymin = ymin > m_y[ys][y]-m_err[ys][y] && m_y[ys][y] != NOPOINT ? m_y[ys][y]-m_err[ys][y] : ymin;
                ymax = ymax < m_y[ys][y]+m_err[ys][y] && m_y[ys][y] != NOPOINT ? m_y[ys][y]+m_err[ys][y] : ymax;
            }
        }

		// If we can't get reasonable limits then something is up
		if (xmax == -DBL_MAX || ymax == -DBL_MAX || xmin == DBL_MAX || ymin == DBL_MAX) {
			showError("No Data To Plot!");
			return true;
		}
		if (ymin <= 1 && ymin > 0) ymin = 0;

        if (m_xmin != AUTOMATIC) xmin = m_xmin;
        if (m_xmax != AUTOMATIC) xmax = m_xmax;
        if (m_ymin != AUTOMATIC) ymin = m_ymin;
        if (m_ymax != AUTOMATIC) ymax = m_ymax;

        double maxX = (xmin)>(xmax)?(xmin):(xmax);
        double maxY = (ymin)>(ymax)?(ymin):(ymax);
        double minX = (xmin)>(xmax)?(xmax):(xmin);
        double minY = (ymin)>(ymax)?(ymax):(ymin);

        double xorder = 1;
        double yorder = 1;

        if ((maxX-minX) > 10) while ((maxX-minX) / xorder > 10) { xorder *= 10; }
        if ((maxX-minX) != 0 && (maxX-minX) <= 10) while ((maxX-minX) / xorder <= 1) { xorder /= 10; }
        if ((maxY-minY) > 10) while ((maxY-minY) / yorder > 10) { yorder *= 10; }
        if ((maxY-minY) != 0 && (maxY-minY) <= 10) while ((maxY-minY) / yorder <= 1 ) { yorder /= 10; }

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

        // Determine the number of required decimal places
        int y_numdec = 0;
        char y_fmt[7];
		if (Ybt < 1) y_numdec = 1;
		if (Ybt < 0.1) y_numdec = 2;
		if (Ybt < 0.01) y_numdec = 3;
		if (Ybt < 0.001) y_numdec = 4;
		if (Ybt < 0.0001) y_numdec = 5;
        sprintf(y_fmt, "%%.%df", y_numdec);

        int x_numdec = 0;
        char x_fmt[7];
		if (Xbt < 1) x_numdec = 1;
		if (Xbt < 0.1) x_numdec = 2;
		if (Xbt < 0.01) x_numdec = 3;
		if (Xbt < 0.001) x_numdec = 4;
		if (Xbt < 0.0001) x_numdec = 5;
        sprintf(x_fmt, "%%.%df", x_numdec);

		// Always make sure we have a 0 anchor if the graph is +ve and -ve
		// It's not really NEEDED, but it's a nice asethetic feature.
		if (ymin < 0 && ymax > 0 && m_ymin == AUTOMATIC) {
			// Make sure that x in ymin+x*Xbt=0 is an integer
			// Otherwise, shift ymin down until it is

			if (-1*ymin/Ybt != floor(-1*ymin/Ybt))
			{
				// Diff is the fraction of a step we have to move down
				double diff = 1-(-1*ymin/Ybt - floor(-1*ymin/Ybt));
				ymin -= diff*Ybt;
				ymin_st = ymin;
				ymin_bt = ymin;
			}
		}

        Glib::RefPtr<Pango::Layout> pangoLayout = Pango::Layout::create (cr);
        char buffer[10];
        if (y_numdec == 0) sprintf(buffer,"%i",(int)(ymax_bt));
        else sprintf(buffer,y_fmt,ymax_bt); 
        Pango::FontDescription label_font_descr("sans normal 9");
        Pango::FontDescription name_font_descr("sans normal 10");
        pangoLayout->set_font_description(label_font_descr);

        pangoLayout->set_text(buffer);
        pangoLayout->update_from_cairo_context(cr);
        pangoLayout->get_size(ylab_width,lab_height);
        ylab_width /= Pango::SCALE;
        lab_height /= Pango::SCALE;

		if (m_xname != "")
		{
			pangoLayout->set_font_description(name_font_descr);
			pangoLayout->set_text(m_xname);
			pangoLayout->get_size(xname_width,xname_height);
			pangoLayout->update_from_cairo_context(cr);
			xname_width /= Pango::SCALE;
			xname_height /= Pango::SCALE;
		}
		if (m_yname != "")
		{
			pangoLayout->set_font_description(name_font_descr);
			pangoLayout->set_text(m_yname);
			pangoLayout->get_size(yname_height,yname_width);
			pangoLayout->update_from_cairo_context(cr);
			yname_width /= Pango::SCALE;
			yname_height /= Pango::SCALE;
		}

        const double xlength = width-pad_left-pad_right-ylab_width-y_bt_size-label_pad-yname_width;
        const double ylength = height-pad_top-pad_bottom-lab_height-x_bt_size-label_pad-xname_height;
        double xscale = xlength/(xmax-xmin);
        double yscale = -ylength/(ymax-ymin);

        // Translate the window so the bottom left of the graph is
        // (0,0) on the screen
		double x_translate = pad_left+ylab_width+y_bt_size+2*label_pad+yname_width;
		double y_translate = height-pad_bottom-lab_height-x_bt_size-2*label_pad-xname_height;
        cr->translate(x_translate, y_translate);

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

        if (Xst != 0)
        {
            for (double x = xmin_st; x <= xmax_st; x+=Xst)
            {
                cr->move_to((x-xmin)*xscale, 0);
                cr->rel_line_to(0, x_st_size);
                cr->stroke();
            }
            for (double x = xmin_bt; x <= xmax_bt; x+=Xbt)
            {
                cr->move_to((x-xmin)*xscale, 0);
                cr->rel_line_to(0, x_bt_size);
                cr->stroke();
            }
        }
        if (Yst != 0)
        {
            for (double y = ymin_st; y <= ymax_st; y+=Yst)
            {
                cr->move_to(0,(y-ymin)*yscale);
                cr->rel_line_to(-y_st_size, 0);
                cr->stroke();
            }
            for (double y = ymin_bt; y <= ymax_bt; y+=Ybt)
            {
                cr->move_to(0,(y-ymin)*yscale);
                cr->rel_line_to(-y_bt_size, 0);
                cr->stroke();
            }
        }
        cr->restore();

		// Add x label
		if (m_xname != "")
		{
			Glib::RefPtr<Pango::Layout> pangoLayout = Pango::Layout::create (cr);
			pangoLayout->set_font_description(name_font_descr);
			pangoLayout->set_text(m_xname);
			pangoLayout->update_from_cairo_context(cr);
			cr->move_to(xscale*(xmax-xmin)/2-(xname_width)/2.0,x_bt_size+xname_height+2*label_pad);
			pangoLayout->add_to_cairo_context(cr);
		}

		// Add y label
		if (m_yname != "")
		{
			Glib::RefPtr<Pango::Layout> pangoLayout = Pango::Layout::create (cr);
			pangoLayout->set_font_description(name_font_descr);
			pangoLayout->set_text(m_yname);
			pangoLayout->update_from_cairo_context(cr);
			cr->move_to(-y_bt_size-yname_width-3*label_pad-ylab_width, yscale*(ymax-ymin)/2+(yname_height)/2);
			cr->save();
		    cr->rotate(-3.14159/2);
			pangoLayout->add_to_cairo_context(cr);
			cr->restore();
		}

        // Add the tick labels
        cr->set_source_rgba(0,0,0,1);
        cr->set_line_width(1.0);
        if (Xbt != 0)
        {
            for (double x = xmin_bt; x <= xmax_bt; x+=Xbt)
            {
                Glib::RefPtr<Pango::Layout> pangoLayout = Pango::Layout::create (cr);
                char buffer[10];
                if (x_numdec == 0) sprintf(buffer,"%i",(int)x);
                else sprintf(buffer,x_fmt,x); 
                pangoLayout->set_font_description(label_font_descr);
                pangoLayout->set_text(buffer);
                pangoLayout->update_from_cairo_context(cr);
                int w,h;
                pangoLayout->get_size(w,h);
                cr->move_to((x-xmin)*xscale-(w/Pango::SCALE)/2.0,x_bt_size+label_pad);
                pangoLayout->add_to_cairo_context(cr);
            }
        }
        if (Ybt != 0)
        {
            for (double y = ymin_bt; y <= ymax_bt; y+=Ybt)
            {
                Glib::RefPtr<Pango::Layout> pangoLayout = Pango::Layout::create (cr);
                cr->move_to(-ylab_width-y_bt_size-2*label_pad,(y-ymin)*yscale-lab_height/2);
                char buffer[10];
                if (y_numdec == 0) sprintf(buffer,"%i",(int)y);
                else sprintf(buffer,y_fmt,y); 
                pangoLayout->set_font_description(label_font_descr);
                pangoLayout->set_alignment(Pango::ALIGN_RIGHT);
                pangoLayout->set_width(Pango::SCALE*(ylab_width));
                pangoLayout->set_text(buffer);
                pangoLayout->update_from_cairo_context(cr);
                pangoLayout->add_to_cairo_context(cr);
            }
        }
		cr->stroke();

        // Plot data values
        for (unsigned int i  = 0; i < m_x.size(); ++i) 
        {
            // Cull data to be within the x,y axes
			std::vector<double> cull_x, cull_y, cull_err;
			std::vector<int> cull_indexes;
            for (unsigned int j = 0; j < m_x[i].size(); ++j)
            {
                if (
					m_x[i][j] >= xmin &&
					m_x[i][j] <= xmax &&
					m_y[i][j] >= ymin &&
					m_y[i][j] <= ymax
				   )
                {
                    cull_x.push_back(m_x.at(i).at(j));
                    cull_y.push_back(m_y.at(i).at(j));
                    cull_err.push_back(m_err.at(i).at(j));
					cull_indexes.push_back(j);
                } else {

					// Draw lines in zoomed in view even if end points are not visible if robost
					if (m_pens[i].robustLines) {
						if (j < m_x[i].size()-1) {
							if (m_x[i][j] < xmin && m_x[i][j+1] >= xmin) {
								cull_x.push_back(xmin);
								cull_y.push_back(m_y.at(i).at(j));
								cull_err.push_back(0);
							}
						}
						if (j > 0) {
							if (m_x[i][j-1] < xmax && m_x[i][j] >= xmax) {
								cull_x.push_back(xmax);
								cull_y.push_back(m_y.at(i).at(j));
								cull_err.push_back(0);
							}
						}
					}
				}
			}


            if (!cull_x.empty())
            {
			 	// First draw the error bars
                cr->move_to((cull_x[0]-xmin)*xscale,(cull_y[0]-ymin)*yscale);
				if (cull_y[0] != NOPOINT) drawerr(cr, cull_err[0],yscale,m_pens[i].pointsize,m_pens[i].errcolor); 
                for (unsigned int j = 1; j < cull_x.size(); ++j) 
                {
					cr->move_to((cull_x[j]-xmin)*xscale,(cull_y[j]-ymin)*yscale);
					if (cull_y[j] != NOPOINT) drawerr(cr,cull_err[j],yscale,m_pens[i].pointsize,m_pens[i].errcolor); 
                }
				cr->stroke();

				// Next draw the lines
				if (m_pens[i].linewidth > 0)
				{
					bool onLine = false;
					cr->set_source_rgba(m_pens[i].color.r,m_pens[i].color.g,m_pens[i].color.b,m_pens[i].color.a);
					cr->set_line_width(m_pens[i].linewidth);
					if (cull_y[0] != NOPOINT) { cr->move_to((cull_x[0]-xmin)*xscale,(cull_y[0]-ymin)*yscale); onLine = true; }
					for (unsigned int j = 1; j < cull_x.size(); ++j) 
					{
						if (cull_y[j] != NOPOINT) {
							if (onLine)	cr->line_to((cull_x[j]-xmin)*xscale,(cull_y[j]-ymin)*yscale);
							onLine = true;
						}
						else {
							onLine = false;
						}
					}
					cr->stroke();
				}

				// Next draw the shapes
                cr->move_to((cull_x[0]-xmin)*xscale,(cull_y[0]-ymin)*yscale);
				if (cull_y[0] != NOPOINT)
				{
					// Check for mouse positions
					if (checkForPointUnderCursor) {
						if (mouse_x-x_translate > (cull_x[0]-xmin)*xscale-m_pens[i].pointsize*0.5 &&
							mouse_x-x_translate < (cull_x[0]-xmin)*xscale+m_pens[i].pointsize*0.5) {
							if (mouse_y-y_translate > (cull_y[0]-ymin)*yscale-m_pens[i].pointsize*0.5 &&
								mouse_y-y_translate < (cull_y[0]-ymin)*yscale+m_pens[i].pointsize*0.5) {
								this->cursorHoveredOverPoint(i,cull_indexes.at(0));
							}
						}
					}
					drawshape(cr,m_pens[i].pointsize,m_pens[i].shape,m_pens[i].filled,m_pens[i].color);
				} 
				else {
					drawshape(cr,m_pens[i].pointsize,NONE,m_pens[i].filled,m_pens[i].color);
				}

                for (unsigned int j = 1; j < cull_x.size(); ++j) 
                {
					cr->move_to((cull_x[j]-xmin)*xscale,(cull_y[j]-ymin)*yscale);
					if (cull_y[j] != NOPOINT)
					{
						// Check for mouse positions
						if (checkForPointUnderCursor) {
							if (mouse_x-x_translate > (cull_x[j]-xmin)*xscale-m_pens[i].pointsize*0.5 &&
								mouse_x-x_translate < (cull_x[j]-xmin)*xscale+m_pens[i].pointsize*0.5) {
								if (mouse_y-y_translate > (cull_y[j]-ymin)*yscale-m_pens[i].pointsize*0.5 &&
									mouse_y-y_translate < (cull_y[j]-ymin)*yscale+m_pens[i].pointsize*0.5) {
									this->cursorHoveredOverPoint(i,cull_indexes.at(j));
								}
							}
						}
						drawshape(cr,m_pens[i].pointsize,m_pens[i].shape,m_pens[i].filled,m_pens[i].color);
					} else {
						drawshape(cr,m_pens[i].pointsize,NONE,m_pens[i].filled,m_pens[i].color);
					}
                }
				cr->stroke();
            }
        }

        // Add zoom box
        if (in_zoom)
        {
            double start, end;
            start = (zoom_start - pad_left-ylab_width-y_bt_size-2*label_pad-yname_width);
            end = (zoom_end - pad_left-ylab_width-y_bt_size-2*label_pad-yname_width);

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

		// Add crosshairs
		if (in_crosshairs)
		{
            double y = (ch_y - ylength-pad_top);//-pad_bottom-lab_height-x_bt_size-2*label_pad-xname_height);
            double x = (ch_x - pad_left-ylab_width-y_bt_size-2*label_pad-yname_width);
			cr->set_line_width(2.0);

			// Draw horizonal line
			cr->set_source_rgba(0,0.5,0,0.75);
			cr->move_to(0, y);
			cr->rel_line_to(xlength,0);
			cr->stroke();

			// Draw vertical line
			cr->set_source_rgba(0.5,0,0.5,0.75);
			cr->move_to(x, 0);
			cr->rel_line_to(0,-ylength);
			cr->stroke();
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
    p.color.r = 0;
    p.color.g = 0;
    p.color.b = 0;
    p.errcolor.r = 0.5;
    p.errcolor.g = 0.5;
    p.errcolor.b = 0.5;

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

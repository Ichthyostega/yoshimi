/*
    WidgetPDial.cpp

    Original ZynAddSubFX author Nasca Octavian Paul
    Copyright (C) 2002-2005 Nasca Octavian Paul
    Copyright 2009-2010, Alan Calvert
    Copyright 2016 Will Godfrey
    Copyright 2017 Jesper Lloyd
    Copyright 2020-2025, Will Godfrey & others

    This file is part of yoshimi, which is free software: you can redistribute
    it and/or modify it under the terms of the GNU General Public
    License as published by the Free Software Foundation; either version 2 of
    the License, or (at your option) any later version.

    yoshimi is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.   See the GNU General Public License (version 2 or
    later) for more details.

    You should have received a copy of the GNU General Public License along with
    yoshimi; if not, write to the Free Software Foundation, Inc., 51 Franklin
    Street, Fifth Floor, Boston, MA  02110-1301, USA.

    This file is a derivative of the ZynAddSubFX original.

*/

#include "WidgetPDial.h"
#include "Misc/NumericFuncs.h"

#include <FL/fl_draw.H>
#include <FL/Fl_Tooltip.H>
#include <FL/Fl_Group.H>

#include <FL/platform.H>
#include <cairo.h>
#include <cairo-xlib.h>

using func::limit;

// Prior to fltk 1.4, there was no built in support for handling screen scales.
// This may not be the correct way to handle the scale factor.
inline float get_scale_factor(Fl_Widget *widget) {
#if FL_API_VERSION > 10400
    Fl_Window* window = widget->window();
    return Fl::screen_scale(window ? window->screen_num() : 0);
#else
    return 1.0f;
#endif
}

WidgetPDial::WidgetPDial(int x,int y, int w, int h, const char *label) : Fl_Dial(x,y,w,h,label)
{
    Fl_Group *save = Fl_Group::current();
    dyntip = new DynTooltip();
    Fl_Group::current(save);

    oldvalue = 0.0;
}

WidgetPDial::~WidgetPDial()
{
    delete dyntip;
}

void WidgetPDial::setValueType(ValueType type_)
{
    dyntip->setValueType(type_);
}

void WidgetPDial::setGraphicsType(ValueType type_)
{
    dyntip->setGraphicsType(type_);
}

void WidgetPDial::tooltip(const char * tip)
{
    if (tip)
        dyntip->setTooltipText(tip);
}

/*
  Override these Fl_Valuator methods to update
  the tooltip value when the widget value is changed.
*/
void WidgetPDial::value(double val)
{
    Fl_Valuator::value(val);
    dyntip->setValue(val);
    dyntip->setOnlyValue(true);
}

double WidgetPDial::value()
{
    return Fl_Valuator::value();
}

int WidgetPDial::handle(int event)
{

    double dragsize, v, min = minimum(), max = maximum();
    int my, mx;
    int res = 0;

    switch (event)
    {
    case FL_PUSH:
    case FL_DRAG: // done this way to suppress warnings
        if (event == FL_PUSH)
        {
            Fl::belowmouse(this); /* Ensures other widgets receive FL_RELEASE */
            do_callback();
            oldvalue = value();
        }
        my = -((Fl::event_y() - y()) * 2 - h());
        mx = ((Fl::event_x() - x()) * 2 - w());
        my = (my + mx);

        dragsize = 200.0;
        if (Fl::event_state(FL_CTRL) != 0)
            dragsize *= 10;
        else if (Fl::event_button() == 2)
            dragsize *= 3;
        if (Fl::event_button() != 3)
        {
            v = oldvalue + my / dragsize * (max - min);
            v = clamp(v);
            value(v);
            value_damage();
            if (this->when() != 0)
                do_callback();
        }
        res = 1;
        break;
    case FL_MOUSEWHEEL:
        if (!Fl::event_inside(this))
        {
            return 1;
        }
        my = - Fl::event_dy();
        dragsize = 25.0f;
        if (Fl::event_state(FL_CTRL) != 0)
            dragsize *= 5; // halved this for better fine resolution
        value(limit(value() + my / dragsize * (max - min), min, max));
        value_damage();
        if (this->when() != 0)
            do_callback();
        res = 1;
        break;
    case FL_ENTER:
        res = 1;
        break;
    case FL_HIDE:
    case FL_UNFOCUS:
        break;
    case FL_LEAVE:
        res = 1;
        break;
    case FL_RELEASE:
        if (this->when() == 0)
            do_callback();
        res = 1;
        break;
    }

    dyntip->setValue(value());
    dyntip->tipHandle(event);
    return res;
}

void WidgetPDial::drawgradient(int cx,int cy,int sx,double m1,double m2)
{
    for (int i = (int)(m1 * sx); i < (int)(m2 * sx); ++i)
    {
        double tmp = 1.0 - powf(i * 1.0 / sx, 2.0);
        pdialcolor(140 + (int) (tmp * 90), 140 + (int)(tmp * 90), 140 + (int)(tmp * 100));
        fl_arc(cx + sx / 2 - i / 2, cy + sx / 2 - i / 2, i, i, 0, 360);
    }
}

void WidgetPDial::draw()
{
    float scale = get_scale_factor(this);
    int cx = x() * scale, cy = y() * scale, sx = w() * scale, sy = h() * scale;
    double d = (sx>sy)?sy:sx; // d = the smallest side -2
    double dh = d/2;
       /*
        * doing away with the fixed outer band. It's out of place now!
        * fl_color(170,170,170);
        * fl_pie(cx - 2, cy - 2, d + 4, d + 4, 0, 360);
        */
    double val = (value() - minimum()) / (maximum() - minimum());

  ///////////////////////////////////////OOO Strip-down : do not call cairo_xlib_surface_create()
    Fl_Dial::draw();
}

inline void WidgetPDial::pdialcolor(int r,int g,int b)
{
    if (active_r())
        fl_color(r, g, b);
    else
        fl_color(160 - (160 - r) / 3, 160 - (160 - b) / 3, 160 - (160 - b) / 3);
}

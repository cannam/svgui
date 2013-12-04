/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006-2013 Chris Cannam and QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "LogNumericalScale.h"
#include "VerticalScaleLayer.h"

#include "base/LogRange.h"

#include <QPainter>

#include <cmath>

#include "view/View.h"

//#define DEBUG_TIME_VALUE_LAYER 1

int
LogNumericalScale::getWidth(View *,
			    QPainter &paint)
{
    return paint.fontMetrics().width("-000.00") + 10;
}

void
LogNumericalScale::paintVertical(View *v,
				 const VerticalScaleLayer *layer,
				 QPainter &paint,
				 int x0,
				 float minlog,
				 float maxlog)
{
    int w = getWidth(v, paint) + x0;

    int n = 10;

    float val = minlog;
    float inc = (maxlog - val) / n; // even increments of log scale

    // smallest increment as displayed
    float minDispInc = LogRange::unmap(minlog + inc) - LogRange::unmap(minlog);

#ifdef DEBUG_TIME_VALUE_LAYER
    cerr << "min = " << minlog << ", max = " << maxlog << ", inc = " << inc << ", minDispInc = " << minDispInc << endl;
#endif

    char buffer[40];

    float round = 1.f;
    int dp = 0;

    if (minDispInc > 0) {
        int prec = trunc(log10f(minDispInc));
        if (prec < 0) dp = -prec;
        round = powf(10.f, prec);
#ifdef DEBUG_TIME_VALUE_LAYER
        cerr << "round = " << round << ", prec = " << prec << ", dp = " << dp << endl;
#endif
    }

    int prevy = -1;
                
    for (int i = 0; i < n; ++i) {

	int y, ty;
        bool drawText = true;

	if (i == n-1 &&
	    v->height() < paint.fontMetrics().height() * (n*2)) {
	    if (layer->getScaleUnits() != "") drawText = false;
	}

        float dispval = LogRange::unmap(val);
	dispval = floor(dispval / round) * round;

#ifdef DEBUG_TIME_VALUE_LAYER
	cerr << "val = " << val << ", dispval = " << dispval << endl;
#endif

	y = layer->getYForValue(v, dispval);

	ty = y - paint.fontMetrics().height() + paint.fontMetrics().ascent() + 2;
	
	if (prevy >= 0 && (prevy - y) < paint.fontMetrics().height()) {
	    val += inc;
	    continue;
        }

	double dv = dispval;
	int digits = trunc(log10f(dv));
	int sf = dp + (digits > 0 ? digits : 0);
	if (sf < 4) sf = 4;
	sprintf(buffer, "%.*g", sf, dv);

	QString label = QString(buffer);

	paint.drawLine(w - 5, y, w, y);

        if (drawText) {
	    paint.drawText(w - paint.fontMetrics().width(label) - 6,
			   ty, label);
        }

        prevy = y;
	val += inc;
    }
}

/* -*- c-basic-offset: 4 -*-  vi:set ts=8 sts=4 sw=4: */

/*
  A waveform viewer and audio annotation editor.
  Chris Cannam, Queen Mary University of London, 2005-2006
    
  This is experimental software.  Not for distribution.
*/

/**
 * A rotary dial widget.
 *
 * Based on an original design by Thorsten Wilms.
 *
 * Implemented as a widget for the Rosegarden MIDI and audio sequencer
 * and notation editor by Chris Cannam.
 *
 * Extracted into a standalone Qt3 widget by Pedro Lopez-Cabanillas
 * and adapted for use in QSynth.
 * 
 * Ported to Qt4 by Chris Cannam.
 *
 * This file copyright 2003-2005 Chris Cannam, copyright 2005 Pedro
 * Lopez-Cabanillas.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.  See the file
 * COPYING included with this distribution for more information.
 */

#include "AudioDial.h"

#include <cmath>
#include <iostream>

#include <QTimer>
#include <QPainter>
#include <QPixmap>
#include <QColormap>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QInputDialog>

using std::endl;
using std::cerr;


//!!! Pedro updated his version to use my up/down response code from RG -- need to grab that code in preference to this version from Rui


//-------------------------------------------------------------------------
// AudioDial - Instance knob widget class.
//

#define AUDIO_DIAL_MIN (0.25 * M_PI)
#define AUDIO_DIAL_MAX (1.75 * M_PI)
#define AUDIO_DIAL_RANGE (AUDIO_DIAL_MAX - AUDIO_DIAL_MIN)


// Constructor.
AudioDial::AudioDial(QWidget *parent) :
    QDial(parent),
    m_knobColor(Qt::black), m_meterColor(Qt::white),
    m_defaultValue(0)
{
    m_mouseDial = false;
    m_mousePressed = false;
}


// Destructor.
AudioDial::~AudioDial (void)
{
}


void AudioDial::paintEvent(QPaintEvent *)
{
    QPainter paint;

    float angle = AUDIO_DIAL_MIN // offset
	+ (AUDIO_DIAL_RANGE *
	   (float(QDial::value() - QDial::minimum()) /
	    (float(QDial::maximum() - QDial::minimum()))));
    int degrees = int(angle * 180.0 / M_PI);

    int ns = notchSize();
    int numTicks = 1 + (maximum() + ns - minimum()) / ns;
	
    QColor knobColor(m_knobColor);
    if (knobColor == Qt::black)
	knobColor = palette().mid().color();

    QColor meterColor(m_meterColor);
    if (!isEnabled())
	meterColor = palette().mid().color();
    else if (m_meterColor == Qt::white)
	meterColor = palette().highlight().color();

    int m_size = width() < height() ? width() : height();
    int scale = 1;
    int width = m_size - 2*scale, height = m_size - 2*scale;

    paint.begin(this);
    paint.setRenderHint(QPainter::Antialiasing, true);
    paint.translate(1, 1);

    QPen pen;
    QColor c;

    // Knob body and face...

    c = knobColor;
    pen.setColor(knobColor);
    pen.setWidth(scale * 2);
    pen.setCapStyle(Qt::FlatCap);
	
    paint.setPen(pen);
    paint.setBrush(c);

    int indent = (int)(width * 0.15 + 1);

    paint.drawEllipse(indent-1, indent-1, width-2*indent, width-2*indent);

    pen.setWidth(3 * scale);
    int pos = indent-1 + (width-2*indent) / 20;
    int darkWidth = (width-2*indent) * 3 / 4;
    while (darkWidth) {
	c = c.light(102);
	pen.setColor(c);
	paint.setPen(pen);
	paint.drawEllipse(pos, pos, darkWidth, darkWidth);
	if (!--darkWidth) break;
	paint.drawEllipse(pos, pos, darkWidth, darkWidth);
	if (!--darkWidth) break;
	paint.drawEllipse(pos, pos, darkWidth, darkWidth);
	++pos; --darkWidth;
    }

    // Tick notches...

    if ( notchesVisible() ) {
//	std::cerr << "Notches visible" << std::endl;
	pen.setColor(palette().dark().color());
	pen.setWidth(scale);
	paint.setPen(pen);
	for (int i = 0; i < numTicks; ++i) {
	    int div = numTicks;
	    if (div > 1) --div;
	    drawTick(paint, AUDIO_DIAL_MIN + (AUDIO_DIAL_MAX - AUDIO_DIAL_MIN) * i / div,
		     width, true);
	}
    }

    // The bright metering bit...

    c = meterColor;
    pen.setColor(c);
    pen.setWidth(indent);
    paint.setPen(pen);

//    std::cerr << "degrees " << degrees << ", gives us " << -(degrees - 45) * 16 << std::endl;

    int arcLen = -(degrees - 45) * 16;
    if (arcLen == 0) arcLen = -16;

    paint.drawArc(indent/2, indent/2,
		  width-indent, width-indent, (180 + 45) * 16, arcLen);

    paint.setBrush(Qt::NoBrush);

    // Shadowing...

    pen.setWidth(scale);
    paint.setPen(pen);

    // Knob shadow...

    int shadowAngle = -720;
    c = knobColor.dark();
    for (int arc = 120; arc < 2880; arc += 240) {
	pen.setColor(c);
	paint.setPen(pen);
	paint.drawArc(indent, indent,
		      width-2*indent, width-2*indent, shadowAngle + arc, 240);
	paint.drawArc(indent, indent,
		      width-2*indent, width-2*indent, shadowAngle - arc, 240);
	c = c.light(110);
    }

    // Scale shadow...

    shadowAngle = 2160;
    c = palette().dark().color();
    for (int arc = 120; arc < 2880; arc += 240) {
	pen.setColor(c);
	paint.setPen(pen);
	paint.drawArc(scale/2, scale/2,
		      width-scale, width-scale, shadowAngle + arc, 240);
	paint.drawArc(scale/2, scale/2,
		      width-scale, width-scale, shadowAngle - arc, 240);
	c = c.light(108);
    }

    // Undraw the bottom part...

    pen.setColor(palette().background().color());
    pen.setWidth(scale * 4);
    paint.setPen(pen);
    paint.drawArc(scale/2, scale/2,
		  width-scale, width-scale, -45 * 16, -92 * 16);

    // Scale ends...

    pen.setColor(palette().dark().color());
    pen.setWidth(scale);
    paint.setPen(pen);
    for (int i = 0; i < numTicks; ++i) {
	if (i != 0 && i != numTicks - 1) continue;
	int div = numTicks;
	if (div > 1) --div;
	drawTick(paint, AUDIO_DIAL_MIN + (AUDIO_DIAL_MAX - AUDIO_DIAL_MIN) * i / div,
		 width, false);
    }

    // Pointer notch...

    float hyp = float(width) / 2.0;
    float len = hyp - indent;
    --len;

    float x0 = hyp;
    float y0 = hyp;

    float x = hyp - len * sin(angle);
    float y = hyp + len * cos(angle);

    c = palette().dark().color();
    pen.setColor(isEnabled() ? c.dark(130) : c);
    pen.setWidth(scale * 2);
    paint.setPen(pen);
    paint.drawLine(int(x0), int(y0), int(x), int(y));

    paint.end();
}


void AudioDial::drawTick(QPainter &paint,
			 float angle, int size, bool internal)
{
    float hyp = float(size) / 2.0;
    float x0 = hyp - (hyp - 1) * sin(angle);
    float y0 = hyp + (hyp - 1) * cos(angle);

//    cerr << "drawTick: angle " << angle << ", size " << size << ", internal " << internal << endl;
    
    if (internal) {

	float len = hyp / 4;
	float x1 = hyp - (hyp - len) * sin(angle);
	float y1 = hyp + (hyp - len) * cos(angle);
		
	paint.drawLine(int(x0), int(y0), int(x1), int(y1));

    } else {

	float len = hyp / 4;
	float x1 = hyp - (hyp + len) * sin(angle);
	float y1 = hyp + (hyp + len) * cos(angle);

	paint.drawLine(int(x0), int(y0), int(x1), int(y1));
    }
}


void AudioDial::setKnobColor(const QColor& color)
{
    m_knobColor = color;
    update();
}


void AudioDial::setMeterColor(const QColor& color)
{
    m_meterColor = color;
    update();
}


void AudioDial::setMouseDial(bool mouseDial)
{
    m_mouseDial = mouseDial;
}


void AudioDial::setDefaultValue(int defaultValue)
{
    m_defaultValue = defaultValue;
}


// Alternate mouse behavior event handlers.
void AudioDial::mousePressEvent(QMouseEvent *mouseEvent)
{
    if (m_mouseDial) {
	QDial::mousePressEvent(mouseEvent);
    } else if (mouseEvent->button() == Qt::LeftButton) {
	m_mousePressed = true;
	m_posMouse = mouseEvent->pos();
    } else if (mouseEvent->button() == Qt::MidButton) {
	int dv = m_defaultValue;
	if (dv < minimum()) dv = minimum();
	if (dv > maximum()) dv = maximum();
	setValue(m_defaultValue);
    }
}


void AudioDial::mouseDoubleClickEvent(QMouseEvent *mouseEvent)
{
    if (m_mouseDial) {
	QDial::mouseDoubleClickEvent(mouseEvent);
    } else if (mouseEvent->button() == Qt::LeftButton) {
	bool ok = false;
	int newValue = QInputDialog::getInteger
	    (this,
	     tr("Enter new value"),
	     tr("Select a new value in the range %1 to %2:")
	     .arg(minimum()).arg(maximum()),
	     value(), minimum(), maximum(), pageStep(), &ok);
	if (ok) {
	    setValue(newValue);
	}
    }
}


void AudioDial::mouseMoveEvent(QMouseEvent *mouseEvent)
{
    if (m_mouseDial) {
	QDial::mouseMoveEvent(mouseEvent);
    } else if (m_mousePressed) {
	const QPoint& posMouse = mouseEvent->pos();
	int v = QDial::value()
	    + (posMouse.x() - m_posMouse.x())
	    + (m_posMouse.y() - posMouse.y());
	if (v > QDial::maximum())
	    v = QDial::maximum();
	else
	    if (v < QDial::minimum())
		v = QDial::minimum();
	m_posMouse = posMouse;
	QDial::setValue(v);
    }
}


void AudioDial::mouseReleaseEvent(QMouseEvent *mouseEvent)
{
    if (m_mouseDial) {
	QDial::mouseReleaseEvent(mouseEvent);
    } else if (m_mousePressed) {
	m_mousePressed = false;
    }
}

#ifdef INCLUDE_MOCFILES
#include "AudioDial.moc.cpp"
#endif


/* -*- c-basic-offset: 4 -*-  vi:set ts=8 sts=4 sw=4: */

/*
    A waveform viewer and audio annotation editor.
    Chris Cannam, Queen Mary University of London, 2005-2006
    
    This is experimental software.  Not for distribution.
*/

#ifndef _AUDIO_DIAL_H_
#define _AUDIO_DIAL_H_

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

#include <QDial>
#include <map>

/**
 * AudioDial is a nicer-looking QDial that by default reacts to mouse
 * movement on horizontal and vertical axes instead of in a radial
 * motion.  Move the mouse up or right to increment the value, down or
 * left to decrement it.  AudioDial also responds to the mouse wheel.
 *
 * The programming interface for this widget is compatible with QDial,
 * with the addition of properties for the knob colour and meter
 * colour and a boolean property mouseDial that determines whether to
 * respond to radial mouse motion in the same way as QDial (the
 * default is no).
 */

class AudioDial : public QDial
{
    Q_OBJECT
    Q_PROPERTY( QColor knobColor READ getKnobColor WRITE setKnobColor )
    Q_PROPERTY( QColor meterColor READ getMeterColor WRITE setMeterColor )
    Q_PROPERTY( bool mouseDial READ getMouseDial WRITE setMouseDial )

public:
    AudioDial(QWidget *parent = 0);
    ~AudioDial();

    const QColor& getKnobColor()  const { return m_knobColor;  }
    const QColor& getMeterColor() const { return m_meterColor; }
    bool getMouseDial() const { return m_mouseDial; }

public slots:
    /**
     * Set the colour of the knob.  The default is to inherit the
     * colour from the widget's palette.
     */
    void setKnobColor(const QColor &color);

    /**
     * Set the colour of the meter (the highlighted area around the
     * knob that shows the current value).  The default is to inherit
     * the colour from the widget's palette.
     */
    void setMeterColor(const QColor &color);
    
    /**
     * Specify that the dial should respond to radial mouse movements
     * in the same way as QDial.
     */
    void setMouseDial(bool mouseDial);

    void setDefaultValue(int defaultValue);

protected:
    void drawTick(QPainter &paint, float angle, int size, bool internal);
    virtual void paintEvent(QPaintEvent *);

    // Alternate mouse behavior event handlers.
    virtual void mousePressEvent(QMouseEvent *pMouseEvent);
    virtual void mouseMoveEvent(QMouseEvent *pMouseEvent);
    virtual void mouseReleaseEvent(QMouseEvent *pMouseEvent);
    virtual void mouseDoubleClickEvent(QMouseEvent *pMouseEvent);

private:
    QColor m_knobColor;
    QColor m_meterColor;
    
    int m_defaultValue;

    // Alternate mouse behavior tracking.
    bool m_mouseDial;
    bool m_mousePressed;
    QPoint m_posMouse;
};


#endif  // __AudioDial_h

// end of AudioDial.h

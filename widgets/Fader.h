/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef FADER_H
#define FADER_H

/**
 * Horizontal audio fader and meter widget.
 * Based on the vertical fader and meter widget from:
 * 
 * Hydrogen
 * Copyright(c) 2002-2005 by Alex >Comix< Cominu [comix@users.sourceforge.net]
 *
 * http://www.hydrogen-music.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY, without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: Fader.h,v 1.14 2005/08/10 08:03:30 comix Exp $
 */


#include <string>
#include <iostream>

#include <QWidget>
#include <QPixmap>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPaintEvent>

class Fader : public QWidget
{
    Q_OBJECT

public:
    Fader(QWidget *parent, bool withoutKnob = false);
    ~Fader();

    void setValue(float newValue);
    float getValue();

    void setPeakLeft(float);
    float getPeakLeft() { return m_peakLeft; }

    void setPeakRight(float);
    float getPeakRight() { return m_peakRight; }

    virtual void mousePressEvent(QMouseEvent *ev);
    virtual void mouseDoubleClickEvent(QMouseEvent *ev);
    virtual void mouseMoveEvent(QMouseEvent *ev);
    virtual void mouseReleaseEvent(QMouseEvent *ev);
    virtual void wheelEvent( QWheelEvent *ev );
    virtual void paintEvent(QPaintEvent *ev);

signals:
    void valueChanged(float); // 0.0 -> 1.0

private:
    int getMaxX() const;

    bool m_withoutKnob;
    float m_value;
    float m_peakLeft;
    float m_peakRight;

    bool m_mousePressed;
    int m_mousePressX;
    float m_mousePressValue;

    QPixmap m_back;
    QPixmap m_leds;
    QPixmap m_knob;
    QPixmap m_clip;
};

#endif

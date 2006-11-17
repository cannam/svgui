/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _THUMBWHEEL_H_
#define _THUMBWHEEL_H_

#include <QWidget>

class Thumbwheel : public QWidget
{
    Q_OBJECT

public:
    Thumbwheel(Qt::Orientation orientation, QWidget *parent = 0);
    virtual ~Thumbwheel();

    int getMinimumValue() const;
    int getMaximumValue() const;
    int getDefaultValue() const;
    float getSpeed() const;
    bool getTracking() const;
    bool getShowScale() const;
    int getValue() const;

    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseDoubleClickEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void wheelEvent(QWheelEvent *e);
    virtual void paintEvent(QPaintEvent *e);

    QSize sizeHint() const;

signals:
    void valueChanged(int);

public slots:
    void setMinimumValue(int min);
    void setMaximumValue(int max);
    void setDefaultValue(int deft);
    void setSpeed(float speed);
    void setTracking(bool tracking);
    void setShowScale(bool show);
    void setValue(int value);
    void resetToDefault();

private:
    int m_min;
    int m_max;
    int m_default;
    int m_value;
    float m_rotation;
    Qt::Orientation m_orientation;
    float m_speed;
    bool m_tracking;
    bool m_showScale;
    bool m_clicked;
    bool m_atDefault;
    QPoint m_clickPos;
    float m_clickRotation;
};

#endif

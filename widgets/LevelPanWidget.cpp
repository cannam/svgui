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

#include "LevelPanWidget.h"

#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>

#include "layer/ColourMapper.h"
#include "base/AudioLevel.h"

#include <iostream>
#include <cmath>

using std::cerr;
using std::endl;

static const int maxLevel = 4;
static const int maxPan = 2; // range is -maxPan to maxPan

LevelPanWidget::LevelPanWidget(QWidget *parent) :
    QWidget(parent),
    m_level(maxLevel),
    m_pan(0),
    m_editable(true)
{
}

LevelPanWidget::~LevelPanWidget()
{
}

QSize
LevelPanWidget::sizeHint() const
{
    static double ratio = 0.0;
    if (ratio == 0.0) {
        double baseEm;
#ifdef Q_OS_MAC
        baseEm = 17.0;
#else
        baseEm = 15.0;
#endif
        double em = QFontMetrics(QFont()).height();
        ratio = em / baseEm;
    }

    int pixels = 40;
    int scaled = int(pixels * ratio + 0.5);
    if (pixels != 0 && scaled == 0) scaled = 1;
    return QSize(scaled, scaled);
}

void
LevelPanWidget::setLevel(float flevel)
{
    int level = AudioLevel::multiplier_to_fader
	(flevel, maxLevel, AudioLevel::ShortFader);
    if (level < 0) level = 0;
    if (level > maxLevel) level = maxLevel;
    if (level != m_level) {
	m_level = level;
	float convertsTo = getLevel();
	if (fabsf(convertsTo - flevel) > 1e-5) {
	    emitLevelChanged();
	}
	update();
    }
}

void
LevelPanWidget::setPan(float pan)
{
    m_pan = int(round(pan * maxPan));
    if (m_pan < -maxPan) m_pan = -maxPan;
    if (m_pan > maxPan) m_pan = maxPan;
    update();
}

void
LevelPanWidget::setEditable(bool editable)
{
    m_editable = editable;
    update();
}

float
LevelPanWidget::getLevel() const
{
    return float(AudioLevel::fader_to_multiplier
		 (m_level, maxLevel, AudioLevel::ShortFader));
}

float
LevelPanWidget::getPan() const
{
    return float(m_pan) / float(maxPan);
}

void
LevelPanWidget::emitLevelChanged()
{
    cerr << "emitting levelChanged(" << getLevel() << ")" << endl;
    emit levelChanged(getLevel());
}

void
LevelPanWidget::emitPanChanged()
{
    cerr << "emitting panChanged(" << getPan() << ")" << endl;
    emit panChanged(getPan());
}

void
LevelPanWidget::mousePressEvent(QMouseEvent *e)
{
    mouseMoveEvent(e);
}

void
LevelPanWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_editable) return;
    
    int level, pan;
    toCell(rect(), e->pos(), level, pan);
    if (level == m_level && pan == m_pan) {
	return;
    }
    if (level != m_level) {
	m_level = level;
	emitLevelChanged();
    }
    if (pan != m_pan) {
	m_pan = pan;
	emitPanChanged();
    }
    update();
}

void
LevelPanWidget::mouseReleaseEvent(QMouseEvent *e)
{
    mouseMoveEvent(e);
}

void
LevelPanWidget::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() & Qt::ControlModifier) {
	if (e->delta() > 0) {
	    if (m_pan < maxPan) {
		++m_pan;
		emitPanChanged();
		update();
	    }
	} else {
	    if (m_pan > -maxPan) {
		--m_pan;
		emitPanChanged();
		update();
	    }
	}
    } else {
	if (e->delta() > 0) {
	    if (m_level < maxLevel) {
		++m_level;
		emitLevelChanged();
		update();
	    }
	} else {
	    if (m_level > 0) {
		--m_level;
		emitLevelChanged();
		update();
	    }
	}
    }
}

void
LevelPanWidget::toCell(QRectF rect, QPointF loc, int &level, int &pan) const
{
    double w = rect.width(), h = rect.height();

    int npan = maxPan * 2 + 1;
    int nlevel = maxLevel + 1;

    double wcell = w / npan, hcell = h / nlevel;

    level = int((h - loc.y()) / hcell);
    if (level < 0) level = 0;
    if (level > maxLevel) level = maxLevel;

    pan = int(loc.x() / wcell) - maxPan;
    if (pan < -maxPan) pan = -maxPan;
    if (pan > maxPan) pan = maxPan;
}

QSizeF
LevelPanWidget::cellSize(QRectF rect) const
{
    double w = rect.width(), h = rect.height();
    int npan = maxPan * 2 + 1;
    int nlevel = maxLevel + 1;
    double wcell = w / npan, hcell = h / nlevel;
    return QSizeF(wcell, hcell);
}

QPointF
LevelPanWidget::cellCentre(QRectF rect, int level, int pan) const
{
    QSizeF cs = cellSize(rect);
    return QPointF(cs.width() * (pan + maxPan) + cs.width() / 2.,
		   rect.height() - cs.height() * (level + 1) + cs.height() / 2.);
}

QSizeF
LevelPanWidget::cellLightSize(QRectF rect) const
{
    double extent = 3. / 4.;
    QSizeF cs = cellSize(rect);
    double m = std::min(cs.width(), cs.height());
    return QSizeF(m * extent, m * extent);
}

QRectF
LevelPanWidget::cellLightRect(QRectF rect, int level, int pan) const
{
    QSizeF cls = cellLightSize(rect);
    QPointF cc = cellCentre(rect, level, pan);
    return QRectF(cc.x() - cls.width() / 2., 
		  cc.y() - cls.height() / 2.,
		  cls.width(),
		  cls.height());
}

double
LevelPanWidget::thinLineWidth(QRectF rect) const
{
    double tw = ceil(rect.width() / (maxPan * 2. * 10.));
    double th = ceil(rect.height() / (maxLevel * 10.));
    return std::min(th, tw);
}

void
LevelPanWidget::renderTo(QPaintDevice *dev, QRectF rect, bool asIfEditable) const
{
    QPainter paint(dev);
    ColourMapper mapper(ColourMapper::Sunset, 0, maxLevel);

    paint.setRenderHint(QPainter::Antialiasing, true);

    QPen pen;

    double thin = thinLineWidth(rect);
    
    pen.setColor(QColor(127, 127, 127, 127));
    pen.setWidthF(cellLightSize(rect).width() + thin);
    pen.setCapStyle(Qt::RoundCap);
    paint.setPen(pen);

    for (int pan = -maxPan; pan <= maxPan; ++pan) {
	paint.drawLine(cellCentre(rect, 0, pan), cellCentre(rect, maxLevel, pan));
    }

    if (isEnabled()) {
	pen.setColor(Qt::black);
    } else {
	pen.setColor(Qt::darkGray);
    }

    if (!asIfEditable && m_level == 0) {
        pen.setWidthF(thin * 2);
        pen.setCapStyle(Qt::RoundCap);
        paint.setPen(pen);
        paint.drawLine(rect.topLeft(), rect.bottomRight());
        paint.drawLine(rect.bottomLeft(), rect.topRight());
        return;
    }
    
    pen.setWidthF(thin);
    pen.setCapStyle(Qt::FlatCap);
    paint.setPen(pen);
    
    for (int level = 0; level <= m_level; ++level) {
	if (isEnabled()) {
	    paint.setBrush(mapper.map(level));
	}
	QRectF clr = cellLightRect(rect, level, m_pan);
	if (m_level == 0) {
	    paint.drawLine(clr.topLeft(), clr.bottomRight());
	    paint.drawLine(clr.bottomLeft(), clr.topRight());
	} else {
	    paint.drawEllipse(clr);
	}
    }
}

void
LevelPanWidget::paintEvent(QPaintEvent *)
{
    renderTo(this, rect(), m_editable);
}




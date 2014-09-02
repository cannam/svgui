/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam and QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "Overview.h"
#include "layer/Layer.h"
#include "data/model/Model.h"
#include "base/ZoomConstraint.h"

#include <QPaintEvent>
#include <QPainter>
#include <iostream>

//#define DEBUG_OVERVIEW 1


Overview::Overview(QWidget *w) :
    View(w, false),
    m_clickedInRange(false)
{
    setObjectName(tr("Overview"));
    m_followPan = false;
    m_followZoom = false;
    setPlaybackFollow(PlaybackIgnore);
    m_modelTestTime.start();
}

void
Overview::modelChangedWithin(int startFrame, int endFrame)
{
    bool zoomChanged = false;

    int frameCount = getModelsEndFrame() - getModelsStartFrame();
    int zoomLevel = frameCount / width();
    if (zoomLevel < 1) zoomLevel = 1;
    zoomLevel = getZoomConstraintBlockSize(zoomLevel,
					   ZoomConstraint::RoundUp);
    if (zoomLevel != m_zoomLevel) {
        zoomChanged = true;
    }

    if (!zoomChanged) {
        if (m_modelTestTime.elapsed() < 1000) {
            for (LayerList::const_iterator i = m_layerStack.begin();
                 i != m_layerStack.end(); ++i) {
                if ((*i)->getModel() &&
                    (!(*i)->getModel()->isOK() ||
                     !(*i)->getModel()->isReady())) {
                    return;
                }
            }
        } else {
            m_modelTestTime.restart();
        }
    }

    View::modelChangedWithin(startFrame, endFrame);
}

void
Overview::modelReplaced()
{
    m_playPointerFrame = getAlignedPlaybackFrame();
    View::modelReplaced();
}

void
Overview::registerView(View *view)
{
    m_views.insert(view);
    update(); 
}

void
Overview::unregisterView(View *view)
{
    m_views.erase(view);
    update();
}

void
Overview::globalCentreFrameChanged(int 
#ifdef DEBUG_OVERVIEW
                                   f
#endif
    )
{
#ifdef DEBUG_OVERVIEW
    cerr << "Overview::globalCentreFrameChanged: " << f << endl;
#endif
    update();
}

void
Overview::viewCentreFrameChanged(View *v, int
#ifdef DEBUG_OVERVIEW
                                 f
#endif
    )
{
#ifdef DEBUG_OVERVIEW
    cerr << "Overview[" << this << "]::viewCentreFrameChanged(" << v << "): " << f << endl;
#endif
    if (m_views.find(v) != m_views.end()) {
	update();
    }
}    

void
Overview::viewZoomLevelChanged(View *v, int, bool)
{
    if (v == this) return;
    if (m_views.find(v) != m_views.end()) {
	update();
    }
}

void
Overview::viewManagerPlaybackFrameChanged(int f)
{
#ifdef DEBUG_OVERVIEW
    cerr << "Overview[" << this << "]::viewManagerPlaybackFrameChanged(" << f << "): " << f << endl;
#endif

    bool changed = false;

    f = getAlignedPlaybackFrame();

    if (getXForFrame(m_playPointerFrame) != getXForFrame(f)) changed = true;
    m_playPointerFrame = f;

    if (changed) update();
}

void
Overview::paintEvent(QPaintEvent *e)
{
    // Recalculate zoom in case the size of the widget has changed.

#ifdef DEBUG_OVERVIEW
    cerr << "Overview::paintEvent: width is " << width() << ", centre frame " << m_centreFrame << endl;
#endif

    int startFrame = getModelsStartFrame();
    int frameCount = getModelsEndFrame() - getModelsStartFrame();
    int zoomLevel = frameCount / width();
    if (zoomLevel < 1) zoomLevel = 1;
    zoomLevel = getZoomConstraintBlockSize(zoomLevel,
					   ZoomConstraint::RoundUp);
    if (zoomLevel != m_zoomLevel) {
	m_zoomLevel = zoomLevel;
	emit zoomLevelChanged(m_zoomLevel, m_followZoom);
    }

    int centreFrame = startFrame + m_zoomLevel * (width() / 2);
    if (centreFrame > (startFrame + getModelsEndFrame())/2) {
	centreFrame = (startFrame + getModelsEndFrame())/2;
    }
    if (centreFrame != m_centreFrame) {
#ifdef DEBUG_OVERVIEW
        cerr << "Overview::paintEvent: Centre frame changed from "
                  << m_centreFrame << " to " << centreFrame << " and thus start frame from " << getStartFrame();
#endif
	m_centreFrame = centreFrame;
#ifdef DEBUG_OVERVIEW
        cerr << " to " << getStartFrame() << endl;
#endif
	emit centreFrameChanged(m_centreFrame, false, PlaybackIgnore);
    }

    View::paintEvent(e);

    QPainter paint;
    paint.begin(this);

    QRect r(rect());

    if (e) {
	r = e->rect();
	paint.setClipRect(r);
    }

    paint.setPen(getForeground());

    int y = 0;

    int prevx0 = -10;
    int prevx1 = -10;

    for (ViewSet::iterator i = m_views.begin(); i != m_views.end(); ++i) {
	if (!*i) continue;

	View *w = (View *)*i;

	int f0 = w->getFrameForX(0);
	int f1 = w->getFrameForX(w->width());

        if (f0 >= 0) {
            int rf0 = w->alignToReference(f0);
            f0 = alignFromReference(rf0);
        }
        if (f1 >= 0) {
            int rf1 = w->alignToReference(f1);
            f1 = alignFromReference(rf1);
        }

	int x0 = getXForFrame(f0);
	int x1 = getXForFrame(f1);

	if (x0 != prevx0 || x1 != prevx1) {
	    y += height() / 10 + 1;
	    prevx0 = x0;
	    prevx1 = x1;
	}

	if (x1 <= x0) x1 = x0 + 1;
	
	paint.drawRect(x0, y, x1 - x0, height() - 2 * y);
    }

    paint.end();
}

void
Overview::mousePressEvent(QMouseEvent *e)
{
    m_clickPos = e->pos();
    int clickFrame = getFrameForX(m_clickPos.x());
    if (clickFrame > 0) m_dragCentreFrame = clickFrame;
    else m_dragCentreFrame = 0;
    m_clickedInRange = true;

    for (ViewSet::iterator i = m_views.begin(); i != m_views.end(); ++i) {
	if (*i && (*i)->getAligningModel() == getAligningModel()) {
            m_dragCentreFrame = (*i)->getCentreFrame();
            break;
        }
    }
}

void
Overview::mouseReleaseEvent(QMouseEvent *e)
{
    if (m_clickedInRange) {
	mouseMoveEvent(e);
    }
    m_clickedInRange = false;
}

void
Overview::mouseMoveEvent(QMouseEvent *e)
{
    if (!m_clickedInRange) return;

    int xoff = int(e->x()) - int(m_clickPos.x());
    int frameOff = xoff * m_zoomLevel;
    
    int newCentreFrame = m_dragCentreFrame;
    if (frameOff > 0) {
	newCentreFrame += frameOff;
    } else if (newCentreFrame >= int(-frameOff)) {
	newCentreFrame += frameOff;
    } else {
	newCentreFrame = 0;
    }

    if (newCentreFrame >= getModelsEndFrame()) {
	newCentreFrame = getModelsEndFrame();
	if (newCentreFrame > 0) --newCentreFrame;
    }
    
    if (std::max(m_centreFrame, newCentreFrame) -
	std::min(m_centreFrame, newCentreFrame) > int(m_zoomLevel)) {
        int rf = alignToReference(newCentreFrame);
#ifdef DEBUG_OVERVIEW
        cerr << "Overview::mouseMoveEvent: x " << e->x() << " and click x " << m_clickPos.x() << " -> frame " << newCentreFrame << " -> rf " << rf << endl;
#endif
        if (m_followPlay == PlaybackScrollContinuous ||
            m_followPlay == PlaybackScrollPageWithCentre) {
            emit centreFrameChanged(rf, true, PlaybackScrollContinuous);
        } else {
            emit centreFrameChanged(rf, true, PlaybackIgnore);
        }            
    }
}

void
Overview::mouseDoubleClickEvent(QMouseEvent *e)
{
    int frame = getFrameForX(e->x());
    int rf = 0;
    if (frame > 0) rf = alignToReference(frame);
#ifdef DEBUG_OVERVIEW
    cerr << "Overview::mouseDoubleClickEvent: frame " << frame << " -> rf " << rf << endl;
#endif
    m_clickedInRange = false; // we're not starting a drag with the second click
    emit centreFrameChanged(rf, true, PlaybackScrollContinuous);
}

void
Overview::enterEvent(QEvent *)
{
    emit contextHelpChanged(tr("Click and drag to navigate; double-click to jump"));
}

void
Overview::leaveEvent(QEvent *)
{
    emit contextHelpChanged("");
}



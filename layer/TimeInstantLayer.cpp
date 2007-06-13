/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "TimeInstantLayer.h"

#include "data/model/Model.h"
#include "base/RealTime.h"
#include "view/View.h"
#include "base/Profiler.h"
#include "base/Clipboard.h"

#include "data/model/SparseOneDimensionalModel.h"

#include "widgets/ItemEditDialog.h"

#include <QPainter>
#include <QMouseEvent>

#include <iostream>
#include <cmath>

TimeInstantLayer::TimeInstantLayer() :
    Layer(),
    m_model(0),
    m_editing(false),
    m_editingPoint(0, tr("New Point")),
    m_editingCommand(0),
    m_colour(QColor(200, 50, 255)),
    m_plotStyle(PlotInstants)
{
    
}

void
TimeInstantLayer::setModel(SparseOneDimensionalModel *model)
{
    if (m_model == model) return;
    m_model = model;

    connect(m_model, SIGNAL(modelChanged()), this, SIGNAL(modelChanged()));
    connect(m_model, SIGNAL(modelChanged(size_t, size_t)),
	    this, SIGNAL(modelChanged(size_t, size_t)));

    connect(m_model, SIGNAL(completionChanged()),
	    this, SIGNAL(modelCompletionChanged()));

    std::cerr << "TimeInstantLayer::setModel(" << model << ")" << std::endl;

    emit modelReplaced();
}

Layer::PropertyList
TimeInstantLayer::getProperties() const
{
    PropertyList list;
    list.push_back("Colour");
    list.push_back("Plot Type");
    return list;
}

QString
TimeInstantLayer::getPropertyLabel(const PropertyName &name) const
{
    if (name == "Colour") return tr("Colour");
    if (name == "Plot Type") return tr("Plot Type");
    return "";
}

Layer::PropertyType
TimeInstantLayer::getPropertyType(const PropertyName &) const
{
    return ValueProperty;
}

int
TimeInstantLayer::getPropertyRangeAndValue(const PropertyName &name,
                                           int *min, int *max, int *deflt) const
{
    int val = 0;

    if (name == "Colour") {

	if (min) *min = 0;
	if (max) *max = 5;
        if (deflt) *deflt = 0;

	if (m_colour == Qt::black) val = 0;
	else if (m_colour == Qt::darkRed) val = 1;
	else if (m_colour == Qt::darkBlue) val = 2;
	else if (m_colour == Qt::darkGreen) val = 3;
	else if (m_colour == QColor(200, 50, 255)) val = 4;
	else if (m_colour == QColor(255, 150, 50)) val = 5;

    } else if (name == "Plot Type") {
	
	if (min) *min = 0;
	if (max) *max = 1;
        if (deflt) *deflt = 0;
	
	val = int(m_plotStyle);

    } else {
	
	val = Layer::getPropertyRangeAndValue(name, min, max, deflt);
    }

    return val;
}

QString
TimeInstantLayer::getPropertyValueLabel(const PropertyName &name,
				    int value) const
{
    if (name == "Colour") {
	switch (value) {
	default:
	case 0: return tr("Black");
	case 1: return tr("Red");
	case 2: return tr("Blue");
	case 3: return tr("Green");
	case 4: return tr("Purple");
	case 5: return tr("Orange");
	}
    } else if (name == "Plot Type") {
	switch (value) {
	default:
	case 0: return tr("Instants");
	case 1: return tr("Segmentation");
	}
    }
    return tr("<unknown>");
}

void
TimeInstantLayer::setProperty(const PropertyName &name, int value)
{
    if (name == "Colour") {
	switch (value) {
	default:
	case 0:	setBaseColour(Qt::black); break;
	case 1: setBaseColour(Qt::darkRed); break;
	case 2: setBaseColour(Qt::darkBlue); break;
	case 3: setBaseColour(Qt::darkGreen); break;
	case 4: setBaseColour(QColor(200, 50, 255)); break;
	case 5: setBaseColour(QColor(255, 150, 50)); break;
	}
    } else if (name == "Plot Type") {
	setPlotStyle(PlotStyle(value));
    }
}

void
TimeInstantLayer::setBaseColour(QColor colour)
{
    if (m_colour == colour) return;
    m_colour = colour;
    emit layerParametersChanged();
}

void
TimeInstantLayer::setPlotStyle(PlotStyle style)
{
    if (m_plotStyle == style) return;
    m_plotStyle = style;
    emit layerParametersChanged();
}

bool
TimeInstantLayer::isLayerScrollable(const View *v) const
{
    QPoint discard;
    return !v->shouldIlluminateLocalFeatures(this, discard);
}

SparseOneDimensionalModel::PointList
TimeInstantLayer::getLocalPoints(View *v, int x) const
{
    // Return a set of points that all have the same frame number, the
    // nearest to the given x coordinate, and that are within a
    // certain fuzz distance of that x coordinate.

    if (!m_model) return SparseOneDimensionalModel::PointList();

    long frame = v->getFrameForX(x);

    SparseOneDimensionalModel::PointList onPoints =
	m_model->getPoints(frame);

    if (!onPoints.empty()) {
	return onPoints;
    }

    SparseOneDimensionalModel::PointList prevPoints =
	m_model->getPreviousPoints(frame);
    SparseOneDimensionalModel::PointList nextPoints =
	m_model->getNextPoints(frame);

    SparseOneDimensionalModel::PointList usePoints = prevPoints;

    if (prevPoints.empty()) {
	usePoints = nextPoints;
    } else if (long(prevPoints.begin()->frame) < v->getStartFrame() &&
	       !(nextPoints.begin()->frame > v->getEndFrame())) {
	usePoints = nextPoints;
    } else if (nextPoints.begin()->frame - frame <
	       frame - prevPoints.begin()->frame) {
	usePoints = nextPoints;
    }

    if (!usePoints.empty()) {
	int fuzz = 2;
	int px = v->getXForFrame(usePoints.begin()->frame);
	if ((px > x && px - x > fuzz) ||
	    (px < x && x - px > fuzz + 1)) {
	    usePoints.clear();
	}
    }

    return usePoints;
}

QString
TimeInstantLayer::getFeatureDescription(View *v, QPoint &pos) const
{
    int x = pos.x();

    if (!m_model || !m_model->getSampleRate()) return "";

    SparseOneDimensionalModel::PointList points = getLocalPoints(v, x);

    if (points.empty()) {
	if (!m_model->isReady()) {
	    return tr("In progress");
	} else {
	    return tr("No local points");
	}
    }

    long useFrame = points.begin()->frame;

    RealTime rt = RealTime::frame2RealTime(useFrame, m_model->getSampleRate());
    
    QString text;

    if (points.begin()->label == "") {
	text = QString(tr("Time:\t%1\nNo label"))
	    .arg(rt.toText(true).c_str());
    } else {
	text = QString(tr("Time:\t%1\nLabel:\t%2"))
	    .arg(rt.toText(true).c_str())
	    .arg(points.begin()->label);
    }

    pos = QPoint(v->getXForFrame(useFrame), pos.y());
    return text;
}

bool
TimeInstantLayer::snapToFeatureFrame(View *v, int &frame,
				     size_t &resolution,
				     SnapType snap) const
{
    if (!m_model) {
	return Layer::snapToFeatureFrame(v, frame, resolution, snap);
    }

    resolution = m_model->getResolution();
    SparseOneDimensionalModel::PointList points;

    if (snap == SnapNeighbouring) {
	
	points = getLocalPoints(v, v->getXForFrame(frame));
	if (points.empty()) return false;
	frame = points.begin()->frame;
	return true;
    }    

    points = m_model->getPoints(frame, frame);
    int snapped = frame;
    bool found = false;

    for (SparseOneDimensionalModel::PointList::const_iterator i = points.begin();
	 i != points.end(); ++i) {

	if (snap == SnapRight) {

	    if (i->frame >= frame) {
		snapped = i->frame;
		found = true;
		break;
	    }

	} else if (snap == SnapLeft) {

	    if (i->frame <= frame) {
		snapped = i->frame;
		found = true; // don't break, as the next may be better
	    } else {
		break;
	    }

	} else { // nearest

	    SparseOneDimensionalModel::PointList::const_iterator j = i;
	    ++j;

	    if (j == points.end()) {

		snapped = i->frame;
		found = true;
		break;

	    } else if (j->frame >= frame) {

		if (j->frame - frame < frame - i->frame) {
		    snapped = j->frame;
		} else {
		    snapped = i->frame;
		}
		found = true;
		break;
	    }
	}
    }

    frame = snapped;
    return found;
}

void
TimeInstantLayer::paint(View *v, QPainter &paint, QRect rect) const
{
    if (!m_model || !m_model->isOK()) return;

//    Profiler profiler("TimeInstantLayer::paint", true);

    int x0 = rect.left(), x1 = rect.right();

    long frame0 = v->getFrameForX(x0);
    long frame1 = v->getFrameForX(x1);

    SparseOneDimensionalModel::PointList points(m_model->getPoints
						(frame0, frame1));

    bool odd = false;
    if (m_plotStyle == PlotSegmentation && !points.empty()) {
	int index = m_model->getIndexOf(*points.begin());
	odd = ((index % 2) == 1);
    }

    paint.setPen(m_colour);

    QColor brushColour(m_colour);
    brushColour.setAlpha(100);
    paint.setBrush(brushColour);

    QColor oddBrushColour(brushColour);
    if (m_plotStyle == PlotSegmentation) {
	if (m_colour == Qt::black) {
	    oddBrushColour = Qt::gray;
	} else if (m_colour == Qt::darkRed) {
	    oddBrushColour = Qt::red;
	} else if (m_colour == Qt::darkBlue) {
	    oddBrushColour = Qt::blue;
	} else if (m_colour == Qt::darkGreen) {
	    oddBrushColour = Qt::green;
	} else {
	    oddBrushColour = oddBrushColour.light(150);
	}
	oddBrushColour.setAlpha(100);
    }

//    std::cerr << "TimeInstantLayer::paint: resolution is "
//	      << m_model->getResolution() << " frames" << std::endl;

    QPoint localPos;
    long illuminateFrame = -1;

    if (v->shouldIlluminateLocalFeatures(this, localPos)) {
	SparseOneDimensionalModel::PointList localPoints =
	    getLocalPoints(v, localPos.x());
	if (!localPoints.empty()) illuminateFrame = localPoints.begin()->frame;
    }
	
    int prevX = -1;
    int textY = v->getTextLabelHeight(this, paint);
    
    for (SparseOneDimensionalModel::PointList::const_iterator i = points.begin();
	 i != points.end(); ++i) {

	const SparseOneDimensionalModel::Point &p(*i);
	SparseOneDimensionalModel::PointList::const_iterator j = i;
	++j;

	int x = v->getXForFrame(p.frame);
	if (x == prevX && p.frame != illuminateFrame) continue;

	int iw = v->getXForFrame(p.frame + m_model->getResolution()) - x;
	if (iw < 2) {
	    if (iw < 1) {
		iw = 2;
		if (j != points.end()) {
		    int nx = v->getXForFrame(j->frame);
		    if (nx < x + 3) iw = 1;
		}
	    } else {
		iw = 2;
	    }
	}
		
	if (p.frame == illuminateFrame) {
	    paint.setPen(Qt::black); //!!!
	} else {
	    paint.setPen(brushColour);
	}

	if (m_plotStyle == PlotInstants) {
	    if (iw > 1) {
		paint.drawRect(x, 0, iw - 1, v->height() - 1);
	    } else {
		paint.drawLine(x, 0, x, v->height() - 1);
	    }
	} else {

	    if (odd) paint.setBrush(oddBrushColour);
	    else paint.setBrush(brushColour);
	    
	    int nx;
	    
	    if (j != points.end()) {
		const SparseOneDimensionalModel::Point &q(*j);
		nx = v->getXForFrame(q.frame);
	    } else {
		nx = v->getXForFrame(m_model->getEndFrame());
	    }

	    if (nx >= x) {
		
		if (illuminateFrame != p.frame &&
		    (nx < x + 5 || x >= v->width() - 1)) {
		    paint.setPen(Qt::NoPen);
		}

		paint.drawRect(x, -1, nx - x, v->height() + 1);
	    }

	    odd = !odd;
	}

	paint.setPen(m_colour);
	
	if (p.label != "") {

	    // only draw if there's enough room from here to the next point

	    int lw = paint.fontMetrics().width(p.label);
	    bool good = true;

	    if (j != points.end()) {
		int nx = v->getXForFrame(j->frame);
		if (nx >= x && nx - x - iw - 3 <= lw) good = false;
	    }

	    if (good) {
		paint.drawText(x + iw + 2, textY, p.label);
	    }
	}

	prevX = x;
    }
}

void
TimeInstantLayer::drawStart(View *v, QMouseEvent *e)
{
    std::cerr << "TimeInstantLayer::drawStart(" << e->x() << ")" << std::endl;

    if (!m_model) return;

    long frame = v->getFrameForX(e->x());
    if (frame < 0) frame = 0;
    frame = frame / m_model->getResolution() * m_model->getResolution();

    m_editingPoint = SparseOneDimensionalModel::Point(frame, tr("New Point"));

    if (m_editingCommand) m_editingCommand->finish();
    m_editingCommand = new SparseOneDimensionalModel::EditCommand(m_model,
								  tr("Draw Point"));
    m_editingCommand->addPoint(m_editingPoint);

    m_editing = true;
}

void
TimeInstantLayer::drawDrag(View *v, QMouseEvent *e)
{
    std::cerr << "TimeInstantLayer::drawDrag(" << e->x() << ")" << std::endl;

    if (!m_model || !m_editing) return;

    long frame = v->getFrameForX(e->x());
    if (frame < 0) frame = 0;
    frame = frame / m_model->getResolution() * m_model->getResolution();
    m_editingCommand->deletePoint(m_editingPoint);
    m_editingPoint.frame = frame;
    m_editingCommand->addPoint(m_editingPoint);
}

void
TimeInstantLayer::drawEnd(View *, QMouseEvent *e)
{
    std::cerr << "TimeInstantLayer::drawEnd(" << e->x() << ")" << std::endl;
    if (!m_model || !m_editing) return;
    QString newName = tr("Add Point at %1 s")
	.arg(RealTime::frame2RealTime(m_editingPoint.frame,
				      m_model->getSampleRate())
	     .toText(false).c_str());
    m_editingCommand->setName(newName);
    m_editingCommand->finish();
    m_editingCommand = 0;
    m_editing = false;
}

void
TimeInstantLayer::editStart(View *v, QMouseEvent *e)
{
    std::cerr << "TimeInstantLayer::editStart(" << e->x() << ")" << std::endl;

    if (!m_model) return;

    SparseOneDimensionalModel::PointList points = getLocalPoints(v, e->x());
    if (points.empty()) return;

    m_editingPoint = *points.begin();

    if (m_editingCommand) {
	m_editingCommand->finish();
	m_editingCommand = 0;
    }

    m_editing = true;
}

void
TimeInstantLayer::editDrag(View *v, QMouseEvent *e)
{
    std::cerr << "TimeInstantLayer::editDrag(" << e->x() << ")" << std::endl;

    if (!m_model || !m_editing) return;

    long frame = v->getFrameForX(e->x());
    if (frame < 0) frame = 0;
    frame = frame / m_model->getResolution() * m_model->getResolution();

    if (!m_editingCommand) {
	m_editingCommand = new SparseOneDimensionalModel::EditCommand(m_model,
								      tr("Drag Point"));
    }

    m_editingCommand->deletePoint(m_editingPoint);
    m_editingPoint.frame = frame;
    m_editingCommand->addPoint(m_editingPoint);
}

void
TimeInstantLayer::editEnd(View *, QMouseEvent *e)
{
    std::cerr << "TimeInstantLayer::editEnd(" << e->x() << ")" << std::endl;
    if (!m_model || !m_editing) return;
    if (m_editingCommand) {
	QString newName = tr("Move Point to %1 s")
	    .arg(RealTime::frame2RealTime(m_editingPoint.frame,
					  m_model->getSampleRate())
		 .toText(false).c_str());
	m_editingCommand->setName(newName);
	m_editingCommand->finish();
    }
    m_editingCommand = 0;
    m_editing = false;
}

bool
TimeInstantLayer::editOpen(View *v, QMouseEvent *e)
{
    if (!m_model) return false;

    SparseOneDimensionalModel::PointList points = getLocalPoints(v, e->x());
    if (points.empty()) return false;

    SparseOneDimensionalModel::Point point = *points.begin();

    ItemEditDialog *dialog = new ItemEditDialog
        (m_model->getSampleRate(),
         ItemEditDialog::ShowTime |
         ItemEditDialog::ShowText);

    dialog->setFrameTime(point.frame);
    dialog->setText(point.label);

    if (dialog->exec() == QDialog::Accepted) {

        SparseOneDimensionalModel::Point newPoint = point;
        newPoint.frame = dialog->getFrameTime();
        newPoint.label = dialog->getText();
        
        SparseOneDimensionalModel::EditCommand *command =
            new SparseOneDimensionalModel::EditCommand(m_model, tr("Edit Point"));
        command->deletePoint(point);
        command->addPoint(newPoint);
        command->finish();
    }

    delete dialog;
    return true;
}

void
TimeInstantLayer::moveSelection(Selection s, size_t newStartFrame)
{
    if (!m_model) return;

    SparseOneDimensionalModel::EditCommand *command =
	new SparseOneDimensionalModel::EditCommand(m_model,
						   tr("Drag Selection"));

    SparseOneDimensionalModel::PointList points =
	m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    for (SparseOneDimensionalModel::PointList::iterator i = points.begin();
	 i != points.end(); ++i) {

	if (s.contains(i->frame)) {
	    SparseOneDimensionalModel::Point newPoint(*i);
	    newPoint.frame = i->frame + newStartFrame - s.getStartFrame();
	    command->deletePoint(*i);
	    command->addPoint(newPoint);
	}
    }

    command->finish();
}

void
TimeInstantLayer::resizeSelection(Selection s, Selection newSize)
{
    if (!m_model) return;

    SparseOneDimensionalModel::EditCommand *command =
	new SparseOneDimensionalModel::EditCommand(m_model,
						   tr("Resize Selection"));

    SparseOneDimensionalModel::PointList points =
	m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    double ratio =
	double(newSize.getEndFrame() - newSize.getStartFrame()) /
	double(s.getEndFrame() - s.getStartFrame());

    for (SparseOneDimensionalModel::PointList::iterator i = points.begin();
	 i != points.end(); ++i) {

	if (s.contains(i->frame)) {

	    double target = i->frame;
	    target = newSize.getStartFrame() + 
		double(target - s.getStartFrame()) * ratio;

	    SparseOneDimensionalModel::Point newPoint(*i);
	    newPoint.frame = lrint(target);
	    command->deletePoint(*i);
	    command->addPoint(newPoint);
	}
    }

    command->finish();
}

void
TimeInstantLayer::deleteSelection(Selection s)
{
    if (!m_model) return;

    SparseOneDimensionalModel::EditCommand *command =
	new SparseOneDimensionalModel::EditCommand(m_model,
						   tr("Delete Selection"));

    SparseOneDimensionalModel::PointList points =
	m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    for (SparseOneDimensionalModel::PointList::iterator i = points.begin();
	 i != points.end(); ++i) {
	if (s.contains(i->frame)) command->deletePoint(*i);
    }

    command->finish();
}

void
TimeInstantLayer::copy(Selection s, Clipboard &to)
{
    if (!m_model) return;

    SparseOneDimensionalModel::PointList points =
	m_model->getPoints(s.getStartFrame(), s.getEndFrame());

    for (SparseOneDimensionalModel::PointList::iterator i = points.begin();
	 i != points.end(); ++i) {
	if (s.contains(i->frame)) {
            Clipboard::Point point(i->frame, i->label);
            to.addPoint(point);
        }
    }
}

bool
TimeInstantLayer::paste(const Clipboard &from, int frameOffset, bool)
{
    if (!m_model) return false;

    const Clipboard::PointList &points = from.getPoints();

    SparseOneDimensionalModel::EditCommand *command =
	new SparseOneDimensionalModel::EditCommand(m_model, tr("Paste"));

    for (Clipboard::PointList::const_iterator i = points.begin();
         i != points.end(); ++i) {
        
        if (!i->haveFrame()) continue;
        size_t frame = 0;
        if (frameOffset > 0 || -frameOffset < i->getFrame()) {
            frame = i->getFrame() + frameOffset;
        }
        SparseOneDimensionalModel::Point newPoint(frame);
        if (i->haveLabel()) {
            newPoint.label = i->getLabel();
        } else if (i->haveValue()) {
            newPoint.label = QString("%1").arg(i->getValue());
        }
        
        command->addPoint(newPoint);
    }

    command->finish();
    return true;
}

QString
TimeInstantLayer::toXmlString(QString indent, QString extraAttributes) const
{
    return Layer::toXmlString(indent, extraAttributes +
			      QString(" colour=\"%1\" plotStyle=\"%2\"")
			      .arg(encodeColour(m_colour)).arg(m_plotStyle));
}

void
TimeInstantLayer::setProperties(const QXmlAttributes &attributes)
{
    QString colourSpec = attributes.value("colour");
    if (colourSpec != "") {
	QColor colour(colourSpec);
	if (colour.isValid()) {
	    setBaseColour(QColor(colourSpec));
	}
    }

    bool ok;
    PlotStyle style = (PlotStyle)
	attributes.value("plotStyle").toInt(&ok);
    if (ok) setPlotStyle(style);
}


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
#include "base/ColourDatabase.h"

#include "data/model/SparseOneDimensionalModel.h"

#include "widgets/ItemEditDialog.h"
#include "widgets/ListInputDialog.h"

#include <QPainter>
#include <QMouseEvent>
#include <QTextStream>

#include <iostream>
#include <cmath>

TimeInstantLayer::TimeInstantLayer() :
    SingleColourLayer(),
    m_model(0),
    m_editing(false),
    m_editingPoint(0, tr("New Point")),
    m_editingCommand(0),
    m_plotStyle(PlotInstants)
{
}

TimeInstantLayer::~TimeInstantLayer()
{
}

void
TimeInstantLayer::setModel(SparseOneDimensionalModel *model)
{
    if (m_model == model) return;
    m_model = model;

    connectSignals(m_model);

    std::cerr << "TimeInstantLayer::setModel(" << model << ")" << std::endl;

    emit modelReplaced();
}

Layer::PropertyList
TimeInstantLayer::getProperties() const
{
    PropertyList list = SingleColourLayer::getProperties();
    list.push_back("Plot Type");
    return list;
}

QString
TimeInstantLayer::getPropertyLabel(const PropertyName &name) const
{
    if (name == "Plot Type") return tr("Plot Type");
    return SingleColourLayer::getPropertyLabel(name);
}

Layer::PropertyType
TimeInstantLayer::getPropertyType(const PropertyName &name) const
{
    if (name == "Plot Type") return ValueProperty;
    return SingleColourLayer::getPropertyType(name);
}

int
TimeInstantLayer::getPropertyRangeAndValue(const PropertyName &name,
                                           int *min, int *max, int *deflt) const
{
    int val = 0;

    if (name == "Plot Type") {
	
	if (min) *min = 0;
	if (max) *max = 1;
        if (deflt) *deflt = 0;
	
	val = int(m_plotStyle);

    } else {
	
	val = SingleColourLayer::getPropertyRangeAndValue(name, min, max, deflt);
    }

    return val;
}

QString
TimeInstantLayer::getPropertyValueLabel(const PropertyName &name,
                                        int value) const
{
    if (name == "Plot Type") {
	switch (value) {
	default:
	case 0: return tr("Instants");
	case 1: return tr("Segmentation");
	}
    }
    return SingleColourLayer::getPropertyValueLabel(name, value);
}

void
TimeInstantLayer::setProperty(const PropertyName &name, int value)
{
    if (name == "Plot Type") {
	setPlotStyle(PlotStyle(value));
    } else {
        SingleColourLayer::setProperty(name, value);
    }
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

    paint.setPen(getBaseQColor());

    QColor brushColour(getBaseQColor());
    brushColour.setAlpha(100);
    paint.setBrush(brushColour);

    QColor oddBrushColour(brushColour);
    if (m_plotStyle == PlotSegmentation) {
	if (getBaseQColor() == Qt::black) {
	    oddBrushColour = Qt::gray;
	} else if (getBaseQColor() == Qt::darkRed) {
	    oddBrushColour = Qt::red;
	} else if (getBaseQColor() == Qt::darkBlue) {
	    oddBrushColour = Qt::blue;
	} else if (getBaseQColor() == Qt::darkGreen) {
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
	    paint.setPen(getForegroundQColor(v));
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

	paint.setPen(getBaseQColor());
	
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
TimeInstantLayer::eraseStart(View *v, QMouseEvent *e)
{
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
TimeInstantLayer::eraseDrag(View *v, QMouseEvent *e)
{
}

void
TimeInstantLayer::eraseEnd(View *v, QMouseEvent *e)
{
    if (!m_model || !m_editing) return;

    m_editing = false;

    SparseOneDimensionalModel::PointList points = getLocalPoints(v, e->x());
    if (points.empty()) return;
    if (points.begin()->frame != m_editingPoint.frame) return;

    m_editingCommand = new SparseOneDimensionalModel::EditCommand
        (m_model, tr("Erase Point"));

    m_editingCommand->deletePoint(m_editingPoint);

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

    //!!! This fails, because simply being "on the same pane as" a
    // particular model is not enough to give this layer the same
    // alignment as it.  If it was generated by deriving from another
    // layer's model, that would be... but it wasn't necessarily

            point.setReferenceFrame(m_model->alignToReference(i->frame));
            to.addPoint(point);
        }
    }
}

bool
TimeInstantLayer::clipboardAlignmentDiffers(const Clipboard &clip) const
{
    //!!! hoist -- all pastable layers will need this

    //!!! This fails, because simply being "on the same pane as" a
    // particular model is not enough to give this layer the same
    // alignment as it.  If it was generated by deriving from another
    // layer's model, that would be... but it wasn't necessarily

    if (!m_model) return false;

    std::cerr << "TimeInstantLayer::clipboardAlignmentDiffers" << std::endl;

    for (Clipboard::PointList::const_iterator i = clip.getPoints().begin();
         i != clip.getPoints().end(); ++i) {

        // In principle, we want to know whether the aligned version
        // of the reference frame in our model is the same as the
        // source frame contained in the clipboard point.  However,
        // because of rounding during alignment, that won't
        // necessarily be the case even if the clipboard point came
        // from our model!  What we need to check is whether, if we
        // aligned the clipboard point's frame back to the reference
        // using this model's alignment, we would obtain the same
        // reference frame as that for the clipboard point.

        // What if the clipboard point has no reference frame?  Then
        // we have to treat it as having its own frame as the
        // reference (i.e. having been copied from the reference
        // model).
        
        long sourceFrame = i->getFrame();
        long referenceFrame = sourceFrame;
        if (i->haveReferenceFrame()) {
            referenceFrame = i->getReferenceFrame();
        }
        long myMappedFrame = m_model->alignToReference(sourceFrame);

        std::cerr << "sourceFrame = " << sourceFrame << ", referenceFrame = " << referenceFrame << " (have = " << i->haveReferenceFrame() << "), myMappedFrame = " << myMappedFrame << std::endl;

        if (myMappedFrame != referenceFrame) return true;
    }

    return false;
}

bool
TimeInstantLayer::paste(const Clipboard &from, int frameOffset, bool)
{
    if (!m_model) return false;

    const Clipboard::PointList &points = from.getPoints();

    //!!!
    
    // Clipboard::haveReferenceFrames() will return true if any of the
    // items in the clipboard came from an aligned, non-reference model.
    
    // We need to know whether these points came from our model or not
    // -- if they did, we don't want to align them.

    // If they didn't come from our model, and if reference frames are
    // available, then we want to offer to align them.  If reference
    // frames are unavailable but they came from the reference model,
    // we want to offer to align them too.


    //!!!

    // Each point may have a reference frame that may differ from the
    // point's given frame (in its source model).  If it has no
    // reference frame, we have to assume the source model was not
    // aligned or was the reference model: when cutting or copying
    // points from a layer, we must always set their reference frame
    // correctly if we are aligned.
    // 
    // When pasting:
    // - if point's reference and aligned frames differ:
    //   - if this layer is aligned:
    //     - if point's aligned frame matches this layer's aligned version
    //       of point's reference frame:
    //       - we can paste at reference frame or our frame
    //     - else
    //       - we can paste at reference frame, result of aligning reference
    //         frame in our model, or literal source frame
    //   - else
    //     - we can paste at reference (our) frame, or literal source frame
    // - else
    //   - if this layer is aligned:
    //     - we can paste at reference (point's only available) frame,
    //       or result of aligning reference frame in our model
    //   - else
    //     - we can only paste at reference frame
    // 
    // Which of these alternatives are useful?
    //
    // Example: we paste between two tracks that are aligned to the
    // same reference, and the points are at 10s and 20s in the source
    // track, corresponding to 5s and 10s in the reference but 20s and
    // 30s in the target track.
    // 
    // The obvious default is to paste at 20s and 30s; if we aren't
    // doing that, would it be better to paste at 5s and 10s or at 10s
    // and 20s?  We probably don't ever want to do the former, do we?
    // We either want to be literal all the way through, or aligned
    // all the way through.

    bool realign = false;

    if (clipboardAlignmentDiffers(from)) {

        std::cerr << "Offer alignment option..." << std::endl;

        QStringList options;
        options << "Use times unchanged from the original layer";
        options << "Re-align times to match the same points in the reference layer";

        bool ok = false;

        QString selected = ListInputDialog::getItem
            (0, tr("Choose alignment"),
             tr("The points you are pasting originated in a layer with different alignment from the current layer.  Would you like to re-align them when pasting?"),
             options, 0, &ok);
        if (!ok) return false;

        if (selected == options[1]) realign = true;
    }


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

int
TimeInstantLayer::getDefaultColourHint(bool darkbg, bool &impose)
{
    impose = false;
    return ColourDatabase::getInstance()->getColourIndex
        (QString(darkbg ? "Bright Purple" : "Purple"));
}

void
TimeInstantLayer::toXml(QTextStream &stream,
                        QString indent, QString extraAttributes) const
{
    SingleColourLayer::toXml(stream, indent,
                             extraAttributes +
                             QString(" plotStyle=\"%1\"")
                             .arg(m_plotStyle));
}

void
TimeInstantLayer::setProperties(const QXmlAttributes &attributes)
{
    SingleColourLayer::setProperties(attributes);

    bool ok;
    PlotStyle style = (PlotStyle)
	attributes.value("plotStyle").toInt(&ok);
    if (ok) setPlotStyle(style);
}


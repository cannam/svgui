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

#ifndef _TIME_VALUE_LAYER_H_
#define _TIME_VALUE_LAYER_H_

#include "Layer.h"
#include "data/model/SparseTimeValueModel.h"

#include <QObject>
#include <QColor>

class View;
class QPainter;

class TimeValueLayer : public Layer
{
    Q_OBJECT

public:
    TimeValueLayer();

    virtual void paint(View *v, QPainter &paint, QRect rect) const;

    virtual int getVerticalScaleWidth(View *v, QPainter &) const;
    virtual void paintVerticalScale(View *v, QPainter &paint, QRect rect) const;

    virtual QString getFeatureDescription(View *v, QPoint &) const;

    virtual bool snapToFeatureFrame(View *v, int &frame,
				    size_t &resolution,
				    SnapType snap) const;

    virtual void drawStart(View *v, QMouseEvent *);
    virtual void drawDrag(View *v, QMouseEvent *);
    virtual void drawEnd(View *v, QMouseEvent *);

    virtual void editStart(View *v, QMouseEvent *);
    virtual void editDrag(View *v, QMouseEvent *);
    virtual void editEnd(View *v, QMouseEvent *);

    virtual bool editOpen(View *v, QMouseEvent *);

    virtual void moveSelection(Selection s, size_t newStartFrame);
    virtual void resizeSelection(Selection s, Selection newSize);
    virtual void deleteSelection(Selection s);

    virtual void copy(Selection s, Clipboard &to);
    virtual bool paste(const Clipboard &from, int frameOffset,
                       bool interactive);

    virtual const Model *getModel() const { return m_model; }
    void setModel(SparseTimeValueModel *model);

    virtual PropertyList getProperties() const;
    virtual QString getPropertyLabel(const PropertyName &) const;
    virtual PropertyType getPropertyType(const PropertyName &) const;
    virtual QString getPropertyGroupName(const PropertyName &) const;
    virtual int getPropertyRangeAndValue(const PropertyName &,
                                         int *min, int *max, int *deflt) const;
    virtual QString getPropertyValueLabel(const PropertyName &,
					  int value) const;
    virtual void setProperty(const PropertyName &, int value);

    void setBaseColour(QColor);
    QColor getBaseColour() const { return m_colour; }

    void setFillColourMap(int);
    int getFillColourMap() const { return m_colourMap; }

    enum PlotStyle {
	PlotPoints,
	PlotStems,
	PlotConnectedPoints,
	PlotLines,
	PlotCurve,
	PlotSegmentation
    };

    void setPlotStyle(PlotStyle style);
    PlotStyle getPlotStyle() const { return m_plotStyle; }

    enum VerticalScale {
        AutoAlignScale,
        LinearScale,
        LogScale,
        PlusMinusOneScale
    };
    
    void setVerticalScale(VerticalScale scale);
    VerticalScale getVerticalScale() const { return m_verticalScale; }

    virtual bool isLayerScrollable(const View *v) const;

    virtual bool isLayerEditable() const { return true; }

    virtual int getCompletion(View *) const { return m_model->getCompletion(); }

    virtual bool needsTextLabelHeight() const {
        return m_plotStyle == PlotSegmentation && m_model->hasTextLabels();
    }

    virtual bool getValueExtents(float &min, float &max,
                                 bool &logarithmic, QString &unit) const;

    virtual bool getDisplayExtents(float &min, float &max) const;

    virtual QString toXmlString(QString indent = "",
				QString extraAttributes = "") const;

    void setProperties(const QXmlAttributes &attributes);

protected:
    void getScaleExtents(View *, float &min, float &max, bool &log) const;
    int getYForValue(View *, float value) const;
    float getValueForY(View *, int y) const;
    QColor getColourForValue(View *v, float value) const;

    SparseTimeValueModel::PointList getLocalPoints(View *v, int) const;

    SparseTimeValueModel *m_model;
    bool m_editing;
    SparseTimeValueModel::Point m_originalPoint;
    SparseTimeValueModel::Point m_editingPoint;
    SparseTimeValueModel::EditCommand *m_editingCommand;
    QColor m_colour;
    int m_colourMap;
    PlotStyle m_plotStyle;
    VerticalScale m_verticalScale;
};

#endif


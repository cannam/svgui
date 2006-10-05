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

#ifndef _COLOUR_3D_PLOT_H_
#define _COLOUR_3D_PLOT_H_

#include "Layer.h"

#include "data/model/DenseThreeDimensionalModel.h"

class View;
class QPainter;
class QImage;

/**
 * This is a view that displays dense 3-D data (time, some sort of
 * binned y-axis range, value) as a colour plot with value mapped to
 * colour range.  Its source is a DenseThreeDimensionalModel.
 *
 * This was the original implementation for the spectrogram view, but
 * it was replaced with a more efficient implementation that derived
 * the spectrogram itself from a DenseTimeValueModel instead of using
 * a three-dimensional model.  This class is retained in case it
 * becomes useful, but it will probably need some cleaning up if it's
 * ever actually used.
 */

class Colour3DPlotLayer : public Layer
{
    Q_OBJECT

public:
    Colour3DPlotLayer();
    ~Colour3DPlotLayer();

    virtual const ZoomConstraint *getZoomConstraint() const {
        return m_model ? m_model->getZoomConstraint() : 0;
    }
    virtual const Model *getModel() const { return m_model; }
    virtual void paint(View *v, QPainter &paint, QRect rect) const;

    virtual int getVerticalScaleWidth(View *v, QPainter &) const;
    virtual void paintVerticalScale(View *v, QPainter &paint, QRect rect) const;

    virtual QString getFeatureDescription(View *v, QPoint &) const;

    virtual bool snapToFeatureFrame(View *v, int &frame, 
				    size_t &resolution,
				    SnapType snap) const;

    virtual bool isLayerScrollable(const View *v) const;

    void setModel(const DenseThreeDimensionalModel *model);

    virtual int getCompletion(View *) const { return m_model->getCompletion(); }

    virtual bool getValueExtents(float &, float &, bool &, QString &) const { return false; }

    virtual QString getPropertyLabel(const PropertyName &) const { return ""; }
/*
    virtual PropertyList getProperties() const;
    virtual PropertyType getPropertyType(const PropertyName &) const;
    virtual int getPropertyRangeAndValue(const PropertyName &,
					   int *min, int *max) const;
    virtual QString getPropertyValueLabel(const PropertyName &,
					  int value) const;
    virtual void setProperty(const PropertyName &, int value);
*/

    void setProperties(const QXmlAttributes &) { }
    
protected slots:
    void cacheInvalid();
    void cacheInvalid(size_t startFrame, size_t endFrame);

protected:
    const DenseThreeDimensionalModel *m_model; // I do not own this
    
    mutable QImage *m_cache;

    virtual void paintDense(View *v, QPainter &paint, QRect rect) const;
};

#endif

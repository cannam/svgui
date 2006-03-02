/* -*- c-basic-offset: 4 -*-  vi:set ts=8 sts=4 sw=4: */

/*
    A waveform viewer and audio annotation editor.
    Chris Cannam, Queen Mary University of London, 2005-2006
    
    This is experimental software.  Not for distribution.
*/

#ifndef _WAVEFORM_LAYER_H_
#define _WAVEFORM_LAYER_H_

#include <QRect>
#include <QColor>

#include "base/Layer.h"

#include "model/RangeSummarisableTimeValueModel.h"

class View;
class QPainter;
class QPixmap;

class WaveformLayer : public Layer
{
    Q_OBJECT

public:
    WaveformLayer();
    ~WaveformLayer();

    virtual const ZoomConstraint *getZoomConstraint() const { return m_model; }
    virtual const Model *getModel() const { return m_model; }
    virtual void paint(View *v, QPainter &paint, QRect rect) const;

    virtual QString getFeatureDescription(View *v, QPoint &) const;

    virtual int getVerticalScaleWidth(View *v, QPainter &) const;
    virtual void paintVerticalScale(View *v, QPainter &paint, QRect rect) const;

    void setModel(const RangeSummarisableTimeValueModel *model);

    virtual PropertyList getProperties() const;
    virtual PropertyType getPropertyType(const PropertyName &) const;
    virtual QString getPropertyGroupName(const PropertyName &) const;
    virtual int getPropertyRangeAndValue(const PropertyName &,
					   int *min, int *max) const;
    virtual QString getPropertyValueLabel(const PropertyName &,
					  int value) const;
    virtual void setProperty(const PropertyName &, int value);

    /**
     * Set the gain multiplier for sample values in this view.
     *
     * The default is 1.0.
     */
    void setGain(float gain);
    float getGain() const { return m_gain; }

    /**
     * Set the basic display colour for waveforms.
     *
     * The default is black.
     *!!! NB should default to white if the associated View !hasLightBackground()
     */
    void setBaseColour(QColor);
    QColor getBaseColour() const { return m_colour; }

    /**
     * Set whether to display mean values as a lighter-coloured area
     * beneath the peaks.  Rendering will be slightly faster without
     * but arguably prettier with.
     *
     * The default is to display means.
     */
    void setShowMeans(bool);
    bool getShowMeans() const { return m_showMeans; }

    /**
     * Set whether to use shades of grey (or of the base colour) to
     * provide additional perceived vertical resolution (i.e. using
     * half-filled pixels to represent levels that only just meet the
     * pixel unit boundary).  This provides a small improvement in
     * waveform quality at a small cost in rendering speed.
     * 
     * The default is to use greyscale.
     */
    void setUseGreyscale(bool);
    bool getUseGreyscale() const { return m_greyscale; }


    enum ChannelMode { SeparateChannels, MergeChannels };

    /**
     * Specify whether multi-channel audio data should be displayed
     * with a separate axis per channel (SeparateChannels), or with a
     * single synthetic axis showing channel 0 above the axis and
     * channel 1 below (MergeChannels).
     * 
     * MergeChannels does not work for files with more than 2
     * channels.
     * 
     * The default is SeparateChannels.
     */
    void setChannelMode(ChannelMode);
    ChannelMode getChannelMode() const { return m_channelMode; }


    /**
     * Specify the channel to use from the source model.  A value of
     * -1 means to show all available channels (laid out to the
     * channel mode). The default is -1.
     */
    void setChannel(int);
    int getChannel() const { return m_channel; }


    enum Scale { LinearScale, MeterScale, dBScale };

    /**
     * Specify the vertical scale for sample levels.  With LinearScale,
     * the scale is directly proportional to the raw [-1, +1)
     * floating-point audio sample values.  With dBScale the
     * vertical scale is proportional to dB level (truncated at
     * -50dB).  MeterScale provides a hybrid variable scale based on
     * IEC meter scale, intended to provide a clear overview at
     * relatively small heights.
     *
     * Note that the effective gain (see setGain()) is applied before
     * vertical scaling.
     *
     * The default is LinearScale.
     */
    void setScale(Scale);
    Scale getScale() const { return m_scale; }

    /**
     * Enable or disable aggressive pixmap cacheing.  If enabled,
     * waveforms will be rendered to an off-screen pixmap and
     * refreshed from there instead of being redrawn from the peak
     * data each time.  This may be faster if the data and zoom level
     * do not change often, but it may be slower for frequently zoomed
     * data and it will only work if the waveform is the "bottom"
     * layer on the displayed widget, as each refresh will erase
     * anything beneath the waveform.
     *
     * This is intended specifically for a panner widget display in
     * which the waveform never moves, zooms, or changes, but some
     * graphic such as a panner outline is frequently redrawn over the
     * waveform.  This situation would necessitate a lot of waveform
     * refresh if the default cacheing strategy was used.
     *
     * The default is not to use aggressive cacheing.
     */
    void setAggressiveCacheing(bool);
    bool getAggressiveCacheing() const { return m_aggressive; }

    virtual int getCompletion() const;

    virtual QString toXmlString(QString indent = "",
				QString extraAttributes = "") const;

    void setProperties(const QXmlAttributes &attributes);

protected:
    int dBscale(float sample, int m) const;

    const RangeSummarisableTimeValueModel *m_model; // I do not own this

    /// Return value is number of channels displayed
    size_t getChannelArrangement(size_t &min, size_t &max, bool &merging) const;

    float        m_gain;
    QColor       m_colour;
    bool         m_showMeans;
    bool         m_greyscale;
    ChannelMode  m_channelMode;
    int          m_channel;
    Scale        m_scale;
    bool         m_aggressive;

    mutable QPixmap *m_cache;
    mutable bool m_cacheValid;
    mutable int m_cacheZoomLevel;
};

#endif

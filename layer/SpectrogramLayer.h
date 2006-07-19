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

#ifndef _SPECTROGRAM_LAYER_H_
#define _SPECTROGRAM_LAYER_H_

#include "base/Layer.h"
#include "base/Window.h"
#include "base/RealTime.h"
#include "base/Thread.h"
#include "model/PowerOfSqrtTwoZoomConstraint.h"
#include "model/DenseTimeValueModel.h"

#include "fileio/FFTFuzzyAdapter.h"

#include <QMutex>
#include <QWaitCondition>
#include <QImage>
#include <QPixmap>

class View;
class QPainter;
class QImage;
class QPixmap;
class QTimer;
class FFTFuzzyAdapter;


/**
 * SpectrogramLayer represents waveform data (obtained from a
 * DenseTimeValueModel) in spectrogram form.
 */

class SpectrogramLayer : public Layer,
			 public PowerOfSqrtTwoZoomConstraint
{
    Q_OBJECT

public:
    enum Configuration { FullRangeDb, MelodicRange, MelodicPeaks };
    
    SpectrogramLayer(Configuration = FullRangeDb);
    ~SpectrogramLayer();

    virtual const ZoomConstraint *getZoomConstraint() const { return this; }
    virtual const Model *getModel() const { return m_model; }
    virtual void paint(View *v, QPainter &paint, QRect rect) const;

    virtual int getVerticalScaleWidth(View *v, QPainter &) const;
    virtual void paintVerticalScale(View *v, QPainter &paint, QRect rect) const;

    virtual bool getCrosshairExtents(View *, QPainter &, QPoint cursorPos,
                                     std::vector<QRect> &extents) const;
    virtual void paintCrosshairs(View *, QPainter &, QPoint) const;

    virtual QString getFeatureDescription(View *v, QPoint &) const;

    virtual bool snapToFeatureFrame(View *v, int &frame,
				    size_t &resolution,
				    SnapType snap) const;

    void setModel(const DenseTimeValueModel *model);

    virtual PropertyList getProperties() const;
    virtual QString getPropertyLabel(const PropertyName &) const;
    virtual PropertyType getPropertyType(const PropertyName &) const;
    virtual QString getPropertyGroupName(const PropertyName &) const;
    virtual int getPropertyRangeAndValue(const PropertyName &,
					   int *min, int *max) const;
    virtual QString getPropertyValueLabel(const PropertyName &,
					  int value) const;
    virtual void setProperty(const PropertyName &, int value);

    /**
     * Specify the channel to use from the source model.
     * A value of -1 means to mix all available channels.
     * The default is channel 0.
     */
    void setChannel(int);
    int getChannel() const;

    void setWindowSize(size_t);
    size_t getWindowSize() const;
    
    void setWindowHopLevel(size_t level);
    size_t getWindowHopLevel() const;

    void setWindowType(WindowType type);
    WindowType getWindowType() const;

    void setZeroPadLevel(size_t level);
    size_t getZeroPadLevel() const;

    /**
     * Set the gain multiplier for sample values in this view.
     * The default is 1.0.
     */
    void setGain(float gain);
    float getGain() const;

    /**
     * Set the threshold for sample values to qualify for being shown
     * in the FFT, in voltage units.
     *
     * The default is 0.0.
     */
    void setThreshold(float threshold);
    float getThreshold() const;

    void setMinFrequency(size_t);
    size_t getMinFrequency() const;

    void setMaxFrequency(size_t); // 0 -> no maximum
    size_t getMaxFrequency() const;

    enum ColourScale {
	LinearColourScale,
	MeterColourScale,
	dBColourScale,
        OtherColourScale,
	PhaseColourScale
    };

    /**
     * Specify the scale for sample levels.  See WaveformLayer for
     * details of meter and dB scaling.  The default is dBColourScale.
     */
    void setColourScale(ColourScale);
    ColourScale getColourScale() const;

    enum FrequencyScale {
	LinearFrequencyScale,
	LogFrequencyScale
    };
    
    /**
     * Specify the scale for the y axis.
     */
    void setFrequencyScale(FrequencyScale);
    FrequencyScale getFrequencyScale() const;

    enum BinDisplay {
	AllBins,
	PeakBins,
	PeakFrequencies
    };
    
    /**
     * Specify the processing of frequency bins for the y axis.
     */
    void setBinDisplay(BinDisplay);
    BinDisplay getBinDisplay() const;

    void setNormalizeColumns(bool n);
    bool getNormalizeColumns() const;

    void setNormalizeVisibleArea(bool n);
    bool getNormalizeVisibleArea() const;

    enum ColourScheme { DefaultColours, WhiteOnBlack, BlackOnWhite,
			RedOnBlue, YellowOnBlack, BlueOnBlack, Rainbow };

    void setColourScheme(ColourScheme scheme);
    ColourScheme getColourScheme() const;

    /**
     * Specify the colourmap rotation for the colour scale.
     */
    void setColourRotation(int);
    int getColourRotation() const;

    virtual VerticalPosition getPreferredFrameCountPosition() const {
	return PositionTop;
    }

    virtual bool isLayerOpaque() const { return true; }

    float getYForFrequency(View *v, float frequency) const;
    float getFrequencyForY(View *v, int y) const;

    virtual int getCompletion(View *v) const;

    virtual bool getValueExtents(float &min, float &max,
                                 bool &logarithmic, QString &unit) const;

    virtual bool getDisplayExtents(float &min, float &max) const;

    virtual bool setDisplayExtents(float min, float max);

    virtual QString toXmlString(QString indent = "",
				QString extraAttributes = "") const;

    void setProperties(const QXmlAttributes &attributes);

    virtual void setLayerDormant(const View *v, bool dormant);

    virtual bool isLayerScrollable(const View *v) const { return false; }

protected slots:
    void cacheInvalid();
    void cacheInvalid(size_t startFrame, size_t endFrame);

    void fillTimerTimedOut();

protected:
    const DenseTimeValueModel *m_model; // I do not own this
    
    int                 m_channel;
    size_t              m_windowSize;
    WindowType          m_windowType;
    size_t              m_windowHopLevel;
    size_t              m_zeroPadLevel;
    size_t              m_fftSize;
    float               m_gain;
    float               m_threshold;
    int                 m_colourRotation;
    size_t              m_minFrequency;
    size_t              m_maxFrequency;
    ColourScale         m_colourScale;
    ColourScheme        m_colourScheme;
    QColor              m_crosshairColour;
    FrequencyScale      m_frequencyScale;
    BinDisplay          m_binDisplay;
    bool                m_normalizeColumns;
    bool                m_normalizeVisibleArea;

    enum { NO_VALUE = 0 }; // colour index for unused pixels

    class ColourMap
    {
    public:
        QColor getColour(unsigned char index) const {
            return m_colours[index];
        }
    
        void setColour(unsigned char index, QColor colour) {
            m_colours[index] = colour;
        }

    private:
        QColor m_colours[256];
    };

    ColourMap m_colourMap;

    struct PixmapCache
    {
        QPixmap pixmap;
        QRect validArea;
        long startFrame;
        size_t zoomLevel;
    };
    typedef std::map<const View *, PixmapCache> ViewPixmapCache;
    void invalidatePixmapCaches();
    void invalidatePixmapCaches(size_t startFrame, size_t endFrame);
    mutable ViewPixmapCache m_pixmapCaches;
    mutable QImage m_drawBuffer;

    mutable QTimer *m_updateTimer;

    mutable size_t m_candidateFillStartFrame;
    bool m_exiting;

    void setColourmap();
    void rotateColourmap(int distance);

    static float calculateFrequency(size_t bin,
				    size_t windowSize,
				    size_t windowIncrement,
				    size_t sampleRate,
				    float previousPhase,
				    float currentPhase,
				    bool &steadyState);

    unsigned char getDisplayValue(View *v, float input) const;
    float getInputForDisplayValue(unsigned char uc) const;

    int getColourScaleWidth(QPainter &) const;

    float getEffectiveMinFrequency() const;
    float getEffectiveMaxFrequency() const;

    struct LayerRange {
	long   startFrame;
	int    zoomLevel;
	size_t modelStart;
	size_t modelEnd;
    };
    bool getXBinRange(View *v, int x, float &windowMin, float &windowMax) const;
    bool getYBinRange(View *v, int y, float &freqBinMin, float &freqBinMax) const;

    bool getYBinSourceRange(View *v, int y, float &freqMin, float &freqMax) const;
    bool getAdjustedYBinSourceRange(View *v, int x, int y,
				    float &freqMin, float &freqMax,
				    float &adjFreqMin, float &adjFreqMax) const;
    bool getXBinSourceRange(View *v, int x, RealTime &timeMin, RealTime &timeMax) const;
    bool getXYBinSourceRange(View *v, int x, int y, float &min, float &max,
			     float &phaseMin, float &phaseMax) const;

    size_t getWindowIncrement() const {
        if (m_windowHopLevel == 0) return m_windowSize;
        else if (m_windowHopLevel == 1) return (m_windowSize * 3) / 4;
        else return m_windowSize / (1 << (m_windowHopLevel - 1));
    }

    size_t getZeroPadLevel(const View *v) const;
    size_t getFFTSize(const View *v) const;
    FFTFuzzyAdapter *getFFTAdapter(const View *v) const;
    void invalidateFFTAdapters();

    typedef std::pair<FFTFuzzyAdapter *, int> FFTFillPair; // adapter, last fill
    typedef std::map<const View *, FFTFillPair> ViewFFTMap;
    typedef std::vector<float> FloatVector;
    mutable ViewFFTMap m_fftAdapters;

    class MagnitudeRange {
    public:
        MagnitudeRange() : m_min(0), m_max(0) { }
        bool operator==(const MagnitudeRange &r) {
            return r.m_min == m_min && r.m_max == m_max;
        }
        bool isSet() const { return (m_min != 0 || m_max != 0); }
        void set(float min, float max) {
            m_min = convert(min);
            m_max = convert(max);
            if (m_max < m_min) m_max = m_min;
        }
        bool sample(float f) {
            unsigned int ui = convert(f);
            bool changed = false;
            if (isSet()) {
                if (ui < m_min) { m_min = ui; changed = true; }
                if (ui > m_max) { m_max = ui; changed = true; }
            } else {
                m_max = m_min = ui;
                changed = true;
            }
            return changed;
        }            
        bool sample(const MagnitudeRange &r) {
            bool changed = false;
            if (isSet()) {
                if (r.m_min < m_min) { m_min = r.m_min; changed = true; }
                if (r.m_max > m_max) { m_max = r.m_max; changed = true; }
            } else {
                m_min = r.m_min;
                m_max = r.m_max;
                changed = true;
            }
            return changed;
        }            
        float getMin() const { return float(m_min) / UINT_MAX; }
        float getMax() const { return float(m_max) / UINT_MAX; }
    private:
        unsigned int m_min;
        unsigned int m_max;
        unsigned int convert(float f) {
            if (f < 0.f) f = 0.f;
            if (f > 1.f) f = 1.f;
            return (unsigned int)(f * UINT_MAX);
        }
    };

    typedef std::map<const View *, MagnitudeRange> ViewMagMap;
    mutable ViewMagMap m_viewMags;
    mutable std::vector<MagnitudeRange> m_columnMags;
    void invalidateMagnitudes();
    bool updateViewMagnitudes(View *v) const;
};

#endif

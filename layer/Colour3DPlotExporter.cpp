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

#include "Colour3DPlotExporter.h"

#include "data/model/EditableDenseThreeDimensionalModel.h"
#include "data/model/FFTModel.h"

#include "VerticalBinLayer.h"

Colour3DPlotExporter::Colour3DPlotExporter(Sources sources, Parameters params) :
    m_sources(sources),
    m_params(params)
{
    SVCERR << "Colour3DPlotExporter::Colour3DPlotExporter: constructed at "
           << this << endl;
}

Colour3DPlotExporter::~Colour3DPlotExporter()
{
    SVCERR << "Colour3DPlotExporter[" << this << "]::~Colour3DPlotExporter"
           << endl;
}

void
Colour3DPlotExporter::discardSources()
{
    SVCERR << "Colour3DPlotExporter[" << this << "]::discardSources"
           << endl;
    QMutexLocker locker(&m_mutex);
    m_sources.verticalBinLayer = nullptr;
    m_sources.source = {};
    m_sources.fft = {};
    m_sources.provider = nullptr;
}

QString
Colour3DPlotExporter::toDelimitedDataString(QString delimiter,
                                            DataExportOptions options,
                                            sv_frame_t startFrame,
                                            sv_frame_t duration) const
{
    QMutexLocker locker(&m_mutex);

    BinDisplay binDisplay = m_params.binDisplay;

    (void)options; //!!!

    auto model =
        ModelById::getAs<DenseThreeDimensionalModel>(m_sources.source);
    auto fftModel =
        ModelById::getAs<FFTModel>(m_sources.fft);

    auto layer = m_sources.verticalBinLayer;
    auto provider = m_sources.provider;

    if (!model || !layer) {
        SVCERR << "ERROR: Colour3DPlotExporter::toDelimitedDataString: Source model and layer required" << endl;
        return {};
    }
    if ((binDisplay == BinDisplay::PeakFrequencies) && !fftModel) {
        SVCERR << "ERROR: Colour3DPlotExporter::toDelimitedDataString: FFT model required in peak frequencies mode" << endl;
        return {};
    }

    int minbin = 0;
    int sh = model->getHeight();
    int nbins = sh;

    //!!! todo: consider what to do about the actual Colour 3D Plot
    //!!! Layer. In the existing application, this is exported full
    //!!! height. If we switch to using this code, we will be
    //!!! exporting only the displayed height. This is backward
    //!!! incompatible, but also not directly interpretable without
    //!!! any guide in the exported file as to what the bin indices
    //!!! are. Perhaps we should have a flag to export full height,
    //!!! and default to using it.

    //!!! todo: what about the other export types besides
    //!!! delimited-data-string ?

    //!!! todo: export selections only (we have the necessaries here,
    //!!! but it needs support higher up)

    //!!! todo: option to include timestamps for columns
    
    if (provider) {

        minbin = layer->getIBinForY(provider, provider->getPaintHeight());
        if (minbin >= sh) minbin = sh - 1;
        if (minbin < 0) minbin = 0;
    
        nbins = layer->getIBinForY(provider, 0) - minbin + 1;
        if (minbin + nbins > sh) nbins = sh - minbin;
    }

    int w = model->getWidth();

    QString s;
    
    for (int i = 0; i < w; ++i) {
        
        sv_frame_t fr = model->getStartFrame() + i * model->getResolution();
        if (fr < startFrame || fr >= startFrame + duration) {
            continue;
        }
        
        //!!! (+ phase layer type)

        auto column = model->getColumn(i);
        column = ColumnOp::Column(column.data() + minbin,
                                  column.data() + minbin + nbins);

        // The scale factor is always applied
        column = ColumnOp::applyGain(column, m_params.scaleFactor);
        
        QStringList list;

        switch (m_params.timestampFormat) {
        case TimestampFormat::None:
            break;
        case TimestampFormat::Frames:
            list << QString("%1").arg(fr);
            break;
        case TimestampFormat::Seconds:
            list << QString("%1").arg(RealTime::frame2RealTime
                                      (fr, model->getSampleRate()).toDouble());
            break;
        }
        
        if (binDisplay == BinDisplay::PeakFrequencies) {
            
            FFTModel::PeakSet peaks = fftModel->getPeakFrequencies
                (FFTModel::AllPeaks, i, minbin, minbin + nbins - 1);

            // We don't apply normalisation or gain to the output, but
            // we *do* perform thresholding when exporting the
            // peak-frequency spectrogram, to give the user an
            // opportunity to cut irrelevant peaks. And to make that
            // match the display, we have to apply both normalisation
            // and gain locally for thresholding

            auto toTest = ColumnOp::normalize(column, m_params.normalization);
            toTest = ColumnOp::applyGain(toTest, m_params.gain);
            
            for (const auto &p: peaks) {

                int bin = p.first;

                if (toTest[bin - minbin] < m_params.threshold) {
                    continue;
                }

                double freq = p.second;
                double value = column[bin - minbin];
                
                list << QString("%1").arg(freq) << QString("%1").arg(value);
            }

        } else {
        
            if (binDisplay == BinDisplay::PeakBins) {
                column = ColumnOp::peakPick(column);
            }
        
            for (auto value: column) {
                list << QString("%1").arg(value);
            }
        }
        
        s += list.join(delimiter) + "\n";
    }

    return s;
}

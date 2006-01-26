/* -*- c-basic-offset: 4 -*-  vi:set ts=8 sts=4 sw=4: */

/*
    A waveform viewer and audio annotation editor.
    Chris Cannam, Queen Mary University of London, 2005-2006
    
    This is experimental software.  Not for distribution.
*/

#include "LayerFactory.h"

#include "WaveformLayer.h"
#include "SpectrogramLayer.h"
#include "TimeRulerLayer.h"
#include "TimeInstantLayer.h"
#include "TimeValueLayer.h"
#include "Colour3DPlotLayer.h"

#include "model/RangeSummarisableTimeValueModel.h"
#include "model/DenseTimeValueModel.h"
#include "model/SparseOneDimensionalModel.h"
#include "model/SparseTimeValueModel.h"
#include "model/DenseThreeDimensionalModel.h"

LayerFactory *
LayerFactory::m_instance = new LayerFactory;

LayerFactory *
LayerFactory::instance()
{
    return m_instance;
}

LayerFactory::~LayerFactory()
{
}

QString
LayerFactory::getLayerPresentationName(LayerType type)
{
    switch (type) {
    case Waveform:     return Layer::tr("Waveform");
    case Spectrogram:  return Layer::tr("Spectrogram");
    case TimeRuler:    return Layer::tr("Ruler");
    case TimeInstants: return Layer::tr("Time Instants");
    case TimeValues:   return Layer::tr("Time Values");
    case Colour3DPlot: return Layer::tr("Colour 3D Plot");

    case MelodicRangeSpectrogram:
	// The user can change all the parameters of this after the
	// fact -- there's nothing permanently melodic-range about it
	// that should be encoded in its name
	return Layer::tr("Spectrogram");

    default: break;
    }

    return Layer::tr("Layer");
}

LayerFactory::LayerTypeSet
LayerFactory::getValidLayerTypes(Model *model)
{
    LayerTypeSet types;

    if (dynamic_cast<DenseThreeDimensionalModel *>(model)) {
	types.insert(Colour3DPlot);
    }

    if (dynamic_cast<DenseTimeValueModel *>(model)) {
	types.insert(Spectrogram);
	types.insert(MelodicRangeSpectrogram);
    }

    if (dynamic_cast<RangeSummarisableTimeValueModel *>(model)) {
	types.insert(Waveform);
    }

    if (dynamic_cast<SparseOneDimensionalModel *>(model)) {
	types.insert(TimeInstants);
    }

    if (dynamic_cast<SparseTimeValueModel *>(model)) {
	types.insert(TimeValues);
    }

    // We don't count TimeRuler here as it doesn't actually display
    // the data, although it can be backed by any model

    return types;
}

LayerFactory::LayerTypeSet
LayerFactory::getValidEmptyLayerTypes()
{
    LayerTypeSet types;
    types.insert(TimeInstants);
    types.insert(TimeValues);
    //!!! and in principle Colour3DPlot -- now that's a challenge
    return types;
}

LayerFactory::LayerType
LayerFactory::getLayerType(const Layer *layer)
{
    if (dynamic_cast<const WaveformLayer *>(layer)) return Waveform;
    if (dynamic_cast<const SpectrogramLayer *>(layer)) return Spectrogram;
    if (dynamic_cast<const TimeRulerLayer *>(layer)) return TimeRuler;
    if (dynamic_cast<const TimeInstantLayer *>(layer)) return TimeInstants;
    if (dynamic_cast<const TimeValueLayer *>(layer)) return TimeValues;
    if (dynamic_cast<const Colour3DPlotLayer *>(layer)) return Colour3DPlot;
    return UnknownLayer;
}

QString
LayerFactory::getLayerIconName(LayerType type)
{
    switch (type) {
    case Waveform: return "waveform";
    case Spectrogram: return "spectrogram";
    case TimeRuler: return "timeruler";
    case TimeInstants: return "instants";
    case TimeValues: return "values";
    case Colour3DPlot: return "colour3d";
    default: return "unknown";
    }
}

QString
LayerFactory::getLayerTypeName(LayerType type)
{
    switch (type) {
    case Waveform: return "waveform";
    case Spectrogram: return "spectrogram";
    case TimeRuler: return "timeruler";
    case TimeInstants: return "timeinstants";
    case TimeValues: return "timevalues";
    case Colour3DPlot: return "colour3dplot";
    default: return "unknown";
    }
}

LayerFactory::LayerType
LayerFactory::getLayerTypeForName(QString name)
{
    if (name == "waveform") return Waveform;
    if (name == "spectrogram") return Spectrogram;
    if (name == "timeruler") return TimeRuler;
    if (name == "timeinstants") return TimeInstants;
    if (name == "timevalues") return TimeValues;
    if (name == "colour3dplot") return Colour3DPlot;
    return UnknownLayer;
}

void
LayerFactory::setModel(Layer *layer, Model *model)
{
    if (trySetModel<WaveformLayer, RangeSummarisableTimeValueModel>(layer, model))
	return;

    if (trySetModel<SpectrogramLayer, DenseTimeValueModel>(layer, model))
	return;

    if (trySetModel<TimeRulerLayer, Model>(layer, model))
	return;

    if (trySetModel<TimeInstantLayer, SparseOneDimensionalModel>(layer, model))
	return;

    if (trySetModel<TimeValueLayer, SparseTimeValueModel>(layer, model))
	return;

    if (trySetModel<Colour3DPlotLayer, DenseThreeDimensionalModel>(layer, model))
	return;

    if (trySetModel<SpectrogramLayer, DenseTimeValueModel>(layer, model))
	return;
}

Model *
LayerFactory::createEmptyModel(LayerType layerType, Model *baseModel)
{
    if (layerType == TimeInstants) {
	return new SparseOneDimensionalModel(baseModel->getSampleRate(), 1);
    } else if (layerType == TimeValues) {
	return new SparseTimeValueModel(baseModel->getSampleRate(), 1,
					0.0, 0.0, true);
    } else {
	return 0;
    }
}

Layer *
LayerFactory::createLayer(LayerType type, View *view,
			  Model *model, int channel)
{
    Layer *layer = 0;

    switch (type) {

    case Waveform:
	layer = new WaveformLayer(view);
	static_cast<WaveformLayer *>(layer)->setChannel(channel);
	break;

    case Spectrogram:
	layer = new SpectrogramLayer(view);
	static_cast<SpectrogramLayer *>(layer)->setChannel(channel);
	break;

    case TimeRuler:
	layer = new TimeRulerLayer(view);
	break;

    case TimeInstants:
	layer = new TimeInstantLayer(view);
	break;

    case TimeValues:
	layer = new TimeValueLayer(view);
	break;

    case Colour3DPlot:
	layer = new Colour3DPlotLayer(view);
	break;

    case MelodicRangeSpectrogram: 
	layer = new SpectrogramLayer(view, SpectrogramLayer::MelodicRange);
	static_cast<SpectrogramLayer *>(layer)->setChannel(channel);
	break;

    default: break;
    }

    if (!layer) {
	std::cerr << "LayerFactory::createLayer: Unknown layer type " 
		  << type << std::endl;
    } else {
	if (model) setModel(layer, model);
	std::cerr << "LayerFactory::createLayer: Setting object name "
		  << getLayerPresentationName(type).toStdString() << " on " << layer << std::endl;
	layer->setObjectName(getLayerPresentationName(type));
    }

    return layer;
}


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

#include "PluginParameterBox.h"

#include "AudioDial.h"

#include "plugin/PluginXml.h"

#include "base/RangeMapper.h"

#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QLayout>
#include <QLabel>

#include <iostream>
#include <string>

#include <cmath>

PluginParameterBox::PluginParameterBox(Vamp::PluginBase *plugin, QWidget *parent) :
    QFrame(parent),
    m_plugin(plugin)
{
    m_layout = new QGridLayout;
    setLayout(m_layout);
    populate();
}

PluginParameterBox::~PluginParameterBox()
{
}

void
PluginParameterBox::populate()
{
    Vamp::PluginBase::ParameterList params = m_plugin->getParameterDescriptors();
    Vamp::PluginBase::ProgramList programs = m_plugin->getPrograms();

    m_params.clear();

    if (params.empty() && programs.empty()) {
        m_layout->addWidget
            (new QLabel(tr("This plugin has no adjustable parameters.")),
             0, 0);
    }

    int offset = 0;

    if (!programs.empty()) {

        std::string currentProgram = m_plugin->getCurrentProgram();

        QComboBox *programCombo = new QComboBox;
        programCombo->setMaxVisibleItems(20);

        for (int i = 0; i < programs.size(); ++i) {
            programCombo->addItem(programs[i].c_str());
            if (programs[i] == currentProgram) {
                programCombo->setCurrentIndex(i);
            }
        }

        m_layout->addWidget(new QLabel(tr("Program")), 0, 0);
        m_layout->addWidget(programCombo, 0, 1, 1, 2);

        connect(programCombo, SIGNAL(currentIndexChanged(const QString &)),
                this, SLOT(programComboChanged(const QString &)));

        offset = 1;
    }

    for (size_t i = 0; i < params.size(); ++i) {

        QString name = params[i].name.c_str();
        QString description = params[i].description.c_str();
        QString unit = params[i].unit.c_str();

        float min = params[i].minValue;
        float max = params[i].maxValue;
        float deft = params[i].defaultValue;
        float value = m_plugin->getParameter(params[i].name);

        float qtz = 0.0;
        if (params[i].isQuantized) qtz = params[i].quantizeStep;

        std::vector<std::string> valueNames = params[i].valueNames;

        // construct an integer range

        int imin = 0, imax = 100;

        if (qtz > 0.0) {
            imax = int((max - min) / qtz);
        } else {
            qtz = (max - min) / 100.0;
        }

        //!!! would be nice to ensure the default value corresponds to
        // an integer!

        QLabel *label = new QLabel(description);
        m_layout->addWidget(label, i + offset, 0);

        ParamRec rec;
        rec.param = params[i];
        rec.dial = 0;
        rec.spin = 0;
        rec.check = 0;
        rec.combo = 0;
        
        if (params[i].isQuantized && !valueNames.empty()) {
            
            QComboBox *combobox = new QComboBox;
            combobox->setObjectName(name);
            for (unsigned int j = 0; j < valueNames.size(); ++j) {
                combobox->addItem(valueNames[j].c_str());
                if (lrintf((value - min) / qtz) == j) {
                    combobox->setCurrentIndex(j);
                }
            }
            connect(combobox, SIGNAL(activated(int)),
                    this, SLOT(dialChanged(int)));
            m_layout->addWidget(combobox, i + offset, 1, 1, 2);
            rec.combo = combobox;

        } else if (min == 0.0 && max == 1.0 && qtz == 1.0) {
            
            QCheckBox *checkbox = new QCheckBox;
            checkbox->setObjectName(name);
            checkbox->setCheckState(value == 0.0 ? Qt::Unchecked : Qt::Checked);
            connect(checkbox, SIGNAL(stateChanged(int)),
                    this, SLOT(checkBoxChanged(int)));
            m_layout->addWidget(checkbox, i + offset, 2);
            rec.check = checkbox;

        } else {
            
            AudioDial *dial = new AudioDial;
            dial->setObjectName(description);
            dial->setMinimum(imin);
            dial->setMaximum(imax);
            dial->setPageStep(1);
            dial->setNotchesVisible((imax - imin) <= 12);
            dial->setDefaultValue(lrintf((deft - min) / qtz));
            dial->setValue(lrintf((value - min) / qtz));
            dial->setFixedWidth(32);
            dial->setFixedHeight(32);
            dial->setRangeMapper(new LinearRangeMapper
                                 (imin, imax, min, max, unit));
            dial->setShowToolTip(true);
            connect(dial, SIGNAL(valueChanged(int)),
                    this, SLOT(dialChanged(int)));
            m_layout->addWidget(dial, i + offset, 1);

            QDoubleSpinBox *spinbox = new QDoubleSpinBox;
            spinbox->setObjectName(name);
            spinbox->setMinimum(min);
            spinbox->setMaximum(max);
            spinbox->setSuffix(QString(" %1").arg(unit));
            spinbox->setSingleStep(qtz);
            spinbox->setValue(value);
            spinbox->setDecimals(4);
            connect(spinbox, SIGNAL(valueChanged(double)),
                    this, SLOT(spinBoxChanged(double)));
            m_layout->addWidget(spinbox, i + offset, 2);
            rec.dial = dial;
            rec.spin = spinbox;
        }

        m_params[name] = rec;
        m_descriptionMap[description] = name;
    }
}

void
PluginParameterBox::dialChanged(int ival)
{
    QObject *obj = sender();
    QString name = obj->objectName();

    if (m_params.find(name) == m_params.end() &&
        m_descriptionMap.find(name) != m_descriptionMap.end()) {
        name = m_descriptionMap[name];
    }

    if (m_params.find(name) == m_params.end()) {
        std::cerr << "WARNING: PluginParameterBox::dialChanged: Unknown parameter \"" << name.toStdString() << "\"" << std::endl;
        return;
    }

    Vamp::PluginBase::ParameterDescriptor params = m_params[name].param;

    float min = params.minValue;
    float max = params.maxValue;

    float newValue;

    float qtz = 0.0;
    if (params.isQuantized) qtz = params.quantizeStep;

    AudioDial *ad = dynamic_cast<AudioDial *>(obj);
    
    if (ad && ad->rangeMapper()) {
        
        newValue = ad->mappedValue();
        if (newValue < min) newValue = min;
        if (newValue > max) newValue = max;
        if (qtz != 0.0) {
            ival = lrintf((newValue - min) / qtz);
            newValue = min + ival * qtz;
        }

    } else {
        if (qtz == 0.0) {
            qtz = (max - min) / 100.0;
        }
        newValue = min + ival * qtz;
    }

    QDoubleSpinBox *spin = m_params[name].spin;
    if (spin) {
        spin->blockSignals(true);
        spin->setValue(newValue);
        spin->blockSignals(false);
    }

    m_plugin->setParameter(name.toStdString(), newValue);

    emit pluginConfigurationChanged(PluginXml(m_plugin).toXmlString());
}

void
PluginParameterBox::checkBoxChanged(int state)
{
    QObject *obj = sender();
    QString name = obj->objectName();

    if (m_params.find(name) == m_params.end() &&
        m_descriptionMap.find(name) != m_descriptionMap.end()) {
        name = m_descriptionMap[name];
    }

    if (m_params.find(name) == m_params.end()) {
        std::cerr << "WARNING: PluginParameterBox::checkBoxChanged: Unknown parameter \"" << name.toStdString() << "\"" << std::endl;
        return;
    }

    Vamp::PluginBase::ParameterDescriptor params = m_params[name].param;

    if (state) m_plugin->setParameter(name.toStdString(), 1.0);
    else m_plugin->setParameter(name.toStdString(), 0.0);

    emit pluginConfigurationChanged(PluginXml(m_plugin).toXmlString());
}

void
PluginParameterBox::spinBoxChanged(double value)
{
    QObject *obj = sender();
    QString name = obj->objectName();

    if (m_params.find(name) == m_params.end() &&
        m_descriptionMap.find(name) != m_descriptionMap.end()) {
        name = m_descriptionMap[name];
    }

    if (m_params.find(name) == m_params.end()) {
        std::cerr << "WARNING: PluginParameterBox::spinBoxChanged: Unknown parameter \"" << name.toStdString() << "\"" << std::endl;
        return;
    }

    Vamp::PluginBase::ParameterDescriptor params = m_params[name].param;

    float min = params.minValue;
    float max = params.maxValue;

    float qtz = 0.0;
    if (params.isQuantized) qtz = params.quantizeStep;
    
    if (qtz > 0.0) {
        int step = int((value - min) / qtz);
        value = min + step * qtz;
    }

    int imin = 0, imax = 100;
    
    if (qtz > 0.0) {
        imax = int((max - min) / qtz);
    } else {
        qtz = (max - min) / 100.0;
    }

    int ival = (value - min) / qtz;

    AudioDial *dial = m_params[name].dial;
    if (dial) {
        dial->blockSignals(true);
        dial->setValue(ival);
        dial->blockSignals(false);
    }

    m_plugin->setParameter(name.toStdString(), value);

    emit pluginConfigurationChanged(PluginXml(m_plugin).toXmlString());
}

void
PluginParameterBox::programComboChanged(const QString &newProgram)
{
    m_plugin->selectProgram(newProgram.toStdString());

    for (std::map<QString, ParamRec>::iterator i = m_params.begin();
         i != m_params.end(); ++i) {

        Vamp::PluginBase::ParameterDescriptor &param = i->second.param;
        float value = m_plugin->getParameter(param.name);

        if (i->second.spin) {
            i->second.spin->blockSignals(true);
            i->second.spin->setValue(value);
            i->second.spin->blockSignals(false);
        }

        if (i->second.dial) {

            float min = param.minValue;
            float max = param.maxValue;

            float qtz = 0.0;
            if (param.isQuantized) qtz = param.quantizeStep;

            if (qtz == 0.0) {
                qtz = (max - min) / 100.0;
            }

            i->second.dial->blockSignals(true);
            i->second.dial->setValue(lrintf((value - min) / qtz));
            i->second.dial->blockSignals(false);
        }

        if (i->second.combo) {
            i->second.combo->blockSignals(true);
            i->second.combo->setCurrentIndex(value);
            i->second.combo->blockSignals(false);
        }
    }

    emit pluginConfigurationChanged(PluginXml(m_plugin).toXmlString());
}


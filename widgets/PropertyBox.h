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

#ifndef _PROPERTY_BOX_H_
#define _PROPERTY_BOX_H_

#include "base/PropertyContainer.h"

#include <QFrame>
#include <map>

class QLayout;
class QWidget;
class QGridLayout;
class QVBoxLayout;
class QLabel;
class LEDButton;

class PropertyBox : public QFrame
{
    Q_OBJECT

public:
    PropertyBox(PropertyContainer *);
    ~PropertyBox();

    PropertyContainer *getContainer() { return m_container; }

signals:
    void changePlayGain(float);
    void changePlayGainDial(int);
    void changePlayPan(float);
    void changePlayPanDial(int);
    void showLayer(bool);

public slots:
    void propertyContainerPropertyChanged(PropertyContainer *);
    void pluginConfigurationChanged(QString);
    void layerVisibilityChanged(bool);

protected slots:
    void propertyControllerChanged(int);

    void playGainChanged(float);
    void playGainDialChanged(int);
    void playPanChanged(float);
    void playPanDialChanged(int);

    void populateViewPlayFrame();

    void unitDatabaseChanged();

    void editPlugin();

protected:
    void updatePropertyEditor(PropertyContainer::PropertyName);

    QLabel *m_nameWidget;
    QWidget *m_mainWidget;
    QGridLayout *m_layout;
    PropertyContainer *m_container;
    QFrame *m_viewPlayFrame;
    QVBoxLayout *m_mainBox;
    LEDButton *m_showButton;
    std::map<QString, QLayout *> m_groupLayouts;
    std::map<QString, QWidget *> m_propertyControllers;
};

#endif

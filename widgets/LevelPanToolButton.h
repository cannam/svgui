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

#ifndef LEVEL_PAN_TOOLBUTTON_H
#define LEVEL_PAN_TOOLBUTTON_H

#include <QToolButton>

class LevelPanWidget;

class LevelPanToolButton : public QToolButton
{
    Q_OBJECT

public:
    LevelPanToolButton(QWidget *parent = 0);
    ~LevelPanToolButton();
    
    /// Return level as a gain value in the range [0,1]
    float getLevel() const; 
    
    /// Return pan as a value in the range [-1,1]
    float getPan() const;

    void setImageSize(int pixels);
			
public slots:
    /// Set level in the range [0,1] -- will be rounded
    void setLevel(float);

    /// Set pan in the range [-1,1] -- will be rounded
    void setPan(float);

    /// Redraw icon for toolbar button based on level-pan widget contents
    void redraw();

signals:
    void levelChanged(float);
    void panChanged(float);

private slots:
    void selfLevelChanged(float);
    void selfClicked();
    
protected:
    LevelPanWidget *m_lpw;
    int m_pixels;
    bool m_muted;
    float m_savedLevel;
};

#endif

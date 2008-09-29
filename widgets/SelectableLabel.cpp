/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2008 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "SelectableLabel.h"

#include <iostream>
#include <QApplication>

SelectableLabel::SelectableLabel(QWidget *p) :
    QLabel(p),
    m_selected(false)
{
    setTextFormat(Qt::RichText);
//    setLineWidth(2);
//    setFixedWidth(480);
    setupStyle();
}

SelectableLabel::~SelectableLabel()
{
}

void
SelectableLabel::setUnselectedText(QString text)
{
    m_unselectedText = text;
    if (!m_selected) {
        setText(m_unselectedText);
        resize(sizeHint());
    }
}

void
SelectableLabel::setSelectedText(QString text)
{
    m_selectedText = text;
    if (m_selected) {
        setText(m_selectedText);
        resize(sizeHint());
    }
}

void
SelectableLabel::setupStyle()
{
    QPalette palette = QApplication::palette();

    if (m_selected) {
        setWordWrap(true);
        setStyleSheet
            (QString("QLabel:hover { background: %1; color: %3; } "
                     "QLabel:!hover { background: %2; color: %3 } "
                     "QLabel { padding: 7px }")
             .arg(palette.button().color().name())
             .arg(palette.mid().color().light().name())
             .arg(palette.text().color().name()));
    } else {
        setWordWrap(false);
        setStyleSheet
            (QString("QLabel:hover { background: %1; color: %3; } "
                     "QLabel:!hover { background: %2; color: %3 } "
                     "QLabel { padding: 7px }")
             .arg(palette.button().color().name())
             .arg(palette.light().color().name())
             .arg(palette.text().color().name()));

//        setStyleSheet("QLabel:hover { background: #e0e0e0; color: black; } QLabel:!hover { background: white; color: black } QLabel { padding: 7px }");
    }
}    

void
SelectableLabel::setSelected(bool s)
{
    if (m_selected == s) return;
    m_selected = s;
    if (m_selected) {
        setText(m_selectedText);
    } else {
        setText(m_unselectedText);
    }
    setupStyle();
    parentWidget()->resize(parentWidget()->sizeHint());
}

void
SelectableLabel::toggle()
{
    setSelected(!m_selected);
}

void
SelectableLabel::mousePressEvent(QMouseEvent *e)
{
    setSelected(true);
    emit selectionChanged();
}

void
SelectableLabel::mouseDoubleClickEvent(QMouseEvent *e)
{
    std::cerr << "mouseDoubleClickEvent" << std::endl;
    emit doubleClicked();
}

void
SelectableLabel::enterEvent(QEvent *)
{
//    std::cerr << "enterEvent" << std::endl;
//    QPalette palette = QApplication::palette();
//    palette.setColor(QPalette::Window, Qt::gray);
//    setStyleSheet("background: gray");
//    setPalette(palette);
}

void
SelectableLabel::leaveEvent(QEvent *)
{
//    std::cerr << "leaveEvent" << std::endl;
//    setStyleSheet("background: white");
//    QPalette palette = QApplication::palette();
//    palette.setColor(QPalette::Window, Qt::gray);
//    setPalette(palette);
}

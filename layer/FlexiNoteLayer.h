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

#ifndef _FLEXINOTE_LAYER_H_
#define _FLEXINOTE_LAYER_H_

#define NOTE_HEIGHT 16

#include "SingleColourLayer.h"
#include "data/model/FlexiNoteModel.h"

#include <QObject>
#include <QColor>

class View;
class QPainter;

class FlexiNoteLayer : public SingleColourLayer
{
    Q_OBJECT

public:
    FlexiNoteLayer();

    virtual void paint(View *v, QPainter &paint, QRect rect) const;

    virtual QString getFeatureDescription(View *v, QPoint &) const;

    virtual bool snapToFeatureFrame(View *v, int &frame,
                    size_t &resolution,
                    SnapType snap) const;

    virtual void drawStart(View *v, QMouseEvent *);
    virtual void drawDrag(View *v, QMouseEvent *);
    virtual void drawEnd(View *v, QMouseEvent *);

    virtual void eraseStart(View *v, QMouseEvent *);
    virtual void eraseDrag(View *v, QMouseEvent *);
    virtual void eraseEnd(View *v, QMouseEvent *);

    virtual void editStart(View *v, QMouseEvent *);
    virtual void editDrag(View *v, QMouseEvent *);
    virtual void editEnd(View *v, QMouseEvent *);

    virtual void splitStart(View *v, QMouseEvent *);
    virtual void splitEnd(View *v, QMouseEvent *);
    
    virtual void addNote(View *v, QMouseEvent *e);

    virtual void mouseMoveEvent(View *v, QMouseEvent *);

    virtual bool editOpen(View *v, QMouseEvent *);

    virtual void moveSelection(Selection s, size_t newStartFrame);
    virtual void resizeSelection(Selection s, Selection newSize);
    virtual void deleteSelection(Selection s);

    virtual void copy(View *v, Selection s, Clipboard &to);
    virtual bool paste(View *v, const Clipboard &from, int frameOffset,
                       bool interactive);

    virtual const Model *getModel() const { return m_model; }
    void setModel(FlexiNoteModel *model);

    virtual PropertyList getProperties() const;
    virtual QString getPropertyLabel(const PropertyName &) const;
    virtual PropertyType getPropertyType(const PropertyName &) const;
    virtual QString getPropertyGroupName(const PropertyName &) const;
    virtual int getPropertyRangeAndValue(const PropertyName &,
                                         int *min, int *max, int *deflt) const;
    virtual QString getPropertyValueLabel(const PropertyName &,
                      int value) const;
    virtual void setProperty(const PropertyName &, int value);

    enum VerticalScale {
        AutoAlignScale,
        LinearScale,
        LogScale,
        MIDIRangeScale
    };
    
    //GF: Tonioni: context sensitive note edit actions (denoted clockwise from top).
    enum EditMode {
        DragNote,
        RightBoundary,
        SplitNote,
        LeftBoundary
    };
    
    void setIntelligentActions(bool on) { m_intelligentActions=on; }

    void setVerticalScale(VerticalScale scale);
    VerticalScale getVerticalScale() const { return m_verticalScale; }

    virtual bool isLayerScrollable(const View *v) const;

    virtual bool isLayerEditable() const { return true; }

    virtual int getCompletion(View *) const { return m_model->getCompletion(); }

    virtual bool getValueExtents(float &min, float &max,
                                 bool &log, QString &unit) const;

    virtual bool getDisplayExtents(float &min, float &max) const;
    virtual bool setDisplayExtents(float min, float max);

    virtual int getVerticalZoomSteps(int &defaultStep) const;
    virtual int getCurrentVerticalZoomStep() const;
    virtual void setVerticalZoomStep(int);
    virtual RangeMapper *getNewVerticalZoomRangeMapper() const;

    virtual int getVerticalScaleWidth(View *, bool, QPainter &) const { return 0; }

    /**
     * Add a note-on.  Used when recording MIDI "live".  The note will
     * not be finally added to the layer until the corresponding
     * note-off.
     */
    void addNoteOn(long frame, int pitch, int velocity);
    
    /**
     * Add a note-off.  This will cause a note to appear, if and only
     * if there is a matching pending note-on.
     */
    void addNoteOff(long frame, int pitch);

    /**
     * Abandon all pending note-on events.
     */
    void abandonNoteOns();

    virtual void toXml(QTextStream &stream, QString indent = "",
                       QString extraAttributes = "") const;

    void setProperties(const QXmlAttributes &attributes);
    
    void setVerticalRangeToNoteRange(View *v);

protected:
    void getScaleExtents(View *, float &min, float &max, bool &log) const;
    int getYForValue(View *v, float value) const;
    float getValueForY(View *v, int y) const;
    bool shouldConvertMIDIToHz() const;

    virtual int getDefaultColourHint(bool dark, bool &impose);

    FlexiNoteModel::PointList getLocalPoints(View *v, int) const;

    bool getPointToDrag(View *v, int x, int y, FlexiNoteModel::Point &) const;
    bool getNoteToEdit(View *v, int x, int y, FlexiNoteModel::Point &) const;
    void getRelativeMousePosition(View *v, FlexiNoteModel::Point &note, int x, int y, bool &closeToLeft, bool &closeToRight, bool &closeToTop, bool &closeToBottom) const;
    void updateNoteValue(View *v, FlexiNoteModel::Point &note) const;

    FlexiNoteModel *m_model;
    bool m_editing;
    bool m_intelligentActions;
    int m_dragPointX;
    int m_dragPointY;
    int m_dragStartX;
    int m_dragStartY;
    FlexiNoteModel::Point m_originalPoint;
    FlexiNoteModel::Point m_editingPoint;
    long m_greatestLeftNeighbourFrame;
    long m_smallestRightNeighbourFrame;
    FlexiNoteModel::EditCommand *m_editingCommand;
    VerticalScale m_verticalScale;
    EditMode m_editMode;

    typedef std::set<FlexiNoteModel::Point, FlexiNoteModel::Point::Comparator> FlexiNoteSet;
    FlexiNoteSet m_pendingNoteOns;

    mutable float m_scaleMinimum;
    mutable float m_scaleMaximum;

    bool shouldAutoAlign() const;

    void finish(FlexiNoteModel::EditCommand *command) {
        Command *c = command->finish();
        if (c) CommandHistory::getInstance()->addCommand(c, false);
    }
};

#endif


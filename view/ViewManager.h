/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam and QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _VIEW_MANAGER_H_
#define _VIEW_MANAGER_H_

#include <QObject>
#include <QTimer>

#include <map>

#include "base/Selection.h"
#include "base/Command.h"
#include "base/Clipboard.h"

class AudioPlaySource;
class Model;

enum PlaybackFollowMode {
    PlaybackScrollContinuous,
    PlaybackScrollPage,
    PlaybackIgnore
};

class View;

/**
 * The ViewManager manages properties that may need to be synchronised
 * between separate Views.  For example, it handles signals associated
 * with changes to the global pan and zoom, and it handles selections.
 *
 * Views should be implemented in such a way as to work
 * correctly whether they are supplied with a ViewManager or not.
 */

class ViewManager : public QObject
{
    Q_OBJECT

public:
    ViewManager();
    virtual ~ViewManager();

    void setAudioPlaySource(AudioPlaySource *source);

    bool isPlaying() const;

    unsigned long getGlobalCentreFrame() const;
    void setGlobalCentreFrame(unsigned long);
    unsigned long getGlobalZoom() const;

    unsigned long getPlaybackFrame() const;
    void setPlaybackFrame(unsigned long frame);

    bool haveInProgressSelection() const;
    const Selection &getInProgressSelection(bool &exclusive) const;
    void setInProgressSelection(const Selection &selection, bool exclusive);
    void clearInProgressSelection();

    const MultiSelection &getSelection() const;

    const MultiSelection::SelectionList &getSelections() const;
    void setSelection(const Selection &selection);
    void addSelection(const Selection &selection);
    void removeSelection(const Selection &selection);
    void clearSelections();

    /**
     * Return the selection that contains a given frame.
     * If defaultToFollowing is true, and if the frame is not in a
     * selected area, return the next selection after the given frame.
     * Return the empty selection if no appropriate selection is found.
     */
    Selection getContainingSelection(size_t frame, bool defaultToFollowing) const;

    Clipboard &getClipboard() { return m_clipboard; }

    enum ToolMode {
	NavigateMode,
	SelectMode,
        EditMode,
	DrawMode
    };
    ToolMode getToolMode() const { return m_toolMode; }
    void setToolMode(ToolMode mode);

    bool getPlayLoopMode() const { return m_playLoopMode; }
    void setPlayLoopMode(bool on);

    bool getPlaySelectionMode() const { return m_playSelectionMode; }
    void setPlaySelectionMode(bool on);

    size_t getPlaybackSampleRate() const;
    size_t getMainModelSampleRate() const { return m_mainModelSampleRate; }
    void setMainModelSampleRate(size_t sr) { m_mainModelSampleRate = sr; }

    enum OverlayMode {
        NoOverlays,
        MinimalOverlays,
        StandardOverlays,
        AllOverlays
    };
    void setOverlayMode(OverlayMode mode);
    OverlayMode getOverlayMode() const { return m_overlayMode; }

    bool shouldShowCentreLine() const {
        return m_overlayMode != NoOverlays;
    }
    bool shouldShowFrameCount() const {
        return m_overlayMode != NoOverlays;
    }
    bool shouldShowDuration() const {
        return m_overlayMode > MinimalOverlays;
    }
    bool shouldShowVerticalScale() const {
        return m_overlayMode > MinimalOverlays;
    }
    bool shouldShowSelectionExtents() const {
        return m_overlayMode > MinimalOverlays;
    }
    bool shouldShowLayerNames() const {
        return m_overlayMode == AllOverlays;
    }
    bool shouldShowScaleGuides() const {
        return m_overlayMode != NoOverlays;
    }

    void setZoomWheelsEnabled(bool enable);
    bool getZoomWheelsEnabled() const { return m_zoomWheelsEnabled; }

signals:
    /** Emitted when user causes the global centre frame to change. */
    void globalCentreFrameChanged(unsigned long frame);

    /** Emitted when user scrolls a view, but doesn't affect global centre. */
    void viewCentreFrameChanged(View *v, unsigned long frame);

    /** Emitted when a view zooms.  The originator identifies the view. */
    void zoomLevelChanged(void *originator, unsigned long zoom, bool locked);

    /** Emitted when a view zooms. */
    void zoomLevelChanged();

    /** Emitted when the playback frame changes. */
    void playbackFrameChanged(unsigned long frame);

    /** Emitted when the output levels change. Values in range 0.0 -> 1.0. */
    void outputLevelsChanged(float left, float right);

    /** Emitted when the selection has changed. */
    void selectionChanged();

    /** Emitted when the in-progress (rubberbanding) selection has changed. */
    void inProgressSelectionChanged();

    /** Emitted when the tool mode has been changed. */
    void toolModeChanged();

    /** Emitted when the play loop mode has been changed. */
    void playLoopModeChanged();
    void playLoopModeChanged(bool);

    /** Emitted when the play selection mode has been changed. */
    void playSelectionModeChanged();
    void playSelectionModeChanged(bool);

    /** Emitted when the overlay mode has been changed. */
    void overlayModeChanged();

    /** Emitted when the zoom wheels have been toggled. */
    void zoomWheelsEnabledChanged();

public slots:
    void viewCentreFrameChanged(unsigned long, bool, PlaybackFollowMode);

protected slots:
    void checkPlayStatus();
    void playStatusChanged(bool playing);
    void seek(unsigned long);
    void considerZoomChange(void *, unsigned long, bool);

protected:
    AudioPlaySource *m_playSource;
    unsigned long m_globalCentreFrame;
    unsigned long m_globalZoom;
    mutable unsigned long m_playbackFrame;
    size_t m_mainModelSampleRate;

    float m_lastLeft;
    float m_lastRight;

    MultiSelection m_selections;
    Selection m_inProgressSelection;
    bool m_inProgressExclusive;

    Clipboard m_clipboard;

    ToolMode m_toolMode;

    bool m_playLoopMode;
    bool m_playSelectionMode;

    void setSelections(const MultiSelection &ms);
    void signalSelectionChange();

    class SetSelectionCommand : public Command
    {
    public:
	SetSelectionCommand(ViewManager *vm, const MultiSelection &ms);
	virtual ~SetSelectionCommand();
	virtual void execute();
	virtual void unexecute();
	virtual QString getName() const;

    protected:
	ViewManager *m_vm;
	MultiSelection m_oldSelection;
	MultiSelection m_newSelection;
    };

    OverlayMode m_overlayMode;
    bool m_zoomWheelsEnabled;
};

#endif


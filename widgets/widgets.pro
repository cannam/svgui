TEMPLATE = lib

SV_UNIT_PACKAGES = vamp-sdk
load(../sv.prf)

CONFIG += sv staticlib qt thread warn_on stl rtti exceptions
QT += xml

TARGET = svwidgets

DEPENDPATH += . ..
INCLUDEPATH += . ..
OBJECTS_DIR = tmp_obj
MOC_DIR = tmp_moc

# Input
HEADERS += AudioDial.h \
           Fader.h \
           ItemEditDialog.h \
           LayerTree.h \
           LEDButton.h \
           ListInputDialog.h \
           NotifyingCheckBox.h \
           NotifyingComboBox.h \
           NotifyingPushButton.h \
           Panner.h \
           PluginParameterBox.h \
           PluginParameterDialog.h \
           PropertyBox.h \
           PropertyStack.h \
           RangeInputDialog.h \
           SubdividingMenu.h \
           Thumbwheel.h \
           WindowShapePreview.h \
           WindowTypeSelector.h
SOURCES += AudioDial.cpp \
           Fader.cpp \
           ItemEditDialog.cpp \
           LayerTree.cpp \
           LEDButton.cpp \
           ListInputDialog.cpp \
           NotifyingCheckBox.cpp \
           NotifyingComboBox.cpp \
           NotifyingPushButton.cpp \
           Panner.cpp \
           PluginParameterBox.cpp \
           PluginParameterDialog.cpp \
           PropertyBox.cpp \
           PropertyStack.cpp \
           RangeInputDialog.cpp \
           SubdividingMenu.cpp \
           Thumbwheel.cpp \
           WindowShapePreview.cpp \
           WindowTypeSelector.cpp

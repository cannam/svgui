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

#ifndef _MODEL_DATA_TABLE_DIALOG_H_
#define _MODEL_DATA_TABLE_DIALOG_H_

#include <QDialog>

class Model;
class ModelDataTableModel;
class QTableView;

class ModelDataTableDialog : public QDialog
{
    Q_OBJECT
    
public:
    ModelDataTableDialog(Model *model, QWidget *parent = 0);
    ~ModelDataTableDialog();

protected:
    ModelDataTableModel *m_table;
    QTableView *m_tableView;
};

#endif

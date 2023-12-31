/*! \file masterpanelcell.cpp

    This file is part of SorpSim and is distributed under terms in the file LICENSE.

    Developed by Zhiyao Yang and Dr. Ming Qu for ORNL.

    \author Zhiyao Yang (zhiyaoYang)
    \author Dr. Ming Qu
    \author Nicholas Fette (nfette)

    \copyright 2015, UT-Battelle, LLC
    \copyright 2017-2018, Nicholas Fette

*/

#include <QDebug>
#include <QMessageBox>

#include "masterpanelcell.h"
#include "masterdialog.h"
#include "dataComm.h"
#include "mainwindow.h"

extern globalparameter globalpara;
extern bool initializing;

masterPanelCell::masterPanelCell(Node *node, QWidget *parent) :
    QLineEdit(parent)
{
    myNode = node;
    myType = -1;
    myValue = -1;
    previousValue = -1;

    spIndex = -1;
    paraName = "None";

    QObject::connect(this,SIGNAL(focussed(bool)),this,SLOT(recordPreviousValues(bool)));

    setFixedWidth(50);
}

void masterPanelCell::setType(int type)
{
        myType = type;
        setCell(type);
        if(!initializing)
            sendNewIndex(myNode,paraName,type);
}

void masterPanelCell::setCell(int type)
{
    switch (type) {
    case 0:
        setText(QString::number(myValue,'g',4));
        setStyleSheet("QLineEdit{background: rgb(204, 255, 204);}");
        setReadOnly(false);
        break;
    case 1:
        setText("");
        setStyleSheet("QLineEdit{background: white;}");
        setReadOnly(true);
        break;
    }
}

void masterPanelCell::setValue(double value)
{
    myValue = value;
    setText(QString::number(myValue,'g',4));
    setReadOnly(false);
}

void masterPanelCell::setParaName(QString name)
{
    paraName = name;
}

void masterPanelCell::recordPreviousValues(bool focused)
{
    isFocused = focused;
    if(focused)
        previousValue = text().toDouble();
}

void masterPanelCell::focusInEvent(QFocusEvent *e)
{
    QLineEdit::focusInEvent(e);
    if(myType==0)
        emit(focussed(true));
}

void masterPanelCell::focusOutEvent(QFocusEvent *e)
{
    QLineEdit::focusOutEvent(e);
    if(myType==0)
        emit(focussed(false));
}

void masterPanelCell::textChanged()
{
    if(myType==0&&text().toDouble()-previousValue>0.01)
        sendNewValue(myNode,paraName,text().toDouble(),previousValue);
}

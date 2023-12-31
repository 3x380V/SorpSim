/*! \file edittabledialog.cpp
    \brief dialog to edit the column variables of an existing table

    This file is part of SorpSim and is distributed under terms in the file LICENSE.

    Developed by Zhiyao Yang and Dr. Ming Qu for ORNL.

    \author Zhiyao Yang (zhiyaoYang)
    \author Dr. Ming Qu
    \author Nicholas Fette (nfette)

    \copyright 2015, UT-Battelle, LLC
    \copyright 2017-2018, Nicholas Fette

*/

#include <QDebug>
#include <QLabel>
#include <QLayout>
#include <QListView>
#include <QStringList>
#include <QStatusBar>

#include "edittabledialog.h"
#include "ui_edittabledialog.h"
#include "mainwindow.h"
#include "dataComm.h"
#include "myscene.h"
#include "unit.h"
#include "sorputils.h"

extern int sceneActionIndex;
extern bool istableinput;
extern QStringList inputEntries;
extern QStringList outputEntries;
extern globalparameter globalpara;
extern myScene * theScene;
extern QStatusBar*theStatusBar;
extern unit * dummy;
extern int globalcount;
extern QToolBar* theToolBar;
extern QMenuBar* theMenuBar;
QStringList inputQ;
QStringList outputQ;
QStringList inputR;
QStringList outputR;
extern MainWindow*theMainwindow;

editTableDialog::editTableDialog(QString theTableName, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::editTableDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog);
    setWindowModality(Qt::ApplicationModal);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Edit Table");

    inputModel = new QStringListModel;
    outputModel = new QStringListModel;
    tableName = theTableName;

    ui->inputList->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->outputList->setEditTriggers(QAbstractItemView::NoEditTriggers);

    QLayout *mainLayout = layout();
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);

    if(!loadTheTable())
    {
        globalpara.reportError("Fail to load current table information.",this);
        reject();
    }
    ui->tableNameLE->setText(theTableName);
}

editTableDialog::~editTableDialog()
{
    delete ui;
}

void editTableDialog::setInputModel(QStringList inputList)
{
    inputQ = inputList;
    inputR = inputList;
    QStringList temp;
    for(int i = 0; i<inputQ.count();i++)
    {
        temp = inputQ[i].split(",");
        inputQ[i] = temp[0]+" - "+temp[1];
        inputR[i] = temp[0]+"\n"+temp[1];
        QString string = temp[2];
        inputQ[i].append(","+addUnit(string));
        inputR[i].append("\n["+addUnit(string)+"]");
    }
    inputModel->setStringList(inputQ);
    ui->inputList->setModel(inputModel);
}

void editTableDialog::setOutputModel(QStringList outputList)
{
    outputQ = outputList;
    outputR = outputList;
    QStringList temp;
    for(int i = 0; i<outputQ.count();i++)
    {
        temp = outputQ[i].split(",");
        if(temp[1]=="SA"||temp[1]=="SO")
        {
            outputQ[i] = temp[0];
            outputR[i] = temp[0];
            QString string = temp[1];
            outputQ[i].append(","+addUnit(string));
            outputR[i].append("\n["+addUnit(string)+"]");
        }
        else
        {
            outputQ[i] = temp[0]+" - "+temp[1];
            outputR[i] = temp[0]+"\n"+temp[1];
            QString string = temp[2];
            outputQ[i].append(","+addUnit(string));
            outputR[i].append("\n["+addUnit(string)+"]");

        }
    }
    outputModel->setStringList(outputQ);
    ui->outputList->setModel(outputModel);
}

void editTableDialog::on_addInputButton_clicked()
{
    this->hide();
    sceneActionIndex=4;
    theToolBar->setEnabled(false);
    theMenuBar->setEnabled(false);
    istableinput = true;
    QApplication::setOverrideCursor(QCursor(Qt::CrossCursor));
    theStatusBar->showMessage("Double click on a state point or component to add its parameters as table input.\nOr press ESC to cancel.");
    // TODO: add a callback to handle ESC key press
}

void editTableDialog::on_addOutputButton_clicked()
{
    this->hide();
    sceneActionIndex = 4;
    theToolBar->setEnabled(false);
    theMenuBar->setEnabled(false);
    istableinput = false;
    QApplication::setOverrideCursor(QCursor(Qt::CrossCursor));
    theStatusBar->showMessage("Double click on a state point or component "
                              "to add its parameters as table output.\nOr press ESC to cancel.");
    // TODO: add a callback to handle ESC key press
}

void editTableDialog::on_cancel_mouse_select()
{
    this->show();
}

void editTableDialog::on_removeInputButton_clicked()
{
    int currentIndex = ui->inputList->currentIndex().row();
    inputModel->removeRows(currentIndex,1);
    inputEntries.removeAt(currentIndex);
}

void editTableDialog::on_addCOPButton_clicked()
{
    unit * iterator = dummy;
    bool positive = false, negative = false;
    for(int i = 0; i < globalcount;i++)
    {
        iterator = iterator->next;
        if(iterator->icop == 1)
            positive = true;
        if(iterator->icop == -1)
            negative = true;
    }

    if(!outputEntries.contains("System COP,SO"))
    {
        if(positive&&negative)
        {
            outputEntries.append("System COP,SO");
            setOutputModel(outputEntries);
        }
        else
            globalpara.reportError("There is not enough heat duties "
                                   "of components added as system input/output "
                                   "in the system!\n(Hint: there must be at least "
                                   "one denominator and one numerator \nfrom component "
                                   "setting for COP calculation.)",this);
    }
}

void editTableDialog::on_addCapaButton_clicked()
{
    unit * iterator = dummy;
    bool positive = false;
    for(int i = 0; i < globalcount;i++)
    {
        iterator = iterator->next;
        if(iterator->icop == 1)
            positive = true;
    }

    if(!outputEntries.contains("System Capacity,SA"))
    {
        if(positive)
        {
            outputEntries.append("System Capacity,SA");
            setOutputModel(outputEntries);
        }
        else
            globalpara.reportError("There is not enough heat "
                                   "duties of components added as "
                                   "system output in the system!\n(Hint: "
                                   "there must be at least one numerator \nfrom "
                                   "component setting for COP calculation.)",this);
    }
}

void editTableDialog::on_removeOutputButton_clicked()
{
    int currentIndex = ui->outputList->currentIndex().row();
    outputModel->removeRows(currentIndex,1);
    outputEntries.removeAt(currentIndex);
}

void editTableDialog::on_OKButton_clicked()
{
    if(inputModel->rowCount()<1||outputModel->rowCount()<1)
        globalpara.reportError("Please specify at least one input and one output.",this);
    else
    {
        tableName = ui->tableNameLE->text().replace(QRegularExpression("[^a-zA-Z0-9_]"), "");
        if(tableName.isEmpty())
        {
            for(int i = 1;tableNameUsed(tableName);i++)
                tableName = "table_"+QString::number(i);
        }
        if(tableName.at(0).isDigit())
            tableName = "table_"+tableName;

        setOutputModel(outputEntries);
        setInputModel(inputEntries);

        if(setupXml())
        {
            inputNumber = inputQ.count();
            inputEntries.clear();
            outputEntries.clear();
            inputQ.clear();
            outputQ.clear();
            // After accept(), this will close and send its finished(int) signal
            // to the tableDialog that spawned it.
            // It will also get automatically deleted.
            accept();
        }
        else
            globalpara.reportError("Fail to set up xml file for table.",this);
    }
}

void editTableDialog::on_cancelButton_clicked()
{
    inputEntries.clear();
    outputEntries.clear();
    inputQ.clear();
    outputQ.clear();
    reject();
}

bool editTableDialog::setupXml()
{
    QFile file(Sorputils::sorpTempDir().absoluteFilePath("tableTemp.xml"));
    QTextStream stream;
    stream.setDevice(&file);

    if(!file.open(QIODevice::ReadWrite|QIODevice::Text))
    {
        globalpara.reportError("Fail to open case file for table.",this);
        return false;
    }

    QDomDocument doc;
    if(!doc.setContent(&file))
    {
        globalpara.reportError("Fail to load xml document for table.",this);
        file.close();
        return false;
    }

    QDomElement tableData = doc.elementsByTagName("TableData").at(0).toElement();
    auto tablesByTitle = Sorputils::mapElementsByAttribute(tableData.childNodes(), "title");
    // check if the table name is already used, if not, create the new element
    if(!(tableName == oldTableName || !tablesByTitle.contains(tableName)))
    {
        globalpara.reportError("This table name is already used.",this);
        file.close();
        return false;
    }

    QDomElement newTable = tablesByTitle[oldTableName];
    newTable.setAttribute("title", tableName);

    QDomNode tableHeader = newTable.elementsByTagName("header").at(0);
    newTable.removeChild(tableHeader);
    tableHeader = doc.createElement("header");
    QDomText header = doc.createTextNode(inputR.join(";")+";"+outputR.join(";"));
    tableHeader.appendChild(header);
    newTable.appendChild(tableHeader);

    QDomNode tableInput = newTable.elementsByTagName("inputEntries").at(0);
    newTable.removeChild(tableInput);
    tableInput = doc.createElement("inputEntries");
    QDomText inputs = doc.createTextNode(inputEntries.join(";"));
    tableInput.appendChild(inputs);
    newTable.appendChild(tableInput);

    QDomNode tableOutput = newTable.elementsByTagName("outputEntries").at(0);
    newTable.removeChild(tableOutput);
    tableOutput = doc.createElement("outputEntries");
    QDomText outputs = doc.createTextNode(outputEntries.join(";"));
    tableOutput.appendChild(outputs);
    newTable.appendChild(tableOutput);

    QDomElement newRun;
    for(int i = 0; i < runs;i++)
    {
        QDomNodeList theRuns = newTable.elementsByTagName("Run");
        newRun = theRuns.at(i).toElement();

        for(int j = 0; j < oldInput.count();j++)
        {
            if(!inputEntries.contains(oldInput.at(j)))
            {
                QDomNode const inputToRemove = newRun.elementsByTagName("Input").at(j);
                newRun.removeChild(inputToRemove);
            }
        }

        for(int j = 0; j < oldOutput.count();j++)
        {
            if(!outputEntries.contains(oldOutput.at(j)))
            {
                QDomNode const outputToRemove = newRun.elementsByTagName("Output").at(j);
                newRun.removeChild(outputToRemove);
            }
        }
    }

    for(int i = 0; i< runs; i++)
    {
        QDomNodeList theRuns = newTable.elementsByTagName("Run");
        newRun = theRuns.at(i).toElement();

        for(int j = 0; j < inputEntries.count();j++)
        {
            if(!oldInput.contains(inputEntries.at(j)))
            {
                QDomElement newInput = doc.createElement("Input");
                newInput.setAttribute("type",translateInput(j,1));

                QDomElement newIndex = doc.createElement("index");
                QDomText theIndex = doc.createTextNode(translateInput(j,2));
                newIndex.appendChild(theIndex);
                newInput.appendChild(newIndex);

                QDomElement newPara = doc.createElement("parameter");
                QDomText thePara = doc.createTextNode(translateInput(j,3));
                newPara.appendChild(thePara);
                newInput.appendChild(newPara);

                QDomElement newValue = doc.createElement("value");
                QDomText theValue= doc.createTextNode("0");
                newValue.appendChild(theValue);
                newInput.appendChild(newValue);

                newRun.appendChild(newInput);
            }
        }

        for(int j = 0; j < outputEntries.count();j++)
        {
            if(!oldOutput.contains(outputEntries.at(j)))
            {
                QDomElement newOutput = doc.createElement("Output");
                newOutput.setAttribute("type",translateOutput(j,1));

                QDomElement newIndex = doc.createElement("index");
                QDomText theIndex = doc.createTextNode(translateOutput(j,2));
                newIndex.appendChild(theIndex);
                newOutput.appendChild(newIndex);

                QDomElement newPara = doc.createElement("parameter");
                QDomText thePara = doc.createTextNode(translateOutput(j,3));
                newPara.appendChild(thePara);
                newOutput.appendChild(newPara);

                QDomElement newValue = doc.createElement("value");
                QDomText theValue = doc.createTextNode("0");
                newValue.appendChild(theValue);
                newOutput.appendChild(newValue);

                newRun.appendChild(newOutput);
            }
        }
    }
    file.resize(0);
    doc.save(stream,4);
    file.close();
    return true;
}

bool editTableDialog::loadTheTable()
{
    oldTableName = tableName;
    QFile file(Sorputils::sorpTempDir().absoluteFilePath("tableTemp.xml"));
    QTextStream stream;
    stream.setDevice(&file);

    if(!file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        globalpara.reportError("Fail to open case file for table.",this);
        return false;
    }

    QDomDocument doc;
    if(!doc.setContent(&file))
    {
        globalpara.reportError("Fail to load xml document for table.",this);
        file.close();
        return false;
    }

    QDomElement tableData = doc.elementsByTagName("TableData").at(0).toElement();
    auto tablesByTitle = Sorputils::mapElementsByAttribute(tableData.childNodes(), "title");
    if (!tablesByTitle.contains(tableName))
    {
        globalpara.reportError("Fail to find the table data.",this);
        file.close();
        return false;
    }

    QDomElement theTable = tablesByTitle.value(tableName);
    QString iEntries = theTable.elementsByTagName("inputEntries").at(0).toElement().text();
    QString oEntries = theTable.elementsByTagName("outputEntries").at(0).toElement().text();
    QString runsCount = theTable.attribute("runs");
    ui->runLabel->setText("Run#:"+runsCount);
    runs = runsCount.toInt();

    inputEntries = iEntries.split(";");
    outputEntries = oEntries.split(";");
    oldInput = inputEntries;
    oldOutput = outputEntries;
    setInputModel(inputEntries);
    setOutputModel(outputEntries);
    file.close();
    return true;
}

QString editTableDialog::translateInput(int index, int item)
{
    /*item = 1 - type
     *item = 2 - index
     *item = 3 - parameter*/
    QString inputTemp = inputEntries.at(index);
    QStringList tempList = inputTemp.split(",");
    QString infoTemp = tempList.last();

    if(QString(infoTemp.at(0))=="U")
    {
        switch(item) {
        case 1:
            return "unit";
        case 2:
            if(infoTemp.at(2).isDigit())//possibly there are more than 9 units
                return QString(infoTemp.at(1))+QString(infoTemp.at(2));
            return infoTemp.at(1);
        case 3:
            if(infoTemp.at(2).isDigit())
                return QString(infoTemp.at(3))+QString(infoTemp.at(4));
            return QString(infoTemp.at(2))+QString(infoTemp.at(3));
        }
    }
    if(QString(infoTemp.at(0)) == "P")
    {
        switch(item) {
        case 1:
            return "sp";
        case 2:
            if(infoTemp.at(3).isDigit())//possibly there are more than 9 units
                return QString(infoTemp.at(1))+QString(infoTemp.at(2))+" "+QString(infoTemp.at(3));
            return QString(infoTemp.at(1))+" "+QString(infoTemp.at(2));
        case 3:
            if(infoTemp.at(3).isDigit())
                return infoTemp.at(4);
            return infoTemp.at(3);
        }
    }
    qDebug()<<"error at the"+QString::number(index)+"th entry";
    return "error";
}

QString editTableDialog::translateOutput(int index, int item)
{
    /*item = 1 - type
     *item = 2 - index
     *item = 3 - parameter*/
    QString outputTemp = outputEntries.at(index);
    QStringList tempList = outputTemp.split(",");
    QString infoTemp = tempList.last();

    if(QString(infoTemp.at(0))=="U")
    {
        switch(item) {
        case 1:
            return "unit";
        case 2:
            if(infoTemp.at(2).isDigit())//possibly there are more than 9 units
                return QString(infoTemp.at(1))+QString(infoTemp.at(2));
            return infoTemp.at(1);
        case 3:
            if(infoTemp.at(2).isDigit())
                return QString(infoTemp.at(3))+QString(infoTemp.at(4));
            return QString(infoTemp.at(2))+QString(infoTemp.at(3));
        }
    }
    if(QString(infoTemp.at(0)) == "P")
    {
        switch(item) {
        case 1:
            return "sp";
        case 2:
            if(infoTemp.at(3).isDigit())//in case there're more than 9 units
                return QString(infoTemp.at(1))+QString(infoTemp.at(2))+QString(infoTemp.at(3));
            return QString(infoTemp.at(1))+QString(infoTemp.at(2));
        case 3:
            if(infoTemp.at(3).isDigit())
                return infoTemp.at(4);
            return infoTemp.at(3);
        }
    }
    if(QString(infoTemp.at(0))=="S")
    {
        switch(item) {
        case 1:
            return "global";
        case 2:
            return "0";
        case 3:
            if(QString(infoTemp.at(1))=="A")
                return "CAP";
            if(QString(infoTemp.at(1))=="O")
                return "COP";
        }
    }
    qDebug()<<"error at the"+QString::number(index)+"th entry";
    return "error";
}

bool editTableDialog::tableNameUsed(QString name)
{
    if(tableName.isEmpty() || tableName==oldTableName)
        return false;

    QFile file(Sorputils::sorpTempDir().absoluteFilePath("tableTemp.xml"));
    if(!file.open(QIODevice::ReadWrite|QIODevice::Text))
    {
        globalpara.reportError("Fail to open case file to check if the table name is used.",this);
        return true;
    }
    QDomDocument doc;
    if(!doc.setContent(&file))
    {
        globalpara.reportError("Fail to load xml document to check if the table name is used.",this);
        file.close();
        return true;
    }
    file.close();
    QDomElement tableData = doc.elementsByTagName("TableData").at(0).toElement();
    auto tablesByTitle = Sorputils::mapElementsByAttribute(tableData.childNodes(), "title");

    return !tablesByTitle.contains(tableName);
}

void editTableDialog::keyPressEvent(QKeyEvent *event)
{
    if(event->key()==Qt::Key_Escape)
    {

    }
}

QString editTableDialog::addUnit(QString string)
{
    QStringList list = string.split("");
    if(list[list.count()-2]==QString("T"))
    {
        if(list[list.count()-3] == QString("H"))//HT:heat transfer
            return (globalpara.unitname_heatquantity);
        if(list[list.count()-3] == QString("W"))//WT: wetness level
            return " ";
        if(list[list.count()-3] == QString("N"))// NT: NTU value
            return " ";
        //T: temperature
        return (globalpara.unitname_temperature);
    }
    if(list[list.count()-2]==QString("P"))//P: pressure
        return (globalpara.unitname_pressure);
    if(list[list.count()-2]==QString("F"))
    {
        if(list[list.count()-3]!=QString("E"))//F: mass flow rate
            return (globalpara.unitname_massflow);
        //EF: effectiveness
        return " ";
    }
    if(list[list.count()-2] == QString("H"))//H: enthalpy
        return (globalpara.unitname_enthalpy);
    if(list[list.count()-2] == QString("A"))
    {
        if(list[list.count()-3]==QString("U"))//UA: ua value
            return (globalpara.unitname_UAvalue);
        if(list[list.count()-3]==QString("C"))//CA: cat value
            return (globalpara.unitname_temperature);
        if(list[list.count()-3]==QString("S"))//SA: system capacity
            return (globalpara.unitname_heatquantity);
        if(list[list.count()-3]==QString("N"))//NA: NTUa
            return " ";
    }
    if(list[list.count()-2] == QString("M"))
    {
        if(list[list.count()-3]==QString("N"))//NM: NTUm
            return " ";
        //LM: LMTD value
        return (globalpara.unitname_temperature);
    }
    if(list[list.count()-2] == QString("R"))
    {
        if(list[list.count()-3]==QString("M"))//MR:moisture removal rate
            return (globalpara.unitname_massflow);
    }
    if(list[list.count()-2] == QString("E"))
    {
        if(list[list.count()-3]==QString("M"))//ME:water evaporation rate
            return (globalpara.unitname_massflow);
        if(list[list.count()-3] == QString("H"))//HE: humidity efficiency
            return " ";
    }
    if(list[list.count()-2]==QString("C")||list[list.count()-2]==QString("W"))
        return (globalpara.unitname_concentration);
    //NW: NTUw,
    return " ";
}

bool editTableDialog::event(QEvent *e)
{
    if(e->type()==QEvent::ActivationChange)
    {
        if(qApp->activeWindow()==this)
        {
            theMainwindow->show();
            theMainwindow->raise();
            this->raise();
            this->setFocus();
        }
    }
    return QDialog::event(e);
}

/*! \file newparaplotdialog.cpp

    This file is part of SorpSim and is distributed under terms in the file LICENSE.

    Developed by Zhiyao Yang and Dr. Ming Qu for ORNL.

    \author Zhiyao Yang (zhiyaoYang)
    \author Dr. Ming Qu
    \author Nicholas Fette (nfette)

    \copyright 2015, UT-Battelle, LLC
    \copyright 2017-2018, Nicholas Fette

*/

#include <QDebug>
#include <QFile>
#include <QMessageBox>

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_legend.h>

#include "newparaplotdialog.h"
#include "ui_newparaplotdialog.h"
#include "mainwindow.h"
#include "dataComm.h"
#include "plotsdialog.h"
#include "sorputils.h"

extern globalparameter globalpara;
extern myScene * theScene;
extern MainWindow* theMainwindow;

//mode = 0:new paraPlot; mode = 1:from table; mode = 2:re-select plot
newParaPlotDialog::newParaPlotDialog(int theMode, QString tableName, QString plotName, QWidget *parent):
QDialog(parent),
ui(new Ui::newParaPlotDialog)
{
    mode = theMode;
    if(mode>0)
    {
        tName = tableName;
        pName = plotName;
    }
    else
        tName = "";
    ui->setupUi(this);
    if(!setTable())
    {
        globalpara.reportError("Fail to read table data from file!",this);
        reject();
    }
    setWindowModality(Qt::ApplicationModal);
    ui->xAutoScaleCB->setChecked(true);
    ui->yAutoScaleCB->setChecked(true);

    QLayout *mainLayout = layout();
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);

    ui->yList->setSelectionMode(QAbstractItemView::MultiSelection);

    if(mode>0)
        readTheFile(tName);
}

newParaPlotDialog::~newParaPlotDialog()
{
    delete ui;
}

void newParaPlotDialog::on_okButton_clicked()
{
    if(ui->xList->selectedItems().count()==0 || ui->yList->selectedItems().count()==0)
        globalpara.reportError("Please select variables for both axises.",this);
    else
    {
        if(mode<2)
        {
            // TODO: don't need to replace
            plotName = ui->plotNameLine->text().replace(QRegularExpression("[^a-zA-Z0-9_]"), "");
            if(plotName.isEmpty())
            {
                for(int i = 1;plotNameUsed(plotName);i++)
                    plotName = "plot_"+QString::number(i);
            }
            if(plotName.at(0).isDigit())
                plotName = "plot_"+plotName;
        }
        else
            plotName = pName;

        if(mode==0)
        {
            if(theScene->plotWindow!=NULL)
                theScene->plotWindow->close();
            setupXml();
            theScene->plotWindow = new plotsDialog("",false,theMainwindow);
            this->accept();
            theScene->plotWindow->exec();
        }
        else if(mode==1)
        {
            if(theScene->plotWindow!=NULL)
                theScene->plotWindow->close();
            setupXml();
            theScene->plotWindow = new plotsDialog("",true,theMainwindow);
            this->accept();
            theScene->plotWindow->exec();
        }
        else // eg. mode 2 or 4 (edit columns)
        {
            // TODO: need to apply changes
            // This setupXml() call doesn't quite work, since there is already such a plot.
            setupXml();
            this->accept();
        }
    }
}

bool newParaPlotDialog::setupXml()
{
    int num=ui->xList->count(),inputIndex;
    int outputCount=ui->yList->selectedItems().count();
    // Needs to be deleted at end of this scope.
    int * outputIndexes = new int[outputCount];

    int j=0;
    for (int i=0; i<num;i++)
    {
        if (ui->yList->item(i)->isSelected())
        {
            outputIndexes[j]=i;
            j++;
        }
        if(ui->xList->item(i)->isSelected())
            inputIndex = i;
    }
    QStringList outAxis_name, scaleInfo;
    for (int i=0; i<num;i++)
    {
        if (ui->yList->item(i)->isSelected())
            outAxis_name.append(ui->yList->item(i)->text());
    }

    if (!ui->xAutoScaleCB->isChecked())//axis_scaleinfo[0] = 0(auto)/ 1(manual, followed by min-max
    {
        scaleInfo.append(QString::number(1));
        scaleInfo.append(QString::number(ui->xMinLine->text().toFloat()));
        scaleInfo.append(QString::number(ui->xMaxLine->text().toFloat()));
        if(ui->xLinearButton->isChecked())
                scaleInfo.append(QString::number(0));
        else scaleInfo.append(QString::number(1));
    }
    else scaleInfo<<QString::number(0)<<QString::number(0)<<QString::number(0)<<QString::number(0);

    if (!ui->yAutoScaleCB->isChecked())
    {
        scaleInfo.append(QString::number(1));
        scaleInfo.append(QString::number(ui->yMinLine->text().toFloat()));
        scaleInfo.append(QString::number(ui->yMaxLine->text().toFloat()));
        if(ui->yLinearButton->isChecked())
                scaleInfo.append(QString::number(0));
        else scaleInfo.append(QString::number(1));
    }
    else scaleInfo<<QString::number(0)<<QString::number(0)<<QString::number(0)<<QString::number(0);

    if(scaleInfo.count()!=8)
    {
        qDebug()<<"the scaleInfo count is wrong!";
        return false;
    }

    // TODO: the only usage of setupXml(), newPropPlotDialog::on_okButton_clicked(), already closed the window.
    // So get rid of this code?
    // Also, if this is meant to lock the XML, that's a bad implementation of a mutex.
    if(mode!=2)
    {
        if(theScene->plotWindow!=NULL)
            theScene->plotWindow->close();
    }

    QFile file;
    if(mode==2)//from plot re-select
    {
        QFile oFile(globalpara.caseName);
        QString plotTempXML = Sorputils::sorpTempDir().absoluteFilePath("plotTemp.xml");
        oFile.copy(plotTempXML);
        file.setFileName(plotTempXML);
    }
    else file.setFileName(globalpara.caseName);

    qDebug()<<file.exists()<<"tname"<<tName<<"pname"<<plotName;
    QTextStream stream;
    stream.setDevice(&file);
    QDomDocument doc;
    QDomElement tableData, plotData, currentTable, newPlot;
    QDomNodeList tableRuns;
    //read file, locate the table in tableData and create a new branch in plotData
    if(!file.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        globalpara.reportError("Failed to open the case file for new parametric plot.",this);
        return false;
    }
    if(!doc.setContent(&file))
    {
        globalpara.reportError("Failed to load xml document for new parametric plot.",this);
        file.close();
        return false;
    }
    tableData = doc.elementsByTagName("TableData").at(0).toElement();
    auto tablesByTitle = Sorputils::mapElementsByAttribute(tableData.childNodes(), "title");
    if(doc.elementsByTagName("plotData").count()==0)
    {
        plotData = doc.createElement("plotData");
        QDomElement root = doc.elementsByTagName("root").at(0).toElement();
        root.appendChild(plotData);
    }
    else
        plotData = doc.elementsByTagName("plotData").at(0).toElement();

    // FIXED: write valid XML for children of <plotData>
    // Check if the plot name is not already used, then create the new element, else raise an error.
    QMap<QString, QDomElement> plotsByTitle;
    QDomNodeList plots = plotData.elementsByTagName("plot");
    for (int i = 0; i < plots.length(); i++)
    {
        QDomElement node = plots.at(i).toElement();
        QString nodeTitle = node.attribute("title");
        plotsByTitle.insert(nodeTitle, node);
    }

    if (plotsByTitle.contains(plotName))
    {
        globalpara.reportError("This <plot> title is already used.",this);
        file.close();
        return false;
    }

    if(mode==0)
        currentTable = tablesByTitle.value(ui->tableCB->currentText());
    else if(mode>0)
        currentTable = tablesByTitle.value(tName);
    // FIXED: write valid XML for children of <plotData>
    // Note: plotName.replace() is not necessary
    // <plot title="{plotName}" plotType="parametric">
    newPlot = doc.createElement("plot");
    plotData.appendChild(newPlot);
    newPlot.setAttribute("title",plotName);
    newPlot.setAttribute("plotType","parametric");
    if(mode==0)
        newPlot.setAttribute("tableName",ui->tableCB->currentText());
    else if(mode>0)
        newPlot.setAttribute("tableName",tName);
    int nRuns = currentTable.attribute("runs").toInt();
    newPlot.setAttribute("runs",QString::number(nRuns));

    //plotName: tableName, # of runs, axisNames, # of outputs, scaleInfo
    newPlot.setAttribute("xAxisName",ui->xList->currentItem()->text());
    newPlot.setAttribute("yAxisName",outAxis_name.join(","));
    newPlot.setAttribute("outputs",QString::number(outAxis_name.count()));
    newPlot.setAttribute("scaleInfo",scaleInfo.join(","));

    newPlot.setAttribute("inputIndex",QString::number(inputIndex));
    QStringList outputIndex;
    for(int i = 0; i < outputCount;i++)
        outputIndex.append(QString::number(outputIndexes[i]));
    newPlot.setAttribute("outputIndex",outputIndex.join(","));

    //run: input(name, value), outputs(number, name, value)
    tableRuns = currentTable.elementsByTagName("Run");
    for(int i = 0; i < nRuns;i++)
    {
        //prepare values for input/outputs
        QDomElement currentTableRun = tableRuns.at(i).toElement();
        int nTableInputs = currentTableRun.elementsByTagName("Input").count();
        QString xValue;
        if(inputIndex<=nTableInputs-1)
        {
            QDomElement tableInput = currentTableRun.elementsByTagName("Input").at(inputIndex).toElement();
            xValue = tableInput.elementsByTagName("value").at(0).toElement().text();
        }
        else
        {
            QDomElement tableInput =
                    currentTableRun.elementsByTagName("Output").at(inputIndex-nTableInputs).toElement();
            xValue= tableInput.elementsByTagName("value").at(0).toElement().text();
        }
        QStringList yValues;
        for(int p = 0; p < outputCount;p++)
        {
            if(outputIndexes[p]<=nTableInputs-1)
            {
                QDomElement tableInput = currentTableRun.elementsByTagName("Input").at(outputIndexes[p]).toElement();
                yValues.append(tableInput.elementsByTagName("value").at(0).toElement().text());
            }
            else
            {
                QDomElement tableOutput =
                        currentTableRun.elementsByTagName("Output").at(outputIndexes[p]-nTableInputs).toElement();
                yValues.append(tableOutput.elementsByTagName("value").at(0).toElement().text());
            }
        }
        //prepare values for input/outputs
        //create new element and insert values
        QDomElement newRun = doc.createElement("Run");
        newRun.setAttribute("No.",QString::number(i));
        newPlot.appendChild(newRun);

        QDomElement newInput = doc.createElement("Input");
        newInput.setAttribute("name",ui->xList->currentItem()->text());
        QDomElement inputValue = doc.createElement("value");
        QDomText theValue = doc.createTextNode(xValue);
        inputValue.appendChild(theValue);
        newInput.appendChild(inputValue);
        newRun.appendChild(newInput);

        for(int j = 0; j < outputCount;j++)
        {
            QDomElement newOutput = doc.createElement("Output");
            newOutput.setAttribute("name",outAxis_name.at(j));
            QDomElement outputValue = doc.createElement("value");
            QDomText theValue = doc.createTextNode(yValues.at(j));
            outputValue.appendChild(theValue);
            newOutput.appendChild(outputValue);
            newRun.appendChild(newOutput);
        }
        //create new element and insert values
    }

    file.resize(0);
    doc.save(stream,4);
    file.close();
    delete[] outputIndexes;
    return true;
}

bool newParaPlotDialog::plotNameUsed(QString name)
{
    if(name.isEmpty())
        return true;
    QFile file;
    QDir dir;
    if(mode==2)
    {
        QString plotTempXML = Sorputils::sorpTempDir().absoluteFilePath("plotTemp.xml");
        file.setFileName(plotTempXML);
    } else
        file.setFileName(globalpara.caseName);

    if(!file.open(QIODevice::ReadWrite|QIODevice::Text))
    {
        globalpara.reportError("Failed to open the case file to check if plot name is used.",this);
        return true;
    }

    QDomDocument doc;
    if(!doc.setContent(&file))
    {
        globalpara.reportError("Failed to load xml document to check if plot name is used.",this);
        file.close();
        return true;
    }
    QDomElement plotData = doc.elementsByTagName("plotData").at(0).toElement();
    QDomNodeList thePlots = plotData.elementsByTagName("plot");
    QMap<QString, QDomElement> plotsByTitle;
    for (int i = 0; i < thePlots.length(); i++) {
        QDomElement iPlot = thePlots.at(i).toElement();
        plotsByTitle.insert(iPlot.attribute("title"), iPlot);
    }
    file.close();
    return plotsByTitle.contains(name);
}

void newParaPlotDialog::on_cancelButton_clicked()
{
    reject();
}

void newParaPlotDialog::on_tableCB_currentIndexChanged(const QString &arg1)
{
    if(!readTheFile(arg1))
        globalpara.reportError("Failed to read the table information from file!",this);
    else
        ui->plotNameLine->setText(arg1);
}

void newParaPlotDialog::on_xAutoScaleCB_toggled(bool checked)
{
    if(!checked)
    {
        ui->xLine->show();
        ui->xMax->show();
        ui->xMaxLine->show();
        ui->xMin->show();
        ui->xMinLine->show();
        ui->xInterval->show();
        ui->xIntervalLine->show();
        ui->xLinearButton->show();
        ui->xLogButton->show();
    }
    else
    {
        ui->xLine->hide();
        ui->xMax->hide();
        ui->xMaxLine->hide();
        ui->xMin->hide();
        ui->xMinLine->hide();
        ui->xInterval->hide();
        ui->xIntervalLine->hide();
        ui->xLinearButton->hide();
        ui->xLogButton->hide();
    }
}

void newParaPlotDialog::on_yAutoScaleCB_toggled(bool checked)
{
    if(!checked)
    {
        ui->yLine->show();
        ui->yMax->show();
        ui->yMaxLine->show();
        ui->yMin->show();
        ui->yMinLine->show();
        ui->yInterval->show();
        ui->yIntervalLine->show();
        ui->yLinearButton->show();
        ui->yLogButton->show();
    }
    else
    {
        ui->yLine->hide();
        ui->yMax->hide();
        ui->yMaxLine->hide();
        ui->yMin->hide();
        ui->yMinLine->hide();
        ui->yInterval->hide();
        ui->yIntervalLine->hide();
        ui->yLinearButton->hide();
        ui->yLogButton->hide();
    }
}

bool newParaPlotDialog::readTheFile(QString tableName)
{
    ui->xList->clear();
    ui->yList->clear();
    QFile file;
    QDir dir;
    if(mode==2)
    {
        QString plotTempXML = Sorputils::sorpTempDir().absoluteFilePath("plotTemp.xml");
        file.setFileName(plotTempXML);
    } else
        file.setFileName(globalpara.caseName);

    QDomDocument doc;
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        globalpara.reportError("Failed to open the case file for the new plot.",this);
        return false;
    }
    if(!doc.setContent(&file))
    {
        globalpara.reportError("Failed to load xml document for the new plot.",this);
        file.close();
        return false;
    }

    QDomElement tables = doc.elementsByTagName("TableData").at(0).toElement();
    auto tablesByTitle = Sorputils::mapElementsByAttribute(tables.childNodes(), "title");
    if (!tablesByTitle.contains(tableName))
    {
        globalpara.reportError("Failed to load specified table from xml .",this);
        file.close();
        return false;
    }
    QDomElement currentTable = tablesByTitle.value(tableName);
    QDomNodeList Runs = currentTable.elementsByTagName("Run");

    int paranum=Runs.at(0).toElement().elementsByTagName("Input").count()+Runs.at(0).toElement().elementsByTagName("Output").count();
    rownum=Runs.count();
    tablevalue= new double *[rownum];
    for(int i=0;i<rownum;i++)
        tablevalue[i]=new double [paranum];

    QDomElement headers = currentTable.elementsByTagName("header").at(0).toElement();
    QStringList headerList = headers.text().split(";");

    QDomElement inputs = currentTable.elementsByTagName("inputEntries").at(0).toElement();
    QStringList inputD = inputs.text().split(";");
    for(int i = 0; i<inputD.count();i++)
    {
//        tempIn = inputD[i].split(",");
//        inputD[i] = tempIn[0]+" - "+tempIn[1];
        QString tempString = headerList.at(i);
        tempString.replace("\n","");
        inputD[i] = tempString;
    }
    QDomElement outputs = currentTable.elementsByTagName("outputEntries").at(0).toElement();
    QStringList outputD = outputs.text().split(";");
    for(int i = 0; i<outputD.count();i++)
    {
//        tempOut = outputD[i].split(",");
//        if(tempOut[1]=="SA"||tempOut[1]=="SO")
//        {
//            outputD[i] = tempOut[0];

//        }
//        else
//        {
//            outputD[i] = tempOut[0]+" - "+tempOut[1];

//        }
        QString tempString = headerList.at(i+inputD.count());
        tempString.replace("\n","");
        outputD[i] = tempString;
    }
    QStringList list = inputD+outputD;

    for(int i = 0;i<list.count();i++)
    {
        ui->xList->addItem(list[i]);
        ui->yList->addItem(list[i]);
    }

    for(int i = 0; i <Runs.count(); i++)
    {
        QDomElement currentRun = Runs.at(i).toElement();
        QDomNodeList inputs = currentRun.elementsByTagName("Input");
        for(int j = 0; j < inputs.count();j++)
        {
            QDomElement currentInput = inputs.at(j).toElement();
            if(currentInput.attribute("type")=="sp")
                tablevalue[i][j]=currentInput.elementsByTagName("value").at(0).toElement().text().toDouble();
            else if(currentInput.attribute("type") == "unit")
                tablevalue[i][j]=currentInput.elementsByTagName("value").at(0).toElement().text().toDouble();
        }

        QDomNodeList outputs = currentRun.elementsByTagName("Output");
        for(int j=inputs.count(); j < inputs.count()+outputs.count();j++)
        {
            QDomElement currentoutput = outputs.at(j-inputs.count()).toElement();
            if(currentoutput.attribute("type")=="sp")
                tablevalue[i][j]=currentoutput.elementsByTagName("value").at(0).toElement().text().toDouble();
            else if(currentoutput.attribute("type") == "unit")
                tablevalue[i][j]=currentoutput.elementsByTagName("value").at(0).toElement().text().toDouble();
            else if(currentoutput.attribute("type") == "global")
                tablevalue[i][j]=currentoutput.elementsByTagName("value").at(0).toElement().text().toDouble();
        }

    }
    // TODO: member tablevalue is never referenced after being set in this function. What is intent of the array?
    // TODO: dynamically allocated arrays are created with new, stored in tablevalue, but never delete[]'d!
    return true;
}

bool newParaPlotDialog::setTable()
{
    QFile file;
    QDir dir;
    if(mode==2)
    {
        QString plotTempXML = Sorputils::sorpTempDir().absoluteFilePath("plotTemp.xml");
        file.setFileName(plotTempXML);
    } else
        file.setFileName(globalpara.caseName);

    QDomDocument doc;
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        globalpara.reportError("Failed to open the case file to get available parametric table information.",this);
        return false;
    }
    if(!doc.setContent(&file))
    {
        globalpara.reportError("Failed to load xml document to get available parametric table information.",this);
        file.close();
        return false;
    }
    if(doc.elementsByTagName("TableData").at(0).toElement().childNodes().count()<1)
    {
        globalpara.reportError("There is no table data available in the file.");
        file.close();
        return false;
    }
    if(mode ==0)
    {
        QDomElement tables = doc.elementsByTagName("TableData").at(0).toElement();
        QStringList myTables;
        for(int i = 0; i < tables.childNodes().count();i++)
            myTables.append(tables.childNodes().at(i).toElement().attribute("title"));

        ui->tableCB->addItems(myTables);
        ui->tableCB->setCurrentIndex(0);
    }
    else
    {
        ui->label->hide();
        ui->tableCB->hide();
        ui->tableName->setText("Table Name:"+tName);
        if(mode==2)
        {
            ui->plotNameLine->hide();
            ui->plotNameLabel->setText("Plot Name:"+pName);
        }
    }
    file.close();
    return true;
}

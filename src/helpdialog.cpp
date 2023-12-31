/*! \file helpdialog.cpp
    \brief Provides the class helpDialog.

    This file is part of SorpSim and is distributed under terms in the file LICENSE.

    Developed by Zhiyao Yang and Dr. Ming Qu for ORNL.

    \author Zhiyao Yang (zhiyaoYang)
    \author Dr. Ming Qu

    \copyright 2015, UT-Battelle, LLC

*/

#include <QDebug>

#include "helpdialog.h"
#include "ui_helpdialog.h"
#include "dataComm.h"
#include "sorputils.h"

extern globalparameter globalpara;

helpDialog::helpDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::helpDialog)
{
    ui->setupUi(this);
    setWindowTitle("Help Content");

    list = ui->itemList;
    content = ui->itemContent;
    content->setReadOnly(true);

    if(!loadList()){
        globalpara.reportError("Error loading help documents.",this);
        close();
    }
}

helpDialog::~helpDialog()
{
    delete ui;
}

void helpDialog::on_itemList_itemActivated(QTreeWidgetItem *item, int column)
{
    content->clear();

    QString itemName = item->text(0);
    itemName.replace(" ","_");    

    QFile ofile(Sorputils::sorpSettings());
    QDomDocument doc;
    if(!ofile.open((QIODevice::ReadOnly|QIODevice::Text))){
        qDebug()<<"no file";
        return;
    }
    if(!doc.setContent(&ofile))
    {
        qDebug()<<"error setting doc";
        ofile.close();
        return;
    }
    QDomElement helpContent;
    if(doc.elementsByTagName("helpContent").count()==0){
        qDebug()<<"no helpContent";
        return;
    }

    helpContent = doc.elementsByTagName("helpContent").at(0).toElement();
    QDomElement helpItem = helpContent.elementsByTagName(itemName).at(0).toElement();
    if(item->parent())
        content->setPlainText(helpItem.text());

    ofile.close();

    if(content->toPlainText()=="")
        content->setPlainText("Welcome to SorpSim Help Document!\n\nPlease double-click to view the items.\n\nMore detailed documentation can be found in the user manual.\n\nThank you for using SorpSim!");
}

bool helpDialog::loadList()
{
    list->clear();

    QFile ofile(Sorputils::sorpSettings());
    QDomDocument doc;
    if(!ofile.open((QIODevice::ReadOnly|QIODevice::Text))){
        qDebug()<<"no file";
        return false;
    }
    if(!doc.setContent(&ofile))
    {

        qDebug()<<"error setting doc";
        ofile.close();
        return false;
    }
    if(doc.elementsByTagName("helpContent").count()==0){
        qDebug()<<"no helpContent";
        return false;
    }
    QDomElement helpContent = doc.elementsByTagName("helpContent").at(0).toElement();
    for(int i = 0; i < helpContent.childNodes().count();i++)
    {
        QDomElement helpItem = helpContent.childNodes().at(i).toElement();
        QTreeWidgetItem*item;
        QString name;
        if(helpItem.hasChildNodes())
        {
            name = helpItem.tagName();
            name.replace("_"," ");
            item = addTreeRoot(name);
            QDomElement subHelpItem;
            for(int j = 0; j < helpItem.childNodes().count();j++)
            {
                subHelpItem = helpItem.childNodes().at(j).toElement();
                name = subHelpItem.tagName();
                name.replace("_"," ");
                if(name!=""){
                    addTreeChild(item,name);
                }
            }
        } else {
            name = helpItem.tagName();
            name.replace("_"," ");
            item = addTreeRoot(name);
        }
    }

    ofile.close();

    if(content->toPlainText()=="")
        content->setPlainText("Welcome to SorpSim Help Document!\n\nPlease double-click to view the items.\n\nMore detailed documentation can be found in the user manual.\n\nThank you for using SorpSim!");

    return true;
}

QTreeWidgetItem *helpDialog::addTreeRoot(QString name, QString description)
{
    QTreeWidgetItem *treeItem = new QTreeWidgetItem(list);
    treeItem->setText(0, name);
    treeItem->setText(1, description);
    return treeItem;
}

QTreeWidgetItem *helpDialog::addTreeChild(QTreeWidgetItem *parent, QString name, QString description)
{
    QTreeWidgetItem *treeItem = new QTreeWidgetItem();
    treeItem->setText(0, name);
    treeItem->setText(1, description);

    parent->addChild(treeItem);
    return treeItem;
}

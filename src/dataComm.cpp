/*! \file dataComm.cpp
    \brief Interface classes between SorpSim and ABSIM

    This file is part of SorpSim and is distributed under terms in the file LICENSE.

    Developed by Zhiyao Yang and Dr. Ming Qu for ORNL.

    \author Zhiyao Yang (zhiyaoYang)
    \author Dr. Ming Qu
    \author Nicholas Fette (nfette)

    \copyright 2015, UT-Battelle, LLC
    \copyright 2017-2018, Nicholas Fette

*/

#include <QString>
#include <QSet>
#include <QVector>
#include <QStringList>

#include "dataComm.h"
#include "unit.h"
#include "sorputils.h"

extern globalparameter globalpara;
extern int globalcount;
extern unit*dummy;

extern calOutputs outputs;

void globalparameter::resetIfixes(const char parameter)
{
    QList<QSet<Node *> > group;
    if(parameter == 't')
     group = globalpara.tGroup;
    else if(parameter == 'p')
     group = globalpara.pGroup;
    else if(parameter == 'c')
     group = globalpara.cGroup;
    else if(parameter == 'f')
     group = globalpara.fGroup;
    else if(parameter == 'w')
     group = globalpara.wGroup;

    if(parameter=='f'){
        bool hasOne = false;
        foreach(QSet<Node*> set,group){
            foreach(Node*node,set){
                if(node->ndum==1){
                    hasOne = true;
                }
            }
        }
    }

    Node*tempNode;
    QSet<Node*>examSet;
    unit*head = dummy;
    for(int i = 0;i<globalcount;i++)//if ifix == 0, remove from groups
    {
        head = head->next;
        for(int j = 0; j<head->usp;j++)
        {
            tempNode = head->myNodes[j];
            if(parameter == 't'&&tempNode->itfix==0)
            {
                globalpara.allSet.clear();
                tempNode->searchAllSet("t");
                examSet = globalpara.allSet;
                foreach(QSet<Node*> theSet,group)
                {
                    if(!theSet.intersect(examSet).isEmpty())
                    {
                        group.removeOne(theSet);
                        theSet.subtract(examSet);
                        group.append(theSet);
                    }
                }
            }
            else if(parameter == 'p'&&tempNode->ipfix==0)
            {
                globalpara.allSet.clear();
                tempNode->searchAllSet("p");
                examSet = globalpara.allSet;
                foreach(QSet<Node*> theSet,group)
                {
                    if(!theSet.intersect(examSet).isEmpty())
                    {
                        group.removeOne(theSet);
                        theSet.subtract(examSet);
                        group.append(theSet);
                    }
                }
            }
            else if(parameter == 'c'&&tempNode->icfix==0)
            {
                globalpara.allSet.clear();
                tempNode->searchAllSet("c");
                examSet = globalpara.allSet;
                foreach(QSet<Node*> theSet,group)
                {
                    if(!theSet.intersect(examSet).isEmpty())
                    {
                        group.removeOne(theSet);
                        theSet.subtract(examSet);
                        group.append(theSet);
                    }
                }
            }
            else if(parameter == 'f'&&tempNode->iffix==0)
            {
                globalpara.allSet.clear();
                tempNode->searchAllSet("f");
                examSet = globalpara.allSet;
                foreach(QSet<Node*> theSet,group)
                {
                    if(!theSet.intersect(examSet).isEmpty())
                    {
                        group.removeOne(theSet);
                        theSet.subtract(examSet);
                        group.append(theSet);
                    }
                }
            }
            else if(parameter == 'w'&&tempNode->iwfix==0)
            {
                globalpara.allSet.clear();
                tempNode->searchAllSet("w");
                examSet = globalpara.allSet;
                foreach(QSet<Node*> theSet,group)
                {
                    if(!theSet.intersect(examSet).isEmpty())
                    {
                        group.removeOne(theSet);
                        theSet.subtract(examSet);
                        group.append(theSet);
                    }
                }
            }
        }
    }

    foreach(QSet<Node*> tempSet,group)
    {
        QSet<int> spIndex;//make sure each group has more than one state points
        foreach(Node*node,tempSet)
        {
            if(!spIndex.contains(node->ndum))
                spIndex.insert(node->ndum);
        }
        if(spIndex.count()<2)
        {
            group.removeOne(tempSet);

            foreach(Node* node,tempSet)
            {
                if(parameter == 't')
                    node->itfix = 1;
                else if(parameter == 'p')
                    node->ipfix = 1;
                else if(parameter == 'c')
                    node->icfix = 1;
                else if(parameter == 'f')
                    node->iffix = 1;
                else if(parameter == 'w')
                    node->iwfix = 1;
                node->passParaToMerged();
            }
        }
        else
        {
            foreach(Node*node,tempSet)
            {
                globalpara.allSet.clear();
                node->searchAllSet(QString(parameter));//switch set searching for parameter
                QSet<Node*>sameSet = globalpara.allSet;
                foreach(Node*node,sameSet)
                {
                    foreach(QSet<Node*> set,group)
                    {
                        if(set!=tempSet&&set.contains(node))//make sure that no one point in two different groups
                        {
                            group.removeOne(set);
                            set.remove(node);
                            group.append(set);
                        }
                    }
                    if(!tempSet.contains(node))//make sure there is no missing point
                    {
                        group.removeOne(tempSet);
                        tempSet.insert(node);
                        group.append(tempSet);
                    }
                }
            }
        }
    }


    QSet<Node*> tempSet;
    for(int i = 0; i < group.count();i++)//assigning ifixes to points
    {
        tempSet = group.at(i);
        foreach(Node*node,tempSet)
        {
            if(parameter == 't')
                node->itfix = i+2;
            else if(parameter == 'p')
                node->ipfix = i+2;
            else if(parameter == 'c')
                node->icfix =i+2;
            else if(parameter == 'f')
                node->iffix = i+2;
            else if(parameter == 'w')
                node->iwfix = i+2;
            node->passParaToMerged();
        }
    }

    if(parameter == 't')
        globalpara.tGroup = group;
    else if(parameter == 'p')
        globalpara.pGroup = group;
    else if(parameter == 'c')
        globalpara.cGroup = group;
    else if(parameter == 'f')
        globalpara.fGroup = group;
    else if(parameter == 'w')
        globalpara.wGroup = group;
}

void globalparameter::reportError(QString errorMessage, QWidget *parent)
{
    QMessageBox::warning(parent, "Warning", errorMessage);
}

void globalparameter::addGroup(const char parameter, QSet<Node *> spSet)
{
    if(parameter == 't')
    {
        globalpara.tGroup.append(spSet);
        resetIfixes('t');
    }
    else if(parameter == 'p')
    {
        globalpara.pGroup.append(spSet);
        resetIfixes('p');
    }
    else if(parameter == 'c')
    {
        globalpara.cGroup.append(spSet);
        resetIfixes('c');
    }
    else if(parameter == 'f')
    {
        globalpara.fGroup.append(spSet);
        resetIfixes('f');
    }
    else if(parameter == 'w')
    {
        globalpara.wGroup.append(spSet);
        resetIfixes('w');
    }
}

bool globalparameter::findNextPtxPoint(Node *thisNode, Node *StartingNode)
{
    if(!thisNode->linked)//would not start/continue from an open point or an inseide point
        return false;
    //initialize from an inlet
    if(thisNode==StartingNode)
    {
        Node*outNode = thisNode;
        if(thisNode->isOutlet)
        {
            Link*theLink = thisNode->myLinks.values().first();
            outNode = theLink->myFromNode;
            if(outNode->isOutlet)
                outNode = theLink->myToNode;
        }
        else if(thisNode->isinside)
        {
            foreach(Node*node,thisNode->FCSet)
            {
                if(!node->isOutlet&&!node->isinside)
                    outNode = node;
            }
        }
        thisNode = outNode;
        //qDebug()<<"starting from "<<thisNode->ndum;
    }

    if(!ptxStream.contains(thisNode->ndum))
        ptxStream.append(thisNode->ndum);

    unit*thisUnit = thisNode->myUnit;
    //qDebug()<<"on sp"<<thisNode->ndum<<"on unit"<<thisUnit->utext->text();

    //if point is inside, count it in and go on with points in the same component
    for(int i = 0; i < thisUnit->usp;i++)
    {
        Node*node = thisUnit->myNodes[i];
        if(node->isinside
                &&outputs.ptxPoints.contains(node->ndum)
                &&node!=StartingNode
                &&thisNode->FCSet.contains(node)
                &&!ptxStream.contains(node->ndum))
        {
            ptxStream.append(node->ndum);
            //since "startNode" is always linked, doubt this part would ever work
            if(node->ndum==StartingNode->ndum&&node!=StartingNode)//loop closed
            {
                return true;
            }
        }
    }

    ///if point is linked, go to next point, if loop close in that one, stop searching through
    ///this component and return true; if loop doesn't close in that one, go on searching
    ///
    ///if there are multiple outlets in the component, try through all the outlets until a closed loop
    QSet<Node*> outlets;
    for(int i = 0; i < thisUnit->usp;i++)
    {
        Node*node = thisUnit->myNodes[i];
        if(node->isOutlet&&thisNode->FCSet.contains(node))
            outlets.insert(node);
    }
    foreach(Node*node,outlets)
    {
        if(outputs.ptxPoints.contains(node->ndum))
        {
            if(ptxStream.contains(node->ndum))
            {
                if(node->ndum==StartingNode->ndum&&node!=thisNode)
                {
                    ptxStream.append(node->ndum);
                    return true;
                }
                return false;
            }
            if(node->linked)
            {
                Link*theLink = node->myLinks.values().first();
                Node*otherNode = theLink->myToNode;
                if(node==theLink->myToNode)
                    otherNode = theLink->myFromNode;

                QList<int> tempStream = ptxStream;
                if(findNextPtxPoint(otherNode,StartingNode))
                    return true;
                ptxStream = tempStream;
            }
        }
    }
    //if it reaches here, it means all outlets are searched and no closed loop found
    return false;
}

void globalparameter::removeRecentFile(QString delFileName)
{
    QFile ofile(Sorputils::sorpSettings());
    QTextStream stream;
    QDomDocument doc;
    if(!ofile.open(QIODevice::ReadWrite|QIODevice::Text))
        return;
    if(!doc.setContent(&ofile))
    {
        ofile.close();
        return;
    }

    QDomElement recentFiles = doc.elementsByTagName("recentFiles").at(0).toElement();
    int fileCount = recentFiles.childNodes().count();
    if(fileCount==0)
        return;

    QStringList fileList;
    globalpara.recentFileList.clear();
    for(int i = 0; i < fileCount;i++)
    {
        QDomElement currentFile = recentFiles.childNodes().at(i).toElement();
        if(currentFile.attribute("fileDir")==delFileName||fileList.contains(currentFile.attribute("fileDir")))
            recentFiles.removeChild(recentFiles.childNodes().at(i));
        else
            fileList<<currentFile.attribute("fileDir");
    }
    foreach(QString fileName,fileList)
        globalpara.recentFileList.append(fileName);

    ofile.resize(0);
    stream.setDevice(&ofile);
    doc.save(stream,4);
    ofile.close();
}

void globalparameter::checkMinMax(globalparameter* theGlobal)
{
    theGlobal->tmax = 1;
    theGlobal->tmin = 100;
    theGlobal->fmax = 1;
    theGlobal->pmax = 1;
    unit* iterator = dummy;
    Node * tempNode;
    for(int i = 0; i < globalcount; i++){
        iterator = iterator->next;
        for(int j = 0; j < iterator->usp; j ++){
            tempNode = iterator->myNodes[j];
            if(theGlobal->tmax<tempNode->t){
                theGlobal->tmax = tempNode->t;
            }
            if(theGlobal->tmin>tempNode->t){
                theGlobal->tmin = tempNode->t;
            }
            if(theGlobal->fmax<tempNode->f){
                theGlobal->fmax = tempNode->f;
            }
            if(theGlobal->pmax<tempNode->p){
                theGlobal->pmax = tempNode->p;
            }
        }
    }
}

void globalparameter::resetGlobalPara()
{
    title = "title";
    tmax = 100;
    tmin = 0;
    fmax = 10;
    pmax = 100;

    maxfev = 500;
    msglvl = 500;
    iuflag = 1;
    ftol = 1e-5;
    xtol = 1e-5;

    cop = 0;
    capacity = 0;

    copT = 0;
    capacityT = 0;

    fluids.clear();
    tGroup.clear();
    fGroup.clear();
    cGroup.clear();
    pGroup.clear();
    wGroup.clear();
}

bool globalparameter::spResSelected()
{
    return (resT|resC|resF|resP|resW|resH);
}

bool globalparameter::compResSelected()
{
    return (resUA|resEFF|resNTU|resCAT|resLMTD|resHT);
}

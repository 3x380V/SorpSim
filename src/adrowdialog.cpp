/*adrowdialog.cpp
 * [SorpSim v1.0 source code]
 * [developed by Zhiyao Yang and Dr. Ming Qu for ORNL]
 * [last updated: 10/12/15]
 *
 * dialog to edit the number of rows in an existing parametric table
 * collects the user input at the dialog, change the global boolean variable accordingly
 * the change in table and XML file is enforced in the tableDialog.cpp
 *
 * called by tabledialog.cpp
 */



#include "adrowdialog.h"
#include "ui_adrowdialog.h"
#include "tabledialog.h"
#include <QMessageBox>
#include <QLayout>
#include "mainwindow.h"

extern MainWindow *theMainwindow;

adRowDialog::adRowDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::adRowDialog),
    adrAccepted(false),
    adrIsInsert(true),
    adrPosition(1),
    adrIar(0),
    adrNr(0)
{
    ui->setupUi(this);
    ui->insertButton->setChecked(true);
    ui->atTopButton->setChecked(true);
    setWindowFlags(Qt::Dialog);
    ui->noRunsb->setMinimum(0);
    ui->aftetsb->setMinimum(0);

    QLayout *mainLayout = layout();
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);


}

adRowDialog::~adRowDialog()
{
    delete ui;
}

void adRowDialog::setTableName(QString name)
{
    ui->tableName->setText(name);
}

void adRowDialog::on_OKButton_clicked()
{
    if((ui->insertButton->isChecked()||ui->removeButton->isChecked())
            &&(ui->atTopButton->isChecked()||ui->atBottomButton->isChecked()||ui->afterButton->isChecked())
            &&ui->noRunsb->value()>0)
    {
        if(ui->insertButton->isChecked())
            adrIsInsert = true;
        if(ui->removeButton->isChecked())
            adrIsInsert = false;
        if(ui->atTopButton->isChecked())
            adrPosition = 1;
        if(ui->atBottomButton->isChecked())
            adrPosition = 2;
        if(ui->afterButton->isChecked())
        {
            adrPosition = 3;
            adrIar = ui->aftetsb->value();
        }
        adrNr = ui->noRunsb->value();
        adrAccepted = true;
        accept();
    }
    else// CAN ADD DETAILED DIRECTION TO WHERE THE PROBLEM IS
    {
        QMessageBox adrBox;
        adrBox.setWindowTitle("Warning!");
        adrBox.setText("Please finish all sections!");
        adrBox.exec();
    }

}

void adRowDialog::on_cancelButton_clicked()
{
    reject();
    adrAccepted = false;
}

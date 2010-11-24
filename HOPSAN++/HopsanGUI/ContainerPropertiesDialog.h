//!
//! @file   ContainerPropertiesDialog.h
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2010-11-24
//!
//! @brief Contains a class for interact with paramters
//!
//$Id: ContainerPropertiesDialog.h 2206 2010-11-24 09:27:21Z petno25 $

#ifndef CONTAINERPROPERTIESDIALOG_H
#define CONTAINERPROPERTIESDIALOG_H

#include <QDialog>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QLabel>
#include <QGridLayout>
//#include <QSpinBox>

class GUIContainerObject;

class ContainerPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
    ContainerPropertiesDialog(GUIContainerObject *pContainerObject, QWidget *pParentWidget);

public slots:
    void show();
    void updateValues();
    void browseUser();
    void browseIso();

private:
    bool mIsoBool;

    QLineEdit *mpNameEdit;

    QLineEdit *mpUserIconPath;
    QLineEdit *mpIsoIconPath;
    QLineEdit *mpNumberOfSamplesBox;
    QLineEdit *mpCQSLineEdit;

    QLabel *mpUserIconLabel;
    QLabel *mpIsoIconLabel;
    QLabel *mpNumberOfSamplesLabel;
    QLabel *mpCQSLable;

    QCheckBox *mpIsoCheckBox;
    QCheckBox *mpDisableUndoCheckBox;

    QPushButton *mpIsoIconBrowseButton;
    QPushButton *mpUserIconBrowseButton;
    QPushButton *mpCancelButton;
    QPushButton *mpApplyButton;
    QPushButton *mpOkButton;

    QDialogButtonBox *mpButtonBox;
    QWidget *mpCentralwidget;
    QGridLayout *mpLayout;
};

#endif // CONTAINERPROPERTIESDIALOG_H

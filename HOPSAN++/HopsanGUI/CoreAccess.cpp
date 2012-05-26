/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   CoreAccess.cpp
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2010-01-01
//!
//! @brief Contains the CoreAccess class, The API class for communication with the HopsanCore
//!
//$Id$

#include "CoreAccess.h"
#include "MainWindow.h"
#include "Widgets/ProjectTabWidget.h"
#include "Widgets/MessageWidget.h"
#include "GUIObjects/GUISystem.h"
#include <QString>
#include <QVector>

//HopsanCore includes
#include "HopsanCore.h"
#include "ComponentSystem.h"

using namespace std;

//! @brief Help function to copy parameter data from core to GUI class
void copyParameterData(const hopsan::Parameter *pCoreParam, CoreParameterData &rGUIParam)
{
    rGUIParam.mName = QString::fromStdString(pCoreParam->getName());
    rGUIParam.mType = QString::fromStdString(pCoreParam->getType());
    rGUIParam.mValue = QString::fromStdString(pCoreParam->getValue());
    rGUIParam.mUnit = QString::fromStdString(pCoreParam->getUnit());
    rGUIParam.mDescription = QString::fromStdString(pCoreParam->getDescription());
    rGUIParam.mIsDynamic = pCoreParam->isDynamic();
    rGUIParam.mIsEnabled = pCoreParam->isEnabled();
}

bool CoreLibraryAccess::hasComponent(QString componentName)
{
    return hopsan::HopsanEssentials::getInstance()->hasComponent(componentName.toStdString());
}


bool CoreLibraryAccess::loadComponentLib(QString fileName)
{
    return hopsan::HopsanEssentials::getInstance()->loadExternalComponentLib(fileName.toStdString());
}

bool CoreLibraryAccess::unLoadComponentLib(QString fileName)
{
    return hopsan::HopsanEssentials::getInstance()->unLoadExternalComponentLib(fileName.toStdString());
}

//! @brief Reserves a type name in the Hopsan Core, to prevent external libs from loading components with that specific typename
bool CoreLibraryAccess::reserveComponentTypeName(const QString typeName)
{
    return hopsan::HopsanEssentials::getInstance()->reserveComponentTypeName(typeName.toStdString());
}

void CoreLibraryAccess::getLoadedLibNames(QVector<QString> &rLibNames)
{
    std::vector<std::string> names;
    hopsan::HopsanEssentials::getInstance()->getExternalComponentLibNames(names);

    rLibNames.clear();
    rLibNames.reserve(names.size());
    for (unsigned int i=0; i<names.size(); ++i)
    {
        rLibNames.push_back(QString::fromStdString(names[i]));
    }
}


size_t CoreMessagesAccess::getNumberOfMessages()
{
    return hopsan::HopsanEssentials::getInstance()->checkMessage();
}

void CoreMessagesAccess::getMessage(QString &rMessage, QString &rType, QString &rTag)
{
    std::string msg, tag, type;
    hopsan::HopsanEssentials::getInstance()->getMessage(msg, type, tag);
    rMessage = QString::fromStdString(msg);
    rTag = QString::fromStdString(tag);
    rType = QString::fromStdString(type);
}


CoreSystemAccess::CoreSystemAccess(QString name, CoreSystemAccess* pParentCoreSystemAccess)
{
    //Create new Core system component
    if (pParentCoreSystemAccess == 0)
    {
        //Create new root system
        mpCoreComponentSystem = hopsan::HopsanEssentials::getInstance()->createComponentSystem();
    }
    else
    {
        //Creating a subsystem, setting internal pointer
        mpCoreComponentSystem = pParentCoreSystemAccess->getCoreSubSystemPtr(name);
    }
}

hopsan::ComponentSystem* CoreSystemAccess::getCoreSystemPtr()
{
    return mpCoreComponentSystem;
}

hopsan::ComponentSystem* CoreSystemAccess::getCoreSubSystemPtr(QString name)
{
    qDebug() << " corecomponentsystemname: " <<  QString::fromStdString(mpCoreComponentSystem->getName()) << "  Subname: " << name;
    return mpCoreComponentSystem->getSubComponentSystem(name.toStdString());
}

CoreSystemAccess::~CoreSystemAccess()
{
    //Dont remove the mpCoreComponentSystem here you must do that manually until we have found a samrter way to do all of this
    //see deleteRootSystemPtr()
    //delete mpCoreComponentSystem;
}

//! @todo This is very strange, needed becouse core systems are deleted from parent if they are subsystems (not if root systems), this is the only way to safely delete the ore object
void CoreSystemAccess::deleteRootSystemPtr()
{
    delete mpCoreComponentSystem;
}

bool CoreSystemAccess::connect(QString compname1, QString portname1, QString compname2, QString portname2)
{
    //*****Core Interaction*****
    return mpCoreComponentSystem->connect(compname1.toStdString(), portname1.toStdString(), compname2.toStdString(), portname2.toStdString());
    //**************************
}

bool CoreSystemAccess::disconnect(QString compname1, QString portname1, QString compname2, QString portname2)
{
    //*****Core Interaction*****
    return mpCoreComponentSystem->disconnect(compname1.toStdString(), portname1.toStdString(), compname2.toStdString(), portname2.toStdString());
    //**************************
}

QString CoreSystemAccess::getHopsanCoreVersion()
{
    return QString::fromStdString(hopsan::HopsanEssentials::getInstance()->getCoreVersion());
}

void CoreSystemAccess::setDesiredTimeStep(double timestep)
{
    mpCoreComponentSystem->setDesiredTimestep(timestep);
}


void CoreSystemAccess::setInheritTimeStep(bool inherit)
{
    mpCoreComponentSystem->setInheritTimestep(inherit);
}


bool CoreSystemAccess::doesInheritTimeStep()
{
    return mpCoreComponentSystem->doesInheritTimestep();
}


double CoreSystemAccess::getDesiredTimeStep()
{
    return mpCoreComponentSystem->getDesiredTimeStep();
}


QString CoreSystemAccess::getRootSystemTypeCQS()
{
    //qDebug() << "getRootTypeCQS: " << componentName;
    return QString::fromStdString(mpCoreComponentSystem->getTypeCQSString());
}

QString CoreSystemAccess::getSubComponentTypeCQS(QString componentName)
{
    //qDebug() << "getSubComponentTypeCQS: " << componentName << " in " << QString::fromStdString(mpCoreComponentSystem->getName());
    QString ans = QString::fromStdString(mpCoreComponentSystem->getSubComponent(componentName.toStdString())->getTypeCQSString());
    //qDebug() << "cqs answer: " << ans;
    return ans;
}


QString CoreSystemAccess::setRootSystemName(QString name)
{
    //qDebug() << "setting root system name to: " << name;
    mpCoreComponentSystem->setName(name.toStdString());
    //qDebug() << "root system name after rename: " << QString::fromStdString(mpCoreComponentSystem->getName());
    return QString::fromStdString(mpCoreComponentSystem->getName());
}


QString CoreSystemAccess::renameSubComponent(QString componentName, QString name)
{
    qDebug() << "rename subcomponent from " << componentName << " to: " << name;
    hopsan::Component *pTempComponent = mpCoreComponentSystem->getSubComponent(componentName.toStdString());
    pTempComponent->setName(name.toStdString());
    qDebug() << "name after: " << QString::fromStdString(pTempComponent->getName());
    return QString::fromStdString(pTempComponent->getName());
}

QString CoreSystemAccess::getRootSystemName()
{
   // qDebug() << "getNAme from core root: " << QString::fromStdString(mpCoreComponentSystem->getName());
    return QString::fromStdString(mpCoreComponentSystem->getName());
}

double CoreSystemAccess::getCurrentTime()
{
    return *(mpCoreComponentSystem->getTimePtr());
}

void CoreSystemAccess::stop()
{
    mpCoreComponentSystem->stopSimulation();
}


void CoreSystemAccess::simulateAllOpenModels(double mStartTime, double mFinishTime, simulationMethod type, size_t nThreads, bool modelsHaveNotChanged)
{
    std::vector<hopsan::ComponentSystem *> systemVector;
    for(int i=0; i<gpMainWindow->mpProjectTabs->count(); ++i)
    {
        systemVector.push_back(gpMainWindow->mpProjectTabs->getSystem(i)->getCoreSystemAccessPtr()->getCoreSystemPtr());
    }

    if(type == MULTICORE)
    {
        systemVector.at(0)->simulateMultipleSystemsMultiThreaded(mStartTime, mFinishTime, nThreads, systemVector, modelsHaveNotChanged);
    }
    else
    {
        systemVector.at(0)->simulateMultipleSystems(mStartTime, mFinishTime, systemVector);
    }
}


QString CoreSystemAccess::getPortType(const QString componentName, const QString portName, const PortTypeIndicatorT portTypeIndicator)
{
    //qDebug() << "name for port fetch " << componentName << " " << portName;

    hopsan::Port *pPort = this->getCorePortPtr(componentName, portName);
    if(pPort)
    {
        switch (portTypeIndicator)
        {
        case INTERNALPORTTYPE:
            return QString::fromStdString( portTypeToString(pPort->getInternalPortType()) );
            break;
        case ACTUALPORTTYPE:
            return QString::fromStdString( portTypeToString(pPort->getPortType()) );
            break;
        case EXTERNALPORTTYPE:
            return QString::fromStdString( portTypeToString(pPort->getExternalPortType()) );
            break;
        default:
            return QString("Invalid  portTypeIndicator specified");
        }
    }
    else
    {
        qDebug() <<  "======== ERROR ========= Could not find Port in getPortType: " << componentName << " " << portName << " in: " << this->getRootSystemName();
        return QString(); //Empty
    }
}

QString CoreSystemAccess::getNodeType(QString componentName, QString portName)
{
    hopsan::Port *pPort = this->getCorePortPtr(componentName, portName);
    if(pPort)
    {
        return QString(pPort->getNodeType().c_str());
    }
    else
    {
        qDebug() <<  "======================================== EMPTY nodetype: " << componentName << " " << portName << " in: " << this->getRootSystemName();
        return QString(); //Empty
    }
}


void CoreSystemAccess::getStartValueDataNamesValuesAndUnits(QString componentName, QString portName, QVector<QString> &rNames, QVector<double> &rValues, QVector<QString> &rUnits)
{
    std::vector<std::string> stdNames, stdUnits;
    std::vector<double> stdValues;
    hopsan::Port *pPort = this->getCorePortPtr(componentName, portName);
    if(pPort)
    {
        pPort->getStartValueDataNamesValuesAndUnits(stdNames, stdValues, stdUnits);
    }
    rNames.resize(stdNames.size());
    rValues.resize(stdValues.size());
    rUnits.resize(stdUnits.size());
    for(size_t i=0; i < stdNames.size(); ++i) //! @todo Make a nicer conversion fron std::vector<std::string> --> QVector<QString>
    {
        rNames[i] = QString::fromStdString(stdNames[i]);
        rValues[i] = stdValues[i];
        rUnits[i] = QString::fromStdString(stdUnits[i]);
    }
}


void CoreSystemAccess::getStartValueDataNamesValuesAndUnits(QString componentName, QString portName, QVector<QString> &rNames, QVector<QString> &rValuesTxt, QVector<QString> &rUnits)
{
    std::vector<std::string> stdNames, stdUnits;
    std::vector<std::string> stdValuesTxt;
    hopsan::Port *pPort = this->getCorePortPtr(componentName, portName);
    if(pPort)
    {
        pPort->getStartValueDataNamesValuesAndUnits(stdNames, stdValuesTxt, stdUnits);
    }
    rNames.resize(stdNames.size());
    rValuesTxt.resize(stdValuesTxt.size());
    rUnits.resize(stdUnits.size());
    for(size_t i=0; i < stdNames.size(); ++i) //! @todo Make a nicer conversion fron std::vector<std::string> --> QVector<QString>
    {
        rNames[i] = QString::fromStdString(stdNames[i]);
        rValuesTxt[i] = QString::fromStdString(stdValuesTxt[i]);
        rUnits[i] = QString::fromStdString(stdUnits[i]);
    }
}


bool CoreSystemAccess::setParameterValue(QString componentName, QString parameterName, QString value, bool force)
{
    return mpCoreComponentSystem->getSubComponent(componentName.toStdString())->setParameterValue(parameterName.toStdString(), value.toStdString(), force);
}


void CoreSystemAccess::removeSubComponent(QString componentName, bool doDelete)
{
    mpCoreComponentSystem->removeSubComponent(componentName.toStdString(), doDelete);
}


vector<double> CoreSystemAccess::getTimeVector(QString componentName, QString portName)
{
    //qDebug() << "getTimeVector, " << componentName << ", " << portName;

    hopsan::Component* pComp = mpCoreComponentSystem->getSubComponentOrThisIfSysPort(componentName.toStdString());
    hopsan::Port* pPort = 0;
    if (pComp != 0)
    {
        pPort = pComp->getPort(portName.toStdString());
    }

    if (pPort != 0)
    {
        vector<double> *ptr = pPort->getTimeVectorPtr();
        if (ptr != 0)
        {
            return *ptr; //Return a copy of the vector
        }
    }

    //Return empty dummy
    vector<double> dummy;
    return dummy;
}


bool CoreSystemAccess::doesKeepStartValues()
{
    return mpCoreComponentSystem->doesKeepStartValues();
}


void CoreSystemAccess::setLoadStartValues(bool load)
{
    mpCoreComponentSystem->setLoadStartValues(load);
}


bool CoreSystemAccess::isSimulationOk()
{
    return mpCoreComponentSystem->isSimulationOk();
}


bool CoreSystemAccess::initialize(double mStartTime, double mFinishTime, int nSamples)
{
    //! @todo write get set wrappers for n log samples, and use only value in core instead of duplicate in gui
    mpCoreComponentSystem->setNumLogSamples(nSamples);
    return mpCoreComponentSystem->initialize(mStartTime, mFinishTime);
}


void CoreSystemAccess::simulate(double mStartTime, double mFinishTime, simulationMethod type, size_t nThreads, bool modelHasNotChanged)
{
    //qDebug() << "simulate(), nThreads = " << nThreads;

    if(type == MULTICORE)
    {
        qDebug() << "Starting multicore simulation";
        mpCoreComponentSystem->simulateMultiThreaded(mStartTime, mFinishTime, nThreads, modelHasNotChanged);
        qDebug() << "Finished multicore simulation";
        //mpCoreComponentSystem->simulateMultiThreadedOld(mStartTime, mFinishTime);
    }
    else
    {
        //qDebug() << "Starting singlecore simulation";
        mpCoreComponentSystem->simulate(mStartTime, mFinishTime);
    }
}


void CoreSystemAccess::finalize()
{
    mpCoreComponentSystem->finalize();
}

QString CoreSystemAccess::createComponent(QString type, QString name)
{
    //qDebug() << "createComponent: " << "type: " << type << " desired name:  " << name << " in system: " << this->getRootSystemName();
    hopsan::Component *pCoreComponent = hopsan::HopsanEssentials::getInstance()->createComponent(type.toStdString());
    if (pCoreComponent != 0)
    {
        mpCoreComponentSystem->addComponent(pCoreComponent);
        if (!name.isEmpty())
        {
            pCoreComponent->setName(name.toStdString());
        }
        //qDebug() << "createComponent: name after add: " << QString::fromStdString(pCoreComponent->getName()) << " added to: " << QString::fromStdString(mpCoreComponentSystem->getName());
        return QString::fromStdString(pCoreComponent->getName());
    }
    else
    {
        qDebug() << "failed to create component of type: " << type << " maybe it is not registered in the core";
        return QString();
    }
}

QString CoreSystemAccess::createSubSystem(QString name)
{
    hopsan::ComponentSystem *pTempComponentSystem = hopsan::HopsanEssentials::getInstance()->createComponentSystem();
    mpCoreComponentSystem->addComponent(pTempComponentSystem);
    if (!name.isEmpty())
    {
        pTempComponentSystem->setName(name.toStdString());
    }
    return QString::fromStdString(pTempComponentSystem->getName());
}

void CoreSystemAccess::getParameters(QString componentName, QVector<CoreParameterData> &rParameterDataVec)
{
    rParameterDataVec.clear();
    hopsan::Component* pComp =  mpCoreComponentSystem->getSubComponent(componentName.toStdString());
    if (pComp!=0)
    {
        const std::vector<hopsan::Parameter*> *pParams = pComp->getParametersVectorPtr();

        rParameterDataVec.resize(pParams->size()); //preAllocate storage
        for(size_t i=0; i<pParams->size(); ++i)
        {
            CoreParameterData data;
            copyParameterData(pParams->at(i), data);
            rParameterDataVec[i] = data;
        }
    }
}

void CoreSystemAccess::getParameter(QString componentName, QString parameterName, CoreParameterData &rData)
{
    hopsan::Component* pComp =  mpCoreComponentSystem->getSubComponent(componentName.toStdString());
    if (pComp!=0)
    {
        const hopsan::Parameter *pParam = pComp->getParameter(parameterName.toStdString());
        if (pParam!=0)
        {
            copyParameterData(pParam, rData);
        }
    }
}

//! @deprecated
//void CoreSystemAccess::getParameters(QString componentName, QVector<QString> &qParameterNames, QVector<QString> &qParameterValues, QVector<QString> &qDescriptions, QVector<QString> &qUnits, QVector<QString> &qTypes)
//{
//    std::vector<std::string> parameterNames, parameterValues, descriptions, units, types;
//    //! @todo should check that component found before atempting to get parameter
//    mpCoreComponentSystem->getSubComponent(componentName.toStdString())->getParameters(parameterNames, parameterValues, descriptions, units, types);
//    for(size_t i=0; i<parameterNames.size(); ++i)
//    {
//        qParameterNames.push_back(QString::fromStdString(parameterNames[i]));
//        qParameterValues.push_back(QString::fromStdString(parameterValues[i]));
//        qDescriptions.push_back(QString::fromStdString(descriptions[i]));
//        qUnits.push_back(QString::fromStdString(units[i]));
//        qTypes.push_back(QString::fromStdString(types[i]));
//    }
//}

QStringList CoreSystemAccess::getParameterNames(QString componentName)
{
    QStringList qParameterNames;
    std::vector<std::string> parameterNames;
    hopsan::Component* pComp =  mpCoreComponentSystem->getSubComponent(componentName.toStdString());
    if (pComp!=0)
    {
        pComp->getParameterNames(parameterNames);
        for(size_t i=0; i<parameterNames.size(); ++i)
        {
            qParameterNames.push_back(QString::fromStdString(parameterNames[i]));
        }
    }

    return qParameterNames;
}

QStringList CoreSystemAccess::getSystemParameterNames()
{
    std::vector<std::string> parameterNames;
    mpCoreComponentSystem->getParameterNames(parameterNames);
    QStringList qParameterNames;
    for(size_t i=0; i<parameterNames.size(); ++i)
    {
        qParameterNames.push_back(QString::fromStdString(parameterNames[i]));
    }
    return qParameterNames;
}

//QString CoreSystemAccess::getParameterUnit(QString /*componentName*/, QString /*parameterName*/)
//{
//    return QString("");
//}

//QString CoreSystemAccess::getParameterDescription(QString /*componentName*/, QString /*parameterName*/)
//{
//    return QString("");
//}

QString CoreSystemAccess::getParameterValue(QString componentName, QString parameterName)
{
    std::string parameterValue="";
    hopsan::Component* pComp = mpCoreComponentSystem->getSubComponent(componentName.toStdString());
    if (pComp != 0)
    {
        pComp->getParameterValue(parameterName.toStdString(), parameterValue);
    }

    return QString::fromStdString(parameterValue);
}

void CoreSystemAccess::deleteSystemPort(QString portname)
{
    mpCoreComponentSystem->deleteSystemPort(portname.toStdString());
}

QString CoreSystemAccess::addSystemPort(QString portname)
{
    //qDebug() << "add system port: " << portname;
    return QString::fromStdString(mpCoreComponentSystem->addSystemPort(portname.toStdString())->getPortName());
}

QString CoreSystemAccess::renameSystemPort(QString oldname, QString newname)
{
    return QString::fromStdString(mpCoreComponentSystem->renameSystemPort(oldname.toStdString(), newname.toStdString()));
}

QString CoreSystemAccess::reserveUniqueName(QString desiredName)
{
    return QString::fromStdString(mpCoreComponentSystem->reserveUniqueName(desiredName.toStdString()));
}

void CoreSystemAccess::unReserveUniqueName(QString name)
{
    mpCoreComponentSystem->unReserveUniqueName(name.toStdString());
}

QString CoreSystemAccess::getPlotDataUnit(const QString compname, const QString portname, const QString dataname)
{
    std::string dummy, unit;
    hopsan::Port* pPort = this->getCorePortPtr(compname, portname);
    if(pPort)
    {
        int idx = pPort->getNodeDataIdFromName(dataname.toStdString());
        if(idx >= 0)
            pPort->getNodeDataNameAndUnit(idx,dummy,unit);
    }
    return QString::fromStdString(unit);
}

//! @todo how to handle fetching from systemports, component names will not be found
void CoreSystemAccess::getPlotDataNamesAndUnits(const QString compname, const QString portname, QVector<QString> &rNames, QVector<QString> &rUnits)
{
    vector<string> corenames, coreunits;
    rNames.clear();
    rUnits.clear();

    hopsan::Port* pPort = this->getCorePortPtr(compname, portname);
    if (pPort && pPort->getPortType() < hopsan::MULTIPORT)
    {
        pPort->getNodeDataNamesAndUnits(corenames, coreunits);
        //Copy into QT datatype vector (assumes bothe received vectors same length (should always be same)
        for (size_t i=0; i<corenames.size(); ++i)
        {
            rNames.push_back(QString::fromStdString(corenames[i]));
            rUnits.push_back(QString::fromStdString(coreunits[i]));
        }
    }
}

void CoreSystemAccess::getPlotData(const QString compname, const QString portname, const QString dataname, QPair<QVector<double>, QVector<double> > &rData)
{
    int dataId = -1;
    hopsan::Port* pPort = this->getCorePortPtr(compname, portname);
    if (pPort)
    {
        if(pPort->isConnected())
        {
            dataId = pPort->getNodeDataIdFromName(dataname.toStdString());
            if (dataId >= 0)
            {
                vector< vector<double> > *pData = pPort->getDataVectorPtr();
                vector<double> *pTime = pPort->getTimeVectorPtr();

                //Ok lets copy all of the data to a Qt vector
                rData.first.clear();
                rData.first.resize(pTime->size());    //Allocate memory for time
                rData.second.clear();           //Allocate memory for data
                rData.second.resize(pData->size());
                for (size_t i=0; i<pData->size() && i<pTime->size(); ++i)
                {
                    rData.first[i] = pTime->at(i);
                    rData.second[i] = pData->at(i).at(dataId);
                }
            }
        }
    }
}

bool CoreSystemAccess::havePlotData(const QString compname, const QString portname, const QString dataname)
{
    hopsan::Port* pPort = this->getCorePortPtr(compname, portname);
    if (pPort)
    {
        if(pPort->isConnected())
        {
            int dataId = -1;
            dataId = pPort->getNodeDataIdFromName(dataname.toStdString());

            if (pPort->getDataVectorPtr()->empty() || pPort->getTimeVectorPtr()->empty())
            {
                return false;
            }
            else
            {
                return true;
            }
        }
    }
    return false;
}


bool CoreSystemAccess::getLastNodeData(const QString compname, const QString portname, const QString dataname, double& rData)
{
    int dataId = -1;
    hopsan::Port* pPort = this->getCorePortPtr(compname, portname);
    if (pPort)
    {
        dataId = pPort->getNodeDataIdFromName(dataname.toStdString());

        if (dataId >= 0)
        {
            vector<double> *pData = pPort->getJustTheDataVectorPtr();
            rData = pData->at(dataId);
            return 1;
        }
    }
    return 0;
}


bool CoreSystemAccess::isPortConnected(QString componentName, QString portName)
{
    hopsan::Port* pPort = this->getCorePortPtr(componentName, portName);
    if(pPort)
    {
        return pPort->isConnected();
    }
    else
    {
        return false;
    }
}


bool CoreSystemAccess::writeNodeData(const QString compname, const QString portname, const QString dataname, double data)
{
    hopsan::Port* pPort = getCorePortPtr(compname,portname);
    int dataId = -1;
    if(pPort)
    {
        dataId = pPort->getNodeDataIdFromName(dataname.toStdString());

        if(dataId >= 0)
        {
            pPort->writeNode(dataId, data);
            return 1;
        }
    }
    return 0;
}


//! @brief Helpfunction that tries to fetch a port pointer
//! @param [in] componentName The name of the component to which the port belongs
//! @param [in] portName The name of the port
//! @returns A pointer to the port or a 0 ptr if component or port not found
hopsan::Port* CoreSystemAccess::getCorePortPtr(QString componentName, QString portName)
{
    //We must use getcomponent here if we want to be able to find root system ptr
    //! @todo see if we can reduce the number f public get functions one, the one which only searches subcomponents make function in core to solve the other access type like bellow
    hopsan::Component* pComp = mpCoreComponentSystem->getSubComponentOrThisIfSysPort(componentName.toStdString());
    if (pComp)
    {
        return pComp->getPort(portName.toStdString());
    }
    return 0;
}


bool CoreSystemAccess::setSystemParameter(const CoreParameterData &rParameter, bool force)
{
    return mpCoreComponentSystem->setSystemParameter(rParameter.mName.toStdString(),
                                                     rParameter.mValue.toStdString(),
                                                     rParameter.mType.toStdString(),
                                                     rParameter.mDescription.toStdString(),
                                                     rParameter.mUnit.toStdString(),
                                                     force);
}


bool CoreSystemAccess::setSystemParameterValue(QString name, QString value, bool force)
{
    return mpCoreComponentSystem->setParameterValue(name.toStdString(), value.toStdString(), force);
}

//! @brief Get the value of a parameter in the system
//! @returns The aprameter value as a QString or "" if parameter not found
QString CoreSystemAccess::getSystemParameterValue(const QString name)
{
    std::string value;
    mpCoreComponentSystem->getParameterValue(name.toStdString(), value);
    return QString::fromStdString(value);
}


//! @todo Dont know if this is actually used
bool CoreSystemAccess::hasSystemParameter(const QString name)
{
    return mpCoreComponentSystem->hasParameter(name.toStdString());
}

//! @brief Rename a system parameter
bool CoreSystemAccess::renameSystemParameter(const QString oldName, const QString newName)
{
    return mpCoreComponentSystem->renameParameter(oldName.toStdString(), newName.toStdString());
}

//! @brief Removes the parameter with given name
//! @param [in] name The name of the system parameter to remove
void CoreSystemAccess::removeSystemParameter(const QString name)
{
    mpCoreComponentSystem->unRegisterParameter(name.toStdString());
}

void CoreSystemAccess::getSystemParameter(const QString name, CoreParameterData &rParameterData)
{
    const hopsan::Parameter *pParam = mpCoreComponentSystem->getParameter(name.toStdString());
    if (pParam!=0)
    {
        copyParameterData(pParam, rParameterData);
    }
}


void CoreSystemAccess::getSystemParameters(QVector<CoreParameterData> &rParameterDataVec)
{
    rParameterDataVec.clear();
    const std::vector<hopsan::Parameter*> *pParams = mpCoreComponentSystem->getParametersVectorPtr();
    rParameterDataVec.resize(pParams->size()); //preAllocate storage
    for(size_t i=0; i<pParams->size(); ++i)
    {
        CoreParameterData data;
        copyParameterData(pParams->at(i), data);
        rParameterDataVec[i] = data;
    }
}

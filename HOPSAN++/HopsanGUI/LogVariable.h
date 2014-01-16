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
//! @file   LogDataHandler.h
//! @author Flumes <flumes@lists.iei.liu.se>
//! @date   2012-12-18
//!
//! @brief Contains the LogData classes
//!
//$Id$

#ifndef LOGVARIABLE_H
#define LOGVARIABLE_H

#include <QSharedPointer>
#include <QVector>
#include <QString>
#include <QColor>
#include <QObject>
#include <QPointer>
#include <QTextStream>

#include "CachableDataVector.h"
#include "common.h"
#include "UnitScale.h"

#define TIMEVARIABLENAME "Time"
#define FREQUENCYVARIABLENAME "Frequency"

// Forward declaration
class VectorVariable;
class LogDataHandler;

QString makeConcatName(const QString componentName, const QString portName, const QString dataName);
void splitConcatName(const QString fullName, QString &rCompName, QString &rPortName, QString &rVarName);

//! @brief This enum describes where a variable come from, the order signifies importance (ModelVariables most important)
enum VariableSourceTypeT {ModelVariableType, ImportedVariableType, ScriptVariableType, TempVariableType, UndefinedVariableSourceType};
QString getVariableSourceTypeAsString(const VariableSourceTypeT type);

//! @brief This enum describes the variable type
enum VariableTypeT {VectorType, TimeDomainType, FrequencyDomainType, RealFrequencyDomainType, ImaginaryFrequencyDomainType, AmplitudeFrequencyDomainType, PhaseFrequencyDomainType, ComplexType, UndefinedVariableType};

//! @class VariableCommonDescription
//! @brief Container class for strings describing a log variable (common data for all generations)
class VariableDescription
{
public:
    VariableDescription() : mVariableSourceType(UndefinedVariableSourceType) {}
    QString mModelPath;
    QString mComponentName;
    QString mPortName;
    QString mDataName;
    QString mDataUnit;
    QString mDataDescription;
    QString mAliasName;
    VariableSourceTypeT mVariableSourceType;

    QString getFullName() const;
    QString getFullNameWithSeparator(const QString sep) const;
    void setFullName(const QString compName, const QString portName, const QString dataName);

    bool operator==(const VariableDescription &other) const;
};


typedef QSharedPointer<VariableDescription> SharedVariableDescriptionT;
typedef QSharedPointer<VectorVariable> SharedVariablePtrT;

SharedVariableDescriptionT createTimeVariableDescription();
SharedVariableDescriptionT createFrequencyVariableDescription();
SharedVariablePtrT createFreeVectorVariable(const QVector<double> &rData, SharedVariableDescriptionT pVarDesc);
SharedVariablePtrT createFreeTimeVectorVariabel(const QVector<double> &rTime);
SharedVariablePtrT createFreeFrequencyVectorVariabel(const QVector<double> &rFrequency);
SharedVariablePtrT createFreeVariable(VariableTypeT type, SharedVariableDescriptionT pVarDesc);

class IndexIntervalCollection
{
public:
    class MinMaxT
    {
    public:
        MinMaxT(int min, int max);
        int mMin, mMax;
    };

    void addValue(const int val);
    void removeValue(const int val);
    int min() const;
    int max() const;
    bool isContinuos() const;
    bool isEmpty() const;
    bool contains(const int val) const;
    void clear();
    int getNumAddedValues() const;

    QList<MinMaxT> getList() const;
    QList<int> getCompleteList() const;

private:
    QList<MinMaxT> mIntervalList;
};

class LogVariableContainer : public QObject
{
    Q_OBJECT
public:
    typedef QMap<int, SharedVariablePtrT> GenerationMapT;

    LogVariableContainer(const QString &rName, LogDataHandler *pParentLogDataHandler);
    ~LogVariableContainer();

    const QString &getName() const;

    void addDataGeneration(const int generation, SharedVariablePtrT pData);
    bool removeDataGeneration(const int generation, const bool force=false);
    void removeAllGenerations();
    bool removeAllImportedGenerations();
    bool purgeOldGenerations(const int purgeEnd, const int nGensToKeep);

    SharedVariablePtrT getDataGeneration(const int gen=-1) const;
    QList<SharedVariablePtrT> getAllDataGenerations() const;
    bool hasDataGeneration(const int gen);
    int getLowestGeneration() const;
    int getHighestGeneration() const;
    int getNumGenerations() const;
    QList<int> getGenerations() const;

    bool isStoringAlias() const;
    bool isGenerationAlias(const int gen) const;
    bool isStoringImported() const;
    bool isGenerationImported(const int gen) const;

    void preventAutoRemove(const int gen);
    void allowAutoRemove(const int gen);

    LogDataHandler *getLogDataHandler();

signals:
    void logVariableBeingRemoved(SharedVariablePtrT);

private:
    void actuallyRemoveDataGen(GenerationMapT::iterator git);
    QString mName;
    LogDataHandler *mpParentLogDataHandler;
    GenerationMapT mDataGenerations;
    IndexIntervalCollection mAliasGenIndexes;
    IndexIntervalCollection mImportedGenIndexes;
    QList<int> mKeepGenerations;
};


class VectorVariable : public QObject
{
    Q_OBJECT
    friend class LogVariableContainer;
    friend class LogDataHandler;

public:
    VectorVariable(const QVector<double> &rData, const int generation, SharedVariableDescriptionT varDesc,
                   SharedMultiDataVectorCacheT pGenerationMultiCache);
    ~VectorVariable();

    // Access variable type enums
    virtual VariableSourceTypeT getVariableSourceType() const;
    virtual VariableTypeT getVariableType() const;

    // Functions that read the data metadata
    const SharedVariableDescriptionT getVariableDescription() const;
    const QString &getAliasName() const;
    QString getFullVariableName() const;
    QString getFullVariableNameWithSeparator(const QString sep) const;
    QString getSmartName() const;
    const QString &getModelPath() const;
    const QString &getComponentName() const;
    const QString &getPortName() const;
    const QString &getDataName() const;
    const QString &getDataUnit() const;
    bool hasAliasName() const;
    int getGeneration() const;
    int getLowestGeneration() const;
    int getHighestGeneration() const;
    int getNumGenerations() const;
    virtual bool isImported() const;
    virtual QString getImportedFileName() const;

    // Data plot scaling
    void setCustomUnitScale(const UnitScale &rUnitScale);
    void removeCustomUnitScale();
    const UnitScale &getCustomUnitScale() const;
    const QString &getPlotScaleDataUnit() const;
    const QString &getActualPlotDataUnit() const;
    double getPlotScale() const;
    double getPlotOffset() const;

    // Functions that only read data
    int getDataSize() const;
    QVector<double> getDataVectorCopy() const;
    double first() const;
    double last() const;
    bool indexInRange(const int idx) const;
    double peekData(const int index, QString &rErr) const;
    double peekData(const int idx) const;
    double averageOfData() const;
    double minOfData(int &rIdx) const;
    double minOfData() const;
    double maxOfData(int &rIdx) const;
    double maxOfData() const;
    void elementWiseGt(QVector<double> &rResult, const double threshold) const;
    void elementWiseLt(QVector<double> &rResult, const double threshold) const;

    // Check out and return pointers to data (move to ram if necessary)
    QVector<double> *beginFullVectorOperation();
    bool endFullVectorOperation(QVector<double> *&rpData);

    // Functions that only read data but that require reimplementation in derived classes
    virtual const SharedVariablePtrT getSharedTimeOrFrequencyVector() const;
    virtual SharedVariablePtrT toFrequencySpectrum(const SharedVariablePtrT pTime, const bool doPowerSpectrum);

    // Functions that modify the data
    void assignFrom(const QVector<double> &rSrc);
    void assignFrom(const double src);
    void addToData(const SharedVariablePtrT pOther);
    void addToData(const double other);
    void subFromData(const SharedVariablePtrT pOther);
    void subFromData(const double other);
    void multData(const SharedVariablePtrT pOther);
    void multData(const double other);
    void divData(const SharedVariablePtrT pOther);
    void divData(const double other);
    void absData();
    double pokeData(const int index, const double value, QString &rErr);
    void append(const double y);

    // Functions that modify data, but that may require reimplementation in derived classes
    virtual void assignFrom(const SharedVariablePtrT pOther);
    virtual void assignFrom(SharedVariablePtrT time, const QVector<double> &rData);
    virtual void assignFrom(QVector<double> &rTime, QVector<double> &rData);
    virtual void append(const double t, const double y);
    virtual void diffBy(SharedVariablePtrT pOther);
    virtual void integrateBy(SharedVariablePtrT pOther);
    virtual void lowPassFilter(SharedVariablePtrT pTime, const double w);


    // Functions to toggle "keep" generation
    void preventAutoRemoval();
    void allowAutoRemoval();

    // Handle disk caching and data streaming
    void setCacheDataToDisk(const bool toDisk);
    bool isCachingDataToDisk() const;
    void sendDataToStream(QTextStream &rStream, QString separator);

    // Access to parent object pointers
    LogVariableContainer *getLogVariableContainer();
    LogDataHandler *getLogDataHandler();

public slots:
    void setPlotScale(double scale);
    void setPlotOffset(double offset);
    void setPlotScaleAndOffset(const double scale, const double offset);

    // Slots that require reimplementation in derived classes
    virtual void setTimePlotScaleAndOffset(const double scale, const double offset);
    virtual void setTimePlotScale(double scale);
    virtual void setTimePlotOffset(double offset);

signals:
    void dataChanged();
    void nameChanged();

protected:
    typedef QVector<double> DataVectorT;
    QPointer<LogVariableContainer> mpParentVariableContainer;

    CachableDataVector *mpCachedDataVector;
    SharedVariableDescriptionT mpVariableDescription;
    SharedVariablePtrT mpSharedTimeOrFrequencyVector;

    UnitScale mCustomUnitScale;
    double mDataPlotScale;
    double mDataPlotOffset;
    int mGeneration;

private:
    QVector<double> *pTempCheckoutData;
};

class ImportedVariableBase
{
public:
    virtual bool isImported() const;
    virtual QString getImportedFileName() const;
protected:
    QString mImportFileName;

};

class ImportedVectorVariable : public VectorVariable, public ImportedVariableBase
{
    Q_OBJECT
public:
    ImportedVectorVariable(const QVector<double> &rData, const int generation, SharedVariableDescriptionT varDesc, const QString &rImportFile,
                           SharedMultiDataVectorCacheT pGenerationMultiCache);
};

class TimeDomainVariable : public VectorVariable
{
    Q_OBJECT
public:
    TimeDomainVariable(SharedVariablePtrT time, const QVector<double> &rData, const int generation, SharedVariableDescriptionT varDesc,
                       SharedMultiDataVectorCacheT pGenerationMultiCache);

    virtual VariableTypeT getVariableType() const;

    void diffBy(SharedVariablePtrT pOther);
    void integrateBy(SharedVariablePtrT pOther);
    void lowPassFilter(SharedVariablePtrT pTime, const double w);
    SharedVariablePtrT toFrequencySpectrum(const SharedVariablePtrT pTime, const bool doPowerSpectrum);
    void assignFrom(const SharedVariablePtrT pOther);
    virtual void assignFrom(SharedVariablePtrT time, const QVector<double> &rData);
    virtual void assignFrom(QVector<double> &rTime, QVector<double> &rData);
    virtual void append(const double t, const double y);

public slots:
    void setTimePlotScaleAndOffset(const double scale, const double offset);
    void setTimePlotScale(double scale);
    void setTimePlotOffset(double offset);
};

class ImportedTimeDomainVariable : public TimeDomainVariable, public ImportedVariableBase
{
    Q_OBJECT
public:
    ImportedTimeDomainVariable(SharedVariablePtrT time, const QVector<double> &rData, const int generation, SharedVariableDescriptionT varDesc,
                               const QString &rImportFile, SharedMultiDataVectorCacheT pGenerationMultiCache);
};

class FrequencyDomainVariable : public VectorVariable
{
    Q_OBJECT
public:
    FrequencyDomainVariable(SharedVariablePtrT frequency, const QVector<double> &rData, const int generation, SharedVariableDescriptionT varDesc,
                            SharedMultiDataVectorCacheT pGenerationMultiCache);
    //! @todo add a bunch of reimplemented functions
};


class ComplexVectorVariable : public VectorVariable
{
    Q_OBJECT
public:
    ComplexVectorVariable(const QVector<double> &rReal, const QVector<double> &rImaginary, const int generation, SharedVariableDescriptionT varDesc,
                          SharedMultiDataVectorCacheT pGenerationMultiCache);
    ComplexVectorVariable(SharedVariablePtrT pReal, SharedVariablePtrT pImaginary, const int generation, SharedVariableDescriptionT varDesc);
    virtual VariableTypeT getVariableType() const;
    //! @todo add a bunch of reimplemented functions
protected:
    CachableDataVector *mpCachedRealVector, *mpCachedImagVector;
    SharedVariablePtrT mpSharedReal, mpSharedImag;
};

void createBode(const SharedVariablePtrT pInput, const SharedVariablePtrT pOutput, int Fmax,
                SharedVariablePtrT &rNyquistData, SharedVariablePtrT &rNyquistDataInv,
                SharedVariablePtrT &rGainData, SharedVariablePtrT &rPhaseData);

#endif // LOGVARIABLE_H

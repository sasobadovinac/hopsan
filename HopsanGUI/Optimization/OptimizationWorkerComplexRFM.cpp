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
//! @file   OptimizationWorkerComplexRFM.h
//! @author Johan Persson <johan.persson@liu.se>
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2014-02-13
//! @version $Id$
//!
//! @brief Contains an optimization worker object for the Complex-RFM algorithm
//!

//Hopsan includes
#include "Dialogs/OptimizationDialog.h"
#include "global.h"
#include "GUIObjects/GUIContainerObject.h"
#include "HcomHandler.h"
#include "OptimizationHandler.h"
#include "OptimizationWorkerComplexRFM.h"
#include "PlotHandler.h"
#include "PlotTab.h"
#include "PlotWindow.h"
#include "Widgets/HcomWidget.h"
#include "Widgets/ModelWidget.h"
#include "ComponentUtilities/matrix.h"
#include "ComponentUtilities/EquationSystemSolver.h"

//C++ includes
#include <math.h>

OptimizationWorkerComplexRFM::OptimizationWorkerComplexRFM(OptimizationHandler *pHandler)
    : OptimizationWorkerComplex(pHandler)
{
    mPercDiff = 0.002;
    mCountMax = 2;
    mUseMetaModel = true;
}


//! @brief Initializes a Complex-RF optimization
void OptimizationWorkerComplexRFM::init()
{
    OptimizationWorkerComplex::init();

    mUseMetaModel = true;
    mMetaModelExist = false;
    mLastWorstId = -1;
    mWorstCounter = 0;

    for(int p=0; p<mNumPoints; ++p)
    {
        mParameters[p].resize(mNumParameters);
        for(int i=0; i<mNumParameters; ++i)
        {
            double r = (double)rand() / (double)RAND_MAX;
            mParameters[p][i] = mParMin[i] + r*(mParMax[i]-mParMin[i]);
            if(mpHandler->mParameterType == OptimizationHandler::Integer)
            {
                mParameters[p][i] = round(mParameters[p][i]);
            }
        }
    }
    mObjectives.resize(mNumPoints);

    mKf = 1.0-pow(mAlpha/2.0, mGamma/mNumPoints);

    LogDataHandler2 *pHandler = mModelPtrs[0]->getViewContainerObject()->getLogDataHandler();
    // Check if exist at any generation first to avoid error message
    if (pHandler->hasVariable("WorstObjective"))
    {
        pHandler->removeVariable("WorstObjective", -1);
    }
    if (pHandler->hasVariable("BestObjective"))
    {
        pHandler->removeVariable("BestObjective", -1);
    }

    // Close these plotwindows before optimization to make sure old data is removed
    //! @todo should have define or const for this name "parplot"
    PlotWindow *pPlotWindow = gpPlotHandler->getPlotWindow("parplot");
    if(pPlotWindow)
    {
        PlotTab *pPlotTab = pPlotWindow->getCurrentPlotTab();
        if(pPlotTab)
        {
            while(!pPlotTab->getCurves().isEmpty())
            {
                pPlotTab->removeCurve(pPlotTab->getCurves().first());
            }
        }
    }

    //Calculate how many iterations to store for meta model
    mStorageSize = mNumParameters*mNumParameters/2.0+1.5*mNumParameters+1.0;
    mMetaModelCoefficients.create(mStorageSize);
    mStorageSize = int(mStorageSize*1.5);
    mStoredObjectives.create(mStorageSize);
    mStoredParameters.clear();
}



//! @brief Executes a Complex-RF optimization. optComplexInit() must be called before this one.
void OptimizationWorkerComplexRFM::run()
{
    //Plot optimization points
    plotPoints();

    mpHandler->mpHcomHandler->mpConsole->mpTerminal->setAbortButtonEnabled(true);

    //Reset convergence reason variable (0 = failed to converge)
    mConvergenceReason=0;

    //Verify that everything is ok
    if(!mpHandler->mpHcomHandler->hasFunction("evalall"))
    {
        printError("Function \"evalall\" not defined.","",false);
        return;
    }
    if(!mpHandler->mpHcomHandler->hasFunction("evalworst"))
    {
        printError("Function \"evalworst\" not defined.","",false);
        return;
    }

    print("Running optimization...", "", true);

    //Turn of terminal output during optimization
    execute("echo off -nonerrors");

    //Evaluate initial objective values
    execute("call evalall");
    logAllPoints();
    mEvaluations = mNumPoints;

    //Calculate best and worst id, and initialize last worst id
    calculateBestAndWorstId();
    mLastWorstId = mWorstId;

    //Store parameters for undo
    mParameters = mParameters;

    //Run optimization loop
    int i=0;
    mMetaModelExist = false;
    int metaModelCounter = 0;
    int timesWeHaveNotRunTheLastCode=10;
    for(; i<mMaxEvals && !mpHandler->mpHcomHandler->isAborted(); ++i)
    {
        if(!mMetaModelExist && mStoredParameters.size() == mStorageSize && mUseMetaModel)
        {
            createMetaModel();
            mMetaModelExist = true;
        }

        //Plot optimization points
        plotPoints();

        //Process UI events (required so that we don't lock up the program)
        qApp->processEvents();

        //Stop if user pressed abort button
        if(mpHandler->mpHcomHandler->isAborted())
        {
            print("Optimization aborted.");
            finalize();
            return;
        }

        //Print progress as percentage of maximum number of evaluations
        updateProgressBar(i);

        //Check convergence
        if(checkForConvergence()) break;

        //Increase all objective values (forgetting principle)
        forget();

        //Calculate best and worst point
        calculateBestAndWorstId();
        int wid = mWorstId;

        //Plot best and worst objective values
        plotObjectiveFunctionValues();

        //Find geometrical center
        findCenter();

        //Reflect worst point
        QVector<double> newPoint;
        newPoint.resize(mNumParameters);
        for(int j=0; j<mNumParameters; ++j)
        {
            //Reflect
            double worst = mParameters[wid][j];
            mParameters[wid][j] = mCenter[j] + (mCenter[j]-worst)*(mAlpha);

            //Add some random noise
            double maxDiff = getMaxParDiff();
            double r = (double)rand() / (double)RAND_MAX;
            mParameters[wid][j] = mParameters[wid][j] + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
            mParameters[wid][j] = qMin(mParameters[wid][j], mParMax[j]);
            mParameters[wid][j] = qMax(mParameters[wid][j], mParMin[j]);
        }
        newPoint = mParameters[wid]; //Remember the new point, in case we need to iterate below

        gpOptimizationDialog->updateParameterOutputs(mObjectives, mParameters, mBestId, mWorstId);

        //Evaluate new point
        if(mMetaModelExist && mUseMetaModel)
        {
            evaluateWithMetaModel();
            metaModelCounter++;
            logWorstPoint();
            ++mMetaModelEvaluations;
        }
        else
        {
            execute("call evalworst");
            logWorstPoint();
            ++mEvaluations;
            if(mpHandler->mpHcomHandler->getVar("ans") == -1)    //This check is needed if abort key is pressed while evaluating
            {
                execute("echo on");
                print("Optimization aborted.");
                finalize();
                return;
            }
            metaModelCounter=0;
            storeValuesForMetaModel(mWorstId);
        }

        //Calculate best and worst points
        mLastWorstId=wid;
        calculateBestAndWorstId();
        wid = mWorstId;

        //See if it is time to update meta model
        if(metaModelCounter > 0 && timesWeHaveNotRunTheLastCode >= mCountMax && mUseMetaModel)
        {
            metaModelCounter = 0;

            mWorstId = mLastWorstId;
            execute("call evalworst");
            logWorstPoint();
            ++mEvaluations;
            storeValuesForMetaModel(mWorstId);
            calculateBestAndWorstId();
            wid=mWorstId;
            if(mpHandler->mpHcomHandler->getVar("ans") == -1)    //This check is needed if abort key is pressed while evaluating
            {
                execute("echo on");
                print("Optimization aborted.");
                finalize();
                return;
            }

            hopsan::Vec oldMetaModelCoefficients = mMetaModelCoefficients;
            createMetaModel();

            double maxDiff = 0;
            for(int j=0; j<mMetaModelCoefficients.length(); ++j)
            {
                double diff = fabs((mMetaModelCoefficients[j]-oldMetaModelCoefficients[j])/mMetaModelCoefficients[j]);
                if(diff > maxDiff)
                {
                    maxDiff = diff;
                }
            }

            if(maxDiff < mPercDiff)
            {
                timesWeHaveNotRunTheLastCode=0;
            }
            else
            {
                timesWeHaveNotRunTheLastCode++;
            }
        }
        else
        {
            timesWeHaveNotRunTheLastCode++;
        }

        //Iterate until worst point is no longer the same
        mWorstCounter=0;
        while(mLastWorstId == wid)
        {
            plotPoints();

            qApp->processEvents();
            if(mpHandler->mpHcomHandler->isAborted())
            {
                execute("echo on");
                print("Optimization aborted.");
                finalize();
                mpHandler->mpHcomHandler->abortHCOM();
                return;
            }

            if(i>mMaxEvals) break;

            double a1 = 1.0-exp(-double(mWorstCounter)/5.0);

            //Reflect worst point
            for(int j=0; j<mNumParameters; ++j)
            {
                double best = mParameters[mBestId][j];
                double maxDiff = getMaxParDiff();
                double r = (double)rand() / (double)RAND_MAX;
                mParameters[wid][j] = (mCenter[j]*(1.0-a1) + best*a1 + newPoint[j])/2.0 + mRfak*(mParMax[j]-mParMin[j])*maxDiff*(r-0.5);
                mParameters[wid][j] = qMin(mParameters[wid][j], mParMax[j]);
                mParameters[wid][j] = qMax(mParameters[wid][j], mParMin[j]);
            }
            newPoint = mParameters[wid];
            gpOptimizationDialog->updateParameterOutputs(mObjectives, mParameters, mBestId, mWorstId);

            //Evaluate new point
            if(mMetaModelExist && mUseMetaModel)
            {
                evaluateWithMetaModel();
                metaModelCounter++;
                logWorstPoint();
                ++mMetaModelEvaluations;
            }
            else
            {
                execute("call evalworst");
                logWorstPoint();
                ++mEvaluations;
                if(mpHandler->mpHcomHandler->getVar("ans") == -1)    //This check is needed if abort key is pressed while evaluating
                {
                    execute("echo on");
                    print("Optimization aborted.");
                    finalize();
                    return;
                }
                metaModelCounter = 0;
                storeValuesForMetaModel(mWorstId);
            }

            if(!mMetaModelExist && mStoredParameters.size() == mStorageSize)
            {
                createMetaModel();
                mMetaModelExist = true;
            }

            //Calculate best and worst points
            mLastWorstId=wid;
            calculateBestAndWorstId();
            wid = mWorstId;

            ++mWorstCounter;
            ++i;
            execute("echo off -nonerrors");


            //See if it is time to update meta model
            if(metaModelCounter > 0 && timesWeHaveNotRunTheLastCode >= mCountMax && mUseMetaModel)
            {
                metaModelCounter = 0;

                mWorstId = mLastWorstId;
                execute("call evalworst");
                logWorstPoint();
                ++mEvaluations;
                storeValuesForMetaModel(mWorstId);
                calculateBestAndWorstId();
                wid=mWorstId;
                if(mpHandler->mpHcomHandler->getVar("ans") == -1)    //This check is needed if abort key is pressed while evaluating
                {
                    execute("echo on");
                    print("Optimization aborted.");
                    finalize();
                    return;
                }

                hopsan::Vec oldMetaModelCoefficients = mMetaModelCoefficients;
                createMetaModel();

                double maxDiff = 0;
                for(int j=0; j<mMetaModelCoefficients.length(); ++j)
                {
                    double diff = fabs((mMetaModelCoefficients[j]-oldMetaModelCoefficients[j])/mMetaModelCoefficients[j]);
                    if(diff > maxDiff)
                    {
                        maxDiff = diff;
                    }
                }

                if(maxDiff < mPercDiff)
                {
                    timesWeHaveNotRunTheLastCode=0;
                }
                else
                {
                    timesWeHaveNotRunTheLastCode++;
                }
            }
            else
            {
                timesWeHaveNotRunTheLastCode++;
            }


//            if(mWorstCounter >= 200)
//            {
//                execute("call evalall");
//                //mWorstCounter = 0;
////                mMetaModelExist = false;
////                mStoredParameters.clear();
//                mUseMetaModel = false;
////                for(int i=0; i<mNumPoints; ++i)
////                {
////                    mWorstId = i;
////                    evaluateWithMetaModel();
////                    logWorstPoint();
////                    ++mMetaModelEvaluations;
////                }
//                break;
//            }

            calculateBestAndWorstId();

            gpOptimizationDialog->updateParameterOutputs(mObjectives, mParameters, mBestId, mWorstId);

        }

        plotParameters();
        plotEntropy();
    }

    execute("echo on");

    switch(mConvergenceReason)
    {
    case 0:
        print("Optimization failed to converge after "+QString::number(i)+" iterations.");
        break;
    case 1:
        print("Optimization converged in function values after "+QString::number(i)+" iterations.");
        break;
    case 2:
        print("Optimization converged in parameter values after "+QString::number(i)+" iterations.");
        break;
    }

    print("\nBest point:");
    for(int i=0; i<mNumParameters; ++i)
    {
        if(mParNames.size() < i+1)
            print("par("+QString::number(i)+"): "+QString::number(mParameters[mBestId][i]));
        else
            print(mParNames[i]+": "+QString::number(mParameters[mBestId][i]));
    }

    mIterations = mMetaModelEvaluations + mEvaluations;

    // Clean up
    finalize();

    return;
}

void OptimizationWorkerComplexRFM::finalize()
{
    OptimizationWorkerComplex::finalize();
}

bool OptimizationWorkerComplexRFM::checkForConvergence()
{
    if(mStoredParameters.size() < mNumPoints)
    {
        return OptimizationWorker::checkForConvergence();
    }

    //Check objective function convergence
    double maxObj = mStoredObjectives[0];
    double minObj = mStoredObjectives[0];
    for(int i=0; i<mStoredObjectives.length(); ++i)
    {
        double obj = mStoredObjectives[i];
        if(obj > maxObj) maxObj = obj;
        if(obj < minObj) minObj = obj;
    }
    if(fabs(maxObj-minObj) <= mFuncTol)
    {
        mConvergenceReason=1;
        for(int i=0; i<mObjectives.size(); ++i)
        {
            mObjectives[i] = mStoredObjectives[i];
            mParameters[i] = mStoredParameters[i];
        }
        return true;
    }
    else if(minObj != 0.0 && fabs(maxObj-minObj)/fabs(minObj) <= mFuncTol)
    {
        mConvergenceReason=1;
        for(int i=0; i<mObjectives.size(); ++i)
        {
            mObjectives[i] = mStoredObjectives[i];
            mParameters[i] = mStoredParameters[i];
        }
        mParameters = mStoredParameters;

        return true;
    }
    else
    {
        return OptimizationWorker::checkForConvergence();
    }

}


void OptimizationWorkerComplexRFM::setOptVar(const QString &var, const QString &value)
{
    OptimizationWorkerComplex::setOptVar(var, value);

    if(var == "percDiff")
    {
        mPercDiff = value.toDouble();
    }
    else if(var == "countMax")
    {
        mCountMax = value.toDouble();
    }
}


double OptimizationWorkerComplexRFM::getOptVar(const QString &var, bool &ok)
{
    double retval = OptimizationWorkerComplex::getOptVar(var, ok);
    if(ok)
    {
        return retval;
    }

    ok = true;
    if(var == "percDiff")
    {
        return mPercDiff;
    }
    else if(var == "countMax")
    {
        return mCountMax;
    }
    else
    {
        ok = false;
        return 0;
    }
}


void OptimizationWorkerComplexRFM::storeValuesForMetaModel(int idx)
{
    mStoredParameters.append(QVector<double>());
    for(int p=0; p<mParameters[idx].size(); ++p)
    {
        mStoredParameters.last().append(mParameters[idx][p]);
    }

    //Shift vector to the left
    for(int i=0; i<mStoredObjectives.length()-1; ++i)
    {
        mStoredObjectives[i] = mStoredObjectives[i+1];
    }
    mStoredObjectives[mStoredObjectives.length()-1] = mObjectives[idx];

    qDebug() << "Stored objectives: ";
    for(int i=0; i<mStoredObjectives.length(); ++i)
    {
        qDebug() << mStoredObjectives[i];
    }

    //Remove first element if stored vectors are too long
    if(mStoredParameters.size() > mStorageSize)
    {
        mStoredParameters.remove(0);        //Assume both vectors always has same length
    }
}


void OptimizationWorkerComplexRFM::createMetaModel()
{
    hopsan::Vec orgMetaModelCoefficients = mMetaModelCoefficients;

    //qDebug() << "Begin: createMetaModel()";

    //Skapa metamodell
    int n=mStorageSize;
    int m=mMetaModelCoefficients.length();

    mMatrix.create(n, m);

    //qDebug() << "Number of rows: " << n;
    //qDebug() << "Number of columns: " << m;
    //qDebug() << "mNumParameters: " << mNumParameters;

    for(int i=0; i<n; ++i)
    {
        //qDebug() << "Assigning matrix element: " << i << 0;
        mMatrix[i][0] = 1;
    }

    for(int i=0; i<n; ++i)
    {
        for(int j=0; j<mNumParameters; ++j)
        {
            //qDebug() << "Assigning matrix element: " << i << ", " << j+1;
            mMatrix[i][j+1] = mStoredParameters[i][j];
        }
    }

    for(int i=0; i<n; ++i)
    {
        int col=mNumParameters+1;
        for(int j=0; j<mNumParameters; ++j)
        {
            for(int k=j; k<mNumParameters; ++k)
            {
                //qDebug() << "Assigning matrix element: " << i << ", " << col;
                mMatrix[i][col] = mStoredParameters[i][j]*mStoredParameters[i][k];
                ++col;
            }
        }
    }

//    mBVec.create(n);
//    for(int i=0; i<n; ++i)
//    {
//        mBVec[i] = mStoredObjectives[i];
//    }


    //Solve system using L and U matrices
    hopsan::Matrix matrixT = mMatrix.transpose();       //Multiply matrix and vector with transpose of matrix, because we need a square matrix
    hopsan::Matrix tempMatrix = matrixT*mMatrix;
    hopsan::Vec tempVec = matrixT*mStoredObjectives;
    int* order = new int[m];
    hopsan::ludcmp(tempMatrix, order);

    //qDebug() << "tempMatrix.size() = " << tempMatrix.rows() << ", " << tempMatrix.cols();
    //qDebug() << "tempVec.size() = " << tempVec.length();
    //qDebug() << "mMetaModelCoefficients.size() = " << mMetaModelCoefficients.length();
    //qDebug() << "order.size() = " << m;

    hopsan::solvlu(tempMatrix,tempVec,mMetaModelCoefficients,order);

    //qDebug() << "End: createMetaModel()";

    //qDebug() << "Meta model coefficients:";
    for(int i=0; i<mMetaModelCoefficients.length(); ++i)
    {
        //qDebug() << mMetaModelCoefficients[i];
    }
    //qDebug() << "";

    double diff=0;
    double mOldObj = mObjectives[mWorstId];
    QVector<double> mOldPars = mParameters[mWorstId];
    for(int i=0; i<mStoredParameters.size(); ++i)
    {

        mParameters[mWorstId] = mStoredParameters[i];
        evaluateWithMetaModel();
        double tempDiff = fabs(mObjectives[mWorstId]-mStoredObjectives[i]);
        diff = diff+pow(tempDiff,2.0);
    }
    mObjectives[mWorstId] = mOldObj;
    mParameters[mWorstId] = mOldPars;
    diff = sqrt(diff);

    if(diff > mStorageSize && mEvaluations > 2000)
    {
        execute("call evalall");
        mUseMetaModel = false;
        qDebug() << "Stopped!";
    }
}


void OptimizationWorkerComplexRFM::printMatrix(hopsan::Matrix &matrix)
{
    execute("echo on");
    print("Matrix:");
    for(int i=0; i<matrix.rows(); ++i)
    {
        QString line;
        for(int j=0; j<matrix.cols(); ++j)
        {
            line.append(QString::number(matrix[i][j])+"   ");
        }
        print(line, "", false);
    }
    execute("echo off -nonerrors");
}


void OptimizationWorkerComplexRFM::evaluateWithMetaModel()
{
    double obj=1*mMetaModelCoefficients[0];

    for(int i=0; i<mNumParameters; ++i)
    {
        obj += mMetaModelCoefficients[i+1]*mParameters[mWorstId][i];
    }

    for(int i=0; i<mNumParameters; ++i)
    {
        for(int j=i; j<mNumParameters; ++j)
        {
            obj += mMetaModelCoefficients[1+i*mNumParameters+j+mNumParameters]*mParameters[mWorstId][i]*mParameters[mWorstId][j];
        }
    }

    //Use regular evaluate if objective is NaN
    if(obj != obj)
    {
        execute("call evalworst");
        storeValuesForMetaModel(mWorstId);
        ++mEvaluations;
        mMetaModelExist = false;
    }
    else
    {
        mObjectives[mWorstId] = obj;
    }
}

/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

#ifndef MECHANICJLINK_HPP_INCLUDED
#define MECHANICJLINK_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file MechanicJLink.hpp
//! @author Petter Krus <petter.krus@liu.se>
//  co-author/auditor **Not yet audited by a second person**
//! @date Mon 11 May 2015 12:42:41
//! @brief Link with inertia
//! @ingroup MechanicComponents
//!
//==This code has been autogenerated using Compgen==
//from 
/*{, C:, HopsanTrunk, componentLibraries, defaultLibrary, Mechanic, \
Rotational}/MechanicJLink.nb*/

using namespace hopsan;

class MechanicJLink : public ComponentQ
{
private:
     double JL;
     double BL;
     double link;
     double x0;
     double theta0;
     double thetamin;
     double thetamax;
     Port *mpPm1;
     Port *mpPmr2;
     double delayParts1[9];
     double delayParts2[9];
     double delayParts3[9];
     double delayParts4[9];
     Matrix jacobianMatrix;
     Vec systemEquations;
     Matrix delayedPart;
     int i;
     int iter;
     int mNoiter;
     double jsyseqnweight[4];
     int order[4];
     int mNstep;
     //Port Pm1 variable
     double fm1;
     double xm1;
     double vm1;
     double cm1;
     double Zcm1;
     double eqMassm1;
     //Port Pmr2 variable
     double tormr2;
     double thetamr2;
     double wmr2;
     double cmr2;
     double Zcmr2;
     double eqInertiamr2;
//==This code has been autogenerated using Compgen==
     //inputVariables
     //outputVariables
     //Expressions variables
     //Port Pm1 pointer
     double *mpND_fm1;
     double *mpND_xm1;
     double *mpND_vm1;
     double *mpND_cm1;
     double *mpND_Zcm1;
     double *mpND_eqMassm1;
     //Port Pmr2 pointer
     double *mpND_tormr2;
     double *mpND_thetamr2;
     double *mpND_wmr2;
     double *mpND_cmr2;
     double *mpND_Zcmr2;
     double *mpND_eqInertiamr2;
     //Delay declarations
//==This code has been autogenerated using Compgen==
     //inputVariables pointers
     //inputParameters pointers
     double *mpJL;
     double *mpBL;
     double *mplink;
     double *mpx0;
     double *mptheta0;
     double *mpthetamin;
     double *mpthetamax;
     //outputVariables pointers
     Delay mDelayedPart10;
     Delay mDelayedPart11;
     Delay mDelayedPart20;
     Delay mDelayedPart21;
     Delay mDelayedPart22;
     EquationSystemSolver *mpSolver = nullptr;

public:
     static Component *Creator()
     {
        return new MechanicJLink();
     }

     void configure()
     {
//==This code has been autogenerated using Compgen==

        mNstep=9;
        jacobianMatrix.create(4,4);
        systemEquations.create(4);
        delayedPart.create(5,6);
        mNoiter=2;
        jsyseqnweight[0]=1;
        jsyseqnweight[1]=0.67;
        jsyseqnweight[2]=0.5;
        jsyseqnweight[3]=0.5;


        //Add ports to the component
        mpPm1=addPowerPort("Pm1","NodeMechanic");
        mpPmr2=addPowerPort("Pmr2","NodeMechanicRotational");
        //Add inputVariables to the component

        //Add inputParammeters to the component
            addInputVariable("JL", "Equivalent inertia at node 2", "kgm2", \
1.,&mpJL);
            addInputVariable("BL", "Visc friction coeff. at node 2", \
"Ns/rad", 1.,&mpBL);
            addInputVariable("link", "Link length x1/sin(thetarot2)", "", \
0.1,&mplink);
            addInputVariable("x0", "x position for zero angle", "", \
-0.1,&mpx0);
            addInputVariable("theta0", "link angle for zero angle", "", \
0.1,&mptheta0);
            addInputVariable("thetamin", "Min angle", "rad", \
-1.05,&mpthetamin);
            addInputVariable("thetamax", "Max angle", "rad", \
1.05,&mpthetamax);
        //Add outputVariables to the component

//==This code has been autogenerated using Compgen==
        //Add constantParameters
        mpSolver = new EquationSystemSolver(this,4);
     }

    void initialize()
     {
        //Read port variable pointers from nodes
        //Port Pm1
        mpND_fm1=getSafeNodeDataPtr(mpPm1, NodeMechanic::Force);
        mpND_xm1=getSafeNodeDataPtr(mpPm1, NodeMechanic::Position);
        mpND_vm1=getSafeNodeDataPtr(mpPm1, NodeMechanic::Velocity);
        mpND_cm1=getSafeNodeDataPtr(mpPm1, NodeMechanic::WaveVariable);
        mpND_Zcm1=getSafeNodeDataPtr(mpPm1, NodeMechanic::CharImpedance);
        mpND_eqMassm1=getSafeNodeDataPtr(mpPm1, \
NodeMechanic::EquivalentMass);
        //Port Pmr2
        mpND_tormr2=getSafeNodeDataPtr(mpPmr2, \
NodeMechanicRotational::Torque);
        mpND_thetamr2=getSafeNodeDataPtr(mpPmr2, \
NodeMechanicRotational::Angle);
        mpND_wmr2=getSafeNodeDataPtr(mpPmr2, \
NodeMechanicRotational::AngularVelocity);
        mpND_cmr2=getSafeNodeDataPtr(mpPmr2, \
NodeMechanicRotational::WaveVariable);
        mpND_Zcmr2=getSafeNodeDataPtr(mpPmr2, \
NodeMechanicRotational::CharImpedance);
        mpND_eqInertiamr2=getSafeNodeDataPtr(mpPmr2, \
NodeMechanicRotational::EquivalentInertia);

        //Read variables from nodes
        //Port Pm1
        fm1 = (*mpND_fm1);
        xm1 = (*mpND_xm1);
        vm1 = (*mpND_vm1);
        cm1 = (*mpND_cm1);
        Zcm1 = (*mpND_Zcm1);
        eqMassm1 = (*mpND_eqMassm1);
        //Port Pmr2
        tormr2 = (*mpND_tormr2);
        thetamr2 = (*mpND_thetamr2);
        wmr2 = (*mpND_wmr2);
        cmr2 = (*mpND_cmr2);
        Zcmr2 = (*mpND_Zcmr2);
        eqInertiamr2 = (*mpND_eqInertiamr2);

        //Read inputVariables from nodes

        //Read inputParameters from nodes
        JL = (*mpJL);
        BL = (*mpBL);
        link = (*mplink);
        x0 = (*mpx0);
        theta0 = (*mptheta0);
        thetamin = (*mpthetamin);
        thetamax = (*mpthetamax);

        //Read outputVariables from nodes

//==This code has been autogenerated using Compgen==


        //Initialize delays
        delayParts1[1] = (mTimestep*tormr2 - 2*JL*wmr2 + BL*mTimestep*wmr2 - \
fm1*link*mTimestep*Cos(thetamr2))/(2*JL + BL*mTimestep);
        mDelayedPart11.initialize(mNstep,delayParts1[1]);
        delayParts2[1] = (-8*JL*thetamr2 + 2*Power(mTimestep,2)*tormr2 + \
2*BL*Power(mTimestep,2)*wmr2 - \
2*fm1*link*Power(mTimestep,2)*Cos(thetamr2))/(4.*JL);
        mDelayedPart21.initialize(mNstep,delayParts2[1]);
        delayParts2[2] = (4*JL*thetamr2 + Power(mTimestep,2)*tormr2 + \
BL*Power(mTimestep,2)*wmr2 - \
fm1*link*Power(mTimestep,2)*Cos(thetamr2))/(4.*JL);
        mDelayedPart22.initialize(mNstep,delayParts2[2]);

        delayedPart[1][1] = delayParts1[1];
        delayedPart[2][1] = delayParts2[1];
        delayedPart[2][2] = mDelayedPart22.getIdx(1);
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];

        simulateOneTimestep();

     }
    void simulateOneTimestep()
     {
        Vec stateVar(4);
        Vec stateVark(4);
        Vec deltaStateVar(4);

        //Read variables from nodes
        //Port Pm1
        cm1 = (*mpND_cm1);
        Zcm1 = (*mpND_Zcm1);
        //Port Pmr2
        cmr2 = (*mpND_cmr2);
        Zcmr2 = (*mpND_Zcmr2);

        //Read inputVariables from nodes

        //Read inputParameters from nodes
        JL = (*mpJL);
        BL = (*mpBL);
        link = (*mplink);
        x0 = (*mpx0);
        theta0 = (*mptheta0);
        thetamin = (*mpthetamin);
        thetamax = (*mpthetamax);

        //LocalExpressions

        //Initializing variable vector for Newton-Raphson
        stateVark[0] = wmr2;
        stateVark[1] = thetamr2;
        stateVark[2] = fm1;
        stateVark[3] = tormr2;

        //Iterative solution using Newton-Rapshson
        for(iter=1;iter<=mNoiter;iter++)
        {
         //JLink
         //Differential-algebraic system of equation parts

          //Assemble differential-algebraic equations
          systemEquations[0] =wmr2 - \
dxLimit(limit(-(Power(mTimestep,2)*(tormr2 + BL*wmr2 - \
fm1*link*Cos(thetamr2)))/(4.*JL) - delayedPart[2][1] - \
delayedPart[2][2],thetamin,thetamax),thetamin,thetamax)*(-((mTimestep*(tormr2 \
- fm1*link*Cos(thetamr2)))/(2*JL + BL*mTimestep)) - delayedPart[1][1]);
          systemEquations[1] =thetamr2 - limit(-(Power(mTimestep,2)*(tormr2 + \
BL*wmr2 - fm1*link*Cos(thetamr2)))/(4.*JL) - delayedPart[2][1] - \
delayedPart[2][2],thetamin,thetamax);
          systemEquations[2] =-cm1 + fm1 + link*wmr2*Zcm1*Cos(thetamr2);
          systemEquations[3] =-cmr2 + tormr2 - wmr2*Zcmr2;

          //Jacobian matrix
          jacobianMatrix[0][0] = 1;
          jacobianMatrix[0][1] = \
(fm1*link*mTimestep*dxLimit(limit(-(Power(mTimestep,2)*(tormr2 + BL*wmr2 - \
fm1*link*Cos(thetamr2)))/(4.*JL) - delayedPart[2][1] - \
delayedPart[2][2],thetamin,thetamax),thetamin,thetamax)*Sin(thetamr2))/(2*JL \
+ BL*mTimestep);
          jacobianMatrix[0][2] = \
-((link*mTimestep*Cos(thetamr2)*dxLimit(limit(-(Power(mTimestep,2)*(tormr2 + \
BL*wmr2 - fm1*link*Cos(thetamr2)))/(4.*JL) - delayedPart[2][1] - \
delayedPart[2][2],thetamin,thetamax),thetamin,thetamax))/(2*JL + \
BL*mTimestep));
          jacobianMatrix[0][3] = \
(mTimestep*dxLimit(limit(-(Power(mTimestep,2)*(tormr2 + BL*wmr2 - \
fm1*link*Cos(thetamr2)))/(4.*JL) - delayedPart[2][1] - \
delayedPart[2][2],thetamin,thetamax),thetamin,thetamax))/(2*JL + \
BL*mTimestep);
          jacobianMatrix[1][0] = \
(BL*Power(mTimestep,2)*dxLimit(-(Power(mTimestep,2)*(tormr2 + BL*wmr2 - \
fm1*link*Cos(thetamr2)))/(4.*JL) - delayedPart[2][1] - \
delayedPart[2][2],thetamin,thetamax))/(4.*JL);
          jacobianMatrix[1][1] = 1 + \
(fm1*link*Power(mTimestep,2)*dxLimit(-(Power(mTimestep,2)*(tormr2 + BL*wmr2 - \
fm1*link*Cos(thetamr2)))/(4.*JL) - delayedPart[2][1] - \
delayedPart[2][2],thetamin,thetamax)*Sin(thetamr2))/(4.*JL);
          jacobianMatrix[1][2] = \
-(link*Power(mTimestep,2)*Cos(thetamr2)*dxLimit(-(Power(mTimestep,2)*(tormr2 \
+ BL*wmr2 - fm1*link*Cos(thetamr2)))/(4.*JL) - delayedPart[2][1] - \
delayedPart[2][2],thetamin,thetamax))/(4.*JL);
          jacobianMatrix[1][3] = \
(Power(mTimestep,2)*dxLimit(-(Power(mTimestep,2)*(tormr2 + BL*wmr2 - \
fm1*link*Cos(thetamr2)))/(4.*JL) - delayedPart[2][1] - \
delayedPart[2][2],thetamin,thetamax))/(4.*JL);
          jacobianMatrix[2][0] = link*Zcm1*Cos(thetamr2);
          jacobianMatrix[2][1] = -(link*wmr2*Zcm1*Sin(thetamr2));
          jacobianMatrix[2][2] = 1;
          jacobianMatrix[2][3] = 0;
          jacobianMatrix[3][0] = -Zcmr2;
          jacobianMatrix[3][1] = 0;
          jacobianMatrix[3][2] = 0;
          jacobianMatrix[3][3] = 1;
//==This code has been autogenerated using Compgen==

          //Solving equation using LU-faktorisation
          mpSolver->solve(jacobianMatrix, systemEquations, stateVark, iter);
          wmr2=stateVark[0];
          thetamr2=stateVark[1];
          fm1=stateVark[2];
          tormr2=stateVark[3];
          //Expressions
          vm1 = -(link*wmr2*Cos(thetamr2));
          xm1 = x0 - link*Sin(thetamr2);
          eqMassm1 = (JL*Power(Sec(thetamr2),2))/Power(link,2);
          eqInertiamr2 = JL;
        }

        //Calculate the delayed parts
        delayParts1[1] = (mTimestep*tormr2 - 2*JL*wmr2 + BL*mTimestep*wmr2 - \
fm1*link*mTimestep*Cos(thetamr2))/(2*JL + BL*mTimestep);
        delayParts2[1] = (-8*JL*thetamr2 + 2*Power(mTimestep,2)*tormr2 + \
2*BL*Power(mTimestep,2)*wmr2 - \
2*fm1*link*Power(mTimestep,2)*Cos(thetamr2))/(4.*JL);
        delayParts2[2] = (4*JL*thetamr2 + Power(mTimestep,2)*tormr2 + \
BL*Power(mTimestep,2)*wmr2 - \
fm1*link*Power(mTimestep,2)*Cos(thetamr2))/(4.*JL);

        delayedPart[1][1] = delayParts1[1];
        delayedPart[2][1] = delayParts2[1];
        delayedPart[2][2] = mDelayedPart22.getIdx(0);
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];

        //Write new values to nodes
        //Port Pm1
        (*mpND_fm1)=fm1;
        (*mpND_xm1)=xm1;
        (*mpND_vm1)=vm1;
        (*mpND_eqMassm1)=eqMassm1;
        //Port Pmr2
        (*mpND_tormr2)=tormr2;
        (*mpND_thetamr2)=thetamr2;
        (*mpND_wmr2)=wmr2;
        (*mpND_eqInertiamr2)=eqInertiamr2;
        //outputVariables

        //Update the delayed variabels
        mDelayedPart11.update(delayParts1[1]);
        mDelayedPart21.update(delayParts2[1]);
        mDelayedPart22.update(delayParts2[2]);

     }
    void deconfigure()
    {
        delete mpSolver;
    }
};
#endif // MECHANICJLINK_HPP_INCLUDED

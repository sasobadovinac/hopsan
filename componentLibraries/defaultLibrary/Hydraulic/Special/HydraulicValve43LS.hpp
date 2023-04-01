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

#ifndef HYDRAULICVALVE43LS_HPP_INCLUDED
#define HYDRAULICVALVE43LS_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file HydraulicValve43LS.hpp
//! @author Petter Krus <petter.krus@liu.se>
//! @date Fri 13 Jun 2014 09:01:52
//! @brief A load sensing hydraulic directional valve
//! @ingroup HydraulicComponents
//!
//==This code has been autogenerated using Compgen==
//from 
/*{, C:, HopsanTrunk, ComponentLibraries, defaultLibrary, Hydraulic, \
Special}/HydraulicValve43LS.nb*/

using namespace hopsan;

class HydraulicValve43LS : public ComponentQ
{
private:
     double rho;
     double Cq;
     double Sd;
     double Frap;
     double Frat;
     double Frbp;
     double Frbt;
     double wls;
     double Xap0;
     double Xat0;
     double Xbp0;
     double Xbt0;
     double Xlsa0;
     double Xlsb0;
     double Xls0;
     double Xvmax;
     double Xvlsmax;
     double plam;
     Port *mpPp;
     Port *mpPt;
     Port *mpPa;
     Port *mpPb;
     Port *mpPls;
     double delayParts1[9];
     double delayParts2[9];
     double delayParts3[9];
     double delayParts4[9];
     double delayParts5[9];
     double delayParts6[9];
     double delayParts7[9];
     double delayParts8[9];
     double delayParts9[9];
     double delayParts10[9];
     Matrix jacobianMatrix;
     Vec systemEquations;
     Matrix delayedPart;
     int i;
     int iter;
     int mNoiter;
     double jsyseqnweight[4];
     int order[10];
     int mNstep;
     //Port Pp variable
     double pp;
     double qp;
     double Tp;
     double dEp;
     double cp;
     double Zcp;
     //Port Pt variable
     double pt;
     double qt;
     double Tt;
     double dEt;
     double ct;
     double Zct;
     //Port Pa variable
     double pa;
     double qa;
     double Ta;
     double dEa;
     double ca;
     double Zca;
     //Port Pb variable
     double pb;
     double qb;
     double Tb;
     double dEb;
     double cb;
     double Zcb;
     //Port Pls variable
     double pls;
     double qls;
     double Tls;
     double dEls;
     double cls;
     double Zcls;
//==This code has been autogenerated using Compgen==
     //inputVariables
     double xv;
     //outputVariables
     //LocalExpressions variables
     double Ks;
     double Kspa;
     double Ksta;
     double Kspb;
     double Kstb;
     double Kslsa;
     double Kslsb;
     double Kslst;
     //Port Pp pointer
     double *mpND_pp;
     double *mpND_qp;
     double *mpND_Tp;
     double *mpND_dEp;
     double *mpND_cp;
     double *mpND_Zcp;
     //Port Pt pointer
     double *mpND_pt;
     double *mpND_qt;
     double *mpND_Tt;
     double *mpND_dEt;
     double *mpND_ct;
     double *mpND_Zct;
     //Port Pa pointer
     double *mpND_pa;
     double *mpND_qa;
     double *mpND_Ta;
     double *mpND_dEa;
     double *mpND_ca;
     double *mpND_Zca;
     //Port Pb pointer
     double *mpND_pb;
     double *mpND_qb;
     double *mpND_Tb;
     double *mpND_dEb;
     double *mpND_cb;
     double *mpND_Zcb;
     //Port Pls pointer
     double *mpND_pls;
     double *mpND_qls;
     double *mpND_Tls;
     double *mpND_dEls;
     double *mpND_cls;
     double *mpND_Zcls;
     //Delay declarations
//==This code has been autogenerated using Compgen==
     //inputVariables pointers
     double *mpxv;
     //inputParameters pointers
     double *mprho;
     double *mpCq;
     double *mpSd;
     double *mpFrap;
     double *mpFrat;
     double *mpFrbp;
     double *mpFrbt;
     double *mpwls;
     double *mpXap0;
     double *mpXat0;
     double *mpXbp0;
     double *mpXbt0;
     double *mpXlsa0;
     double *mpXlsb0;
     double *mpXls0;
     double *mpXvmax;
     double *mpXvlsmax;
     double *mpplam;
     //outputVariables pointers
     Delay mDelayedPart10;
     Delay mDelayedPart20;
     Delay mDelayedPart30;
     Delay mDelayedPart40;
     Delay mDelayedPart50;
     EquationSystemSolver *mpSolver = nullptr;

public:
     static Component *Creator()
     {
        return new HydraulicValve43LS();
     }

     void configure()
     {
//==This code has been autogenerated using Compgen==

        mNstep=9;
        jacobianMatrix.create(10,10);
        systemEquations.create(10);
        delayedPart.create(11,6);
        mNoiter=2;
        jsyseqnweight[0]=1;
        jsyseqnweight[1]=0.67;
        jsyseqnweight[2]=0.5;
        jsyseqnweight[3]=0.5;


        //Add ports to the component
        mpPp=addPowerPort("Pp","NodeHydraulic");
        mpPt=addPowerPort("Pt","NodeHydraulic");
        mpPa=addPowerPort("Pa","NodeHydraulic");
        mpPb=addPowerPort("Pb","NodeHydraulic");
        mpPls=addPowerPort("Pls","NodeHydraulic");
        //Add inputVariables to the component
            addInputVariable("xv","Spool position","m",0.,&mpxv);

        //Add inputParammeters to the component
            addInputVariable("rho", "Oil density", "kg/m3", 870, &mprho);
            addInputVariable("Cq", "Flow coefficient.", "", 0.67,&mpCq);
            addInputVariable("Sd", "spool diameter", "m", 0.01,&mpSd);
            addInputVariable("Frap", "Spool cricle fraction(P-A)", "", \
1.,&mpFrap);
            addInputVariable("Frat", "Spool cricle fraction(A-T)", "", \
1.,&mpFrat);
            addInputVariable("Frbp", "Spool cricle fraction(P-B)", "", \
1.,&mpFrbp);
            addInputVariable("Frbt", "Spool cricle fraction(B-T)", "", \
1.,&mpFrbt);
            addInputVariable("wls", "Area gradient LS-port", "m", \
0.01,&mpwls);
            addInputVariable("Xap0", "Overlap", "m", 0.0003,&mpXap0);
            addInputVariable("Xat0", "Overlap", "m", 0.0003,&mpXat0);
            addInputVariable("Xbp0", "Overlap", "m", 0.0003,&mpXbp0);
            addInputVariable("Xbt0", "Overlap", "m", 0.0003,&mpXbt0);
            addInputVariable("Xlsa0", "Overlap", "m", 0.0003,&mpXlsa0);
            addInputVariable("Xlsb0", "Overlap", "m", 0.0003,&mpXlsb0);
            addInputVariable("Xls0", "Zero opening LS port", "m", \
0.0003,&mpXls0);
            addInputVariable("Xvmax", "Max valve opening", "m", \
0.01,&mpXvmax);
            addInputVariable("Xvlsmax", "Max opening LS port", "m", \
0.001,&mpXvlsmax);
            addInputVariable("plam", "Turbulence onset pressure", "Pa", \
10000.,&mpplam);
        //Add outputVariables to the component

//==This code has been autogenerated using Compgen==
        //Add constantParameters
        mpSolver = new EquationSystemSolver(this,10);
     }

    void initialize()
     {
        //Read port variable pointers from nodes
        //Port Pp
        mpND_pp=getSafeNodeDataPtr(mpPp, NodeHydraulic::Pressure);
        mpND_qp=getSafeNodeDataPtr(mpPp, NodeHydraulic::Flow);
        mpND_Tp=getSafeNodeDataPtr(mpPp, NodeHydraulic::Temperature);
        mpND_dEp=getSafeNodeDataPtr(mpPp, NodeHydraulic::HeatFlow);
        mpND_cp=getSafeNodeDataPtr(mpPp, NodeHydraulic::WaveVariable);
        mpND_Zcp=getSafeNodeDataPtr(mpPp, NodeHydraulic::CharImpedance);
        //Port Pt
        mpND_pt=getSafeNodeDataPtr(mpPt, NodeHydraulic::Pressure);
        mpND_qt=getSafeNodeDataPtr(mpPt, NodeHydraulic::Flow);
        mpND_Tt=getSafeNodeDataPtr(mpPt, NodeHydraulic::Temperature);
        mpND_dEt=getSafeNodeDataPtr(mpPt, NodeHydraulic::HeatFlow);
        mpND_ct=getSafeNodeDataPtr(mpPt, NodeHydraulic::WaveVariable);
        mpND_Zct=getSafeNodeDataPtr(mpPt, NodeHydraulic::CharImpedance);
        //Port Pa
        mpND_pa=getSafeNodeDataPtr(mpPa, NodeHydraulic::Pressure);
        mpND_qa=getSafeNodeDataPtr(mpPa, NodeHydraulic::Flow);
        mpND_Ta=getSafeNodeDataPtr(mpPa, NodeHydraulic::Temperature);
        mpND_dEa=getSafeNodeDataPtr(mpPa, NodeHydraulic::HeatFlow);
        mpND_ca=getSafeNodeDataPtr(mpPa, NodeHydraulic::WaveVariable);
        mpND_Zca=getSafeNodeDataPtr(mpPa, NodeHydraulic::CharImpedance);
        //Port Pb
        mpND_pb=getSafeNodeDataPtr(mpPb, NodeHydraulic::Pressure);
        mpND_qb=getSafeNodeDataPtr(mpPb, NodeHydraulic::Flow);
        mpND_Tb=getSafeNodeDataPtr(mpPb, NodeHydraulic::Temperature);
        mpND_dEb=getSafeNodeDataPtr(mpPb, NodeHydraulic::HeatFlow);
        mpND_cb=getSafeNodeDataPtr(mpPb, NodeHydraulic::WaveVariable);
        mpND_Zcb=getSafeNodeDataPtr(mpPb, NodeHydraulic::CharImpedance);
        //Port Pls
        mpND_pls=getSafeNodeDataPtr(mpPls, NodeHydraulic::Pressure);
        mpND_qls=getSafeNodeDataPtr(mpPls, NodeHydraulic::Flow);
        mpND_Tls=getSafeNodeDataPtr(mpPls, NodeHydraulic::Temperature);
        mpND_dEls=getSafeNodeDataPtr(mpPls, NodeHydraulic::HeatFlow);
        mpND_cls=getSafeNodeDataPtr(mpPls, NodeHydraulic::WaveVariable);
        mpND_Zcls=getSafeNodeDataPtr(mpPls, NodeHydraulic::CharImpedance);

        //Read variables from nodes
        //Port Pp
        pp = (*mpND_pp);
        qp = (*mpND_qp);
        Tp = (*mpND_Tp);
        dEp = (*mpND_dEp);
        cp = (*mpND_cp);
        Zcp = (*mpND_Zcp);
        //Port Pt
        pt = (*mpND_pt);
        qt = (*mpND_qt);
        Tt = (*mpND_Tt);
        dEt = (*mpND_dEt);
        ct = (*mpND_ct);
        Zct = (*mpND_Zct);
        //Port Pa
        pa = (*mpND_pa);
        qa = (*mpND_qa);
        Ta = (*mpND_Ta);
        dEa = (*mpND_dEa);
        ca = (*mpND_ca);
        Zca = (*mpND_Zca);
        //Port Pb
        pb = (*mpND_pb);
        qb = (*mpND_qb);
        Tb = (*mpND_Tb);
        dEb = (*mpND_dEb);
        cb = (*mpND_cb);
        Zcb = (*mpND_Zcb);
        //Port Pls
        pls = (*mpND_pls);
        qls = (*mpND_qls);
        Tls = (*mpND_Tls);
        dEls = (*mpND_dEls);
        cls = (*mpND_cls);
        Zcls = (*mpND_Zcls);

        //Read inputVariables from nodes
        xv = (*mpxv);

        //Read inputParameters from nodes
        rho = (*mprho);
        Cq = (*mpCq);
        Sd = (*mpSd);
        Frap = (*mpFrap);
        Frat = (*mpFrat);
        Frbp = (*mpFrbp);
        Frbt = (*mpFrbt);
        wls = (*mpwls);
        Xap0 = (*mpXap0);
        Xat0 = (*mpXat0);
        Xbp0 = (*mpXbp0);
        Xbt0 = (*mpXbt0);
        Xlsa0 = (*mpXlsa0);
        Xlsb0 = (*mpXlsb0);
        Xls0 = (*mpXls0);
        Xvmax = (*mpXvmax);
        Xvlsmax = (*mpXvlsmax);
        plam = (*mpplam);

        //Read outputVariables from nodes

//==This code has been autogenerated using Compgen==

        //LocalExpressions
        Ks = (1.4142135623730951*Cq)/Sqrt(rho);
        Kspa = 3.14159*Frap*Ks*Sd*limit(-Xap0 + xv,0.,-Xap0 + Xvmax);
        Ksta = 3.14159*Frat*Ks*Sd*limit(-Xap0 - xv,0.,-Xat0 + Xvmax);
        Kspb = 3.14159*Frbp*Ks*Sd*limit(-Xap0 - xv,0.,-Xbp0 + Xvmax);
        Kstb = 3.14159*Frbt*Ks*Sd*limit(-Xap0 + xv,0.,-Xbt0 + Xvmax);
        Kslsa = Ks*wls*limit(-Xlsa0 + xv,0.,-Xlsa0 + Xvlsmax);
        Kslsb = Ks*wls*limit(-Xlsb0 - xv,0.,-Xlsb0 + Xvlsmax);
        Kslst = Ks*wls*limit(Xls0 - Abs(xv),0.,Xls0);

        //Initialize delays

        delayedPart[1][1] = delayParts1[1];
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];
        delayedPart[5][1] = delayParts5[1];
        delayedPart[6][1] = delayParts6[1];
        delayedPart[7][1] = delayParts7[1];
        delayedPart[8][1] = delayParts8[1];
        delayedPart[9][1] = delayParts9[1];
        delayedPart[10][1] = delayParts10[1];
     }
    void simulateOneTimestep()
     {
        Vec stateVar(10);
        Vec stateVark(10);
        Vec deltaStateVar(10);

        //Read variables from nodes
        //Port Pp
        Tp = (*mpND_Tp);
        cp = (*mpND_cp);
        Zcp = (*mpND_Zcp);
        //Port Pt
        Tt = (*mpND_Tt);
        ct = (*mpND_ct);
        Zct = (*mpND_Zct);
        //Port Pa
        Ta = (*mpND_Ta);
        ca = (*mpND_ca);
        Zca = (*mpND_Zca);
        //Port Pb
        Tb = (*mpND_Tb);
        cb = (*mpND_cb);
        Zcb = (*mpND_Zcb);
        //Port Pls
        Tls = (*mpND_Tls);
        cls = (*mpND_cls);
        Zcls = (*mpND_Zcls);

        //Read inputVariables from nodes
        xv = (*mpxv);

        //LocalExpressions
        Ks = (1.4142135623730951*Cq)/Sqrt(rho);
        Kspa = 3.14159*Frap*Ks*Sd*limit(-Xap0 + xv,0.,-Xap0 + Xvmax);
        Ksta = 3.14159*Frat*Ks*Sd*limit(-Xap0 - xv,0.,-Xat0 + Xvmax);
        Kspb = 3.14159*Frbp*Ks*Sd*limit(-Xap0 - xv,0.,-Xbp0 + Xvmax);
        Kstb = 3.14159*Frbt*Ks*Sd*limit(-Xap0 + xv,0.,-Xbt0 + Xvmax);
        Kslsa = Ks*wls*limit(-Xlsa0 + xv,0.,-Xlsa0 + Xvlsmax);
        Kslsb = Ks*wls*limit(-Xlsb0 - xv,0.,-Xlsb0 + Xvlsmax);
        Kslst = Ks*wls*limit(Xls0 - Abs(xv),0.,Xls0);

        //Initializing variable vector for Newton-Raphson
        stateVark[0] = qp;
        stateVark[1] = qt;
        stateVark[2] = qa;
        stateVark[3] = qb;
        stateVark[4] = qls;
        stateVark[5] = pp;
        stateVark[6] = pt;
        stateVark[7] = pa;
        stateVark[8] = pb;
        stateVark[9] = pls;

        //Iterative solution using Newton-Rapshson
        for(iter=1;iter<=mNoiter;iter++)
        {
         //Valve43LS
         //Differential-algebraic system of equation parts

          //Assemble differential-algebraic equations
          systemEquations[0] =qp + Kspa*signedSquareL(-pa + pp,plam) + \
Kspb*signedSquareL(-pb + pp,plam);
          systemEquations[1] =qt + Ksta*signedSquareL(-pa + pt,plam) + \
Kstb*signedSquareL(-pb + pt,plam);
          systemEquations[2] =qa - Kspa*signedSquareL(-pa + pp,plam) - \
Ksta*signedSquareL(-pa + pt,plam);
          systemEquations[3] =qb - Kspb*signedSquareL(-pb + pp,plam) - \
Kstb*signedSquareL(-pb + pt,plam);
          systemEquations[4] =qls + Kslsa*signedSquareL(-pa + pls,plam) + \
Kslsb*signedSquareL(-pb + pls,plam) + Kslst*signedSquareL(pls - pt,plam);
          systemEquations[5] =pp - lowLimit(cp + qp*Zcp*onPositive(pp),0);
          systemEquations[6] =pt - lowLimit(ct + qt*Zct*onPositive(pt),0);
          systemEquations[7] =pa - lowLimit(ca + qa*Zca*onPositive(pa),0);
          systemEquations[8] =pb - lowLimit(cb + qb*Zcb*onPositive(pb),0);
          systemEquations[9] =pls - lowLimit(cls + \
qls*Zcls*onPositive(pls),0);

          //Jacobian matrix
          jacobianMatrix[0][0] = 1;
          jacobianMatrix[0][1] = 0;
          jacobianMatrix[0][2] = 0;
          jacobianMatrix[0][3] = 0;
          jacobianMatrix[0][4] = 0;
          jacobianMatrix[0][5] = Kspa*dxSignedSquareL(-pa + pp,plam) + \
Kspb*dxSignedSquareL(-pb + pp,plam);
          jacobianMatrix[0][6] = 0;
          jacobianMatrix[0][7] = -(Kspa*dxSignedSquareL(-pa + pp,plam));
          jacobianMatrix[0][8] = -(Kspb*dxSignedSquareL(-pb + pp,plam));
          jacobianMatrix[0][9] = 0;
          jacobianMatrix[1][0] = 0;
          jacobianMatrix[1][1] = 1;
          jacobianMatrix[1][2] = 0;
          jacobianMatrix[1][3] = 0;
          jacobianMatrix[1][4] = 0;
          jacobianMatrix[1][5] = 0;
          jacobianMatrix[1][6] = Ksta*dxSignedSquareL(-pa + pt,plam) + \
Kstb*dxSignedSquareL(-pb + pt,plam);
          jacobianMatrix[1][7] = -(Ksta*dxSignedSquareL(-pa + pt,plam));
          jacobianMatrix[1][8] = -(Kstb*dxSignedSquareL(-pb + pt,plam));
          jacobianMatrix[1][9] = 0;
          jacobianMatrix[2][0] = 0;
          jacobianMatrix[2][1] = 0;
          jacobianMatrix[2][2] = 1;
          jacobianMatrix[2][3] = 0;
          jacobianMatrix[2][4] = 0;
          jacobianMatrix[2][5] = -(Kspa*dxSignedSquareL(-pa + pp,plam));
          jacobianMatrix[2][6] = -(Ksta*dxSignedSquareL(-pa + pt,plam));
          jacobianMatrix[2][7] = Kspa*dxSignedSquareL(-pa + pp,plam) + \
Ksta*dxSignedSquareL(-pa + pt,plam);
          jacobianMatrix[2][8] = 0;
          jacobianMatrix[2][9] = 0;
          jacobianMatrix[3][0] = 0;
          jacobianMatrix[3][1] = 0;
          jacobianMatrix[3][2] = 0;
          jacobianMatrix[3][3] = 1;
          jacobianMatrix[3][4] = 0;
          jacobianMatrix[3][5] = -(Kspb*dxSignedSquareL(-pb + pp,plam));
          jacobianMatrix[3][6] = -(Kstb*dxSignedSquareL(-pb + pt,plam));
          jacobianMatrix[3][7] = 0;
          jacobianMatrix[3][8] = Kspb*dxSignedSquareL(-pb + pp,plam) + \
Kstb*dxSignedSquareL(-pb + pt,plam);
          jacobianMatrix[3][9] = 0;
          jacobianMatrix[4][0] = 0;
          jacobianMatrix[4][1] = 0;
          jacobianMatrix[4][2] = 0;
          jacobianMatrix[4][3] = 0;
          jacobianMatrix[4][4] = 1;
          jacobianMatrix[4][5] = 0;
          jacobianMatrix[4][6] = -(Kslst*dxSignedSquareL(pls - pt,plam));
          jacobianMatrix[4][7] = -(Kslsa*dxSignedSquareL(-pa + pls,plam));
          jacobianMatrix[4][8] = -(Kslsb*dxSignedSquareL(-pb + pls,plam));
          jacobianMatrix[4][9] = Kslsa*dxSignedSquareL(-pa + pls,plam) + \
Kslsb*dxSignedSquareL(-pb + pls,plam) + Kslst*dxSignedSquareL(pls - pt,plam);
          jacobianMatrix[5][0] = -(Zcp*dxLowLimit(cp + \
qp*Zcp*onPositive(pp),0)*onPositive(pp));
          jacobianMatrix[5][1] = 0;
          jacobianMatrix[5][2] = 0;
          jacobianMatrix[5][3] = 0;
          jacobianMatrix[5][4] = 0;
          jacobianMatrix[5][5] = 1;
          jacobianMatrix[5][6] = 0;
          jacobianMatrix[5][7] = 0;
          jacobianMatrix[5][8] = 0;
          jacobianMatrix[5][9] = 0;
          jacobianMatrix[6][0] = 0;
          jacobianMatrix[6][1] = -(Zct*dxLowLimit(ct + \
qt*Zct*onPositive(pt),0)*onPositive(pt));
          jacobianMatrix[6][2] = 0;
          jacobianMatrix[6][3] = 0;
          jacobianMatrix[6][4] = 0;
          jacobianMatrix[6][5] = 0;
          jacobianMatrix[6][6] = 1;
          jacobianMatrix[6][7] = 0;
          jacobianMatrix[6][8] = 0;
          jacobianMatrix[6][9] = 0;
          jacobianMatrix[7][0] = 0;
          jacobianMatrix[7][1] = 0;
          jacobianMatrix[7][2] = -(Zca*dxLowLimit(ca + \
qa*Zca*onPositive(pa),0)*onPositive(pa));
          jacobianMatrix[7][3] = 0;
          jacobianMatrix[7][4] = 0;
          jacobianMatrix[7][5] = 0;
          jacobianMatrix[7][6] = 0;
          jacobianMatrix[7][7] = 1;
          jacobianMatrix[7][8] = 0;
          jacobianMatrix[7][9] = 0;
          jacobianMatrix[8][0] = 0;
          jacobianMatrix[8][1] = 0;
          jacobianMatrix[8][2] = 0;
          jacobianMatrix[8][3] = -(Zcb*dxLowLimit(cb + \
qb*Zcb*onPositive(pb),0)*onPositive(pb));
          jacobianMatrix[8][4] = 0;
          jacobianMatrix[8][5] = 0;
          jacobianMatrix[8][6] = 0;
          jacobianMatrix[8][7] = 0;
          jacobianMatrix[8][8] = 1;
          jacobianMatrix[8][9] = 0;
          jacobianMatrix[9][0] = 0;
          jacobianMatrix[9][1] = 0;
          jacobianMatrix[9][2] = 0;
          jacobianMatrix[9][3] = 0;
          jacobianMatrix[9][4] = -(Zcls*dxLowLimit(cls + \
qls*Zcls*onPositive(pls),0)*onPositive(pls));
          jacobianMatrix[9][5] = 0;
          jacobianMatrix[9][6] = 0;
          jacobianMatrix[9][7] = 0;
          jacobianMatrix[9][8] = 0;
          jacobianMatrix[9][9] = 1;
//==This code has been autogenerated using Compgen==

          //Solving equation using LU-faktorisation
          mpSolver->solve(jacobianMatrix, systemEquations, stateVark, iter);
          qp=stateVark[0];
          qt=stateVark[1];
          qa=stateVark[2];
          qb=stateVark[3];
          qls=stateVark[4];
          pp=stateVark[5];
          pt=stateVark[6];
          pa=stateVark[7];
          pb=stateVark[8];
          pls=stateVark[9];
        }

        //Calculate the delayed parts

        delayedPart[1][1] = delayParts1[1];
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];
        delayedPart[5][1] = delayParts5[1];
        delayedPart[6][1] = delayParts6[1];
        delayedPart[7][1] = delayParts7[1];
        delayedPart[8][1] = delayParts8[1];
        delayedPart[9][1] = delayParts9[1];
        delayedPart[10][1] = delayParts10[1];

        //Write new values to nodes
        //Port Pp
        (*mpND_pp)=pp;
        (*mpND_qp)=qp;
        (*mpND_dEp)=dEp;
        //Port Pt
        (*mpND_pt)=pt;
        (*mpND_qt)=qt;
        (*mpND_dEt)=dEt;
        //Port Pa
        (*mpND_pa)=pa;
        (*mpND_qa)=qa;
        (*mpND_dEa)=dEa;
        //Port Pb
        (*mpND_pb)=pb;
        (*mpND_qb)=qb;
        (*mpND_dEb)=dEb;
        //Port Pls
        (*mpND_pls)=pls;
        (*mpND_qls)=qls;
        (*mpND_dEls)=dEls;
        //outputVariables

        //Update the delayed variabels

     }
    void deconfigure()
    {
        delete mpSolver;
    }
};
#endif // HYDRAULICVALVE43LS_HPP_INCLUDED

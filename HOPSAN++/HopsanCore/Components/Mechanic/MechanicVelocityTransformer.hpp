//$Id$

#ifndef MECHANICVELOCITYTRANSFORMER_HPP_INCLUDED
#define MECHANICVELOCITYTRANSFORMER_HPP_INCLUDED

#include "HopsanCore.h"
#include "CoreUtilities/Integrator.h"

class MechanicVelocityTransformer : public ComponentQ
{

private:
    enum {in, out};
    Integrator mInt;

public:
    static Component *Creator()
    {
        std::cout << "running velocity transformer creator" << std::endl;
        return new MechanicVelocityTransformer("DefaultVelocityTransformerName");
    }

    MechanicVelocityTransformer(const string name,
                    const double timestep    = 0.001)
    : ComponentQ(name, timestep)
    {
        //Set member attributes

		//Add ports to the component
        addReadPort("in", "NodeSignal", in);
        addPowerPort("out", "NodeMechanic", out);

        //Register changable parameters to the HOPSAN++ core
    }


	void initialize()
    {
        double signal  = mPortPtrs[in]->readNode(NodeSignal::VALUE);
        mInt.initializeValues(signal, 0.0, mTimestep, mTime);
    }

    void simulateOneTimestep()
    {
        //Get variable values from nodes
        double signal  = mPortPtrs[in]->readNode(NodeSignal::VALUE);
        double c = mPortPtrs[out]->readNode(NodeMechanic::WAVEVARIABLE);
        double Zc = mPortPtrs[out]->readNode(NodeMechanic::CHARIMP);


        //Spring equations
        double v = signal;
        double x = mInt.value(v, x);
        double F = c + Zc*v;

        //Write new values to nodes
        mPortPtrs[out]->writeNode(NodeMechanic::POSITION, x);
        mPortPtrs[out]->writeNode(NodeMechanic::VELOCITY, v);
        mPortPtrs[out]->writeNode(NodeMechanic::FORCE, F);
    }
};

#endif // MECHANICVELOCITYTRANSFORMER_HPP_INCLUDED





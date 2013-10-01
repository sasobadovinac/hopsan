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
//! @file   MechanicAngularVelocitySensor.hpp
//! @author Robert Braun <robert.braun@liu.se>
//! @date   2011-02-22
//!
//! @brief Contains a Mechanic Angular Velocity Sensor Component
//!
//$Id$

#ifndef MECHANICANGULARVELOCITYSENSOR_HPP_INCLUDED
#define MECHANICANGULARVELOCITYSENSOR_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup MechanicalComponents
    //!
    class MechanicAngularVelocitySensor : public ComponentSignal
    {
    private:
        double *mpND_w, *mpND_out;
        Port *mpP1;


    public:
        static Component *Creator()
        {
            return new MechanicAngularVelocitySensor();
        }

        void configure()
        {

            mpP1 = addReadPort("P1", "NodeMechanicRotational", "", Port::NotRequired);
            addOutputVariable("out", "AngularVelocity", "rad/s", &mpND_out);
        }


        void initialize()
        {
            mpND_w = getSafeNodeDataPtr(mpP1, NodeMechanicRotational::AngularVelocity);
            simulateOneTimestep();
        }


        void simulateOneTimestep()
        {
            (*mpND_out) = (*mpND_w);
        }
    };
}

#endif // MECHANICANGULARVELOCITYSENSOR_HPP_INCLUDED

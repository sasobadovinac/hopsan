/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

//$Id$

#ifndef HydraulicCSOURCE_HPP
#define HydraulicCSOURCE_HPP

#include "ComponentEssentials.h"

namespace hopsan {

//!
//! @brief
//! @ingroup MechanicalComponents
//!
class HydraulicCSource : public ComponentC
{

private:
    double *mpIn_c, *mpIn_Zx;
    double *mpP1_c, *mpP1_Zx;

public:
    static Component *Creator()
    {
        return new HydraulicCSource();
    }

    void configure()
    {
        addInputVariable("in_c", "Wave variable input", "Force", 0, &mpIn_c);
        addInputVariable("in_z", "Char. impedance variable input", "N s/m", 0, &mpIn_Zx);
        addPowerPort("P1", "NodeHydraulic");
    }

    void initialize()
    {
        mpP1_c = getSafeNodeDataPtr("P1", NodeHydraulic::WaveVariable);
        mpP1_Zx = getSafeNodeDataPtr("P1", NodeHydraulic::CharImpedance);
        simulateOneTimestep();
    }

    void simulateOneTimestep()
    {
        (*mpP1_c) = (*mpIn_c);
        (*mpP1_Zx) = (*mpIn_Zx);
    }
};
}

#endif // HydraulicCSOURCE_HPP


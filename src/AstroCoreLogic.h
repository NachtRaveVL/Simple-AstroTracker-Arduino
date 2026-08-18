/*  Astruino: Simple automation controller for DIY astronomical tracking systems.
    Copyright (C) 2026 NachtRaveVL
    Astruino Core Logic
*/

#ifndef AstroCoreLogic_H
#define AstroCoreLogic_H

#include "AstroCoordinates.h"
#include "AstroThermal.h"

// Returns true when value lies within +/- tolerance of the requested target.
inline bool astroWithinTolerance(double value, double target, double tolerance)
{
    double delta = value - target;                           // Signed target offset
    return delta >= -tolerance && delta <= tolerance;
}

#endif // /ifndef AstroCoreLogic_H

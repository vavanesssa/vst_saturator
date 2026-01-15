/*
  ==============================================================================

    Waveshaper.h
    ------------
    This file declares the Waveshaper class, which encapsulates the various
    saturation algorithms.

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

class Waveshaper
{
public:
    enum class Shape
    {
        Tube,
        SoftClip,
        HardClip,
        Diode1,
        Diode2,
        LinearFold,
        SinFold,
        ZeroSquare,
        Downsample,
        Asym,
        Rectify,
        XShaper,
        XShaperAsym,
        SineShaper,
        StompBox,
        TapeSat,
        Overdrive,
        SoftSat,
        BitCrush,
        GlitchFold,
        Valve,
        FuzzFac,
        Cheby3,
        Cheby5,
        LogSat,
        HalfWave,
        Cubic,
        OctaverSat,
        Triode,
        Pentode,
        ClassA,
        ClassAB,
        ClassB,
        Germanium,
        Tape15ips,
        Tape7_5ips,
        TapeCassette,
        Tape456,
        TapeSM900,
        Transformer,
        Console,
        APIStyle,
        SSLStyle,
        Silicon,
        FETClean,
        FETDirty,
        OpAmp,
        CMOS,
        Scream,
        Buzz,
        Crackle,
        Wrap,
        Density,
        Cheby7,
        Hyperbolic,
        Exponential,
        Parabolic,
        Wavelet
    };

    static float apply(float x, int waveshapeIndex, float shapeParam);
};

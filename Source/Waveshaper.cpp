/*
  ==============================================================================

    Waveshaper.cpp
    --------------
    This file implements the Waveshaper class.

  ==============================================================================
*/

#include "Waveshaper.h"

float Waveshaper::apply(float x, int waveshapeIndex, float shapeParam)
{
    float output = x;
    switch (waveshapeIndex)
    {
    case 0: // Tube
    {
        float soft = std::tanh(x * (1.0f - shapeParam * 0.5f));
        float hard = (x - x * x * x / 3.0f);
        output = soft * (1.0f - shapeParam) + hard * shapeParam;
        break;
    }
    case 1: // SoftClip
        output = std::tanh(x * (1.0f + shapeParam * 2.0f));
        break;
    case 2: // HardClip
        output = juce::jlimit(-1.0f, 1.0f, x * (1.0f + shapeParam * 3.0f));
        break;
    case 3: // Diode 1
        output = x > 0.0f ? std::tanh(x * (1.0f + shapeParam)) : x * 0.5f;
        break;
    case 4: // Diode 2
        output = x > 0.0f ? x * 0.7f : std::tanh(x * (1.0f + shapeParam * 2.0f));
        break;
    case 5: // Linear Fold
    {
        float threshold = 1.0f - shapeParam * 0.5f;
        if (std::abs(x) > threshold)
            output = threshold - (std::abs(x) - threshold);
        else
            output = x;
        output = juce::jlimit(-1.0f, 1.0f, output);
        break;
    }
    case 6: // Sin Fold
        output = std::sin(x * juce::MathConstants<float>::pi * (1.0f + shapeParam * 2.0f));
        break;
    case 7: // Zero-Square
        output = x * x * (x > 0.0f ? 1.0f : -1.0f) * (1.0f + shapeParam);
        break;
    case 8: // Downsample (simplified for oversampling context)
        output = std::tanh(x * (1.0f + shapeParam));
        break;
    case 9: // Asym
        output = x > 0.0f ? std::tanh(x * (1.0f + shapeParam * 2.0f)) : x * 0.3f;
        break;
    case 10: // Rectify
        output = std::abs(x) * (1.0f - shapeParam * 0.5f);
        break;
    case 11: // X-Shaper
        output = x * (1.0f + shapeParam) / (1.0f + shapeParam * std::abs(x));
        break;
    case 12: // X-Shaper (Asym)
        output = x > 0.0f ? x * (1.0f + shapeParam * 2.0f) / (1.0f + shapeParam * std::abs(x)) : x * 0.5f;
        break;
    case 13: // Sine Shaper
        output = std::sin(std::tanh(x) * juce::MathConstants<float>::pi * 0.5f * (1.0f + shapeParam));
        break;
    case 14: // Stomp Box
        output = std::atan(x * (1.0f + shapeParam * 5.0f)) / juce::MathConstants<float>::pi;
        break;
    case 15: // Tape Sat.
    {
        float wet = std::tanh(x * 1.5f);
        output = x * (1.0f - shapeParam) + wet * shapeParam;
        break;
    }
    case 16: // Overdrive
        output = (2.0f / juce::MathConstants<float>::pi) * std::atan(x * (1.0f + shapeParam * 10.0f));
        break;
    case 17: // Soft Sat.
        output = x / (1.0f + std::abs(x) * shapeParam);
        break;
    case 18: // Bit-Crush
    {
        float levels = 2.0f + (1.0f - shapeParam) * 30.0f;
        output = std::round(x * levels) / levels;
        break;
    }
    case 19: // Glitch Fold
        output = x * std::sin(x * shapeParam * juce::MathConstants<float>::pi);
        break;
    case 20: // Valve
    {
        float bias = 0.2f * shapeParam;
        float x_biased = x + bias;
        output = x_biased / (1.0f + std::abs(x_biased));
        break;
    }
    case 21: // Fuzz Fac
        output = (x > 0.0f ? 1.0f : -1.0f) * (1.0f - std::exp(-std::abs(x * (1.0f + shapeParam * 10.0f))));
        break;
    case 22: // Cheby 3
    {
        float x_limited = juce::jlimit(-1.0f, 1.0f, x);
        output = (4.0f * x_limited * x_limited * x_limited - 3.0f * x_limited) * (0.5f + shapeParam * 0.5f);
        break;
    }
    case 23: // Cheby 5
    {
        float x_limited = juce::jlimit(-1.0f, 1.0f, x);
        output = (16.0f * std::pow(x_limited, 5) - 20.0f * std::pow(x_limited, 3) + 5.0f * x_limited) * (0.5f + shapeParam * 0.5f);
        break;
    }
    case 24: // Log Sat
        output = (x > 0.0f ? 1.0f : -1.0f) * std::log(1.0f + (10.0f + shapeParam * 50.0f) * std::abs(x)) / std::log(11.0f + shapeParam * 50.0f);
        break;
    case 25: // Half Wave
        output = x > 0.0f ? std::tanh(x * (1.0f + shapeParam)) : x;
        break;
    case 26: // Cubic
    {
        float x_scaled = x * (1.0f + shapeParam);
        output = x_scaled - (1.0f / 3.0f) * x_scaled * x_scaled * x_scaled;
        break;
    }
    case 27: // Octaver Sat
        output = (std::abs(x) * 2.0f - 1.0f) * (0.5f + shapeParam * 0.5f);
        break;
    case 28: // Triode
    {
        float mu = 100.0f;
        float kp = 1.2f + shapeParam * 0.8f;
        float vg = x * (1.0f + shapeParam);
        output = (vg > 0.0f) ? std::tanh(vg / kp) * kp : vg / (1.0f + std::abs(vg) * mu * 0.01f);
        break;
    }
    case 29: // Pentode
    {
        float screen = 0.7f + shapeParam * 0.3f;
        float plate = x * (1.5f + shapeParam);
        output = std::tanh(plate * screen) + 0.1f * std::sin(plate * 3.0f) * shapeParam;
        break;
    }
    case 30: // Class A
    {
        float bias = 0.3f * shapeParam;
        float biased = x + bias;
        output = std::tanh(biased * (1.0f + shapeParam)) - bias * 0.5f;
        break;
    }
    case 31: // Class AB
    {
        float threshold = 0.3f - shapeParam * 0.2f;
        if (std::abs(x) < threshold)
        {
            output = x * (1.0f + shapeParam * 2.0f);
        }
        else
        {
            output = (x > 0.0f ? 1.0f : -1.0f) * (threshold + std::tanh((std::abs(x) - threshold) * (2.0f + shapeParam * 3.0f)));
        }
        break;
    }
    case 32: // Class B
    {
        float deadzone = 0.05f + shapeParam * 0.1f;
        if (std::abs(x) < deadzone)
        {
            output = 0.0f;
        }
        else
        {
            output = (x > 0.0f ? 1.0f : -1.0f) * std::tanh((std::abs(x) - deadzone) * (1.0f + shapeParam * 3.0f));
        }
        break;
    }
    case 33: // Germanium
    {
        float temp = 0.8f + shapeParam * 0.4f;
        float biased = x + 0.1f * shapeParam;
        output = (biased > 0.0f ? 1.0f : -1.0f) * (1.0f - std::exp(-std::abs(biased) * temp * 5.0f));
        output *= 0.9f + 0.1f * shapeParam;
        break;
    }
    case 34: // Tape 15ips
    {
        float headroom = 1.2f - shapeParam * 0.3f;
        float compression = std::tanh(x / headroom) * headroom;
        output = compression + 0.05f * x * shapeParam;
        break;
    }
    case 35: // Tape 7.5ips
    {
        float satPoint = 0.6f + shapeParam * 0.3f;
        float warmth = x + 0.15f * x * x * (x > 0.0f ? 1.0f : -1.0f);
        output = std::tanh(warmth / satPoint) * satPoint;
        break;
    }
    case 36: // Tape Cassette
    {
        float hfLoss = 1.0f - shapeParam * 0.4f;
        float saturated = std::tanh(x * (1.0f + shapeParam * 2.0f));
        output = saturated * hfLoss + x * (1.0f - hfLoss) * 0.5f;
        break;
    }
    case 37: // Tape 456
    {
        float hysteresis = x + 0.2f * x * std::abs(x) * shapeParam;
        output = std::tanh(hysteresis * (1.0f + shapeParam * 0.5f));
        break;
    }
    case 38: // Tape SM900
    {
        float modern = std::tanh(x * 1.1f);
        float vintage = x / (1.0f + std::abs(x) * 0.5f);
        output = modern * (1.0f - shapeParam) + vintage * shapeParam;
        break;
    }
    case 39: // Transformer
    {
        float iron = x + 0.3f * std::sin(x * 2.0f) * shapeParam;
        output = std::tanh(iron * (1.0f + shapeParam));
        break;
    }
    case 40: // Console
    {
        float harmonic2 = 0.1f * x * std::abs(x) * shapeParam;
        float harmonic3 = 0.05f * x * x * x * shapeParam;
        output = std::tanh(x + harmonic2 + harmonic3);
        break;
    }
    case 41: // API Style
    {
        float punch = x * (1.0f + shapeParam * 0.5f);
        float clipped = juce::jlimit(-1.0f, 1.0f, punch * 1.5f);
        output = punch * (1.0f - shapeParam * 0.5f) + clipped * shapeParam * 0.5f;
        output = std::tanh(output);
        break;
    }
    case 42: // SSL Style
    {
        float compressed = x / (1.0f + std::abs(x) * shapeParam * 0.5f);
        float harmonic = 0.05f * x * x * x * shapeParam;
        output = compressed + harmonic;
        break;
    }
    case 43: // Silicon
    {
        float gain = 1.0f + shapeParam * 3.0f;
        float clipPoint = 0.8f - shapeParam * 0.2f;
        output = juce::jlimit(-clipPoint, clipPoint, x * gain);
        output = std::tanh(output / clipPoint) * clipPoint;
        break;
    }
    case 44: // FET Clean
    {
        float ratio = 4.0f + shapeParam * 16.0f;
        float threshold = 0.5f;
        if (std::abs(x) > threshold)
        {
            float excess = std::abs(x) - threshold;
            output = (x > 0.0f ? 1.0f : -1.0f) * (threshold + excess / ratio);
        }
        else
        {
            output = x;
        }
        break;
    }
    case 45: // FET Dirty
    {
        float attack = x * (2.0f + shapeParam * 4.0f);
        output = std::tanh(attack) * (0.8f + shapeParam * 0.2f);
        output += 0.1f * std::sin(x * 5.0f) * shapeParam; // Harmonics
        break;
    }
    case 46: // OpAmp
    {
        float gain = 1.0f + shapeParam * 10.0f;
        float clipped = juce::jlimit(-1.0f, 1.0f, x * gain);
        output = clipped * (1.0f - shapeParam * 0.3f) + std::tanh(x * gain) * shapeParam * 0.3f;
        break;
    }
    case 47: // CMOS
    {
        float digital = (x > 0.0f ? 1.0f : -1.0f) * std::pow(std::abs(x), 0.5f);
        float analog = std::tanh(x * (1.0f + shapeParam));
        output = digital * shapeParam + analog * (1.0f - shapeParam);
        break;
    }
    case 48: // Scream
    {
        float scream = x * (3.0f + shapeParam * 7.0f);
        output = std::tanh(scream) + 0.2f * std::sin(scream * 3.0f) * shapeParam;
        output = juce::jlimit(-1.0f, 1.0f, output);
        break;
    }
    case 49: // Buzz
    {
        float buzz = x + 0.3f * std::sin(x * 10.0f * (1.0f + shapeParam * 5.0f));
        output = std::tanh(buzz * (1.0f + shapeParam));
        break;
    }
    case 50: // Crackle
    {
        float noise = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 0.02f * shapeParam;
        output = std::tanh(x * (1.0f + shapeParam * 2.0f)) + noise * std::abs(x);
        break;
    }
    case 51: // Wrap
    {
        float wrapped = x * (1.0f + shapeParam * 3.0f);
        wrapped = std::fmod(wrapped + 3.0f, 2.0f) - 1.0f;
        output = wrapped;
        break;
    }
    case 52: // Density
    {
        float density = std::tanh(x * 2.0f) + std::tanh(x * 0.5f);
        output = density * 0.5f * (1.0f + shapeParam * 0.5f);
        break;
    }
    case 53: // Cheby 7
    {
        float xl = juce::jlimit(-1.0f, 1.0f, x);
        float x2 = xl * xl;
        float x3 = x2 * xl;
        float x5 = x3 * x2;
        float x7 = x5 * x2;
        output = (64.0f * x7 - 112.0f * x5 + 56.0f * x3 - 7.0f * xl) * (0.3f + shapeParam * 0.7f);
        break;
    }
    case 54: // Hyperbolic
    {
        float scale = 0.5f + shapeParam * 1.5f;
        output = std::sinh(x * scale) / std::cosh(x * scale * 2.0f);
        break;
    }
    case 55: // Exponential
    {
        float sign = x > 0.0f ? 1.0f : -1.0f;
        float absX = std::abs(x);
        float rate = 2.0f + shapeParam * 4.0f;
        output = sign * (1.0f - std::exp(-absX * rate));
        break;
    }
    case 56: // Parabolic
    {
        float scaled = x * (1.0f + shapeParam * 2.0f);
        if (std::abs(scaled) < 1.0f)
        {
            output = scaled - (scaled * scaled * scaled) / 3.0f;
        }
        else
        {
            output = (scaled > 0.0f ? 1.0f : -1.0f) * (2.0f / 3.0f);
        }
        break;
    }
    case 57: // Wavelet
    {
        float t = x * (2.0f + shapeParam * 4.0f);
        float t2 = t * t;
        output = (1.0f - t2) * std::exp(-t2 * 0.5f);
        output *= (1.0f + shapeParam);
        break;
    }
    default:
        output = std::tanh(x);
        break;
    }
    return output;
}

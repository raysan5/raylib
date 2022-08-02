/*******************************************************************************************
*
*   reasings - raylib easings library, based on Robert Penner library
*
*   Useful easing functions for values animation
*
*   This header uses:
*       #define REASINGS_STATIC_INLINE      // Inlines all functions code, so it runs faster.
*                                           // This requires lots of memory on system.
*   How to use:
*   The four inputs t,b,c,d are defined as follows:
*   t = current time (in any unit measure, but same unit as duration)
*   b = starting value to interpolate
*   c = the total change in value of b that needs to occur
*   d = total time it should take to complete (duration)
*
*   Example:
*
*   int currentTime = 0;
*   int duration = 100;
*   float startPositionX = 0.0f;
*   float finalPositionX = 30.0f;
*   float currentPositionX = startPositionX;
*
*   while (currentPositionX < finalPositionX)
*   {
*       currentPositionX = EaseSineIn(currentTime, startPositionX, finalPositionX - startPositionX, duration);
*       currentTime++;
*   }
*
*   A port of Robert Penner's easing equations to C (http://robertpenner.com/easing/)
*
*   Robert Penner License
*   ---------------------------------------------------------------------------------
*   Open source under the BSD License.
*
*   Copyright (c) 2001 Robert Penner. All rights reserved.
*
*   Redistribution and use in source and binary forms, with or without modification,
*   are permitted provided that the following conditions are met:
*
*       - Redistributions of source code must retain the above copyright notice,
*         this list of conditions and the following disclaimer.
*       - Redistributions in binary form must reproduce the above copyright notice,
*         this list of conditions and the following disclaimer in the documentation
*         and/or other materials provided with the distribution.
*       - Neither the name of the author nor the names of contributors may be used
*         to endorse or promote products derived from this software without specific
*         prior written permission.
*
*   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
*   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
*   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
*   IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
*   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*   BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
*   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
*   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
*   OF THE POSSIBILITY OF SUCH DAMAGE.
*   ---------------------------------------------------------------------------------
*
*   Copyright (c) 2015-2022 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#ifndef REASINGS_H
#define REASINGS_H

#define REASINGS_STATIC_INLINE     // NOTE: By default, compile functions as static inline

#if defined(REASINGS_STATIC_INLINE)
    #define EASEDEF static inline
#else
    #define EASEDEF extern
#endif

#include <math.h>       // Required for: sinf(), cosf(), sqrtf(), powf()

#ifndef PI
    #define PI 3.14159265358979323846f //Required as PI is not always defined in math.h
#endif

#if defined(__cplusplus)
extern "C" {            // Prevents name mangling of functions
#endif

// Linear Easing functions
EASEDEF float EaseLinearNone(float t, float b, float c, float d) { return (c*t/d + b); }                            // Ease: Linear
EASEDEF float EaseLinearIn(float t, float b, float c, float d) { return (c*t/d + b); }                              // Ease: Linear In
EASEDEF float EaseLinearOut(float t, float b, float c, float d) { return (c*t/d + b); }                             // Ease: Linear Out
EASEDEF float EaseLinearInOut(float t, float b, float c, float d) { return (c*t/d + b); }                           // Ease: Linear In Out

// Sine Easing functions
EASEDEF float EaseSineIn(float t, float b, float c, float d) { return (-c*cosf(t/d*(PI/2.0f)) + c + b); }            // Ease: Sine In
EASEDEF float EaseSineOut(float t, float b, float c, float d) { return (c*sinf(t/d*(PI/2.0f)) + b); }                // Ease: Sine Out
EASEDEF float EaseSineInOut(float t, float b, float c, float d) { return (-c/2.0f*(cosf(PI*t/d) - 1.0f) + b); }      // Ease: Sine Out

// Circular Easing functions
EASEDEF float EaseCircIn(float t, float b, float c, float d) { t /= d; return (-c*(sqrtf(1.0f - t*t) - 1.0f) + b); } // Ease: Circular In
EASEDEF float EaseCircOut(float t, float b, float c, float d) { t = t/d - 1.0f; return (c*sqrtf(1.0f - t*t) + b); }  // Ease: Circular Out
EASEDEF float EaseCircInOut(float t, float b, float c, float d)                                                      // Ease: Circular In Out
{
    if ((t/=d/2.0f) < 1.0f) return (-c/2.0f*(sqrtf(1.0f - t*t) - 1.0f) + b);
    t -= 2.0f; return (c/2.0f*(sqrtf(1.0f - t*t) + 1.0f) + b);
}

// Cubic Easing functions
EASEDEF float EaseCubicIn(float t, float b, float c, float d) { t /= d; return (c*t*t*t + b); }                      // Ease: Cubic In
EASEDEF float EaseCubicOut(float t, float b, float c, float d) { t = t/d - 1.0f; return (c*(t*t*t + 1.0f) + b); }    // Ease: Cubic Out
EASEDEF float EaseCubicInOut(float t, float b, float c, float d)                                                     // Ease: Cubic In Out
{
    if ((t/=d/2.0f) < 1.0f) return (c/2.0f*t*t*t + b);
    t -= 2.0f; return (c/2.0f*(t*t*t + 2.0f) + b);
}

// Quadratic Easing functions
EASEDEF float EaseQuadIn(float t, float b, float c, float d) { t /= d; return (c*t*t + b); }                         // Ease: Quadratic In
EASEDEF float EaseQuadOut(float t, float b, float c, float d) { t /= d; return (-c*t*(t - 2.0f) + b); }              // Ease: Quadratic Out
EASEDEF float EaseQuadInOut(float t, float b, float c, float d)                                                      // Ease: Quadratic In Out
{
    if ((t/=d/2) < 1) return (((c/2)*(t*t)) + b);
    return (-c/2.0f*(((t - 1.0f)*(t - 3.0f)) - 1.0f) + b);
}

// Exponential Easing functions
EASEDEF float EaseExpoIn(float t, float b, float c, float d) { return (t == 0.0f) ? b : (c*powf(2.0f, 10.0f*(t/d - 1.0f)) + b); }       // Ease: Exponential In
EASEDEF float EaseExpoOut(float t, float b, float c, float d) { return (t == d) ? (b + c) : (c*(-powf(2.0f, -10.0f*t/d) + 1.0f) + b); } // Ease: Exponential Out
EASEDEF float EaseExpoInOut(float t, float b, float c, float d)                                                                         // Ease: Exponential In Out
{
    if (t == 0.0f) return b;
    if (t == d) return (b + c);
    if ((t/=d/2.0f) < 1.0f) return (c/2.0f*powf(2.0f, 10.0f*(t - 1.0f)) + b);

    return (c/2.0f*(-powf(2.0f, -10.0f*(t - 1.0f)) + 2.0f) + b);
}

// Back Easing functions
EASEDEF float EaseBackIn(float t, float b, float c, float d) // Ease: Back In
{
    float s = 1.70158f;
    float postFix = t/=d;
    return (c*(postFix)*t*((s + 1.0f)*t - s) + b);
}

EASEDEF float EaseBackOut(float t, float b, float c, float d) // Ease: Back Out
{
    float s = 1.70158f;
    t = t/d - 1.0f;
    return (c*(t*t*((s + 1.0f)*t + s) + 1.0f) + b);
}

EASEDEF float EaseBackInOut(float t, float b, float c, float d) // Ease: Back In Out
{
    float s = 1.70158f;
    if ((t/=d/2.0f) < 1.0f)
    {
        s *= 1.525f;
        return (c/2.0f*(t*t*((s + 1.0f)*t - s)) + b);
    }

    float postFix = t-=2.0f;
    s *= 1.525f;
    return (c/2.0f*((postFix)*t*((s + 1.0f)*t + s) + 2.0f) + b);
}

// Bounce Easing functions
EASEDEF float EaseBounceOut(float t, float b, float c, float d) // Ease: Bounce Out
{
    if ((t/=d) < (1.0f/2.75f))
    {
        return (c*(7.5625f*t*t) + b);
    }
    else if (t < (2.0f/2.75f))
    {
        float postFix = t-=(1.5f/2.75f);
        return (c*(7.5625f*(postFix)*t + 0.75f) + b);
    }
    else if (t < (2.5/2.75))
    {
        float postFix = t-=(2.25f/2.75f);
        return (c*(7.5625f*(postFix)*t + 0.9375f) + b);
    }
    else
    {
        float postFix = t-=(2.625f/2.75f);
        return (c*(7.5625f*(postFix)*t + 0.984375f) + b);
    }
}

EASEDEF float EaseBounceIn(float t, float b, float c, float d) { return (c - EaseBounceOut(d - t, 0.0f, c, d) + b); } // Ease: Bounce In
EASEDEF float EaseBounceInOut(float t, float b, float c, float d) // Ease: Bounce In Out
{
    if (t < d/2.0f) return (EaseBounceIn(t*2.0f, 0.0f, c, d)*0.5f + b);
    else return (EaseBounceOut(t*2.0f - d, 0.0f, c, d)*0.5f + c*0.5f + b);
}

// Elastic Easing functions
EASEDEF float EaseElasticIn(float t, float b, float c, float d) // Ease: Elastic In
{
    if (t == 0.0f) return b;
    if ((t/=d) == 1.0f) return (b + c);

    float p = d*0.3f;
    float a = c;
    float s = p/4.0f;
    float postFix = a*powf(2.0f, 10.0f*(t-=1.0f));

    return (-(postFix*sinf((t*d-s)*(2.0f*PI)/p )) + b);
}

EASEDEF float EaseElasticOut(float t, float b, float c, float d) // Ease: Elastic Out
{
    if (t == 0.0f) return b;
    if ((t/=d) == 1.0f) return (b + c);

    float p = d*0.3f;
    float a = c;
    float s = p/4.0f;

    return (a*powf(2.0f,-10.0f*t)*sinf((t*d-s)*(2.0f*PI)/p) + c + b);
}

EASEDEF float EaseElasticInOut(float t, float b, float c, float d) // Ease: Elastic In Out
{
    if (t == 0.0f) return b;
    if ((t/=d/2.0f) == 2.0f) return (b + c);

    float p = d*(0.3f*1.5f);
    float a = c;
    float s = p/4.0f;

    if (t < 1.0f)
    {
        float postFix = a*powf(2.0f, 10.0f*(t-=1.0f));
        return -0.5f*(postFix*sinf((t*d-s)*(2.0f*PI)/p)) + b;
    }

    float postFix = a*powf(2.0f, -10.0f*(t-=1.0f));

    return (postFix*sinf((t*d-s)*(2.0f*PI)/p)*0.5f + c + b);
}

#if defined(__cplusplus)
}
#endif

#endif // REASINGS_H

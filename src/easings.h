/*******************************************************************************************
*
*   raylib easings (header only file)
*   
*   Useful easing functions for values animation
*
*   This header uses:
*       #define EASINGS_STATIC_INLINE       // Inlines all functions code, so it runs faster.
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
*   Copyright (c) 2015 Ramon Santamaria
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

#ifndef EASINGS_H
#define EASINGS_H

#define EASINGS_STATIC_INLINE     // NOTE: By default, compile functions as static inline

#if defined(EASINGS_STATIC_INLINE)
    #define EASEDEF static inline
#else
    #define EASEDEF extern
#endif

#include <math.h>       // Required for: sin(), cos(), sqrt(), pow()

#ifndef PI
    #define PI 3.14159265358979323846f //Required as PI is not always defined in math.h
#endif

#ifdef __cplusplus
extern "C" {            // Prevents name mangling of functions
#endif

// Linear Easing functions
EASEDEF float EaseLinearNone(float t, float b, float c, float d) { return (c*t/d + b); }
EASEDEF float EaseLinearIn(float t, float b, float c, float d) { return (c*t/d + b); }
EASEDEF float EaseLinearOut(float t, float b, float c, float d) { return (c*t/d + b); }
EASEDEF float EaseLinearInOut(float t,float b, float c, float d) { return (c*t/d + b); }

// Sine Easing functions
EASEDEF float EaseSineIn(float t, float b, float c, float d) { return (-c*cos(t/d*(PI/2)) + c + b); }
EASEDEF float EaseSineOut(float t, float b, float c, float d) { return (c*sin(t/d*(PI/2)) + b); }
EASEDEF float EaseSineInOut(float t, float b, float c, float d) { return (-c/2*(cos(PI*t/d) - 1) + b); }

// Circular Easing functions
EASEDEF float EaseCircIn(float t, float b, float c, float d) { t /= d; return (-c*(sqrt(1 - t*t) - 1) + b); }
EASEDEF float EaseCircOut(float t, float b, float c, float d) { t = t/d - 1; return (c*sqrt(1 - t*t) + b); }
EASEDEF float EaseCircInOut(float t, float b, float c, float d) 
{
    if ((t/=d/2) < 1) return (-c/2*(sqrt(1 - t*t) - 1) + b);
    t -= 2; return (c/2*(sqrt(1 - t*t) + 1) + b);
}

// Cubic Easing functions
EASEDEF float EaseCubicIn(float t, float b, float c, float d) { t /= d; return (c*t*t*t + b); }
EASEDEF float EaseCubicOut(float t, float b, float c, float d) { t = t/d-1; return (c*(t*t*t + 1) + b); }
EASEDEF float EaseCubicInOut(float t, float b, float c, float d) 
{ 
    if ((t/=d/2) < 1) return (c/2*t*t*t + b);
    t -= 2; return (c/2*(t*t*t + 2) + b);
}

// Quadratic Easing functions
EASEDEF float EaseQuadIn(float t, float b, float c, float d) { t /= d; return (c*t*t + b); }
EASEDEF float EaseQuadOut(float t, float b, float c, float d) { t /= d; return (-c*t*(t - 2) + b); }
EASEDEF float EaseQuadInOut(float t, float b, float c, float d) 
{
    if ((t/=d/2) < 1) return (((c/2)*(t*t)) + b);
    t--; return (-c/2*(((t - 2)*t) - 1) + b);
}

// Exponential Easing functions
EASEDEF float EaseExpoIn(float t, float b, float c, float d) { return (t == 0) ? b : (c*pow(2, 10*(t/d - 1)) + b); }
EASEDEF float EaseExpoOut(float t, float b, float c, float d) { return (t == d) ? (b + c) : (c*(-pow(2, -10*t/d) + 1) + b);    }
EASEDEF float EaseExpoInOut(float t, float b, float c, float d) 
{
    if (t == 0) return b;
    if (t == d) return (b + c);
    if ((t/=d/2) < 1) return (c/2*pow(2, 10*(t - 1)) + b);
    
    return (c/2*(-pow(2, -10*--t) + 2) + b);
}

// Back Easing functions
EASEDEF float EaseBackIn(float t, float b, float c, float d) 
{
    float s = 1.70158f;
    float postFix = t/=d;
    return (c*(postFix)*t*((s + 1)*t - s) + b);
}

EASEDEF float EaseBackOut(float t, float b, float c, float d)
{    
    float s = 1.70158f;
    t = t/d - 1;
    return (c*(t*t*((s + 1)*t + s) + 1) + b);
}

EASEDEF float EaseBackInOut(float t, float b, float c, float d)
{
    float s = 1.70158f;
    if ((t/=d/2) < 1) 
    {
        s *= 1.525f;
        return (c/2*(t*t*((s + 1)*t - s)) + b);
    }
    
    float postFix = t-=2;
    s *= 1.525f;
    return (c/2*((postFix)*t*((s + 1)*t + s) + 2) + b);
}

// Bounce Easing functions
EASEDEF float EaseBounceOut(float t, float b, float c, float d) 
{
    if ((t/=d) < (1/2.75f)) 
    {
        return (c*(7.5625f*t*t) + b);
    } 
    else if (t < (2/2.75f)) 
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

EASEDEF float EaseBounceIn(float t, float b, float c, float d) { return (c - EaseBounceOut(d-t, 0, c, d) + b); }
EASEDEF float EaseBounceInOut(float t, float b, float c, float d) 
{
    if (t < d/2) return (EaseBounceIn(t*2, 0, c, d)*0.5f + b);
    else return (EaseBounceOut(t*2-d, 0, c, d)*0.5f + c*0.5f + b);
}

// Elastic Easing functions
EASEDEF float EaseElasticIn(float t, float b, float c, float d) 
{
    if (t == 0) return b;
    if ((t/=d) == 1) return (b + c);
    
    float p = d*0.3f;
    float a = c; 
    float s = p/4;
    float postFix = a*pow(2, 10*(t-=1));
    
    return (-(postFix*sin((t*d-s)*(2*PI)/p )) + b);
}

EASEDEF float EaseElasticOut(float t, float b, float c, float d)
{
    if (t == 0) return b;
    if ((t/=d) == 1) return (b + c);
    
    float p = d*0.3f;
    float a = c; 
    float s = p/4;
    
    return (a*pow(2,-10*t)*sin((t*d-s)*(2*PI)/p) + c + b);    
}

EASEDEF float EaseElasticInOut(float t, float b, float c, float d)
{
    if (t == 0) return b;
    if ((t/=d/2) == 2) return (b + c);
    
    float p = d*(0.3f*1.5f);
    float a = c;
    float s = p/4;

    if (t < 1) 
    {
        float postFix = a*pow(2, 10*(t-=1));
        return -0.5f*(postFix*sin((t*d-s)*(2*PI)/p)) + b;
    }
    
    float postFix = a*pow(2, -10*(t-=1));
    
    return (postFix*sin((t*d-s)*(2*PI)/p)*0.5f + c + b);
}

#ifdef __cplusplus
}
#endif

#endif // EASINGS_H

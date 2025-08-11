#version 100

precision mediump float;

#extension GL_OES_standard_derivatives : enable

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform vec3 viewEye;
uniform vec3 viewCenter;
uniform float runTime;
uniform vec2 resolution;

// The MIT License
// Copyright © 2013 Inigo Quilez
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// A list of useful distance function to simple primitives, and an example on how to
// do some interesting boolean operations, repetition and displacement
//
// More info here: http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm

#define AA 1   // make this 1 if your machine is too slow

//------------------------------------------------------------------

float sdPlane(vec3 p)
{
    return p.y;
}

float sdSphere(vec3 p, float s)
{
    return length(p)-s;
}

float sdBox(vec3 p, vec3 b)
{
    vec3 d = abs(p) - b;
    return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
}

float sdEllipsoid(in vec3 p, in vec3 r)
{
    return (length(p/r) - 1.0)*min(min(r.x,r.y),r.z);
}

float udRoundBox(vec3 p, vec3 b, float r)
{
    return length(max(abs(p)-b,0.0))-r;
}

float sdTorus(vec3 p, vec2 t)
{
    return length(vec2(length(p.xz)-t.x,p.y))-t.y;
}

float sdHexPrism(vec3 p, vec2 h)
{
    vec3 q = abs(p);
#if 0
    return max(q.z-h.y,max((q.x*0.866025+q.y*0.5),q.y)-h.x);
#else
    float d1 = q.z-h.y;
    float d2 = max((q.x*0.866025+q.y*0.5),q.y)-h.x;
    return length(max(vec2(d1,d2),0.0)) + min(max(d1,d2), 0.);
#endif
}

float sdCapsule(vec3 p, vec3 a, vec3 b, float r)
{
    vec3 pa = p-a, ba = b-a;
    float h = clamp(dot(pa,ba)/dot(ba,ba), 0.0, 1.0);
    return length(pa - ba*h) - r;
}

float sdEquilateralTriangle( in vec2 p)
{
    const float k = sqrt(3.0);
    p.x = abs(p.x) - 1.0;
    p.y = p.y + 1.0/k;
    if (p.x + k*p.y > 0.0) p = vec2(p.x - k*p.y, -k*p.x - p.y)/2.0;
    p.x += 2.0 - 2.0*clamp((p.x+2.0)/2.0, 0.0, 1.0);
    return -length(p)*sign(p.y);
}

float sdTriPrism(vec3 p, vec2 h)
{
    vec3 q = abs(p);
    float d1 = q.z-h.y;
#if 1
    // distance bound
    float d2 = max(q.x*0.866025+p.y*0.5,-p.y)-h.x*0.5;
#else
    // correct distance
    h.x *= 0.866025;
    float d2 = sdEquilateralTriangle(p.xy/h.x)*h.x;
#endif
    return length(max(vec2(d1,d2),0.0)) + min(max(d1,d2), 0.);
}

float sdCylinder(vec3 p, vec2 h)
{
  vec2 d = abs(vec2(length(p.xz),p.y)) - h;
  return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

float sdCone(in vec3 p, in vec3 c)
{
    vec2 q = vec2(length(p.xz), p.y);
    float d1 = -q.y-c.z;
    float d2 = max(dot(q,c.xy), q.y);
    return length(max(vec2(d1,d2),0.0)) + min(max(d1,d2), 0.);
}

float sdConeSection(in vec3 p, in float h, in float r1, in float r2)
{
    float d1 = -p.y - h;
    float q = p.y - h;
    float si = 0.5*(r1-r2)/h;
    float d2 = max(sqrt(dot(p.xz,p.xz)*(1.0-si*si)) + q*si - r2, q);
    return length(max(vec2(d1,d2),0.0)) + min(max(d1,d2), 0.);
}

float sdPryamid4(vec3 p, vec3 h) // h = { cos a, sin a, height }
{
    // Tetrahedron = Octahedron - Cube
    float box = sdBox(p - vec3(0,-2.0*h.z,0), vec3(2.0*h.z));

    float d = 0.0;
    d = max(d, abs(dot(p, vec3(-h.x, h.y, 0))));
    d = max(d, abs(dot(p, vec3( h.x, h.y, 0))));
    d = max(d, abs(dot(p, vec3( 0, h.y, h.x))));
    d = max(d, abs(dot(p, vec3( 0, h.y,-h.x))));
    float octa = d - h.z;
    return max(-box,octa); // Subtraction
 }

float length2(vec2 p)
{
    return sqrt(p.x*p.x + p.y*p.y);
}

float length6(vec2 p)
{
    p = p*p*p; p = p*p;
    return pow(p.x + p.y, 1.0/6.0);
}

float length8(vec2 p)
{
    p = p*p; p = p*p; p = p*p;
    return pow(p.x + p.y, 1.0/8.0);
}

float sdTorus82(vec3 p, vec2 t)
{
    vec2 q = vec2(length2(p.xz)-t.x,p.y);
    return length8(q)-t.y;
}

float sdTorus88(vec3 p, vec2 t)
{
    vec2 q = vec2(length8(p.xz)-t.x,p.y);
    return length8(q)-t.y;
}

float sdCylinder6(vec3 p, vec2 h)
{
    return max(length6(p.xz)-h.x, abs(p.y)-h.y);
}

//------------------------------------------------------------------

float opS(float d1, float d2)
{
    return max(-d2,d1);
}

vec2 opU(vec2 d1, vec2 d2)
{
    return (d1.x<d2.x) ? d1 : d2;
}

vec3 opRep(vec3 p, vec3 c)
{
    return mod(p,c)-0.5*c;
}

vec3 opTwist(vec3 p)
{
    float  c = cos(10.0*p.y+10.0);
    float  s = sin(10.0*p.y+10.0);
    mat2   m = mat2(c,-s,s,c);
    return vec3(m*p.xz,p.y);
}

//------------------------------------------------------------------

vec2 map(in vec3 pos)
{
    vec2 res = opU(vec2(sdPlane(    pos), 1.0),
                    vec2(sdSphere(   pos-vec3(0.0,0.25, 0.0), 0.25), 46.9));
    res = opU(res, vec2(sdBox(      pos-vec3(1.0,0.25, 0.0), vec3(0.25)), 3.0));
    res = opU(res, vec2(udRoundBox( pos-vec3(1.0,0.25, 1.0), vec3(0.15), 0.1), 41.0));
    res = opU(res, vec2(sdTorus(    pos-vec3(0.0,0.25, 1.0), vec2(0.20,0.05)), 25.0));
    res = opU(res, vec2(sdCapsule(  pos,vec3(-1.3,0.10,-0.1), vec3(-0.8,0.50,0.2), 0.1 ), 31.9));
    res = opU(res, vec2(sdTriPrism( pos-vec3(-1.0,0.25,-1.0), vec2(0.25,0.05)),43.5));
    res = opU(res, vec2(sdCylinder( pos-vec3(1.0,0.30,-1.0), vec2(0.1,0.2)), 8.0));
    res = opU(res, vec2(sdCone(     pos-vec3(0.0,0.50,-1.0), vec3(0.8,0.6,0.3)), 55.0));
    res = opU(res, vec2(sdTorus82(  pos-vec3(0.0,0.25, 2.0), vec2(0.20,0.05)),50.0));
    res = opU(res, vec2(sdTorus88(  pos-vec3(-1.0,0.25, 2.0), vec2(0.20,0.05)),43.0));
    res = opU(res, vec2(sdCylinder6(pos-vec3(1.0,0.30, 2.0), vec2(0.1,0.2)), 12.0));
    res = opU(res, vec2(sdHexPrism( pos-vec3(-1.0,0.20, 1.0), vec2(0.25,0.05)),17.0));
    res = opU(res, vec2(sdPryamid4( pos-vec3(-1.0,0.15,-2.0), vec3(0.8,0.6,0.25)),37.0));
    res = opU(res, vec2(opS(udRoundBox( pos-vec3(-2.0,0.2, 1.0), vec3(0.15),0.05),
                               sdSphere(   pos-vec3(-2.0,0.2, 1.0), 0.25)), 13.0));
    res = opU(res, vec2(opS(sdTorus82( pos-vec3(-2.0,0.2, 0.0), vec2(0.20,0.1)),
                               sdCylinder( opRep(vec3(atan(pos.x+2.0,pos.z)/6.2831, pos.y, 0.02+0.5*length(pos-vec3(-2.0,0.2, 0.0))), vec3(0.05,1.0,0.05)), vec2(0.02,0.6))), 51.0));
    res = opU(res, vec2(0.5*sdSphere(   pos-vec3(-2.0,0.25,-1.0), 0.2) + 0.03*sin(50.0*pos.x)*sin(50.0*pos.y)*sin(50.0*pos.z), 65.0));
    res = opU(res, vec2(0.5*sdTorus(opTwist(pos-vec3(-2.0,0.25, 2.0)),vec2(0.20,0.05)), 46.7));
    res = opU(res, vec2(sdConeSection(pos-vec3(0.0,0.35,-2.0), 0.15, 0.2, 0.1), 13.67));
    res = opU(res, vec2(sdEllipsoid(pos-vec3(1.0,0.35,-2.0), vec3(0.15, 0.2, 0.05)), 43.17));

    return res;
}

vec2 castRay(in vec3 ro, in vec3 rd)
{
    float tmin = 0.2;
    float tmax = 30.0;

#if 1
    // bounding volume
    float tp1 = (0.0-ro.y)/rd.y; if (tp1>0.0) tmax = min(tmax, tp1);
    float tp2 = (1.6-ro.y)/rd.y; if (tp2>0.0) { if (ro.y>1.6) tmin = max(tmin, tp2);
                                                 else           tmax = min(tmax, tp2); }
#endif

    float t = tmin;
    float m = -1.0;
    for (int i=0; i<64; i++)
    {
        float precis = 0.0005*t;
        vec2 res = map(ro+rd*t);
        if (res.x<precis || t>tmax) break;
        t += res.x;
        m = res.y;
    }

    if (t>tmax) m=-1.0;
    return vec2(t, m);
}


float calcSoftshadow(in vec3 ro, in vec3 rd, in float mint, in float tmax)
{
    float res = 1.0;
    float t = mint;
    for (int i=0; i<16; i++)
    {
        float h = map(ro + rd*t).x;
        res = min(res, 8.0*h/t);
        t += clamp(h, 0.02, 0.10);
        if (h<0.001 || t>tmax) break;
    }
    return clamp(res, 0.0, 1.0);
}

vec3 calcNormal(in vec3 pos)
{
    vec2 e = vec2(1.0,-1.0)*0.5773*0.0005;
    return normalize(e.xyy*map(pos + e.xyy).x +
                      e.yyx*map(pos + e.yyx).x +
                      e.yxy*map(pos + e.yxy).x +
                      e.xxx*map(pos + e.xxx).x);
    /*
    vec3 eps = vec3(0.0005, 0.0, 0.0);
    vec3 nor = vec3(
        map(pos+eps.xyy).x - map(pos-eps.xyy).x,
        map(pos+eps.yxy).x - map(pos-eps.yxy).x,
        map(pos+eps.yyx).x - map(pos-eps.yyx).x);
    return normalize(nor);
    */
}

float calcAO(in vec3 pos, in vec3 nor)
{
    float occ = 0.0;
    float sca = 1.0;
    for (int i=0; i<5; i++)
    {
        float hr = 0.01 + 0.12*float(i)/4.0;
        vec3 aopos =  nor*hr + pos;
        float dd = map(aopos).x;
        occ += -(dd-hr)*sca;
        sca *= 0.95;
    }
    return clamp(1.0 - 3.0*occ, 0.0, 1.0);
}

// http://iquilezles.org/www/articles/checkerfiltering/checkerfiltering.htm
float checkersGradBox(in vec2 p)
{
    // filter kernel
    vec2 w = fwidth(p) + 0.001;
    // analytical integral (box filter)
    vec2 i = 2.0*(abs(fract((p-0.5*w)*0.5)-0.5)-abs(fract((p+0.5*w)*0.5)-0.5))/w;
    // xor pattern
    return 0.5 - 0.5*i.x*i.y;
}

vec3 render(in vec3 ro, in vec3 rd)
{
    vec3 col = vec3(0.7, 0.9, 1.0) +rd.y*0.8;
    vec2 res = castRay(ro,rd);
    float t = res.x;
    float m = res.y;
    if (m>-0.5)
    {
        vec3 pos = ro + t*rd;
        vec3 nor = calcNormal(pos);
        vec3 ref = reflect(rd, nor);

        // material
        col = 0.45 + 0.35*sin(vec3(0.05,0.08,0.10)*(m-1.0));
        if (m<1.5)
        {

            float f = checkersGradBox(5.0*pos.xz);
            col = 0.3 + f*vec3(0.1);
        }

        // lighting
        float occ = calcAO(pos, nor);
        vec3  lig = normalize(vec3(cos(-0.4*runTime), sin(0.7*runTime), -0.6));
        vec3  hal = normalize(lig-rd);
        float amb = clamp(0.5+0.5*nor.y, 0.0, 1.0);
        float dif = clamp(dot(nor, lig), 0.0, 1.0);
        float bac = clamp(dot(nor, normalize(vec3(-lig.x,0.0,-lig.z))), 0.0, 1.0)*clamp(1.0-pos.y,0.0,1.0);
        float dom = smoothstep(-0.1, 0.1, ref.y);
        float fre = pow(clamp(1.0+dot(nor,rd),0.0,1.0), 2.0);

        dif *= calcSoftshadow(pos, lig, 0.02, 2.5);
        dom *= calcSoftshadow(pos, ref, 0.02, 2.5);

        float spe = pow(clamp(dot(nor, hal), 0.0, 1.0),16.0)*
                    dif *
                    (0.04 + 0.96*pow(clamp(1.0+dot(hal,rd),0.0,1.0), 5.0));

        vec3 lin = vec3(0.0);
        lin += 1.30*dif*vec3(1.00,0.80,0.55);
        lin += 0.40*amb*vec3(0.40,0.60,1.00)*occ;
        lin += 0.50*dom*vec3(0.40,0.60,1.00)*occ;
        lin += 0.50*bac*vec3(0.25,0.25,0.25)*occ;
        lin += 0.25*fre*vec3(1.00,1.00,1.00)*occ;
        col = col*lin;
        col += 10.00*spe*vec3(1.00,0.90,0.70);

        col = mix(col, vec3(0.8,0.9,1.0), 1.0-exp(-0.0002*t*t*t));
    }

    return vec3(clamp(col,0.0,1.0));
}

mat3 setCamera(in vec3 ro, in vec3 ta, float cr)
{
    vec3 cw = normalize(ta-ro);
    vec3 cp = vec3(sin(cr), cos(cr),0.0);
    vec3 cu = normalize(cross(cw,cp));
    vec3 cv = normalize(cross(cu,cw));
    return mat3(cu, cv, cw);
}

void main()
{
    vec3 tot = vec3(0.0);
#if AA>1
    for (int m=0; m<AA; m++)
    for (int n=0; n<AA; n++)
    {
        // pixel coordinates
        vec2 o = vec2(float(m),float(n))/float(AA) - 0.5;
        vec2 p = (-resolution.xy + 2.0*(gl_FragCoord.xy+o))/resolution.y;
#else
        vec2 p = (-resolution.xy + 2.0*gl_FragCoord.xy)/resolution.y;
#endif

        // RAY: Camera is provided from raylib
        //vec3 ro = vec3(-0.5+3.5*cos(0.1*time + 6.0*mo.x), 1.0 + 2.0*mo.y, 0.5 + 4.0*sin(0.1*time + 6.0*mo.x));

        vec3 ro = viewEye;
        vec3 ta = viewCenter;

        // camera-to-world transformation
        mat3 ca = setCamera(ro, ta, 0.0);
        // ray direction
        vec3 rd = ca*normalize(vec3(p.xy,2.0));

        // render
        vec3 col = render(ro, rd);

        // gamma
        col = pow(col, vec3(0.4545));

        tot += col;
#if AA>1
    }
    tot /= float(AA*AA);
#endif

    gl_FragColor = vec4(tot, 1.0);
}

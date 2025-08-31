#version 100

#extension GL_EXT_frag_depth : enable           //Extension required for writing depth
#extension GL_OES_standard_derivatives : enable //Extension used for fwidth()
precision mediump float;                // Precision required for OpenGL ES2 (WebGL)

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Custom Input Uniform
uniform vec3 camPos;
uniform vec3 camDir;
uniform vec2 screenCenter;

#define ZERO 0

// SRC: https://learnopengl.com/Advanced-OpenGL/Depth-testing
float CalcDepth(in vec3 rd, in float Idist)
{
    float local_z = dot(normalize(camDir),rd)*Idist;
    return (1.0/(local_z) - 1.0/0.01)/(1.0/1000.0 -1.0/0.01);
}

// SRC: https://iquilezles.org/articles/distfunctions/
float sdHorseshoe(in vec3 p, in vec2 c, in float r, in float le, vec2 w)
{
    p.x = abs(p.x);
    float l = length(p.xy);
    p.xy = mat2(-c.x, c.y, 
              c.y, c.x)*p.xy;
    p.xy = vec2((p.y>0.0 || p.x>0.0)?p.x:l*sign(-c.x),
                (p.x>0.0)?p.y:l);
    p.xy = vec2(p.x,abs(p.y-r))-vec2(le,0.0);
    
    vec2 q = vec2(length(max(p.xy,0.0)) + min(0.0,max(p.x,p.y)),p.z);
    vec2 d = abs(q) - w;
    return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}

// r = sphere's radius
// h = cutting's plane's position
// t = thickness
float sdSixWayCutHollowSphere(vec3 p, float r, float h, float t)
{
    // Six way symetry Transformation
    vec3 ap = abs(p);
    if (ap.x < max(ap.y, ap.z)){
        if (ap.y < ap.z) ap.xz = ap.zx;
        else ap.xy = ap.yx;
    }

    vec2 q = vec2(length(ap.yz), ap.x);
    
    float w = sqrt(r*r-h*h);
    
    return ((h*q.x<w*q.y) ? length(q-vec2(w,h)) : abs(length(q)-r)) - t;
}

// SRC: https://iquilezles.org/articles/boxfunctions
vec2 iBox(in vec3 ro, in vec3 rd, in vec3 rad) 
{
    vec3 m = 1.0/rd;
    vec3 n = m*ro;
    vec3 k = abs(m)*rad;
    vec3 t1 = -n - k;
    vec3 t2 = -n + k;

    return vec2(max(max(t1.x, t1.y), t1.z),
                 min(min(t2.x, t2.y), t2.z));
}

vec2 opU(vec2 d1, vec2 d2)
{
    return (d1.x<d2.x) ? d1 : d2;
}

vec2 map(in vec3 pos)
{
    vec2 res = vec2(sdHorseshoe(pos-vec3(-1.0,0.08, 1.0), vec2(cos(1.3),sin(1.3)), 0.2, 0.3, vec2(0.03,0.5)), 11.5) ;
    res = opU(res, vec2(sdSixWayCutHollowSphere(pos-vec3(0.0, 1.0, 0.0), 4.0, 3.5, 0.5), 4.5)) ;

    return res;
}

// SRC: https://www.shadertoy.com/view/Xds3zN
vec2 raycast(in vec3 ro, in vec3 rd)
{
    vec2 res = vec2(-1.0,-1.0);

    float tmin = 1.0;
    float tmax = 20.0;

    // Raytrace floor plane
    float tp1 = (-ro.y)/rd.y;
    if (tp1>0.0)
    {
        tmax = min(tmax, tp1);
        res = vec2(tp1, 1.0);
    }

    float t = tmin;
    for (int i=0; i<70 ; i++)
    {
        if (t>tmax) break;
        vec2 h = map(ro+rd*t);
        if (abs(h.x) < (0.0001*t))
        { 
            res = vec2(t,h.y); 
            break;
        }
        t += h.x;
    }

    return res;
}


// https://iquilezles.org/articles/rmshadows
float calcSoftshadow(in vec3 ro, in vec3 rd, in float mint, in float tmax)
{
    // bounding volume
    float tp = (0.8-ro.y)/rd.y; if (tp>0.0) tmax = min(tmax, tp);

    float res = 1.0;
    float t = mint;
    for (int i=ZERO; i<24; i++)
    {
        float h = map(ro + rd*t).x;
        float s = clamp(8.0*h/t,0.0,1.0);
        res = min(res, s);
        t += clamp(h, 0.01, 0.2);
        if (res<0.004 || t>tmax) break;
    }
    res = clamp(res, 0.0, 1.0);
    return res*res*(3.0-2.0*res);
}


// https://iquilezles.org/articles/normalsSDF
vec3 calcNormal(in vec3 pos)
{
    vec2 e = vec2(1.0,-1.0)*0.5773*0.0005;
    return normalize(e.xyy*map(pos + e.xyy).x + 
                     e.yyx*map(pos + e.yyx).x + 
                     e.yxy*map(pos + e.yxy).x + 
                     e.xxx*map(pos + e.xxx).x);
}

// https://iquilezles.org/articles/nvscene2008/rwwtt.pdf
float calcAO(in vec3 pos, in vec3 nor)
{
    float occ = 0.0;
    float sca = 1.0;
    for (int i=ZERO; i<5; i++)
    {
        float h = 0.01 + 0.12*float(i)/4.0;
        float d = map(pos + h*nor).x;
        occ += (h-d)*sca;
        sca *= 0.95;
        if (occ>0.35) break;
    }
    return clamp(1.0 - 3.0*occ, 0.0, 1.0)*(0.5+0.5*nor.y);
}

// https://iquilezles.org/articles/checkerfiltering
float checkersGradBox(in vec2 p)
{
    // filter kernel
    vec2 w = fwidth(p) + 0.001;
    // analytical integral (box filter)
    vec2 i = 2.0*(abs(fract((p-0.5*w)*0.5)-0.5)-abs(fract((p+0.5*w)*0.5)-0.5))/w;
    // xor pattern
    return 0.5 - 0.5*i.x*i.y;                  
}

// https://www.shadertoy.com/view/tdS3DG
vec4 render(in vec3 ro, in vec3 rd)
{ 
    // background
    vec3 col = vec3(0.7, 0.7, 0.9) - max(rd.y,0.0)*0.3;
    
    // raycast scene
    vec2 res = raycast(ro,rd);
    float t = res.x;
    float m = res.y;
    if (m>-0.5)
    {
        vec3 pos = ro + t*rd;
        vec3 nor = (m<1.5) ? vec3(0.0,1.0,0.0) : calcNormal(pos);
        vec3 ref = reflect(rd, nor);
        
        // material        
        col = 0.2 + 0.2*sin(m*2.0 + vec3(0.0,1.0,2.0));
        float ks = 1.0;
        
        if (m<1.5)
        {
            float f = checkersGradBox(3.0*pos.xz);
            col = 0.15 + f*vec3(0.05);
            ks = 0.4;
        }

        // lighting
        float occ = calcAO(pos, nor);
        
        vec3 lin = vec3(0.0);

        // sun
        {
            vec3  lig = normalize(vec3(-0.5, 0.4, -0.6));
            vec3  hal = normalize(lig-rd);
            float dif = clamp(dot(nor, lig), 0.0, 1.0);
            //if (dif>0.0001)
                dif *= calcSoftshadow(pos, lig, 0.02, 2.5);
            float spe = pow(clamp(dot(nor, hal), 0.0, 1.0),16.0);
                  spe *= dif;
                  spe *= 0.04+0.96*pow(clamp(1.0-dot(hal,lig),0.0,1.0),5.0);
                //spe *= 0.04+0.96*pow(clamp(1.0-sqrt(0.5*(1.0-dot(rd,lig))),0.0,1.0),5.0);
            lin += col*2.20*dif*vec3(1.30,1.00,0.70);
            lin +=     5.00*spe*vec3(1.30,1.00,0.70)*ks;
        }
        // sky
        {
            float dif = sqrt(clamp(0.5+0.5*nor.y, 0.0, 1.0));
                  dif *= occ;
            float spe = smoothstep(-0.2, 0.2, ref.y);
                  spe *= dif;
                  spe *= 0.04+0.96*pow(clamp(1.0+dot(nor,rd),0.0,1.0), 5.0);
          //if (spe>0.001)
                  spe *= calcSoftshadow(pos, ref, 0.02, 2.5);
            lin += col*0.60*dif*vec3(0.40,0.60,1.15);
            lin +=     2.00*spe*vec3(0.40,0.60,1.30)*ks;
        }
        // back
        {
            float dif = clamp(dot(nor, normalize(vec3(0.5,0.0,0.6))), 0.0, 1.0)*clamp(1.0-pos.y,0.0,1.0);
                  dif *= occ;
            lin += col*0.55*dif*vec3(0.25,0.25,0.25);
        }
        // sss
        {
            float dif = pow(clamp(1.0+dot(nor,rd),0.0,1.0),2.0);
                  dif *= occ;
            lin += col*0.25*dif*vec3(1.00,1.00,1.00);
        }
        
        col = lin;

        col = mix(col, vec3(0.7,0.7,0.9), 1.0-exp(-0.0001*t*t*t));
    }

    return vec4(vec3(clamp(col,0.0,1.0)),t);
}

vec3 CalcRayDir(vec2 nCoord){
    vec3 horizontal = normalize(cross(camDir,vec3(.0 , 1.0, .0)));
    vec3 vertical   = normalize(cross(horizontal,camDir));
    return normalize(camDir + horizontal*nCoord.x + vertical*nCoord.y);
}

mat3 setCamera()
{
    vec3 cw = normalize(camDir);
    vec3 cp = vec3(0.0, 1.0 ,0.0);
    vec3 cu = normalize(cross(cw,cp));
    vec3 cv =          (cross(cu,cw));
    return mat3(cu, cv, cw);
}

void main()
{
    vec2 nCoord = (gl_FragCoord.xy - screenCenter.xy)/screenCenter.y;
    mat3 ca = setCamera();

    // focal length
    float fl = length(camDir);
    vec3 rd = ca*normalize(vec3(nCoord,fl));
    vec3 color = vec3(nCoord/2.0 + 0.5, 0.0);
    float depth = gl_FragCoord.z;
    {
        vec4 res = render(camPos - vec3(0.0, 0.0, 0.0) , rd);
        color = res.xyz;
        depth = CalcDepth(rd,res.w);
    }
    gl_FragColor = vec4(color , 1.0);
    gl_FragDepthEXT = depth;
}
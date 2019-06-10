#version 100

precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Custom variables
#define PI 3.14159265358979323846
uniform float uTime = 0.0;

float divisions = 5.0;
float angle = 0.0;

vec2 VectorRotateTime(vec2 v, float speed)
{
    float time = uTime*speed;
    float localTime = fract(time);  // The time domain this works on is 1 sec.
    
    if ((localTime >= 0.0) && (localTime < 0.25)) angle = 0.0;
    else if ((localTime >= 0.25) && (localTime < 0.50)) angle = PI/4*sin(2*PI*localTime - PI/2);
    else if ((localTime >= 0.50) && (localTime < 0.75)) angle = PI*0.25;
    else if ((localTime >= 0.75) && (localTime < 1.00)) angle = PI/4*sin(2*PI*localTime);
    
    // Rotate vector by angle
    v -= 0.5;
    v =  mat2(cos(angle), -sin(angle), sin(angle), cos(angle))*v;
    v += 0.5;
    
    return v;
}

float Rectangle(in vec2 st, in float size, in float fill) 
{
  float roundSize = 0.5 - size/2.0;
  float left = step(roundSize, st.x);
  float top = step(roundSize, st.y);
  float bottom = step(roundSize, 1.0 - st.y);
  float right = step(roundSize, 1.0 - st.x);

  return (left*bottom*right*top)*fill;
}

void main()
{ 
    vec2 fragPos = fragTexCoord;
    fragPos.xy += uTime/9.0;

    fragPos *= divisions;
    vec2 ipos = floor(fragPos);  // Get the integer coords
    vec2 fpos = fract(fragPos);  // Get the fractional coords

    fpos = VectorRotateTime(fpos, 0.2);
    
    float alpha = Rectangle(fpos, 0.216, 1.0);
    vec3 color = vec3(0.3, 0.3, 0.3);

    gl_FragColor = vec4(color, alpha);
}
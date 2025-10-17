#version 120

#define PI 3.1415926535897932384626433832795

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

uniform vec2 offset;            // Offset of the scale
uniform float zoom;             // Zoom of the scale
// NOTE: Maximum number of shader for-loop iterations depend on GPU,
// for example, on RasperryPi for this examply only supports up to 60
uniform int maxIterations;      // Max iterations per pixel

const float max = 4.0;          // We consider infinite as 4.0: if a point reaches a distance of 4.0 it will escape to infinity

float modI(float a, float b) {
    float m = a - floor((a + 0.5)/b)*b;
    return floor(m + 0.5);
}

void main()
{
    // The pixel coordinates are scaled so they are on the mandelbrot scale
    // NOTE: fragTexCoord already comes as normalized screen coordinates but offset must be normalized before scaling and zoom
    vec2 z = vec2((fragTexCoord.x - 0.5)*2.5, (fragTexCoord.y - 0.5)*1.5)/zoom;
    z.x += offset.x;
    z.y += offset.y;
    float a = z.x;
    float b = z.y;
    float absOld = 0.0;
    float convergeNumber = float(maxIterations);

    int iter = 0;
    while (iter < maxIterations)
    {
        float aa = a*a;
        float bb = b*b;
        float abs = sqrt(aa + bb);
        if (abs > max)
        {
            convergeNumber = float(iter) + (max - absOld)/(abs - absOld);
            break;
        }

        float twoab = 2.0*a*b;
        a = aa - bb + z.x;
        b = twoab + z.y;

        absOld = abs;
        ++iter;
    }

    if (iter >= maxIterations)
    {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else
    {
        float normR = modI(convergeNumber, 55.0)/55.0;
        float normG = modI(convergeNumber, 69.0)/69.0;
        float normB = modI(convergeNumber, 40.0)/40.0;

        gl_FragColor = vec4(sin(normR*PI), sin(normG*PI), sin(normB*PI), 1.0);
    }
}

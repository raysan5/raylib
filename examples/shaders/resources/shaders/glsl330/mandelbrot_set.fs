#version 330

#define PI 3.1415926535897932384626433832795

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Output fragment color
out vec4 finalColor;

uniform vec2 offset;            // Offset of the scale
uniform float zoom;             // Zoom of the scale
uniform int maxIterations;      // Max iterations per pixel

const float max = 4.0;          // We consider infinite as 4.0: if a point reaches a distance of 4.0 it will escape to infinity

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

    int iterations = 0;
    for (iterations = 0; iterations < maxIterations; iterations++)
    {
        float aa = a*a;
        float bb = b*b;
        float abs = sqrt(aa + bb);
        if (abs > max)
        {
            convergeNumber = float(iterations);// + (max - absOld)/(abs - absOld);
            break;
        }

        float twoab = 2.0*a*b;
        a = aa - bb + z.x;
        b = twoab + z.y;

        absOld = abs;
    }

    if (iterations >= maxIterations)
    {
        finalColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
    else
    {
        float normR = float(int(convergeNumber)%55)/55.0;
        float normG = float(int(convergeNumber)%69)/69.0;
        float normB = float(int(convergeNumber)%40)/40.0;

        finalColor = vec4(sin(normR*PI), sin(normG*PI), sin(normB*PI), 1.0);
    }
}

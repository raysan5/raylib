#version 330

// Input vertex attributes (from vertex shader)
in vec2 fragTexCoord;
in vec4 fragColor;

// Output fragment color
out vec4 finalColor;

uniform vec2 c;                 // c.x = real, c.y = imaginary component. Equation done is z^2 + c
uniform vec2 offset;            // Offset of the scale.
uniform float zoom;             // Zoom of the scale.

const int maxIterations = 255;     // Max iterations to do.
const float colorCycles = 2.0;    // Number of times the color palette repeats. Can show higher detail for higher iteration numbers.

// Square a complex number
vec2 ComplexSquare(vec2 z)
{
    return vec2(z.x*z.x - z.y*z.y, z.x*z.y*2.0);
}

// Convert Hue Saturation Value (HSV) color into RGB
vec3 Hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0/3.0, 1.0/3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz)*6.0 - K.www);
    return c.z*mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
    /**********************************************************************************************
      Julia sets use a function z^2 + c, where c is a constant.
      This function is iterated until the nature of the point is determined.

      If the magnitude of the number becomes greater than 2, then from that point onward
      the number will get bigger and bigger, and will never get smaller (tends towards infinity).
      2^2 = 4, 4^2 = 8 and so on.
      So at 2 we stop iterating.

      If the number is below 2, we keep iterating.
      But when do we stop iterating if the number is always below 2 (it converges)?
      That is what maxIterations is for.
      Then we can divide the iterations by the maxIterations value to get a normalized value that we can
      then map to a color.

      We use dot product (z.x * z.x + z.y * z.y) to determine the magnitude (length) squared.
      And once the magnitude squared is > 4, then magnitude > 2 is also true (saves computational power).
    *************************************************************************************************/

    // The pixel coordinates are scaled so they are on the mandelbrot scale
    // NOTE: fragTexCoord already comes as normalized screen coordinates but offset must be normalized before scaling and zoom
    vec2 z = vec2((fragTexCoord.x - 0.5f)*2.5, (fragTexCoord.y - 0.5)*1.5)/zoom;
    z.x += offset.x;
    z.y += offset.y;

    int iterations = 0;
    for (iterations = 0; iterations < maxIterations; iterations++)
    {
        z = ComplexSquare(z) + c;  // Iterate function

        if (dot(z, z) > 4.0) break;
    }

    // Another few iterations decreases errors in the smoothing calculation.
    // See http://linas.org/art-gallery/escape/escape.html for more information.
    z = ComplexSquare(z) + c;
    z = ComplexSquare(z) + c;

    // This last part smooths the color (again see link above).
    float smoothVal = float(iterations) + 1.0 - (log(log(length(z)))/log(2.0));

    // Normalize the value so it is between 0 and 1.
    float norm = smoothVal/float(maxIterations);

    // If in set, color black. 0.999 allows for some float accuracy error.
    if (norm > 0.999) finalColor = vec4(0.0, 0.0, 0.0, 1.0);
    else finalColor = vec4(Hsv2rgb(vec3(norm*colorCycles, 1.0, 1.0)), 1.0);
}

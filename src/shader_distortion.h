
// Vertex shader definition to embed, no external file required
static const char vDistortionShaderStr[] = 
#if defined(GRAPHICS_API_OPENGL_21)
"#version 120                       \n"
#elif defined(GRAPHICS_API_OPENGL_ES2)
"#version 100                       \n"
#endif
#if defined(GRAPHICS_API_OPENGL_ES2) || defined(GRAPHICS_API_OPENGL_21)
"attribute vec3 vertexPosition;     \n"
"attribute vec2 vertexTexCoord;     \n"
"attribute vec4 vertexColor;        \n"
"varying vec2 fragTexCoord;         \n"
"varying vec4 fragColor;            \n"
#elif defined(GRAPHICS_API_OPENGL_33)
"#version 330                       \n"
"in vec3 vertexPosition;            \n"
"in vec2 vertexTexCoord;            \n"
"in vec4 vertexColor;               \n"
"out vec2 fragTexCoord;             \n"
"out vec4 fragColor;                \n"
#endif
"uniform mat4 mvp;            \n"
"void main()                        \n"
"{                                  \n"
"    fragTexCoord = vertexTexCoord; \n"
"    fragColor = vertexColor;       \n"
"    gl_Position = mvp*vec4(vertexPosition, 1.0); \n"
"}                                  \n";

// Fragment shader definition to embed, no external file required
static const char fDistortionShaderStr[] = 
#if defined(GRAPHICS_API_OPENGL_21)
"#version 120                       \n"
#elif defined(GRAPHICS_API_OPENGL_ES2)
"#version 100                       \n"
"precision mediump float;           \n"     // precision required for OpenGL ES2 (WebGL)
#endif
#if defined(GRAPHICS_API_OPENGL_ES2) || defined(GRAPHICS_API_OPENGL_21)
"varying vec2 fragTexCoord;         \n"
"varying vec4 fragColor;            \n"
#elif defined(GRAPHICS_API_OPENGL_33)
"#version 330                       \n"
"in vec2 fragTexCoord;              \n"
"in vec4 fragColor;                 \n"
"out vec4 finalColor;               \n"
#endif
"uniform sampler2D texture0;                                     \n"
#if defined(GRAPHICS_API_OPENGL_ES2) || defined(GRAPHICS_API_OPENGL_21)
"uniform vec2 leftLensCenter;       \n"
"uniform vec2 rightLensCenter;      \n"
"uniform vec2 leftScreenCenter;     \n"
"uniform vec2 rightScreenCenter;    \n"
"uniform vec2 scale;                \n"
"uniform vec2 scaleIn;              \n"
"uniform vec4 hmdWarpParam;         \n"
"uniform vec4 chromaAbParam;        \n"
#elif defined(GRAPHICS_API_OPENGL_33)
"uniform vec2 leftLensCenter = vec2(0.288, 0.5);                 \n"
"uniform vec2 rightLensCenter = vec2(0.712, 0.5);                \n"
"uniform vec2 leftScreenCenter = vec2(0.25, 0.5);                \n"
"uniform vec2 rightScreenCenter = vec2(0.75, 0.5);               \n"
"uniform vec2 scale = vec2(0.25, 0.45);                          \n"
"uniform vec2 scaleIn = vec2(4, 2.2222);                         \n"
"uniform vec4 hmdWarpParam = vec4(1, 0.22, 0.24, 0);             \n"
"uniform vec4 chromaAbParam = vec4(0.996, -0.004, 1.014, 0.0);   \n"
#endif
"void main() \n"
"{ \n"
"   vec2 lensCenter = fragTexCoord.x < 0.5 ? leftLensCenter : rightLensCenter; \n"
"   vec2 screenCenter = fragTexCoord.x < 0.5 ? leftScreenCenter : rightScreenCenter; \n"
"   vec2 theta = (fragTexCoord - lensCenter)*scaleIn; \n"
"   float rSq = theta.x*theta.x + theta.y*theta.y; \n"
"   vec2 theta1 = theta*(hmdWarpParam.x + hmdWarpParam.y*rSq + hmdWarpParam.z*rSq*rSq + hmdWarpParam.w*rSq*rSq*rSq); \n"
"   vec2 thetaBlue = theta1*(chromaAbParam.z + chromaAbParam.w*rSq); \n"
"   vec2 tcBlue = lensCenter + scale*thetaBlue; \n"
"   if (any(bvec2(clamp(tcBlue, screenCenter - vec2(0.25, 0.5), screenCenter + vec2(0.25, 0.5)) - tcBlue))) \n"
"   { \n"
#if defined(GRAPHICS_API_OPENGL_ES2) || defined(GRAPHICS_API_OPENGL_21)
"       gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0); \n"
#elif defined(GRAPHICS_API_OPENGL_33)
"       finalColor = vec4(0.0, 0.0, 0.0, 1.0); \n"
#endif
"   } \n"
"   else \n"
"   { \n"
#if defined(GRAPHICS_API_OPENGL_ES2) || defined(GRAPHICS_API_OPENGL_21)
"       float blue = texture2D(texture0, tcBlue).b; \n"
"       vec2 tcGreen = lensCenter + scale*theta1; \n"
"       float green = texture2D(texture0, tcGreen).g; \n"
#elif defined(GRAPHICS_API_OPENGL_33)
"       float blue = texture(texture0, tcBlue).b; \n"
"       vec2 tcGreen = lensCenter + scale*theta1; \n"
"       float green = texture(texture0, tcGreen).g; \n"
#endif
"       vec2 thetaRed = theta1*(chromaAbParam.x + chromaAbParam.y*rSq); \n"
"       vec2 tcRed = lensCenter + scale*thetaRed; \n"
#if defined(GRAPHICS_API_OPENGL_ES2) || defined(GRAPHICS_API_OPENGL_21)
"       float red = texture2D(texture0, tcRed).r; \n"
"       gl_FragColor = vec4(red, green, blue, 1.0); \n"
#elif defined(GRAPHICS_API_OPENGL_33)
"       float red = texture(texture0, tcRed).r; \n"
"       finalColor = vec4(red, green, blue, 1.0); \n"
#endif
"    } \n"
"} \n";

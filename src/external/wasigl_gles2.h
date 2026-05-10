// GLES2 header for PLATFORM_WASIGL.
// GL functions are imported directly from the host (via @easywasm/gl), so GLAD's
// runtime proc-address loading cannot work (imported functions have no stable WASM
// table indices). This header reuses glad_gles2.h for types/constants, then undefines
// the function-pointer macros and redeclares functions as plain extern so every GL
// call compiles to a direct WASM `call` instruction against the host import.

#ifndef WASIGL_GLES2_H
#define WASIGL_GLES2_H

// Pull in all types, constants, PFNGL typedefs, and extern glad_glXxx declarations
// (no implementation — the variables are extern-declared but never referenced after
// the macros below are replaced with direct calls).
#include "glad_gles2.h"

// Remove the function-pointer redirects added by glad_gles2.h so subsequent
// glXxx() calls reach the extern functions declared below instead.
#undef glActiveTexture
#undef glAttachShader
#undef glBindAttribLocation
#undef glBindBuffer
#undef glBindFramebuffer
#undef glBindRenderbuffer
#undef glBindTexture
#undef glBlendColor
#undef glBlendEquation
#undef glBlendEquationSeparate
#undef glBlendFunc
#undef glBlendFuncSeparate
#undef glBufferData
#undef glBufferSubData
#undef glCheckFramebufferStatus
#undef glClear
#undef glClearColor
#undef glClearDepthf
#undef glClearStencil
#undef glColorMask
#undef glCompileShader
#undef glCompressedTexImage2D
#undef glCompressedTexSubImage2D
#undef glCopyTexImage2D
#undef glCopyTexSubImage2D
#undef glCreateProgram
#undef glCreateShader
#undef glCullFace
#undef glDeleteBuffers
#undef glDeleteFramebuffers
#undef glDeleteProgram
#undef glDeleteRenderbuffers
#undef glDeleteShader
#undef glDeleteTextures
#undef glDepthFunc
#undef glDepthMask
#undef glDepthRangef
#undef glDetachShader
#undef glDisable
#undef glDisableVertexAttribArray
#undef glDrawArrays
#undef glDrawElements
#undef glEnable
#undef glEnableVertexAttribArray
#undef glFinish
#undef glFlush
#undef glFramebufferRenderbuffer
#undef glFramebufferTexture2D
#undef glFrontFace
#undef glGenBuffers
#undef glGenFramebuffers
#undef glGenRenderbuffers
#undef glGenTextures
#undef glGenerateMipmap
#undef glGetActiveAttrib
#undef glGetActiveUniform
#undef glGetAttachedShaders
#undef glGetAttribLocation
#undef glGetBooleanv
#undef glGetBufferParameteriv
#undef glGetError
#undef glGetFloatv
#undef glGetFramebufferAttachmentParameteriv
#undef glGetIntegerv
#undef glGetProgramInfoLog
#undef glGetProgramiv
#undef glGetRenderbufferParameteriv
#undef glGetShaderInfoLog
#undef glGetShaderPrecisionFormat
#undef glGetShaderSource
#undef glGetShaderiv
#undef glGetString
#undef glGetTexParameterfv
#undef glGetTexParameteriv
#undef glGetUniformLocation
#undef glGetUniformfv
#undef glGetUniformiv
#undef glGetVertexAttribPointerv
#undef glGetVertexAttribfv
#undef glGetVertexAttribiv
#undef glHint
#undef glIsBuffer
#undef glIsEnabled
#undef glIsFramebuffer
#undef glIsProgram
#undef glIsRenderbuffer
#undef glIsShader
#undef glIsTexture
#undef glLineWidth
#undef glLinkProgram
#undef glPixelStorei
#undef glPolygonOffset
#undef glReadPixels
#undef glReleaseShaderCompiler
#undef glRenderbufferStorage
#undef glSampleCoverage
#undef glScissor
#undef glShaderBinary
#undef glShaderSource
#undef glStencilFunc
#undef glStencilFuncSeparate
#undef glStencilMask
#undef glStencilMaskSeparate
#undef glStencilOp
#undef glStencilOpSeparate
#undef glTexImage2D
#undef glTexParameterf
#undef glTexParameterfv
#undef glTexParameteri
#undef glTexParameteriv
#undef glTexSubImage2D
#undef glUniform1f
#undef glUniform1fv
#undef glUniform1i
#undef glUniform1iv
#undef glUniform2f
#undef glUniform2fv
#undef glUniform2i
#undef glUniform2iv
#undef glUniform3f
#undef glUniform3fv
#undef glUniform3i
#undef glUniform3iv
#undef glUniform4f
#undef glUniform4fv
#undef glUniform4i
#undef glUniform4iv
#undef glUniformMatrix2fv
#undef glUniformMatrix3fv
#undef glUniformMatrix4fv
#undef glUseProgram
#undef glValidateProgram
#undef glVertexAttrib1f
#undef glVertexAttrib1fv
#undef glVertexAttrib2f
#undef glVertexAttrib2fv
#undef glVertexAttrib3f
#undef glVertexAttrib3fv
#undef glVertexAttrib4f
#undef glVertexAttrib4fv
#undef glVertexAttribPointer
#undef glViewport

// Direct extern declarations — these resolve to host imports (env.glXxx) supplied
// by @easywasm/gl, so every call becomes a WASM `call` instruction with no
// function-table indirection.
extern void           glActiveTexture(GLenum texture);
extern void           glAttachShader(GLuint program, GLuint shader);
extern void           glBindAttribLocation(GLuint program, GLuint index, const GLchar *name);
extern void           glBindBuffer(GLenum target, GLuint buffer);
extern void           glBindFramebuffer(GLenum target, GLuint framebuffer);
extern void           glBindRenderbuffer(GLenum target, GLuint renderbuffer);
extern void           glBindTexture(GLenum target, GLuint texture);
extern void           glBlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern void           glBlendEquation(GLenum mode);
extern void           glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha);
extern void           glBlendFunc(GLenum sfactor, GLenum dfactor);
extern void           glBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
extern void           glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
extern void           glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
extern GLenum         glCheckFramebufferStatus(GLenum target);
extern void           glClear(GLbitfield mask);
extern void           glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
extern void           glClearDepthf(GLfloat d);
extern void           glClearStencil(GLint s);
extern void           glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
extern void           glCompileShader(GLuint shader);
extern void           glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
extern void           glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data);
extern void           glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
extern void           glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern GLuint         glCreateProgram(void);
extern GLuint         glCreateShader(GLenum type);
extern void           glCullFace(GLenum mode);
extern void           glDeleteBuffers(GLsizei n, const GLuint *buffers);
extern void           glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers);
extern void           glDeleteProgram(GLuint program);
extern void           glDeleteRenderbuffers(GLsizei n, const GLuint *renderbuffers);
extern void           glDeleteShader(GLuint shader);
extern void           glDeleteTextures(GLsizei n, const GLuint *textures);
extern void           glDepthFunc(GLenum func);
extern void           glDepthMask(GLboolean flag);
extern void           glDepthRangef(GLfloat n, GLfloat f);
extern void           glDetachShader(GLuint program, GLuint shader);
extern void           glDisable(GLenum cap);
extern void           glDisableVertexAttribArray(GLuint index);
extern void           glDrawArrays(GLenum mode, GLint first, GLsizei count);
extern void           glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices);
extern void           glEnable(GLenum cap);
extern void           glEnableVertexAttribArray(GLuint index);
extern void           glFinish(void);
extern void           glFlush(void);
extern void           glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
extern void           glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
extern void           glFrontFace(GLenum mode);
extern void           glGenBuffers(GLsizei n, GLuint *buffers);
extern void           glGenFramebuffers(GLsizei n, GLuint *framebuffers);
extern void           glGenRenderbuffers(GLsizei n, GLuint *renderbuffers);
extern void           glGenTextures(GLsizei n, GLuint *textures);
extern void           glGenerateMipmap(GLenum target);
extern void           glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
extern void           glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name);
extern void           glGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders);
extern GLint          glGetAttribLocation(GLuint program, const GLchar *name);
extern void           glGetBooleanv(GLenum pname, GLboolean *data);
extern void           glGetBufferParameteriv(GLenum target, GLenum pname, GLint *params);
extern GLenum         glGetError(void);
extern void           glGetFloatv(GLenum pname, GLfloat *data);
extern void           glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint *params);
extern void           glGetIntegerv(GLenum pname, GLint *data);
extern void           glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
extern void           glGetProgramiv(GLuint program, GLenum pname, GLint *params);
extern void           glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint *params);
extern void           glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
extern void           glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
extern void           glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source);
extern void           glGetShaderiv(GLuint shader, GLenum pname, GLint *params);
extern const GLubyte *glGetString(GLenum name);
extern void           glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params);
extern void           glGetTexParameteriv(GLenum target, GLenum pname, GLint *params);
extern GLint          glGetUniformLocation(GLuint program, const GLchar *name);
extern void           glGetUniformfv(GLuint program, GLint location, GLfloat *params);
extern void           glGetUniformiv(GLuint program, GLint location, GLint *params);
extern void           glGetVertexAttribPointerv(GLuint index, GLenum pname, void **pointer);
extern void           glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat *params);
extern void           glGetVertexAttribiv(GLuint index, GLenum pname, GLint *params);
extern void           glHint(GLenum target, GLenum mode);
extern GLboolean      glIsBuffer(GLuint buffer);
extern GLboolean      glIsEnabled(GLenum cap);
extern GLboolean      glIsFramebuffer(GLuint framebuffer);
extern GLboolean      glIsProgram(GLuint program);
extern GLboolean      glIsRenderbuffer(GLuint renderbuffer);
extern GLboolean      glIsShader(GLuint shader);
extern GLboolean      glIsTexture(GLuint texture);
extern void           glLineWidth(GLfloat width);
extern void           glLinkProgram(GLuint program);
extern void           glPixelStorei(GLenum pname, GLint param);
extern void           glPolygonOffset(GLfloat factor, GLfloat units);
extern void           glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels);
extern void           glReleaseShaderCompiler(void);
extern void           glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
extern void           glSampleCoverage(GLfloat value, GLboolean invert);
extern void           glScissor(GLint x, GLint y, GLsizei width, GLsizei height);
extern void           glShaderBinary(GLsizei count, const GLuint *shaders, GLenum binaryFormat, const void *binary, GLsizei length);
extern void           glShaderSource(GLuint shader, GLsizei count, const GLchar *const *string, const GLint *length);
extern void           glStencilFunc(GLenum func, GLint ref, GLuint mask);
extern void           glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask);
extern void           glStencilMask(GLuint mask);
extern void           glStencilMaskSeparate(GLenum face, GLuint mask);
extern void           glStencilOp(GLenum fail, GLenum zfail, GLenum zpass);
extern void           glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
extern void           glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels);
extern void           glTexParameterf(GLenum target, GLenum pname, GLfloat param);
extern void           glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params);
extern void           glTexParameteri(GLenum target, GLenum pname, GLint param);
extern void           glTexParameteriv(GLenum target, GLenum pname, const GLint *params);
extern void           glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
extern void           glUniform1f(GLint location, GLfloat v0);
extern void           glUniform1fv(GLint location, GLsizei count, const GLfloat *value);
extern void           glUniform1i(GLint location, GLint v0);
extern void           glUniform1iv(GLint location, GLsizei count, const GLint *value);
extern void           glUniform2f(GLint location, GLfloat v0, GLfloat v1);
extern void           glUniform2fv(GLint location, GLsizei count, const GLfloat *value);
extern void           glUniform2i(GLint location, GLint v0, GLint v1);
extern void           glUniform2iv(GLint location, GLsizei count, const GLint *value);
extern void           glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
extern void           glUniform3fv(GLint location, GLsizei count, const GLfloat *value);
extern void           glUniform3i(GLint location, GLint v0, GLint v1, GLint v2);
extern void           glUniform3iv(GLint location, GLsizei count, const GLint *value);
extern void           glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
extern void           glUniform4fv(GLint location, GLsizei count, const GLfloat *value);
extern void           glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint w);
extern void           glUniform4iv(GLint location, GLsizei count, const GLint *value);
extern void           glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void           glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void           glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
extern void           glUseProgram(GLuint program);
extern void           glValidateProgram(GLuint program);
extern void           glVertexAttrib1f(GLuint index, GLfloat x);
extern void           glVertexAttrib1fv(GLuint index, const GLfloat *v);
extern void           glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y);
extern void           glVertexAttrib2fv(GLuint index, const GLfloat *v);
extern void           glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z);
extern void           glVertexAttrib3fv(GLuint index, const GLfloat *v);
extern void           glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void           glVertexAttrib4fv(GLuint index, const GLfloat *v);
extern void           glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
extern void           glViewport(GLint x, GLint y, GLsizei width, GLsizei height);

#endif // WASIGL_GLES2_H

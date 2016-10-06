/********************************************************************************//**
\file      OVR_CAPI_GL.h
\brief     OpenGL-specific structures used by the CAPI interface.
\copyright Copyright 2015 Oculus VR, LLC. All Rights reserved.
************************************************************************************/

#ifndef OVR_CAPI_GL_h
#define OVR_CAPI_GL_h

#include "OVR_CAPI.h"

#if !defined(OVR_EXPORTING_CAPI)

/// Creates a TextureSwapChain suitable for use with OpenGL.
///
/// \param[in]  session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in]  desc Specifies the requested texture properties. See notes for more info about texture format.
/// \param[out] out_TextureSwapChain Returns the created ovrTextureSwapChain, which will be valid upon
///             a successful return value, else it will be NULL. This texture swap chain must be eventually
///             destroyed via ovr_DestroyTextureSwapChain before destroying the session with ovr_Destroy.
///
/// \return Returns an ovrResult indicating success or failure. In the case of failure, use 
///         ovr_GetLastErrorInfo to get more information.
///
/// \note The \a format provided should be thought of as the format the distortion compositor will use when reading
/// the contents of the texture. To that end, it is highly recommended that the application requests texture swap chain
/// formats that are in sRGB-space (e.g. OVR_FORMAT_R8G8B8A8_UNORM_SRGB) as the distortion compositor does sRGB-correct
/// rendering. Furthermore, the app should then make sure "glEnable(GL_FRAMEBUFFER_SRGB);" is called before rendering
/// into these textures. Even though it is not recommended, if the application would like to treat the texture as a linear
/// format and do linear-to-gamma conversion in GLSL, then the application can avoid calling "glEnable(GL_FRAMEBUFFER_SRGB);",
/// but should still pass in an sRGB variant for the \a format. Failure to do so will cause the distortion compositor
/// to apply incorrect gamma conversions leading to gamma-curve artifacts.
///
/// \see ovr_GetTextureSwapChainLength
/// \see ovr_GetTextureSwapChainCurrentIndex
/// \see ovr_GetTextureSwapChainDesc
/// \see ovr_GetTextureSwapChainBufferGL
/// \see ovr_DestroyTextureSwapChain
///
OVR_PUBLIC_FUNCTION(ovrResult) ovr_CreateTextureSwapChainGL(ovrSession session,
                                                            const ovrTextureSwapChainDesc* desc,
                                                            ovrTextureSwapChain* out_TextureSwapChain);

/// Get a specific buffer within the chain as a GL texture name
///
/// \param[in]  session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in]  chain Specifies an ovrTextureSwapChain previously returned by ovr_CreateTextureSwapChainGL
/// \param[in]  index Specifies the index within the chain to retrieve. Must be between 0 and length (see ovr_GetTextureSwapChainLength)
///             or may pass -1 to get the buffer at the CurrentIndex location. (Saving a call to GetTextureSwapChainCurrentIndex)
/// \param[out] out_TexId Returns the GL texture object name associated with the specific index requested
///
/// \return Returns an ovrResult indicating success or failure. In the case of failure, use 
///         ovr_GetLastErrorInfo to get more information.
///
OVR_PUBLIC_FUNCTION(ovrResult) ovr_GetTextureSwapChainBufferGL(ovrSession session,
                                                               ovrTextureSwapChain chain,
                                                               int index,
                                                               unsigned int* out_TexId);


/// Creates a Mirror Texture which is auto-refreshed to mirror Rift contents produced by this application.
///
/// A second call to ovr_CreateMirrorTextureGL for a given ovrSession before destroying the first one
/// is not supported and will result in an error return.
///
/// \param[in]  session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in]  desc Specifies the requested mirror texture description.
/// \param[out] out_MirrorTexture Specifies the created ovrMirrorTexture, which will be valid upon a successful return value, else it will be NULL.
///             This texture must be eventually destroyed via ovr_DestroyMirrorTexture before destroying the session with ovr_Destroy.
///
/// \return Returns an ovrResult indicating success or failure. In the case of failure, use 
///         ovr_GetLastErrorInfo to get more information.
///
/// \note The \a format provided should be thought of as the format the distortion compositor will use when writing into the mirror
/// texture. It is highly recommended that mirror textures are requested as sRGB formats because the distortion compositor
/// does sRGB-correct rendering. If the application requests a non-sRGB format (e.g. R8G8B8A8_UNORM) as the mirror texture,
/// then the application might have to apply a manual linear-to-gamma conversion when reading from the mirror texture.
/// Failure to do so can result in incorrect gamma conversions leading to gamma-curve artifacts and color banding.
///
/// \see ovr_GetMirrorTextureBufferGL
/// \see ovr_DestroyMirrorTexture
///
OVR_PUBLIC_FUNCTION(ovrResult) ovr_CreateMirrorTextureGL(ovrSession session,
                                                         const ovrMirrorTextureDesc* desc,
                                                         ovrMirrorTexture* out_MirrorTexture);

/// Get a the underlying buffer as a GL texture name
///
/// \param[in]  session Specifies an ovrSession previously returned by ovr_Create.
/// \param[in]  mirrorTexture Specifies an ovrMirrorTexture previously returned by ovr_CreateMirrorTextureGL
/// \param[out] out_TexId Specifies the GL texture object name associated with the mirror texture
///
/// \return Returns an ovrResult indicating success or failure. In the case of failure, use 
///         ovr_GetLastErrorInfo to get more information.
///
OVR_PUBLIC_FUNCTION(ovrResult) ovr_GetMirrorTextureBufferGL(ovrSession session,
                                                            ovrMirrorTexture mirrorTexture,
                                                            unsigned int* out_TexId);

#endif // !defined(OVR_EXPORTING_CAPI)

#endif    // OVR_CAPI_GL_h

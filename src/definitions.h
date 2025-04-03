#if EDITOR
	static double editor_time = 0.;
#endif

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#define VC_LEANMEAN
#define VC_EXTRALEAN

#include <windows.h>
#include <GL/gl.h>


#if LAPTOP_GPU_FIX
extern "C" {
	__declspec(dllexport) unsigned long NvOptimusEnablement = 1;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

#define oglCreateFramebuffers ((PFNGLCREATEFRAMEBUFFERSPROC)wglGetProcAddress("glCreateFramebuffers"))
#define oglNamedFramebufferDrawBuffers ((PFNGLNAMEDFRAMEBUFFERDRAWBUFFERSPROC)wglGetProcAddress("glNamedFramebufferDrawBuffers"))
#define oglDispatchCompute ((PFNGLDISPATCHCOMPUTEPROC)wglGetProcAddress("glDispatchCompute"))
#define oglTexStorage2D ((PFNGLTEXSTORAGE2DPROC)wglGetProcAddress("glTexStorage2D"))
#define oglBindImageTexture ((PFNGLBINDIMAGETEXTUREPROC)wglGetProcAddress("glBindImageTexture"))
#define oglCreateShaderProgramv ((PFNGLCREATESHADERPROGRAMVPROC)wglGetProcAddress("glCreateShaderProgramv"))
#define oglBindFramebuffer ((PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer"))
#define oglGenFramebuffers ((PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffers"))
#define oglFramebufferTexture2D ((PFNGLFRAMEBUFFERTEXTURE2DPROC)wglGetProcAddress("glFramebufferTexture2D"))
#define oglNamedFramebufferTexture ((PFNGLNAMEDFRAMEBUFFERTEXTUREPROC)wglGetProcAddress("glNamedFramebufferTexture"))
#define oglBindTextureUnit ((PFNGLBINDTEXTUREUNITPROC)wglGetProcAddress("glBindTextureUnit"))
#define oglDebugMessageCallback ((PFNGLDEBUGMESSAGECALLBACKPROC)wglGetProcAddress("glDebugMessageCallback"))
#define oglDrawBuffers ((PFNGLDRAWBUFFERSPROC)wglGetProcAddress("glDrawBuffers"))
#define oglGetProgramiv ((PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv"))
#define oglGetProgramInfoLog ((PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog"))
#define oglDeleteProgram ((PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram"))
#define oglUseProgram ((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"))
#define oglUniform1i ((PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i"))
#define oglUniform2i ((PFNGLUNIFORM2IPROC)wglGetProcAddress("glUniform2i"))
#define oglUniform2f ((PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f"))
#define oglUniform1f ((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"))
#define oglCreateBuffers ((PFNGLCREATEBUFFERSPROC)wglGetProcAddress("glCreateBuffers"))
#define oglNamedBufferStorage ((PFNGLNAMEDBUFFERSTORAGEPROC)wglGetProcAddress("glNamedBufferStorage"))
#define oglBindBufferBase ((PFNGLBINDBUFFERBASEPROC)wglGetProcAddress("glBindBufferBase"))
#define oglGetNamedBufferSubData ((PFNGLGETNAMEDBUFFERSUBDATAPROC)wglGetProcAddress("glGetNamedBufferSubData"))
#define oglGenBuffers ((PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers"))
#define oglBindBuffer ((PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer"))
#define oglBufferData ((PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData"))
#define oglMapBuffer ((PFNGLMAPBUFFERPROC)wglGetProcAddress("glMapBuffer"))
#define oglUnmapBuffer ((PFNGLUNMAPBUFFERPROC)wglGetProcAddress("glUnmapBuffer"))





// oglUniform1f
// oglDispatchCompute


// declare this symbol if your code uses floating point types
//extern "C" int _fltused;

#define FAIL_KILL true
#define PID_QUALIFIER const


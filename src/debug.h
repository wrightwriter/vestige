// This header contains some useful functions for debugging OpenGL.
// Remember to disable them when building your final releases.

#include <windows.h>
#include <GL/gl.h>
#include "glext.h"
#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>
#include <time.h>


// Assuming a maximum of 100 paths, adjust if needed.
#if OPENGL_DEBUG
	// Global array of file paths
	static const char* file_paths[100];
	static unsigned int shader_types[100];
	static size_t shaders_count = 0;
	static bool audio_shader_just_reloaded = false;
#endif

#if EDITOR || OPENGL_DEBUG
	static bool shader_failed_compile = false;
#endif




template<size_t N>
static bool _inline dbg_nocheck_shader_compilation(int (&programs)[N]){
	 #if OPENGL_DEBUG
		size_t arrayLength = N;
		for(int i = 0; i < arrayLength; i++){
			int pid = programs[i];
			int gl_temp_link_status;
			oglGetProgramiv( pid, GL_LINK_STATUS, &gl_temp_link_status);
			if (gl_temp_link_status == 0) {
				char log[1000];
				oglGetProgramInfoLog(pid, 1000 - 1, NULL, log);
				MessageBoxA(NULL, log, log, MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2);
				return false;
			}
		}
	#endif
	return true;
}

template<size_t N>
static bool _inline dbg_check_shader_compilation(int (&programs)[N]){
	#if OPENGL_DEBUG
		size_t arrayLength = N;
		for(int i = 0; i < arrayLength; i++){
			int pid = programs[i];
			int gl_temp_link_status;
			oglGetProgramiv( pid, GL_LINK_STATUS, &gl_temp_link_status);
			if (gl_temp_link_status == 0) {
				char log[1000];
				oglGetProgramInfoLog(pid, 1000 - 1, NULL, log);
				MessageBoxA(NULL, log, file_paths[i], MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2);
				return false;
			}
		}
	#endif
	return true;
}


#if OPENGL_DEBUG

void __stdcall
dbg_gl_message_callback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{

	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION || severity == 37191) // 37191 is buffer leaking to cpu mem warning.
		return;
	MessageBox(NULL, message, message, 0x00000000L);

	//fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
	//	(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
	//	type, severity, message);

	// __debugbreak();
}
#endif

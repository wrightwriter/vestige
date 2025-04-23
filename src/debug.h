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
static bool _inline gl_debug_nocheck(int (&programs)[N]){
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
static bool _inline gl_debug(int (&programs)[N]){
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

#if EDITOR
	void PrintToConsole(const char* message);
#endif

static void _inline reload_from_disk() {
	#if OPENGL_DEBUG
	bool prev_shader_failed_compile = shader_failed_compile;
	shader_failed_compile = false;
	// char test_notif[] = "asrdgasdg";
	// MessageBoxA(NULL, test_notif, test_notif, MB_ICONWARNING | MB_CANCELTRYCONTINUE | MB_DEFBUTTON2);
	//printf("shader reload");
	time_t current_time = time(NULL);
	float MODIFICATION_THRESHOLD = 5;  // 5 seconds

	
	int temp_pids[100];
	bool reload_shader[100];
	bool reload_shader_success[100];


	for(int i = 0; i < shaders_count; i++) {
		reload_shader[i] = false;
		reload_shader_success[i] = true;
	}

	bool success = true;
	for(int i = 0; i < shaders_count; i++) {
		auto file_path = file_paths[i];
		struct stat file_stat;

		stat(file_path, &file_stat);
		time_t current_time = time(NULL);
		double seconds_since_mod = difftime(current_time, file_stat.st_mtime);


		if (seconds_since_mod > MODIFICATION_THRESHOLD) {
				//printf("File %s was modified more than %d seconds ago. Skipping...\n", file_path, MODIFICATION_THRESHOLD);
				continue;
				//return;
		}
		reload_shader[i] = true;
		FILE* file = fopen(file_paths[i], "r");
		if (file) {
			// Read file contents
			fseek(file, 0, SEEK_END);
			long file_size = ftell(file);
			fseek(file, 0, SEEK_SET);


			char* content = (char*)malloc(file_size + 1);
			// char* content = (char*)malloc(file_size + 1);
			// char content[50000];
			//static char content[100000];
			size_t bytesRead = fread(content, 1, file_size, file);
			content[bytesRead] = '\0'; // Null-terminate the string
			// if(i == 0) {
			char* content_ptr = &content[0];
			temp_pids[i] = oglCreateShaderProgramv(shader_types[i], 1, &content_ptr);
			// } else {
				// pids[i] = oglCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, content);
			// }
			// printf("File content (%s):\n%s\n", file_paths[i], content);
			free(content);
			fclose(file);
		} else {
			// Handle file open error
			fprintf(stderr, "Error: Unable to open file %s\n", file_paths[i]);
		}
		

	}

	for(int i = 0; i < shaders_count; i++){
		if(reload_shader[i] == false){
			continue;
		}
		int pid = temp_pids[i];
		int gl_temp_link_status;
		
		oglGetProgramiv( pid, GL_LINK_STATUS, &gl_temp_link_status);
		if (gl_temp_link_status == 0) {
			char log[1000];
			oglGetProgramInfoLog(pid, 1000 - 1, NULL, log);
			printf(file_paths[i]);
			printf("\n");
			printf("\n");
			printf(log);
			#if EDITOR
				PrintToConsole("------- Shader Error -------\n");
				PrintToConsole(file_paths[i]);
				PrintToConsole(log);
			#else
				MessageBoxA(NULL, log, file_paths[i], MB_HELP);
			#endif
			shader_failed_compile = true;
			success = false;
			reload_shader_success[i] = false;
		}
	}

	for(int i = 0; i < shaders_count; i++) {
		if(reload_shader[i]){
			if(!reload_shader_success[i]){
				oglDeleteProgram(temp_pids[i]);
			} else {
				if(programs[i] == PROG_MUSIC){
					audio_shader_just_reloaded = true;
				}
				oglDeleteProgram(programs[i]);
				programs[i] = temp_pids[i];
				
				#if EDITOR
					PrintToConsole("Reloaded shader:\n");
					PrintToConsole(file_paths[i]);
					PrintToConsole("\n");
					PrintToConsole("\n");
					PrintToConsole("\n");
					PrintToConsole("\n");
					PrintToConsole("\n");
					PrintToConsole("\n");
				#endif
			}
		}
	}

	if(prev_shader_failed_compile == true && shader_failed_compile == false){
		#if EDITOR
			PrintToConsole("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
		#endif
	}

	#endif
}


static void _inline try_reload() {
	#if OPENGL_DEBUG
		if(!GetAsyncKeyState(VK_LCONTROL) || !GetAsyncKeyState(0x53)) {
			return;
		}
		Sleep(400);
		reload_from_disk();
	#endif
}



#if OPENGL_DEBUG
#define STRINGIFY2(x) #x // Thanks sooda!
#define STRINGIFY(x) STRINGIFY2(x)


static GLchar* getErrorString(GLenum errorCode)
{
	if (errorCode == GL_NO_ERROR) {
		return (GLchar*) "No error";
	}
	else if (errorCode == GL_INVALID_VALUE) {
		return (GLchar*) "Invalid value";
	}
	else if (errorCode == GL_INVALID_ENUM) {
		return (GLchar*) "Invalid enum";
	}
	else if (errorCode == GL_INVALID_OPERATION) {
		return (GLchar*) "Invalid operation";
	}
	else if (errorCode == GL_STACK_OVERFLOW) {
		return (GLchar*) "Stack overflow";
	}
	else if (errorCode == GL_STACK_UNDERFLOW) {
		return (GLchar*) "Stack underflow";
	}
	else if (errorCode == GL_OUT_OF_MEMORY) {
		return (GLchar*) "Out of memory";
	}
	return (GLchar*) "Unknown";
}

static void assertGlError(const char* error_message)
{
	const GLenum ErrorValue = glGetError();
	if (ErrorValue == GL_NO_ERROR) return;

	const char* APPEND_DETAIL_STRING = ": %s\n";
	const size_t APPEND_LENGTH = strlen(APPEND_DETAIL_STRING) + 1;
	const size_t message_length = strlen(error_message);
	MessageBox(NULL, error_message, getErrorString(ErrorValue), 0x00000000L);
	ExitProcess(0);
}



// capture GL errors
//#include <cstdio>
//#include <cstdint>
void __stdcall
MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{

	if (severity == GL_DEBUG_SEVERITY_NOTIFICATION || severity == 37191)
		return;
	MessageBox(NULL, message, message, 0x00000000L);

	//fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
	//	(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
	//	type, severity, message);

	// __debugbreak();
}
#endif

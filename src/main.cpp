#define WINDOWS_IGNORE_PACKING_MISMATCH


#define EDITOR												1
#define EDITOR_FORCE_RELOAD_FROM_DISK	0
#define VALIDATE_SHADERS_GLSLANG			1
#define OPENGL_DEBUG									1

#define ANTI_TDR											1
#define LAPTOP_GPU_FIX								0

#define FULLSCREEN										0
#define AUTORES												0
#define VSYNC													1

#define DO_PERFECT_FFMPEG_CAPTURE			0


#ifdef RELEASE
	#define DO_PERFECT_FFMPEG_CAPTURE			0
	#define FULLSCREEN										1
	#define ANTI_TDR											1
	#define EDITOR												0
	#define EDITOR_FORCE_RELOAD_FROM_DISK	0
	#define VALIDATE_SHADERS_GLSLANG			0
	#define OPENGL_DEBUG									0
#endif


#define XRES													1280
#define YRES													720


#define MUSIC_DURATION								470

#define MUSICFB												framebuffers[0]
#define PROG_RENDER										programs[0]
#define PROG_POST											programs[1]
#define PROG_MUSIC										programs[2]


#include "definitions.h"
#include "glext.h"
#include "timeapi.h"
#include "shaders/all_shaders.h"


#pragma data_seg(".pids")
	static float music_time;
	static int programs[3];
	static HDC hDC;
	static GLuint ssbo;
	static PFNGLUSEPROGRAMPROC OglUseProgram;
	static PFNGLUNIFORM1FPROC OglUniform1f;
	static PFNGLDISPATCHCOMPUTEPROC OglDispatchCompute;
	static PFNGLCREATESHADERPROGRAMVPROC OglCreateShaderProgramv;

	static constexpr int xres = XRES;
	static constexpr int yres = YRES;

		//xres = XRES; 
		//yres = YRES;

#include "debug.h"
#include "music.h"
#include "editor.h"


#pragma data_seg(".screensettings")
static DEVMODE screenSettingsb = { 
	{0}, 0, 0, sizeof(screenSettingsb), 0, DM_PELSWIDTH|DM_PELSHEIGHT,
	{0}, 0, 0, 0, 0, 0, {0}, 0, 0, XRES, YRES, 0, 0,
	#if(WINVER >= 0x0400)
		0, 0, 0, 0, 0, 0,
			#if (WINVER >= 0x0500) || (_WIN32_WINNT >= 0x0400)
			0, 0
		#endif
	#endif
};



static void __forceinline init_window() {
#if EDITOR
    #define WINDOW_CLASS_NAME "G"
#else
    #define WINDOW_CLASS_NAME (LPCSTR)0xC018
#endif
	#if EDITOR

		for(int i = 0; i < 20; i++){
			editor_keys_pressed[i] = false;
		}
		HINSTANCE hInstance =	GetModuleHandle(NULL);

    WNDCLASS wc = {};
    wc.lpfnWndProc = editor_winapi_window_proc;
    wc.hInstance = hInstance;
    wc.lpszClassName = WINDOW_CLASS_NAME;
    
    RegisterClass(&wc);
		
    int screenWidth = GetSystemMetrics(SM_CXSCREEN); 
    int screenHeight = GetSystemMetrics(SM_CYSCREEN); 

    int windowX = (screenWidth - xres) / 2;
    int windowY = (screenHeight - yres) / 2;
	#endif
	#if AUTORES
		xres = GetSystemMetrics(SM_CXSCREEN);
		yres = GetSystemMetrics(SM_CYSCREEN);
	#else
		//xres_real = GetSystemMetrics(SM_CXSCREEN); 
		//yres_real = GetSystemMetrics(SM_CYSCREEN);
	#endif

	PIXELFORMATDESCRIPTOR pfd = {
		sizeof(pfd), 1, PFD_DRAW_TO_WINDOW|PFD_SUPPORT_OPENGL|PFD_DOUBLEBUFFER, PFD_TYPE_RGBA,
		32, 0, 0, 0, 0, 0, 0, 8, 0, 0, 0, 0, 0, 0, 32, 0, 0, PFD_MAIN_PLANE, 0, 0, 0, 0
	};

	DEVMODE screenSettings = { 
		{0}, 0, 0, sizeof(screenSettings), 0, DM_PELSWIDTH|DM_PELSHEIGHT,
		{0}, 0, 0, 0, 0, 0, {0}, 0, 0, 0, 0, 0, 0,
		#if(WINVER >= 0x0400)
			0, 0, 0, 0, 0, 0,
				#if (WINVER >= 0x0500) || (_WIN32_WINNT >= 0x0400)
				0, 0
			#endif
		#endif
	};

	screenSettings.dmPelsWidth = xres;
	screenSettings.dmPelsHeight = yres;
	screenSettings.dmFields=DM_PELSWIDTH|DM_PELSHEIGHT;

	#if FULLSCREEN
		//SetProcessDPIAware();
		auto disp_settings = ChangeDisplaySettings(&screenSettings, CDS_FULLSCREEN);
		ShowCursor(0);

		#if EDITOR
			hDC = GetDC(hwnd = CreateWindow(
				WINDOW_CLASS_NAME, 0, WS_POPUPWINDOW | WS_VISIBLE, 0, 0, 
				xres, yres,
				0, 0, 0, 0
			));
		#else
			hDC = GetDC(CreateWindow((LPCSTR)0xC018, 0, WS_POPUP | WS_VISIBLE, 0, 0, xres, yres, 0, 0, 0, 0));
		#endif
	#else
		#if EDITOR
			RECT rect;
			rect.left   = 0;
			rect.top    = 0;
			rect.right  = xres;
			rect.bottom = yres;
			DWORD windowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE;
			//DWORD windowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;

			AdjustWindowRect( &rect, windowStyle, FALSE );

			hDC = GetDC(
					hwnd = CreateWindow(
						WINDOW_CLASS_NAME, 0, windowStyle, 
						windowX - xres/2 + 100, windowY - yres/2 + 100, 
						rect.right - rect.left, rect.bottom - rect.top, 
						0, 0, 0, 0)
			);
		#else
			hDC = GetDC(
					CreateWindow((LPCSTR)0xC018, 0, WS_OVERLAPPEDWINDOW | WS_VISIBLE, xres/2, yres/2, xres, yres, 0, 0, 0, 0)
			);
		#endif
	#endif


	SetPixelFormat(hDC, ChoosePixelFormat(hDC, &pfd), &pfd);
	wglMakeCurrent(hDC, wglCreateContext(hDC));

	OglUseProgram = ((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"));
	OglUniform1f = ((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"));
	OglDispatchCompute = ((PFNGLDISPATCHCOMPUTEPROC)wglGetProcAddress("glDispatchCompute"));
	OglCreateShaderProgramv = ((PFNGLCREATESHADERPROGRAMVPROC)wglGetProcAddress("glCreateShaderProgramv"));

	#if OPENGL_DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		oglDebugMessageCallback(dbg_gl_message_callback, 0); 
	#endif

	#if VSYNC
		typedef BOOL (WINAPI *PFNWGLSWAPINTERVALEXTPROC)(int interval);
		auto wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC) wglGetProcAddress("wglSwapIntervalEXT");
		wglSwapIntervalEXT(1);
	#endif
	#if EDITOR
		editor_create_console();
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)editor_console_winapi_message_loop, NULL, 0, NULL);
	#endif
}










static int __forceinline add_program(unsigned int program_type, const char* str, const char* path) {
	#if OPENGL_DEBUG
		shader_strings[shader_count] = str;
		shader_paths[shader_count] = path;
		shader_types[shader_count] = program_type;
		shader_count++;
		return oglCreateShaderProgramv(program_type, 1, &str);
	#else
		return OglCreateShaderProgramv(program_type, 1, &str);
	#endif
}

#include "render.h"

#ifdef _DEBUG
//int WinMainCRTStartup(){
int main(){
#else
#pragma code_seg(".main")
void entrypoint(void) {
#endif
	init_window();
	init_shaders();
	init_resources();


	music_init();

	do {
	#if EDITOR
	  MSG msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) return 0;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	#else
		PeekMessage(0, 0, 0, 0, PM_REMOVE);
	#endif
		main_loop();
		do_editor_stuff();
		SwapBuffers(hDC);
	} while (
	#if OPENGL_DEBUG || EDITOR
		!editor_finished
	#else
		! (GetAsyncKeyState(VK_ESCAPE) || music_time > float(MUSIC_DURATION) - 0.5)
	#endif
	);


	#if EDITOR
		FreeConsole();  // Detach the console
	#endif
	ExitProcess(0);
}



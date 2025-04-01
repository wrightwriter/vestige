#define WINDOWS_IGNORE_PACKING_MISMATCH

#define EDITOR									1
#define EDITOR_FORCE_RELOAD_FROM_DISK			1
#define OPENGL_DEBUG						1
#define FULLSCREEN							0
#define VSYNC										1
#define AUTORES									0


#define XRES										1280
#define YRES										720

//#define SONG_DURATION						420
#define SONG_DURATION						470

#define MUSICFB									framebuffers[0]
#define PROG_RENDER							programs[0]
#define PROG_POST								programs[1]
#define PROG_MUSIC							programs[2]


#include "definitions.h"
#include "glext.h"
#include "timeapi.h"
#include "shaders/all_shaders.h"

#pragma data_seg(".pids")
	static int frame = 1; // ulong? needed?
	static float audio_time;
	static int programs[4];
	static HDC hDC;
	static GLuint ssbo;
	//static GLuint textures[6];
	//static GLuint framebuffers[6];
	static int xres;
	static int yres;

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
			keys_pressed[i] = false;
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
		xres = XRES; 
		yres = YRES;
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
		//auto disp_settings = ChangeDisplaySettings(&screenSettings, CDS_FULLSCREEN);
		auto disp_settings = ChangeDisplaySettings(&screenSettings, CDS_FULLSCREEN);
		//if(DISP_CHANGE_SUCCESSFUL != disp_settings){
//DISP_CHANGE_SUCCESSFUL
		// if(disp_settings == -2){
		// 	MessageBox(NULL, "error", "error", 0x00000000L);
		// }
		ShowCursor(0);
		hDC = GetDC(CreateWindow((LPCSTR)0xC018, 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, xres, yres, 0, 0, 0, 0));
		//hDC = GetDC(CreateWindow((LPCSTR)0xC018, 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0));
	#else
		hDC = GetDC(
			#if EDITOR
				hwnd = 
			#endif
			#if EDITOR
				CreateWindow( WINDOW_CLASS_NAME , 0, WS_OVERLAPPEDWINDOW | WS_VISIBLE, windowX - xres/2, windowY - yres/2, xres, yres, 0, 0, hInstance, 0)
			#else
				CreateWindow((LPCSTR)0xC018, 0, WS_POPUP | WS_VISIBLE, 0, 0, xres, yres, 0, 0, 0, 0)
			#endif
		);
	#endif

	SetPixelFormat(hDC, ChoosePixelFormat(hDC, &pfd), &pfd);
	wglMakeCurrent(hDC, wglCreateContext(hDC));

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

	glDisable(GL_BLEND); // needed? bytes?
}



static int __forceinline add_program(unsigned int program_type, const char* str, const char* path) {
	#if OPENGL_DEBUG
		file_paths[shaders_count] = path;
		shader_types[shaders_count] = program_type;
		shaders_count++;
		return oglCreateShaderProgramv(program_type, 1, &str);
	#else
		return oglCreateShaderProgramv(program_type, 1, &str);
	#endif
}

#include "render.h"

//int __cdecl main(int argc, char* argv[]){
#ifdef _DEBUG
//int WinMainCRTStartup(){
int main(){
#else
#pragma code_seg(".main")
void entrypoint(void) {
//#pragma code_seg(".main")
#endif
//int main(){
//void entrypoint(void) {

	init_window();
	init_shaders();
	init_resources();


	audio_init();

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
	#if OPENGL_DEBUG
		true
	#else
		!GetAsyncKeyState(VK_ESCAPE) || audio_time > SONG_DURATION
	#endif
	);

	#if EDITOR
		FreeConsole();  // Detach the console
	#endif
	ExitProcess(0);
}

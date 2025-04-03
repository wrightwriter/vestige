#define WINDOWS_IGNORE_PACKING_MISMATCH

#define EDITOR												0
#define EDITOR_FORCE_RELOAD_FROM_DISK	0
#define VALIDATE_SHADERS_GLSLANG			0
#define OPENGL_DEBUG									0

#define ANTI_TDR											1
#define LAPTOP_GPU_FIX								0

#define FULLSCREEN										1
#define AUTORES												0
#define VSYNC													1

#define DO_PERFECT_FFMPEG_CAPTURE			0


#define XRES													1280
#define YRES													720


//#define MUSIC_DURATION							420
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
	static int frame = 1;
	static float music_time;
	static int programs[3];
	static HDC hDC;
	static GLuint ssbo;
	static PFNGLUSEPROGRAMPROC OglUseProgram;
	static PFNGLUNIFORM1IPROC OglUniform1i;
	static PFNGLUNIFORM1FPROC OglUniform1f;
	static PFNGLDISPATCHCOMPUTEPROC OglDispatchCompute;
	static PFNGLCREATESHADERPROGRAMVPROC OglCreateShaderProgramv;



//#define oglDispatchCompute ((PFNGLDISPATCHCOMPUTEPROC)wglGetProcAddress("glDispatchCompute"))
	//static GLuint textures[6];
	//static GLuint framebuffers[6];
	//static int xres;
	//static int yres;

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
		//if(DISP_CHANGE_SUCCESSFUL != disp_settings){
//DISP_CHANGE_SUCCESSFUL
		// if(disp_settings == -2){
		// 	MessageBox(NULL, "error", "error", 0x00000000L);
		// }
		ShowCursor(0);
		//hDC = GetDC(CreateWindow((LPCSTR)0xC018, 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, xres, yres, 0, 0, 0, 0));

		#if EDITOR
		//int titleBarHeight = GetSystemMetrics(SM_CYCAPTION);
		//int borderHeight = GetSystemMetrics(SM_CYFRAME) * 1;  // Multiply by 2 for top and bottom borders
		//int totalNonClientHeight = titleBarHeight + borderHeight;
		//
		//// Adjust for window borders and title bar
		//RECT adjustedRect = { 0, 0, xres, yres };
		//AdjustWindowRect(&adjustedRect, WS_OVERLAPPEDWINDOW, FALSE);
		//
		//int windowWidth = adjustedRect.right - adjustedRect.left;
		//int windowHeight = adjustedRect.bottom - adjustedRect.top;
		
		//int titleBarHeight = GetSystemMetrics(SM_CYCAPTION);
		//int borderHeight = GetSystemMetrics(SM_CYFRAME) * 2; // Top & bottom
		//int borderWidth = GetSystemMetrics(SM_CXFRAME) * 2;  // Left & right

		//// Adjust for window borders and title bar
		//RECT adjustedRect = { 0, 0, xres, yres };
		//AdjustWindowRect(&adjustedRect, WS_OVERLAPPEDWINDOW, FALSE);

		//// Get total window size including non-client areas
		//int windowWidth = adjustedRect.right - adjustedRect.left;
		//int windowHeight = adjustedRect.bottom - adjustedRect.top;



			hDC = GetDC(hwnd = CreateWindow(
				//WINDOW_CLASS_NAME, 0, WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, -totalNonClientHeight, 
				//WINDOW_CLASS_NAME, 0, WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 
				//WINDOW_CLASS_NAME, 0, WS_POPUP | WS_VISIBLE, 0, 0, 
				WINDOW_CLASS_NAME, 0, WS_POPUPWINDOW | WS_VISIBLE, 0, 0, 
				//xres, yres + totalNonClientHeight, 
				xres, yres,
				0, 0, 0, 0
			));
//			LONG style = GetWindowLong(hwnd, GWL_STYLE);
//style &= ~WS_CAPTION;  // Removes the title bar
//SetWindowLong(hwnd, GWL_STYLE, style);
//SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);

			//hDC = GetDC(CreateWindow(WINDOW_CLASS_NAME, 0, WS_POPUP | WS_VISIBLE, 0, 0, xres, yres, 0, 0, 0, 0));
			//SetWindowPos(hwnd, HWND_TOP, 0, -20, screenWidth, screenHeight, SWP_NOZORDER);
			//LONG style = GetWindowLong(hwnd, GWL_STYLE);
			//style &= ~WS_OVERLAPPEDWINDOW;  // Removes the title bar, borders, and maximize/minimize buttons
			//SetWindowLong(hwnd, GWL_STYLE, style);

			//style |= WS_POPUP; // Use WS_POPUP to make it borderless
			// Add the title bar (and border)
			//style |= WS_OVERLAPPEDWINDOW;
			// Remove the title bar and border

			//}
		#else
			hDC = GetDC(CreateWindow((LPCSTR)0xC018, 0, WS_POPUPWINDOW | WS_VISIBLE, 0, 0, xres, yres, 0, 0, 0, 0));
		#endif
		//hDC = GetDC(CreateWindow((LPCSTR)0xC018, 0, WS_POPUP | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 0, 0, 0, 0, 0, 0));
	#else
		hDC = GetDC(
			#if EDITOR
				hwnd = 
			#endif
			#if EDITOR
				CreateWindow( WINDOW_CLASS_NAME , 0, WS_POPUPWINDOW | WS_VISIBLE, windowX - xres/2 + 1300, windowY - yres/2 + 100, xres, yres, 0, 0, hInstance, 0)
				//CreateWindow( WINDOW_CLASS_NAME , 0, WS_POPUPWINDOW | WS_VISIBLE, windowX - xres/2, windowY - yres/2, xres, yres, 0, 0, hInstance, 0)
				//CreateWindow( WINDOW_CLASS_NAME , 0, WS_POPUPWINDOW | WS_VISIBLE, windowX - xres/2, windowY - yres/2, xres, yres, 0, 0, hInstance, 0)
				//CreateWindow( WINDOW_CLASS_NAME , 0, WS_BORDER | WS_VISIBLE, windowX - xres/2, windowY - yres/2, xres, yres, 0, 0, hInstance, 0)
			#else
				CreateWindow((LPCSTR)0xC018, 0, WS_POPUP | WS_VISIBLE, 0, 0, xres, yres, 0, 0, 0, 0)
			#endif
		);
	#endif

	SetPixelFormat(hDC, ChoosePixelFormat(hDC, &pfd), &pfd);
	wglMakeCurrent(hDC, wglCreateContext(hDC));

	OglUseProgram = ((PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram"));
	OglUniform1i = ((PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i"));
	OglUniform1f = ((PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f"));
	OglDispatchCompute = ((PFNGLDISPATCHCOMPUTEPROC)wglGetProcAddress("glDispatchCompute"));
	OglCreateShaderProgramv = ((PFNGLCREATESHADERPROGRAMVPROC)wglGetProcAddress("glCreateShaderProgramv"));

//oglCreateShaderProgramv

//#define oglUseProgram //#define 
// 
//#define oglDispatchCompute 

	//SetWindowPos(hwnd, HWND_TOPMOST,0, -titleBarHeight - 200, windowWidth + 100, windowHeight + 100, SWP_NOZORDER | SWP_NOMOVE | SWP_NOSIZE);

	//auto disp_settings = ChangeDisplaySettings(&screenSettings, CDS_FULLSCREEN);
	//SWP_NOMOVE
	//SetWindowPos(hwnd, HWND_TOP,0, -titleBarHeight - 200, windowWidth + 400, windowHeight + 400, SWP_NOMOVE | SWP_NOSIZE);

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

	//glDisable(GL_BLEND); // needed? bytes?
}











static int __forceinline add_program(unsigned int program_type, const char* str, const char* path) {
	#if OPENGL_DEBUG
		shader_strings[shaders_count] = str;
		file_paths[shaders_count] = path;
		shader_types[shaders_count] = program_type;
		shaders_count++;
		return oglCreateShaderProgramv(program_type, 1, &str);
	#else
		return OglCreateShaderProgramv(program_type, 1, &str);
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


	music_init();

	//music_seek(85);

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
		true
	#else
		! (GetAsyncKeyState(VK_ESCAPE) || music_time > float(MUSIC_DURATION) - 0.5)
	#endif
	);


	#if EDITOR
		FreeConsole();  // Detach the console
	#endif
	ExitProcess(0);
}



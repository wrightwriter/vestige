#pragma once


// Function to send text to the console window
void PrintToConsole(const char* message) {
#if EDITOR
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

		WriteConsoleA(hConsole, message, lstrlenA(message), NULL, NULL);
#endif
}


#if EDITOR
	static HWND hwnd;
	static HWND hwndConsole; 
	static float mouse_x_ndc = 0;
	static float mouse_y_ndc = 0;
	static bool win_focused = false;
	static bool gui_toggled = true;
	static bool keys_pressed[20] = {};
	static bool paused = false;

	#define key_space_down  keys_pressed[0]
	#define key_left_down  keys_pressed[1]
	#define key_right_down  keys_pressed[2]
	#define key_lmb_down  keys_pressed[3]
	#define key_aaaaa_down  keys_pressed[4]

	void CreateChildConsole() {
		if (!AllocConsole()) {
			// DWORD dwError = GetLastError();
			// char errorMessage[256];
			// snprintf(errorMessage, sizeof(errorMessage), "AllocConsole failed with error %lu", dwError);
			// MessageBox(NULL, errorMessage, "Error", MB_OK | MB_ICONERROR);
			// return;
		}
		hwndConsole = GetConsoleWindow();
		ShowWindow(hwndConsole, SW_SHOW);
		RECT rect;
		GetWindowRect(hwnd, &rect);
		SetWindowPos(hwndConsole, HWND_TOP, rect.left, rect.bottom - 300, rect.right - rect.left, 300, SWP_NOZORDER);
		SetForegroundWindow(hwndConsole);
	}

	// Main message loop
	void MessageLoop() {
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}


	LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
			if(win_focused)
			switch (uMsg) {
					case WM_DESTROY:
							PostQuitMessage(0);
							return 0;
							
					case WM_LBUTTONDOWN:  
							key_lmb_down = true;
							return 0;
					case WM_KEYDOWN:
							if (wParam == VK_SPACE) {
								key_space_down = true;
							}
							if (wParam == VK_LEFT) {
								key_left_down = true;
							}
							if (wParam == VK_RIGHT) {
								key_right_down = true;
							}
							if (wParam == VK_OEM_3) {
								key_aaaaa_down = true;
							}
							break;
			}
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
#endif

static void __forceinline do_editor_stuff(){
	// Focus main window
#if EDITOR

	double audio_time = audio_get_time_seconds();
	if(key_aaaaa_down){
		gui_toggled = !gui_toggled;
	}
	//bool space = GetAsyncKeyState(VK_SPACE);
	 POINT cursorPos;
	{
		GetCursorPos(&cursorPos);
		ScreenToClient(hwnd, &cursorPos); 
		
    RECT rect;
    GetClientRect(hwnd, &rect);  // Get window size

    int winWidth = rect.right - rect.left;
    int winHeight = rect.bottom - rect.top;

    // Convert to NDC
    mouse_x_ndc = (2.0f * cursorPos.x) / winWidth - 1.0f;
    mouse_y_ndc = 1.0f - (2.0f * cursorPos.y) / winHeight;
	}
	{
		win_focused = GetForegroundWindow() == hwnd;
	}
	//if(win_focused){
	if(true){
	 	// Get the dimensions of the OpenGL window
	 	RECT rect;
	 	GetWindowRect(hwnd, &rect);

	 	// Update the console window's position to stay at the bottom
	 	//SetWindowPos(hwndConsole, HWND_TOP, rect.left, rect.bottom - 300, rect.right - rect.left, 300, SWP_NOZORDER);
	 	//SetWindowPos(hwndConsole, 0, rect.left, rect.bottom, rect.right - rect.left, 300, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	 	SetWindowPos(hwndConsole, HWND_TOP, rect.left, rect.bottom, rect.right - rect.left, 300, SWP_NOACTIVATE | SWP_NOSIZE | SWP_SHOWWINDOW);
	 	//SetWindowPos(hwndConsole, 0, rect.left, rect.bottom, rect.right - rect.left, 300, SWP_NOACTIVATE | SWP_SHOWWINDOW);

		

	 	// Print a message to the console
	 	//PrintToConsole("This is a message in the child console.\n");
	}
	{
		if(key_space_down){ 
			if(paused){		// unpause
				audio_unmute();
				audio_seek(editor_time);
			} else {			// pause
				audio_mute();
				editor_time = audio_time;
			}
			paused = !paused;
		}
		if(!paused){
			editor_time = audio_time;
		}
	}


	
	//if(key_lmb_down && win_focused){
	if((GetAsyncKeyState(VK_LBUTTON) & 0x8000) && win_focused){
		double target_time = SONG_DURATION * (mouse_x_ndc + 1) / 2;
		if(target_time < 0){
			target_time = 0;
		}
		if(paused){
			editor_time = target_time;
		}
		audio_seek(target_time);
	}

	{
		char buffer[50];
    sprintf(buffer, "%.2f", editor_time);
		SetWindowText(hwnd, buffer);
	}

	//if( audio_time > 45){
	//	audio_seek(0);
	//}
	if( audio_time > SONG_DURATION - 0.5){
		audio_seek(0);
	}
	{
		oglUseProgram(0);



		if(gui_toggled){
			float width = 0.05;
			float height = 0.05;
			float pos_y = -1;
			float pos_x = editor_time/float(SONG_DURATION)*2.0 - 1.;

			
			glPointSize(30.0f); // Set point size (in pixels)
			glColor3f(0.,0,0);
			glBegin(GL_POINTS);  
				glVertex2f(pos_x, pos_y);  // Point at the center
			glEnd();
		}
		//glRects(-1, -1, 1, 1);
	}
	if(shader_failed_compile){
		glPointSize(70.0f); // Set point size (in pixels)
	 	glBegin(GL_POINTS);  
	 	glVertex2f(-0.8, 0.8);  // Point at the center
	 	glEnd();
	
	}

	for(int i = 0; i < 20; i++){
		keys_pressed[i] = false;
	}

	if(audio_shader_just_reloaded){
		audio_render();
		audio_seek(audio_time);
		PrintToConsole("-------- Audio shader reloaded! -------- \n");
	}

	audio_shader_just_reloaded = false;
#endif
	
}


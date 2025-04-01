#pragma once


void editor_print_to_console(const char* message) {
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
	#define key_s_down  keys_pressed[5]

	void editor_create_console() {
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
	void editor_console_winapi_message_loop() {
		MSG msg;
		while (GetMessage(&msg, NULL, 0, 0)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}


	LRESULT CALLBACK editor_winapi_window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
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
							if (wParam == 'S') {
								key_s_down = true;
							}
							break;
			}
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
#endif

#if EDITOR
	void editor_print_to_console(const char* message);
#endif

static void _inline editor_reload_from_disk() {
	#if OPENGL_DEBUG
	bool prev_shader_failed_compile = shader_failed_compile;
	shader_failed_compile = false;

	static bool is_first_launch = true;
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
				#if EDITOR_FORCE_RELOAD_FROM_DISK
					if(!is_first_launch){
						continue;
					}
				#else
					continue;
				#endif
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
				editor_print_to_console("------- Shader Error -------\n");
				editor_print_to_console(file_paths[i]);
				editor_print_to_console(log);
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
					editor_print_to_console("Reloaded shader:\n");
					editor_print_to_console(file_paths[i]);
					editor_print_to_console("\n");
					editor_print_to_console("\n");
					editor_print_to_console("\n");
					editor_print_to_console("\n");
					editor_print_to_console("\n");
					editor_print_to_console("\n");
				#endif
			}
		}
	}

	if(prev_shader_failed_compile == true && shader_failed_compile == false){
		#if EDITOR
			editor_print_to_console("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");
		#endif
	}

	is_first_launch = false;

	#endif
}


static void _inline editor_try_reload() {
	#if OPENGL_DEBUG
		if(!GetAsyncKeyState(VK_LCONTROL) || !GetAsyncKeyState(0x53)) {
			return;
		}
		Sleep(400); // Genius
		editor_reload_from_disk();
	#endif
}



static void __forceinline do_editor_stuff(){
	// Focus main window
#if EDITOR
	double audio_time = audio_get_time_seconds();

	// --- Toggle gui
	if(key_aaaaa_down){
		gui_toggled = !gui_toggled;
	}

	// --- Mouse position
	{
		POINT cursorPos;
		GetCursorPos(&cursorPos);
		ScreenToClient(hwnd, &cursorPos); 
		
    RECT rect;
    GetClientRect(hwnd, &rect); 

    int winWidth = rect.right - rect.left;
    int winHeight = rect.bottom - rect.top;

    mouse_x_ndc = (2.0f * cursorPos.x) / winWidth - 1.0f;
    mouse_y_ndc = 1.0f - (2.0f * cursorPos.y) / winHeight;
	}
	{
		win_focused = GetForegroundWindow() == hwnd;
	}
	// --- Position console
	{
	 	RECT rect;
	 	GetWindowRect(hwnd, &rect);

	 	SetWindowPos(hwndConsole, HWND_TOP, rect.left, rect.bottom, rect.right - rect.left, 300, SWP_NOACTIVATE | SWP_NOSIZE | SWP_SHOWWINDOW);
	}

	// --- Pause
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



	// --- Update titlebar
	{
		char buffer[50];
    sprintf(buffer, "%.2f", editor_time);
		SetWindowText(hwnd, buffer);
	}


	// --- Draw Gui
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

		if(shader_failed_compile){
			glPointSize(70.0f); // Set point size (in pixels)
			glBegin(GL_POINTS);  
			glVertex2f(-0.8, 0.8);  // Point at the center
			glEnd();
		}
	}

	// --- Seek
	{
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

		if( audio_time > SONG_DURATION - 0.5){
			audio_seek(0);
		}
	}

	// --- Re-render audio
	if(audio_shader_just_reloaded){
		audio_render();
		audio_seek(audio_time);
		editor_print_to_console("-------- Audio shader reloaded! -------- \n");
	}

	// --- Save wav
	{
		if(key_s_down){
			audio_save_wav();
		}
	}

	// --- Reset keys & vars
	{
		for(int i = 0; i < 20; i++){
			keys_pressed[i] = false;
		}
		audio_shader_just_reloaded = false;
	}
#endif
	
}



#pragma once


void editor_print_to_console(const char* message) {
#if EDITOR
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

		WriteConsoleA(hConsole, message, lstrlenA(message), NULL, NULL);
#endif
}


void dbg_validate_shaders(){
	#if EDITOR && VALIDATE_SHADERS_GLSLANG
    // char shader_validation_errors[MAX_ERROR_SIZE] = {0};
    for (size_t i = 0; i < shaders_count; i++) {
        // Write shader to a temporary file
        FILE* file = fopen("shader.glsl", "w");
        if (!file) {
            MessageBoxA(0, "Failed to create temp shader file.", "Error", MB_OK | MB_ICONERROR);
            return;
        }
        fprintf(file, "%s", shader_strings[i]);
        fclose(file);

				const char* shaderSuffix;
				switch (shader_types[i]) {
						case GL_VERTEX_SHADER: shaderSuffix = "vert"; break; 
						case GL_FRAGMENT_SHADER: shaderSuffix = "frag"; break; 
						case GL_COMPUTE_SHADER: shaderSuffix = "comp"; break; 
						default: shaderSuffix = "unknown"; break;
				}

        char command[512];
        snprintf(command, sizeof(command), "\"glslangValidator.exe\" -e main --auto-map-bindings --auto-map-locations --glsl-version 460 --no-link -S %s shader.glsl",
                 shaderSuffix);

        SECURITY_ATTRIBUTES saAttr = {sizeof(SECURITY_ATTRIBUTES), NULL, TRUE};
        HANDLE hReadPipe, hWritePipe;
        CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 0);

        STARTUPINFOA si = {sizeof(STARTUPINFO)};
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdOutput = hWritePipe;
        si.hStdError = hWritePipe;

        PROCESS_INFORMATION pi;
        if (CreateProcessA(NULL, command, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
            CloseHandle(hWritePipe);

            char buffer[(4096 * 40)] = {0};
            DWORD bytesRead;
            ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
            buffer[bytesRead] = '\0'; // Null-terminate

            CloseHandle(hReadPipe);

            if (strstr(buffer, "ERROR")) { // If output contains "ERROR", log it
            	MessageBoxA(0, "Validation fail", "Error", MB_OK | MB_ICONERROR);
                // strcat(shader_validation_errors, "------ SHADER ");
                // strcat(shader_validation_errors, file_paths[i]);
                // strcat(shader_validation_errors, " ------\n");
                // strcat(shader_validation_errors, buffer);
                // strcat(shader_validation_errors, "\n");
            }
        		editor_print_to_console(buffer);

            WaitForSingleObject(pi.hProcess, INFINITE);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        } else {
            MessageBoxA(0, "Failed to run glslangValidator.exe", "Error", MB_OK | MB_ICONERROR);
            return;
        }
    }
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

	static bool editor_finished = false;

	//static float editor_loop_start = 0;
	//static float editor_loop_end = MUSIC_DURATION - 0.002;

	static float editor_loop_start = 60 + 18;
	static float editor_loop_end = 60 + 18 + 8;

	static bool editor_loop_popup_finished = false;
	static LARGE_INTEGER editor_timer_start, editor_timer_freq;
	static float editor_average_ms = 0.;
	
	static bool editor_just_started = true;

	FILE* ffmpeg = nullptr;
	static bool editor_is_recording = false;

	#define key_space_down  keys_pressed[0]
	#define key_left_down  keys_pressed[1]
	#define key_right_down  keys_pressed[2]
	#define key_lmb_down  keys_pressed[3]
	#define key_aaaaa_down  keys_pressed[4]
	#define key_s_down  keys_pressed[5]
	#define key_l_down keys_pressed[6]
	#define key_r_down keys_pressed[7]

	void editor_create_console() {
		QueryPerformanceFrequency(&editor_timer_freq);
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
							if (wParam == 'L') {
								key_l_down = true;
							}
							if (wParam == 'R') {
								key_r_down = true;
							}
							break;
			}
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
#endif

#if EDITOR
	void editor_print_to_console(const char* message);
#endif


void editor_start_timer() {
#if EDITOR
	QueryPerformanceCounter(&editor_timer_start);
#endif
}


void editor_end_timer(const char* label) {
#if EDITOR
	char out_buffer[1024];
	LARGE_INTEGER editor_timer_end;

	QueryPerformanceCounter(&editor_timer_end);

	double elapsed_ms = (double)(editor_timer_end.QuadPart - editor_timer_start.QuadPart) * 1000.0 / editor_timer_freq.QuadPart;
	
	sprintf(out_buffer, "%.2f ms %s", float(elapsed_ms), label);
	//wsprintfA(out_buffer, , elapsed_ms, label);

	editor_print_to_console(out_buffer);
	editor_print_to_console("\n");
#endif
}

void editor_do_fps_counter(){
#if EDITOR
	static bool intialized = false;


	//static LARGE_INTEGER timer_start;
	static LARGE_INTEGER last_time;
	static LARGE_INTEGER curr_time;


	QueryPerformanceCounter(&curr_time);


	//QueryPerformanceCounter(&timer_start);
	
	//if(!initia)


	if(intialized){
		double new_ms = (double)(curr_time.QuadPart - last_time.QuadPart) * 1000.0 / editor_timer_freq.QuadPart;

		float lerp_speed = 0.04;
		editor_average_ms = new_ms * lerp_speed + editor_average_ms * (1.-lerp_speed);
	}

	last_time = curr_time;

	intialized = true;
#endif
}

static void _inline editor_reload_from_disk() {
	#if EDITOR
	editor_start_timer();
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

	bool a_shader_reloaded = false;
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
			a_shader_reloaded = true;
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

	if(a_shader_reloaded && !shader_failed_compile){
		//char char_buff[100];
		//sprintf("%f.2ms\n")
		editor_end_timer(" - shader compilation");
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



// Global variables to store the input values

// Window procedure to handle the interactions with the custom window
LRESULT CALLBACK editor_loop_popup_window_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
#if EDITOR
    switch (uMsg) {
        case WM_COMMAND:
            if (LOWORD(wParam) == 103) { // OK button clicked
                char buffer1[16], buffer2[16];
                GetWindowText(GetDlgItem(hwnd, 101), buffer1, sizeof(buffer1));
                GetWindowText(GetDlgItem(hwnd, 102), buffer2, sizeof(buffer2));
								editor_loop_start = atoi(buffer1);
								editor_loop_end = atoi(buffer2);
								editor_loop_popup_finished = true;
                PostMessage(hwnd, WM_CLOSE, 0, 0);
								DestroyWindow(hwnd);
                return 0;
            }
            break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
#endif
	return LRESULT(0);
}

// Function to create the input window and get two integers from the user
void editor_do_loop_popup(const char* title) {
#if EDITOR
		editor_loop_popup_finished = false;
    WNDCLASS wc = {0};
    wc.lpfnWndProc = editor_loop_popup_window_proc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "InputDialogClass";
    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, "InputDialogClass", title, WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                               CW_USEDEFAULT, CW_USEDEFAULT, 300, 400, NULL, NULL, GetModuleHandle(NULL), NULL);

    if (!hwnd) {
        MessageBoxA(NULL, "Failed to create window", "Error", MB_OK | MB_ICONERROR);
				return;
    }

    CreateWindow("STATIC", "First Value:", WS_VISIBLE | WS_CHILD, 10, 40, 200, 20, hwnd, NULL, NULL, NULL);
    CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 10, 70, 200, 20, hwnd, (HMENU)101, NULL, NULL);

    CreateWindow("STATIC", "Second Value:", WS_VISIBLE | WS_CHILD, 10, 100, 200, 20, hwnd, NULL, NULL, NULL);
    CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_NUMBER, 10, 130, 200, 20, hwnd, (HMENU)102, NULL, NULL);

    CreateWindow("BUTTON", "OK", WS_VISIBLE | WS_CHILD, 10, 160, 50, 20, hwnd, (HMENU)103, NULL, NULL);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) && !editor_loop_popup_finished) {
        if (!IsDialogMessage(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
#endif
}


// Read OpenGL framebuffer and send to FFMPEG
void editor_ffmpeg_capture_frame() {
#if EDITOR
	// PBOs for non-blocking readback
	static GLuint pboIds[2] = { 0, 0 };
	static int pboIndex = 0;
	static bool pboInitialized = false;

	if(!pboInitialized){
		oglGenBuffers(2, pboIds); // Generate 2 PBOs for double-buffering
		for (int i = 0; i < 2; i++) {
				oglBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[i]);
				oglBufferData(GL_PIXEL_PACK_BUFFER, xres * yres * 3, NULL, GL_STREAM_READ);
		}
		oglBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		pboInitialized = true;
	}

	{
    int nextIndex = (pboIndex + 1) % 2; // Swap PBOs
    glReadBuffer(GL_BACK);

    // Bind new PBO and read pixels into it asynchronously
    oglBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[pboIndex]);
    glReadPixels(0, 0, xres, yres, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    // Process previous PBO
    oglBindBuffer(GL_PIXEL_PACK_BUFFER, pboIds[nextIndex]);
    unsigned char* pixels = (unsigned char*)oglMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
		if (pixels) {
			fwrite(pixels, 1, xres * yres * 3, ffmpeg); // Directly write without flipping
			oglUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    }
    //if (pixels) {
    //    // Flip image vertically (since OpenGL has bottom-left origin)
    //    unsigned char* flippedPixels = (unsigned char*)malloc(xres * yres * 3);
    //    for (int y = 0; y < yres; y++) {
    //        memcpy(flippedPixels + (yres - 1 - y) * xres * 3, pixels + y * xres * 3, xres * 3);
    //    }

    //    fwrite(flippedPixels, 1, xres * yres * 3, ffmpeg);
    //    free(flippedPixels);

    //    oglUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    //}
    oglBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
    pboIndex = nextIndex; // Swap buffers
	}
#endif
}


static void __forceinline do_editor_stuff(){
	// Focus main window
#if EDITOR
	bool should_toggle_recording = key_r_down;
	if(editor_just_started){
		music_seek(editor_loop_start);
	}
	#if DO_PERFECT_FFMPEG_CAPTURE
		static bool finished_perfect_ffmpeg_capture = false;
		if(!finished_perfect_ffmpeg_capture){
			if(editor_just_started){
				music_mute();
				should_toggle_recording = true;
			//} else if(editor_time > MUSIC_DURATION){
			//} else if(editor_time > MUSIC_DURATION - 0.2){
			} else if(editor_time > MUSIC_DURATION - 0.2){
				should_toggle_recording = true;
				finished_perfect_ffmpeg_capture = true;
			}
		}

	#endif
	if(should_toggle_recording){
		if(!editor_is_recording){
			//ffmpeg = _popen(
			//		"ffmpeg -y -f rawvideo -pixel_format rgb24 -video_size 1280x720 -r 60 -i - " // HARDCODED RESOLUTION
			//		"-c:v libx264 -preset fast -crf 18 -pix_fmt yuv420p output.mp4",
			//		"wb"
			//);

			ffmpeg = _popen(
				"ffmpeg -y "                                // Overwrite output file if it exists
				"-f rawvideo "                              // Input format: raw pixel data
				"-thread_queue_size 512 "
				"-pixel_format rgb24 "                      // Pixel format: 24-bit RGB (8 bits per channel)
				"-video_size 1280x720 "                     // Input resolution (must match OpenGL framebuffer)
				"-r 60 "                                    // Frame rate: 30 FPS
				"-i - "                                     // Read input from stdin (piped from our program)
				#if DO_PERFECT_FFMPEG_CAPTURE
					"-i song.wav "  // Input WAV file
				#endif
				"-vf vflip "                                // Flip the image vertically (fix OpenGL bottom-to-top issue)
				//"-vf scale=3840:2160:flags=lanczos,vflip "  // Upscale to 4K with Lanczos filter and flip the image
				"-c:v libx264 "                             // Use H.264 codec for video compression
				"-preset fast "                             // Optimize for faster encoding

				// ----	QUALITY ---- //
				"-crf 18 "                                  // Quality setting (lower = better, 18 = visually lossless)

				"-pix_fmt yuv420p "                         // Convert RGB to YUV 4:2:0 for playback compatibility

				"-g 120 "                                   // Keyframe interval for YouTube (2 seconds at 60 FPS)
				"-c:a aac "                                 // Audio codec: AAC
				"-b:a 320k "                                // Audio bit rate: 320 kbps
				//"-ar 48000 "                                // Audio sample rate: 48 kHz
				"-ar 44100 "                                // Audio sample rate: 48 kHz

				//"outputc.mp4",                               // Output file name
				//"output_bw10.mp4",                               // Output file name
				"output_bw1.mp4",                               // Output file name
				"wb"
			);
			if (!ffmpeg) {
					MessageBox(NULL, "Failed to start FFMPEG!", "Error", MB_ICONERROR);
					//return 1;
			}
			editor_is_recording = true;
		} else {
			_pclose(ffmpeg);
			editor_is_recording = false;
			#if DO_PERFECT_FFMPEG_CAPTURE
					MessageBox(NULL, "Finished FFMPEG capture!", "Woo", MB_ICONERROR);
			#endif
		}
	}

	if(editor_is_recording){
		editor_ffmpeg_capture_frame();
	}

	double music_time = music_get_time_seconds();

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


		#if FULLSCREEN
			winWidth = xres;
			winHeight = yres;
		#else

		#endif

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

		#if !FULLSCREEN
			SetWindowPos(hwndConsole, HWND_TOP, rect.left, rect.bottom, rect.right - rect.left, 300, SWP_NOACTIVATE | SWP_NOSIZE | SWP_SHOWWINDOW);
		#endif
	}

	if(key_l_down){
		editor_do_loop_popup("Loop start");

		if(editor_loop_start == 0 && editor_loop_end == 0){
			editor_loop_end = MUSIC_DURATION;
		}
		if(editor_loop_start < 0){
			editor_loop_start = 0;
		}

		if(editor_loop_end > MUSIC_DURATION){
			editor_loop_end = MUSIC_DURATION;
		}
		
	}

	// --- Pause
	{
		if(key_space_down){ 
			if(paused){		// unpause
				music_unmute();
				music_seek(editor_time);
			} else {			// pause
				music_mute();
				editor_time = music_time;
			}
			paused = !paused;
		}
		if(!paused){
			#if DO_PERFECT_FFMPEG_CAPTURE
				editor_time += 1./60.;
			#else
				editor_time = music_time;
			#endif
		}
	}


	// --- Update titlebar
	{
		char buffer[650];
		//editor_average_ms
    sprintf(buffer, "%.2fs ------- %.2fms ------- %.2fps                 loop: %.2f - %.2f ", editor_time, editor_average_ms, 1000./editor_average_ms, editor_loop_start, editor_loop_end);
		SetWindowText(hwnd, buffer);
		SetWindowText(hwndConsole, buffer);
	}


	// --- Draw Gui

	#if !DO_PERFECT_FFMPEG_CAPTURE
		//if(false)
	{
		oglUseProgram(0);

		if(gui_toggled){
			float width = 0.05;
			float height = 0.05;
			float pos_y = -1;
			float pos_x = editor_time/float(MUSIC_DURATION)*2.0 - 1.;

			
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
	#endif

	// --- Seek
	{
		bool is_mouse_seeking = (GetAsyncKeyState(VK_LBUTTON) & 0x8000);
		bool is_kbd_seeking = key_left_down || key_right_down;
		bool should_seek = is_kbd_seeking || is_mouse_seeking;
		if(should_seek && win_focused){
			double target_time;
			if(is_mouse_seeking){
				target_time = MUSIC_DURATION * (mouse_x_ndc + 1) / 2;
			} else if (is_kbd_seeking){
				float seek_amt_s = 10.;
				target_time += float(key_right_down) * seek_amt_s;
				target_time -= float(key_left_down) * seek_amt_s;
			}

			if(target_time < 0){
				target_time = 0;
			}
			if(paused){
				editor_time = target_time;
			}
			music_seek(target_time);
		}

		if( music_time > editor_loop_end - 0.04){
			music_seek(editor_loop_start);
		}
	}

	// --- Re-render audio
	if(audio_shader_just_reloaded){
		music_render();
		music_seek(music_time);
		editor_print_to_console("-------- Audio shader reloaded! -------- \n");
	}

	// --- Save wav
	{
		if(key_s_down){
			music_save_wav();
		}
	}

	// --- Reset keys & vars
	{
		for(int i = 0; i < 20; i++){
			keys_pressed[i] = false;
		}
		audio_shader_just_reloaded = false;
	}
	editor_just_started = false;
#endif
	
}



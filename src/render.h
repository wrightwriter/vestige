#pragma once


static void __forceinline init_shaders() {
		programs[0] = add_program(GL_COMPUTE_SHADER, render_comp, "src/shaders/render.comp");
		programs[1] = add_program(GL_FRAGMENT_SHADER, post_frag, "src/shaders/post.frag");
		programs[2] = add_program(GL_COMPUTE_SHADER, music_comp, "src/shaders/music.comp");
		int _programs[] = {programs[0], programs[1], programs[2]};
		dbg_check_shader_compilation(_programs);
		dbg_validate_shaders();
		// dbg_nocheck_shader_compilation(_programs);
		//editor_reload_from_disk();
}

static void __forceinline init_resources() {
	// ssbo

	{
		//oglNamedBufferStorage(ssbo,1000000 * 200,0,0); // 1 megabyte, too much
		//oglNamedBufferStorage(ssbo, 262144000,0,GL_CLIENT_STORAGE_BIT); // 
		oglCreateBuffers(1, &ssbo);
		//oglNamedBufferStorage(ssbo, 262144000,0,0); // 
		oglNamedBufferStorage(ssbo, 292144000,0,0); // 
		oglBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
	}

	//// framebuffer
	//for(int i = 0; i < 4; i++) {
	//	glGenTextures(1, &textures[i] );
	//	glBindTexture(GL_TEXTURE_2D, textures[i] );
	//	GLint internal_format;
	//	glTexImage2D(GL_TEXTURE_2D, 0, internal_format = GL_RGBA32F, MUSICRESX, MUSICRESY, 0, GL_RGBA, GL_FLOAT, 0);
	//	// parameters
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	//	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//	{
	//		oglCreateFramebuffers(1, &framebuffers[i]);
	//		//oglBindFramebuffer(GL_FRAMEBUFFER)
	//		oglNamedFramebufferTexture(
	//			framebuffers[i],
	//			GL_COLOR_ATTACHMENT0,
	//			textures[i],
	//			0
	//		);
	//		const GLenum attachments[] = { GL_COLOR_ATTACHMENT0 };
	//		oglNamedFramebufferDrawBuffers(framebuffers[i], 1, attachments);
	//	}
	//	oglBindTextureUnit(i + 1, textures[i]);
	//	oglBindImageTexture(i + 1, textures[i], 0, 0, 0, GL_READ_WRITE, internal_format );
	//}
}

static void __forceinline pre_loop() { 

}




static void __forceinline main_loop() {
	editor_do_fps_counter();
	editor_try_reload();
	#if EDITOR
		audio_time = editor_time;
	#else
		audio_time = audio_get_time_seconds();
	#endif

	//glViewport(0,0,XRES, YRES);
	//glViewport(0,0,xres, yres);
	//glViewport(0,0,1280, 720);
	{
		oglUseProgram(PROG_RENDER);
		//oglUniform1i(0, frame);
		oglUniform1f(1, audio_time);
		oglUniform2f(2, xres, yres);
		oglDispatchCompute(64 * 64 * 2 / 16*2, 1, 1);
		frame += 1;
	}
	{
		//glViewport(xres / 2 - xres_sketch/2,0,xres_sketch, yres_sketch);
		//oglBindFramebuffer(GL_FRAMEBUFFER, 0);
		//oglBindFramebuffer(GL_FRAMEBUFFER, framebuffers[0]);
		// glDrawBuffer(GL_BACK);

		oglUseProgram(PROG_POST);
		oglUniform1i(0, frame);
		oglUniform1f(1, audio_time);
		oglUniform2f(2, xres, yres);
		glRects(-1, -1, 1, 1);
	}

	
	//((PFNGLBLITNAMEDFRAMEBUFFERPROC)wglGetProcAddress( "glBlitNamedFramebuffer"))(
	//	0, framebuffers[0], 0, 0,
	//	1280, 720, 
	//	0,0,
	//	1920, 1080, 
	//	GL_COLOR_BUFFER_BIT, GL_LINEAR
	//);
	//((PFNGLBLITNAMEDFRAMEBUFFERPROC)wglGetProcAddress( "glBlitNamedFramebuffer"))(
	//	framebuffers[0], 0, 0, 0,
	//	1920, 1080, 
	//	0,0,
	//	1920, 1080, 
	//	GL_COLOR_BUFFER_BIT, GL_LINEAR
	//);
}

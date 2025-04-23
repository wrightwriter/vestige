#pragma once


static void __forceinline init_shaders() {
		programs[0] = add_program(GL_COMPUTE_SHADER, render_comp, "src/shaders/render.comp");
		programs[1] = add_program(GL_FRAGMENT_SHADER, post_frag, "src/shaders/post.frag");
		programs[2] = add_program(GL_COMPUTE_SHADER, music_comp, "src/shaders/music.comp");
		int _programs[] = {programs[0], programs[1], programs[2]};
		gl_debug(_programs);
		// gl_debug_nocheck(_programs);
		//reload_from_disk();
}

static void __forceinline init_resources() {
	// ssbo

	{
		oglCreateBuffers(1, &ssbo);
		oglNamedBufferStorage(ssbo,1000000 * 180,0,0); // 1 megabyte, too much
		oglBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);
	}
	// framebuffer

	//for(int i = 0; i < 4; i++) {
	//	glGenTextures(1, &textures[i] );
	//	glBindTexture(GL_TEXTURE_2D, textures[i] );

	//	GLint internal_format;

	//	bool create_fb = false;
	//	if(i == 1 || i == 2 || i == 3){
	//		glTexImage2D(GL_TEXTURE_2D, 0, internal_format = GL_R32UI, XRES, YRES, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, 0);
	//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//	}else {
	//		create_fb = true;

	//	//glReadPixels(0, 0, MUSICRESX, MUSICRESY, GL_LUMINANCE_ALPHA, GL_FLOAT, &lpSoundBuffer);
	//		glTexImage2D(GL_TEXTURE_2D, 0, internal_format = GL_RGBA32F, MUSICRESX, MUSICRESY, 0, GL_LUMINANCE_ALPHA, GL_FLOAT, 0);

	//		// parameters
	//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	//		// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	//	}

	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);



	//	if(create_fb){
	//		oglCreateFramebuffers(1, &framebuffers[i]);
	//		//oglBindFramebuffer(GL_FRAMEBUFFER)
	//		oglNamedFramebufferTexture(
	//			framebuffers[i],
	//			GL_COLOR_ATTACHMENT0,
	//			textures[i],
	//			0
	//		);
	//		//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureMusic[1], 0);

	//		// PFNGLNAMEDFRAMEBUFFERDRAWBUFFERPROC
	//		const GLenum attachments[] = { GL_COLOR_ATTACHMENT0 };
	//		// oglDrawBuffers(1, attachments);
	//		oglNamedFramebufferDrawBuffers(framebuffers[i], 1, attachments);
	//	}
	//	oglBindTextureUnit(i + 1, textures[i]);
	//	oglBindImageTexture(i + 1, textures[i], 0, 0, 0, GL_READ_WRITE, internal_format );
	//	
	//	// oglNamedFramebufferDrawBuffer(framebuffers[i], )
	//}
}

static void __forceinline pre_loop() { 

}




static void __forceinline main_loop() {
	try_reload();
	#if EDITOR
		audio_time = editor_time;
	#else
		audio_time = audio_get_time_seconds();
	#endif


	//glViewport(0,0,XRES, YRES);
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
		oglBindFramebuffer(GL_FRAMEBUFFER, 0);
		// glDrawBuffer(GL_BACK);

		oglUseProgram(PROG_POST);
		oglUniform1i(0, frame);
		oglUniform1f(1, audio_time);
		oglUniform2f(2, xres, yres);
		glRects(-1, -1, 1, 1);
	}

}

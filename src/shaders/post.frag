#version 460 core

layout(location = 0) uniform int F;
layout(location = 1) uniform float T;
layout(location = 2) uniform vec2 R;

layout(std430,binding=0) coherent buffer Aa{uint hist[];};

out vec4 C;

uint seed = 111425u;
uint hash_u(uint _a) {
	uint a = _a;
	a ^= a >> 16;
	a *= 0x7feb352du;
	a ^= a >> 15;
	a *= 0x846ca68bu;
	a ^= a >> 16;
	return a;
}

float hash_f_s(float s){ return ( float( hash_u(uint(s)) ) / float( -1u ) ); }
float hash_f(){ uint s = hash_u(seed); seed = s;return ( float( s ) / float( -1u ) ); }
vec2 hash_v2(){ return vec2(hash_f(), hash_f()); }
vec3 hash_v3_s(float s){ return vec3(hash_f_s(s), hash_f_s(s), hash_f_s(s)); }
vec3 hash_v3(){ return vec3(hash_f(), hash_f(), hash_f()); }

uint get_hist_id(ivec2 c){
	return (c.x + uint(R.x) * c.y + uint(R.x*R.y))%uint(R.x*R.y);
}


const int COL_CNT = 4;
vec3[4] kCols = vec3[](
	vec3(1.,1,1), vec3(1.,0.,1.),
	vec3(1,1.,0.1)*1., vec3(0.5,1,1.)*1.5
);

vec3 mix_cols(float idx){
	//idx = mod(idx,1.);
	int cols_idx = int(idx*float(COL_CNT));
	float fract_idx = fract(idx*float(COL_CNT));
	fract_idx = smoothstep(0.,1.,fract_idx);
	//return oklab_mix( kCols[cols_idx], kCols[(cols_idx + 1)%COL_CNT], fract_idx );
	return mix( kCols[cols_idx], kCols[(cols_idx + 1)%COL_CNT], fract_idx );
}



void main( ){
	vec2 uv = gl_FragCoord.xy/R.xy;

	seed = uint(gl_FragCoord.x +gl_FragCoord.y*1111);
	hash_f();
	//seed = uint(gl_FragCoord.x +gl_FragCoord.y*5214.);

	float end = smoothstep(250.,250.1,T);
	float endb = smoothstep(280.,280.1,T);

	float repd = 0.5 + sin(T*0.5)*2./T;
	float eenv = mod(T,repd);
	eenv = exp(-eenv*40.);

	float K_COL = 0.03
		+ 0.1* smoothstep(90.,190.,T)
		- end + 10.*endb * smoothstep(4.,0.,mod(T,4.));
	;
	float K_GLITCH = 0.;
	K_GLITCH += 
		smoothstep(0.,50.,T);
	K_GLITCH *= pow(hash_f_s(floor(T)), 2);
	K_GLITCH + 0.9* smoothstep(160.,190.,T)*eenv;
	K_GLITCH *= 1.-end + endb;



	if(T > 80 && T < 100.){
		//float env = exp(-mod(T,4.));
		float env = smoothstep(1.,0.,mod(T,4.));
        //float repd = 0.5 + sin(t*0.5)*2./t;
		K_GLITCH = 100. * env;
	}

	uint hist_id =  get_hist_id(ivec2(gl_FragCoord.xy));

	// tonemap
	vec3 col = vec3(hist[hist_id]) * 0.0001;
	vec3 pal = mix_cols(mod(col.x*115.,1.));
	col = col/(1.+col);
	if(hash_f_s(floor(T*0.8))<0.7){
	//if(true){
		col = 1.- col;
	} else {
		col = pow(step(col,vec3(.5)),vec3(0.02));
		col = pow(abs(col),vec3(0.02));
	}

	if(T > 60 && T < 65.){
		col = 1. - col;
		//env = exp(-(T-60.))*4.;
	}

	if(abs(col.x - 0.1) < K_COL
	//&& hash_f_s(floor(T)) < 1
	){
		col -= pal;
	}

	C = vec4(pow(abs(col), vec3(2./0.45454545)),1);

	if(mod(T,9.- endb*5.) < 1.){
		hist[hist_id] = 0;
	} else {
		if (col.x < 0.0 + K_GLITCH){
			ivec2 offs = -ivec2(0, 4);
			//seed += iFrame;
			offs *= ((int(hash_f() < col.x*1111.))*2 - 1)*(1 + 5*int(hash_f_s(floor(T))));
			offs *= int(hash_f_s(floor(T) * 124.)*2.)*2 - 1;
			offs *= 1 + int(endb * smoothstep(3.,0.,mod(T,4.))*20.);
			if(T > 122){
				offs = ivec2(offs.y,offs.x);
			}
			if(T > 316 && hash_f() > 0.5){
				offs = ivec2(offs.y,offs.x)/10;
			}
			
			hist[hist_id] =	hist[get_hist_id((ivec2(gl_FragCoord.xy) + offs))];
		}else {
			hist[hist_id] = 0;
		}
	}
	if(end > 0.1){
		//C*= 0.1;
		C = 1.-C;
	}
//    fragColor = vec4(1,0,0,1);    
	//fragColor = texture(tex_music, uv);
//	fragColor = imageLoad(img_music, ivec2(gl_FragCoord.xy));


	//asdgsdagasdg

	//asdgasdg
	//asdg
	//asdgasdg

	//oasdgasdg

	//fragColor *= 0.000001;
	//asdg

	//fragColor *= fract(T);

	
}



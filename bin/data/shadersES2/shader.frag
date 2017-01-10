precision highp float;

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;

uniform float mod0;
uniform float mod1;
uniform float mod2;
uniform float mod3;
uniform float mod4;
uniform float mod5;
uniform float mod6;
uniform float TIME;

varying vec2 texCoordVarying;

vec2 center = vec2(0.5);
float TAU = 6.283185307;

float colorFlip = 0.75;
float colorVal = mod1;
float zoom = mod2;
float glitch = mod3;
float distortion = mod0;
float kaleido = mod4;
float noiseAmount = mod4;
float bright = mod5 * 0.5;
float trace = mod0;

vec2 RENDERSIZE = vec2(320.0, 240.0);
vec3 NOISEVEC = vec3(443.8975,397.2973, 491.1871);

vec3 hsv2rgb( in vec3 c ) {
    vec3 rgb = clamp( abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),6.0)-3.0)-1.0, 0.0, 1.0 );
	rgb = rgb*rgb*(3.0-2.0*rgb); // cubic smoothing
	return c.z * mix( vec3(1.0), rgb, c.y);
}

vec4 blend(vec4 top, vec4 bottom) {
	return vec4(abs(top.rgb - bottom.rgb), 1.0);
}

float gray(vec4 c) {
	return (c.r + c.g + c.b) / 3.;
}

float centerVal(float around, float size, float x) {
	return around - size + (size * 2. * x);
}

vec2 pb(in vec2 uv, in float per){
    uv.y += (TIME) / per;
    vec2 result = (cos(uv.y * per)) * normalize(vec2(1., cos((uv.y) * per)));
    return result * distortion * 0.05;
}


float noiseGenerate(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * NOISEVEC);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

vec4 vignette(vec2 uv, vec4 color, float amount) {
	vec2 coord = (uv - 0.5) * (RENDERSIZE.x / RENDERSIZE.y) * 1.;

	float rf = sqrt(dot(coord, coord)) * amount;
	float rf2_1 = rf * rf + 1.0;
	float e = 1.0 / (rf2_1 * rf2_1);

	color *= vec4(e);

	return vec4(vec3(color.rgb), 1.0);
}

void main() {
    vec2 uv = texCoordVarying;
  	vec3 hsl = vec3(colorVal, colorFlip, colorFlip * 0.75);
    float noise = noiseGenerate(uv + TIME * 0.1);

    //GLITCH
    vec2 uvDIST, uvWAVE = vec2(0.);

    if(glitch > 0.) {
      float tresh =  pow(fract(TIME * 1236.0453), 2.0) * 0.4 * glitch;
      vec2 block = floor(gl_FragCoord.xy / vec2(32));
      vec2 uv_noise = block / vec2(64);
      uv_noise += floor(vec2(TIME) * vec2(1204.0, 3543.0)) / vec2(64) / RENDERSIZE;

      if (noiseGenerate(uv_noise) < tresh)
        uvDIST += (fract(uv_noise) - 0.5) * .03 * glitch;
    }

    // WAVE
    if(distortion > 0.) {
      uvWAVE = pb(uv, 11. * distortion);
    }

  	// MIRROR EDGE
		vec2 uvFG = 0.5 + (uv - 0.5) * (zoom);
		uvFG = mod(uvFG + center / 2., 1.0);
  	uvFG = 2.0 * abs(uvFG - center);

    // KALEIDO
    float sides = floor(8. * kaleido);

    if(sides > 1.0) {
      float r = distance(center, uvFG);
      float a = atan ((uv.y-center.y), (uv.x-center.x));
      a = mod(a, TAU/sides);
      a = abs(a - TAU/sides/2.);
      uvFG.x = r * cos(a);
      uvFG.y = r * sin(a);
      uvFG = (center + uvFG);
    }

    vec4 colorFG = texture2D(tex0, uvFG + uvDIST + uvWAVE);
    vec4 colorNOISE = texture2D(tex1, texCoordVarying);
		vec4 colorBG = vec4(hsv2rgb(hsl), 1.0) - colorNOISE + centerVal(0., 0.3 * noiseAmount, noise);
    vec4 color = blend(colorBG, colorFG);
 		vec4 colorFeedback = texture2D(tex2, vec2(0.5) + (uv + uvDIST - 0.5) * centerVal(1.0, 0.01, zoom));

  	// VIGNETTE
  	color = vignette(uv, color, 0.85);

  	//LEVELS
  	float gamma = 0.11;
  	float contrast = 0.5;
  	float exposure = 0.2;
  	float blackLevel = 0.12;
  	vec4 inputRange = min(max(color - vec4(gamma), vec4(0.0)) / (vec4(1.0 - exposure) - vec4(gamma)), vec4(1.0));
  	inputRange = pow(inputRange, vec4(1.0 / (1.5 - contrast)));
  	color = mix(vec4(blackLevel), vec4(1.0), inputRange);

 		float feedback = mod3;

    if(feedback > 0.) {
 			float lumaColor = gray(color);
			if(lumaColor < feedback || noise > 1.0 - feedback * 2.) {
				color = colorFeedback - 0.01 * feedback;
			}
	  }

    gl_FragColor = color;
}

precision highp float;

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D imageMask;

varying vec2 texCoordVarying;


vec2 RENDERSIZE = vec2(320.0, 240.0);
vec3 NOISEVEC = vec3(443.8975,397.2973, 491.1871);

float noiseGenerate(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * NOISEVEC);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

vec3 hsv2rgb( in vec3 c ) {
    vec3 rgb = clamp( abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),6.0)-3.0)-1.0, 0.0, 1.0 );
	rgb = rgb*rgb*(3.0-2.0*rgb); // cubic smoothing
	return c.z * mix( vec3(1.0), rgb, c.y);
}

vec4 blend(vec4 top, vec4 bottom) {
	return vec4(abs(top.rgb - bottom.rgb), 1.0);
}

vec4 vignette(vec2 uv, vec4 color, float amount) {
	vec2 coord = (uv - 0.5) * (RENDERSIZE.x / RENDERSIZE.y) * 1.;
	float rf = sqrt(dot(coord, coord)) * amount;
	float rf2_1 = rf * rf + 1.0;
	float e = 1.0 / (rf2_1 * rf2_1);
	color *= vec4(e);
	return color;
}

void main() {
    float colorFlip = 0.6;
    float colorVal = 0.3;

    vec2 uv = texCoordVarying;
    float noise = noiseGenerate(uv + 0.01 * 0.00001);
  	vec3 hsl = vec3(colorVal, colorFlip, colorFlip * 0.75);

    vec4 colorFG = texture2D(tex1, texCoordVarying);
  	vec4 colorNOISE = texture2D(tex0, texCoordVarying);
    vec4 colorBG = vec4(hsv2rgb(hsl), 1.0) - colorNOISE + -0.02 + 0.04 * noise;

    vec4 color = blend(colorBG, colorFG);

  	// VIGNETTE
  	color = vignette(uv, color, 0.7);

  	//LEVELS
  	float gamma = 0.11;
  	float contrast = 0.5;
  	float exposure = 0.2;
  	float blackLevel = 0.21;
  	vec4 inputRange = min(max(color - vec4(gamma), vec4(0.0)) / (vec4(1.0 - exposure) - vec4(gamma)), vec4(1.0));
  	inputRange = pow(inputRange, vec4(1.0 / (1.5 - contrast)));
  	color = mix(vec4(blackLevel), vec4(1.0), inputRange);


    gl_FragColor = color;
}

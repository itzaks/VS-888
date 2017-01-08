precision highp float;

uniform sampler2D tex0;

varying vec2 texCoordVarying;

vec2 RENDERSIZE = vec2(320.0, 240.0);

vec4 vignette(vec2 uv, vec4 color, float amount) {
	vec2 coord = (uv - 0.5) * (RENDERSIZE.x / RENDERSIZE.y) * 1.;
	float rf = sqrt(dot(coord, coord)) * amount;
	float rf2_1 = rf * rf + 1.0;
	float e = 1.0 / (rf2_1 * rf2_1);
	color *= vec4(e);
	return color;
}

void main() {
    vec2 uv = texCoordVarying;
  	vec4 color = texture2D(tex0, texCoordVarying);

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

    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}

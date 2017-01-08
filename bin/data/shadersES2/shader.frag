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
float zoom = 1.0 - (mod2 * .5);
float glitch = mod3;
float distortion = mod3;
float kaleido = mod4;
float bright = mod5 * 0.5;
float trace = mod0;

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

	return vec4(vec3(color.rgb), 1.0);
}

float pattern(vec2 uv) {
	float pattern = sin(TIME * 0.02);
  float col = sin(TIME * 0.01);

	uv -= vec2(0.5);
	uv.x *= RENDERSIZE.x/RENDERSIZE.y;
	uv *= 4.;

	for(float i = 0.; i < 2.0; i++) {
	  float a = i * 4. * (TAU * pattern / 10.);
		col += cos(TAU*(uv.y * cos(a) + uv.x * sin(a))) +cos(TAU*(uv.y * cos(a) + uv.x * sin(-a)));
	}

	return col > 0.5 ? 0. : 1.;
}

vec2 pb(in vec2 uv, in float per){
    uv.y += (TIME) / per;
    vec2 result = (cos(uv.y * per)) * normalize(vec2(1., cos((uv.y) * per)));
    return result * distortion * 0.05;
}

vec4 echoTrace(vec4 freshPixel, vec4 stalePixel) {
  float	brightLevel = (freshPixel.r + freshPixel.b + freshPixel.g) / 3.0;

  if (brightLevel < trace - 0.1)
    brightLevel = 1.0;
  else
    brightLevel = 0.0;

  return mix(freshPixel, stalePixel, brightLevel);
}

void main() {
    vec2 uv = texCoordVarying;
    float noise = noiseGenerate(uv + 0.01 * 0.00001);
  	vec3 hsl = vec3(colorVal, colorFlip, colorFlip * 0.75);

  	// ZOOM
  	vec2 uvFG = texCoordVarying - 0.5;
  	uvFG = center + uvFG * zoom;

  	// KALEIDO
  	float sides = floor(16. * kaleido);

  	if(sides > 1.0) {
  		float angle = 0.;
  		float r = distance(vec2(0.5), uvFG);
  		float a = atan ((uv.y-center.y), (uv.x-center.x));
  		a = mod(a, TAU/sides);
  		a = abs(a - TAU/sides/2.);
  		uvFG.x = 1.0 + r * cos(a + TAU * angle);
  		uvFG.y = 1.0 + r * sin(a + TAU * angle);
  		uvFG = (center + uvFG);
  	}

  	// MIRROR EDGE
  	uvFG = mod(uvFG + center / 2., 1.0);
  	uvFG = 2.0 * abs(uvFG - center);

    // WAVE
    if(distortion > 0.) {
    	uvFG += pb(uvFG, 11. * distortion);
    }

    // PATTERNS
  	float modcolor = 0.;
  	if(bright > 0.)
  		modcolor = bright * pattern(uvFG);

    // TEXTURE SAMPLE
    vec4 colorFG = texture2D(tex0, uvFG) - modcolor;
  	vec4 colorNOISE = texture2D(tex1, texCoordVarying);
    vec4 colorBG = vec4(hsv2rgb(hsl), 1.0) - colorNOISE + -0.02 + 0.04 * noise;
    vec4 colorFeedback = texture2D(tex2, texCoordVarying);
    vec4 color = blend(colorBG, colorFG);

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


    // ECHO TRACE
    if(trace > 0.)
      color = echoTrace(color, colorFeedback);

    gl_FragColor = color;
}

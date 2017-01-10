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
float distortion = mod3;
float kaleido = mod4;
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


float noiseGenerate(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * NOISEVEC);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

void main() {
    vec2 uv = texCoordVarying;
  	vec3 hsl = vec3(colorVal, colorFlip, colorFlip * 0.75);
    float noise = noiseGenerate(uv + TIME * 0.00001);
    vec4 colorFG = texture2D(tex0, uv);
    vec4 colorBG = vec4(hsv2rgb(hsl), 1.0);
    vec4 color = blend(colorBG, colorFG);
 		vec4 colorFeedback = texture2D(tex2, vec2(0.5) + (uv - 0.5) * centerVal(1.0, 0.01, zoom));

 		float feedback = mod4;

    if(feedback > 0.) {
 			float lumaColor = gray(color);
			if(lumaColor < feedback || noise > 1.0 - feedback * 0.7) {
				color = colorFeedback;
			}
			color *= 1.0 - feedback * 0.003;
	  }

    gl_FragColor = color;
}

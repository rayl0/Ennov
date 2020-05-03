#version 410 core

// TODO(rajat): May implement batching for ui rendering 
// depending on the benchmarks, we will have

// NOTE(rajat): All values passed to the fragment shaders are 
// interoplated for each and every frgment.

out vec4 OutColor;
in vec2 TextureCoords;

uniform vec4 Color;
uniform float uiWidth;
uniform float uiHeight;
uniform float radius;

uniform float useTexture;
uniform float useColor;
// uniform float useBorder;

uniform sampler2D UI_Texture;

const float cornerSmoothFactor = 1.0;

float calcRoundedCorners() {
	if (radius <= 0.0) {
		return 1.0;
	}
	vec2 pixelPos = TextureCoords * vec2(uiWidth, uiHeight);
	vec2 minCorner = vec2(radius, radius);
	vec2 maxCorner = vec2(uiWidth - radius, uiHeight - radius);

	vec2 cornerPoint = clamp(pixelPos, minCorner, maxCorner);
	float lowerBound = pow(radius - cornerSmoothFactor, 2);
	float upperBound = pow(radius + cornerSmoothFactor, 2);
	return smoothstep(upperBound, lowerBound, pow(distance(pixelPos, cornerPoint), 2));
}

void main(void) 
{
  float alphavalue = Color.a * calcRoundedCorners();

  OutColor = vec4(1.0f);

  if(useTexture > 0.5f)
  {
     OutColor = texture(UI_Texture, TextureCoords);
  }

  OutColor.rgb = mix(OutColor.rgb, Color.rgb * OutColor.rgb, useColor); 
  OutColor.a *= alphavalue;
}

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

uniform sampler2D Texture;

const float smoothness = 0.7;

void main(void) 
{
  float alphavalue = Color.a;

  if(radius > 0.0)
  {
     vec2 pixelpos = TextureCoords * vec2(uiWidth, uiHeight);
     float xMax = uiWidth - radius;
     float yMax = uiHeight - radius;

     if(pixelpos.x < radius && pixelpos.y < radius) {
        alphavalue *= 1.0 - smoothstep(radius - smoothness, radius + smoothness,
                              length(pixelpos - vec2(radius, radius)));
     } else if(pixelpos.x < radius && pixelpos.y > yMax) {
        alphavalue *= 1.0 - smoothstep(radius - smoothness, radius + smoothness,
                                    length(pixelpos - vec2(radius, yMax)));
     } else if(pixelpos.x > xMax && pixelpos.y > yMax) {
        alphavalue *= 1.0 - smoothstep(radius - smoothness, radius + smoothness,
                                    length(pixelpos - vec2(xMax, yMax)));
     } else if(pixelpos.x > xMax && pixelpos.y < radius) {
       alphavalue *= 1.0 - smoothstep(radius - smoothness, radius + smoothness,
                                    length(pixelpos - vec2(xMax, radius)));
     }
  }
  OutColor = vec4(1.0f);
  OutColor.rgb = mix(OutColor.rgb, Color.rgb * OutColor.rgb, 1.0f); 
  OutColor.a *= alphavalue;
}

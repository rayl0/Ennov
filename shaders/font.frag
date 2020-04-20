#version 420 core 

out vec4 Color;
in vec2 TexCoords;

uniform vec4 FontColor;
uniform sampler2D FontTexture;

uniform float Width;
uniform float Edge;
uniform float BorderWidth;
uniform float BorderEdge;

uniform vec3 BorderColor;

// const vec2 offset = vec2(0.006, 0.006);

void main()
{
   float Distance = 1.0f - texture(FontTexture, TexCoords).a;
   float Alpha = 1.0f - smoothstep(Width, Width + Edge, Distance);

   float Distance2 = 1.0f - texture(FontTexture, TexCoords).a;
   float OutlineAlpha = 1.0f - smoothstep(BorderWidth, BorderWidth + BorderEdge, Distance2);

   float OverallAlpha = Alpha + (1.0 - Alpha) * OutlineAlpha;
   vec3 OverallColor = mix(BorderColor, FontColor.rgb, Alpha / OverallAlpha);
   
   Color = vec4(OverallColor, OverallAlpha);
}

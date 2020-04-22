#version 420 core

out vec4 OutputColor;

in vec4 FragColor;
in vec2 TextureCoord;
in float OutTexIndex;
in vec2 VertexPos;

uniform sampler2D Textures[%i];
const float r2 = 10;

void main()
{
      int SamplerIndex = int(OutTexIndex);
      if(OutTexIndex == -1.0f)
         OutputColor = FragColor;
      else
      { 
         if(((VertexPos.x + 10) / 800 > gl_PointCoord.x && VertexPos.x / 800 < gl_PointCoord.x) &&
            ((VertexPos.y + 10) / 600 > gl_PointCoord.y && VertexPos.y / 600 < gl_PointCoord.y))
         {
             discard;
         }
         OutputColor = FragColor * texture(Textures[SamplerIndex], TextureCoord);
      }
}

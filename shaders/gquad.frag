#version 420 core

out vec4 OutputColor;

in vec4 FragColor;
in vec2 TextureCoord;
in float OutTexIndex;
                 
uniform sampler2D Textures[%i];

void main()
{
      int SamplerIndex = int(OutTexIndex);
      if(OutTexIndex == -1.0f)
         OutputColor = FragColor;
      else
      {
         OutputColor = FragColor * texture(Textures[SamplerIndex], TextureCoord);
      }
}

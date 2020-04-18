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
         if(SamplerIndex == 0)
            OutputColor = FragColor * vec4(1.0f, 1.0f, 1.0f, texture(Textures[SamplerIndex], TextureCoord).r);
         else
            OutputColor = FragColor * texture(Textures[SamplerIndex], TextureCoord);
      }
}

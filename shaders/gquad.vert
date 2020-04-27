             #version 410 core

             layout(location = 0) in vec4 Position;
             layout(location = 1) in vec4 Color;
             layout(location = 2) in float TexIndex;

             out vec4 FragColor;
             out vec2 TextureCoord;
             out float OutTexIndex;

             uniform mat4 ViewProj;

             void main()
             {
                 gl_Position = ViewProj * vec4(Position.xy, 0.0f, 1.0f);
                 FragColor = Color;
                 TextureCoord = Position.zw;
                 OutTexIndex = TexIndex;
             }


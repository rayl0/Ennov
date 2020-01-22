#include "glad/glad.c"
#include "ennov_math.h" 
#include "ennov_platform.h"

#ifdef ENNOV_DEBUG
#define glCall(x) \
  do { x; Assert(glGetError() == 0)} while(0)
#else
#define glCall(x) x;
#endif

struct opengl_rect_common
{
     GLuint VertexArray;
     GLuint VertexBuffer;
     GLuint IndexBuffer;
     GLuint ShaderProgram;
     GLuint Texture;
};

struct opengl_context
{
     opengl_rect_common OpenGLRectangleData;
     bool32 IsInitialized;
};

global_variable opengl_context GlobalGLContext;

void OpenGLInitContext()
{
     if(!GlobalGLContext.IsInitialized)
     {
          GlobalGLContext.IsInitialized = true;
          opengl_rect_common* RectData = &GlobalGLContext.OpenGLRectangleData;
          glCall(glGenVertexArrays(1, &RectData->VertexArray));
          glCall(glBindVertexArray(RectData->VertexArray));

          real32 Verticies[] =
          {
           0.0f, 0.0f, 0.0f, 0.0f,
           0.0f, 1.0f, 0.0f, 1.0f,
           1.0f, 1.0f, 1.0f, 1.0f,
           1.0f, 0.0f, 1.0f, 0.0f
          };

          uint32 Indicies[] =
          {
           0, 1, 2,
           2, 3, 0
          };

          char* VertexShaderSource = {R"(
                 #version 330 core

                 in vec4 Position;
                 in vec4 Color;

                 out vs_out {
                     vec4 FragColor;
                     vec2 TexCoords;
                 } VertexShaderOutput;

                 void main() {
                     gl_Position = vec4(Position.xy, 0.0f, 1.0f);
                     VertexShaderOutput.FragColor = Color;
                     VertexShader.TexCoords = Position.xy;
                 }
           )"};

          char* FragmentShaderSource =  {
             R"(
                   #version 330 core

                   in vs_out {
                      vec4 FragColor;
                      vec2 TexCoords;
                   } FragmentShaderInput;

                   out vec4 OutColor;

                   uniform sampler2D Texture;
                   uniform uint DrawFlags;

                   void main()
                   {
                      if(DrawFlags & (1 << 0))
                         OutColor = texture(Texture,
                                    FragmentShaderInput.TexCoords);
                      else if(DrawFlags & (1 << 1))
                         OutColor = FragColor;
                      else
                         OutColor = FragColor * texture(Texture, FragmentShaderInput.TexCoords);
                   }
              )"
          };

          glCall(glGenBuffers(1, &RectData->VertexBuffer));
          glCall(glGenBuffers(1, &RectData->IndexBuffer));

          glCall(glBindBuffer(GL_ARRAY_BUFFER, RectData->VertexBuffer));
          glCall(glBufferData(GL_ARRAY_BUFFER, sizeof(Verticies), Verticies, GL_STATIC_DRAW));

          glCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RectData->IndexBuffer));
          glCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indicies), Indicies, GL_STATIC_DRAW));
     }
}


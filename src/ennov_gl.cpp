#include "glad/glad.c"
#include "ennov_math.h" 
#include "ennov_platform.h"
#include "ennov.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

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
    gladLoadGL();
     if(!GlobalGLContext.IsInitialized)
     {
          GlobalGLContext.IsInitialized = true;
          opengl_rect_common* RectData = &GlobalGLContext.OpenGLRectangleData;
          glGenVertexArrays(1, &RectData->VertexArray);
          glBindVertexArray(RectData->VertexArray);

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

                 out vs_out {
                     vec2 TexCoords;
                 } VertexShaderOutput;

                 uniform mat4 Projection;

                 void main() {
                     gl_Position = Projection * vec4(Position.xy, 0.0f, 1.0f);
                     VertexShaderOutput.TexCoords = Position.xy;
                 }
           )"};

          char* FragmentShaderSource =  {
             R"(
                   #version 330 core

                   in vs_out {
                      vec2 TexCoords;
                   } FragmentShaderInput;

                   out vec4 OutColor;

                 //  uniform sampler2D Texture;
                 //  uniform uint DrawFlags;
                 //    uniform vec4 FragColor;

                   void main()
                   {
                   //   if(DrawFlags & (1 << 0))
                   //    OutColor = texture(Texture,
                   //                 FragmentShaderInput.TexCoords);
                   //   else if(DrawFlags & (1 << 1))
                         OutColor = vec4(1.0f, 0.5f, 0.3f, 1.0f);
                   //   else
                   //      OutColor = FragColor * texture(Texture, FragmentShaderInput.TexCoords);
                   }
              )"
          };

          glGenBuffers(1, &RectData->VertexBuffer);
          glGenBuffers(1, &RectData->IndexBuffer);

          glBindBuffer(GL_ARRAY_BUFFER, RectData->VertexBuffer);
          glBufferData(GL_ARRAY_BUFFER, sizeof(Verticies), Verticies, GL_STATIC_DRAW);

          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, RectData->IndexBuffer);
          glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indicies), Indicies, GL_STATIC_DRAW);

          uint8 VertexShader = glCreateShader(GL_VERTEX_SHADER);
          glShaderSource(VertexShader, 1, &VertexShaderSource, NULL);
          glCompileShader(VertexShader);

          uint8 FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
          glShaderSource(FragmentShader, 1, &FragmentShaderSource, NULL);
          glCompileShader(FragmentShader);

          RectData->ShaderProgram = glCreateProgram();
          glAttachShader(RectData->ShaderProgram, VertexShader);
          glAttachShader(RectData->ShaderProgram, FragmentShader);
          glLinkProgram(RectData->ShaderProgram);
          glValidateProgram(RectData->ShaderProgram);

          glUseProgram(RectData->ShaderProgram);

          glEnableVertexAttribArray(0);
          glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
     }
}

void OpenGLDrawRectangle(rect_draw_attribs* DrawAttribs, uint32 DrawFlags)
{
    if(GlobalGLContext.IsInitialized)
    {
        opengl_rect_common* RectData = &GlobalGLContext.OpenGLRectangleData;
        glBindVertexArray(RectData->VertexArray);
        glm::mat4 Projection = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f);
        glm::mat4 Model = glm::mat4(1.0f);

        Model = glm::translate(Model, glm::vec3(DrawAttribs->Position.x, DrawAttribs->Position.y, 0.0f));
        Model = glm::scale(Model, glm::vec3(DrawAttribs->Dimensions.x, DrawAttribs->Dimensions.y, 1.0f));

        Projection = Projection * Model;

        glUseProgram(RectData->ShaderProgram);

        uint8 MatrixLocation = glGetUniformLocation(RectData->ShaderProgram, "Projection");
        uint8 DrawFlagsLocation = glGetUniformLocation(RectData->ShaderProgram, "Color");
        glUniformMatrix4fv(MatrixLocation, 1, GL_FALSE, glm::value_ptr(Projection));
        // glUniform1ui(DrawFlagsLocation, DrawFlags);
   
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
    }
    else
    {
        //TODO(Rajat): Logging
    }
}

void DrawRectangle(rect_draw_attribs* DrawAttribs, uint32 DrawFlags)
{
    OpenGLDrawRectangle(DrawAttribs, DrawFlags); 
}


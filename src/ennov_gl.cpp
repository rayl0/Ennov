#include "glad/glad.c"
#include "ennov_math.h" 
#include "ennov_platform.h"
#include "ennov.h"
#include <memory.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#ifdef ENNOV_DEBUG
#define glCall(x) \
  do { x; Assert(glGetError() == 0)} while(0)
#else
#define glCall(x) x;
#endif

#define MAX_BATCH_SPRITES 300

struct batch
{
  u32 Id;
  u32 IsInitialized;
  u32 TextureId;
  glm::mat4 Transform;
  u32 DynamicVertexBuffer;
  u32 VertexArray;
  u32 ShaderProgram;
  f32* VertexBufferData;
  u32 VertexBufferSize;
  u32 VertexBufferCurrentPos;
};

void StartBatch(batch* Batch, loaded_bitmap* Texture, glm::mat4 Transform)
{
    if(Batch->IsInitialized)
        return;
    if(Batch) {
        glGenVertexArrays(1, &Batch->VertexArray);
        glBindVertexArray(Batch->VertexArray);

        glGenBuffers(1, &Batch->DynamicVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, Batch->DynamicVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, 100 * sizeof(float) * 48, NULL, GL_DYNAMIC_DRAW);

        char* VertexShaderSource = {R"(
             #version 330 core

             layout(location = 0) in vec4 Position;
             layout(location = 1) in vec4 Color;

             out vec4 FragColor;
             out vec2 TextureCoord;

             uniform mat4 Projection;

             void main()
             {
                 gl_Position = Projection * vec4(Position.xy, 0.0f, 1.0f);
                 FragColor = Color;
                 TextureCoord = Position.zw;
             }
        )"
        };

        char* FragmentShaderSource = {R"(
             #version 330 core

             out vec4 OutputColor;

             in vec4 FragColor;
             in vec2 TextureCoord;

             uniform sampler2D Texture;

             void main()
             {
               OutputColor = FragColor * texture(Texture, TextureCoord);
             }
        )"};

        u32 VertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(VertexShader, 1, &VertexShaderSource, NULL);
        glCompileShader(VertexShader);

        u32 FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(FragmentShader, 1, &FragmentShaderSource, NULL);
        glCompileShader(FragmentShader);

        Batch->ShaderProgram = glCreateProgram();
        glAttachShader(Batch->ShaderProgram, VertexShader);
        glAttachShader(Batch->ShaderProgram, FragmentShader);
        glLinkProgram(Batch->ShaderProgram);
        glValidateProgram(Batch->ShaderProgram);

        glDeleteShader(VertexShader);
        glDeleteShader(FragmentShader);

        glUseProgram(Batch->ShaderProgram);

        u32 ProjectionLocation = glGetUniformLocation(Batch->ShaderProgram, "Projection");
        glUniformMatrix4fv(ProjectionLocation, 1, GL_FALSE, glm::value_ptr(Transform));

        glGenTextures(1, &Batch->TextureId);
        glBindTexture(GL_TEXTURE_2D, Batch->TextureId);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 

        u32 TextureFormat = GL_RGB;
        u32 ImageFormat = GL_RGB;
        if(Texture->Channels == 4) {
            ImageFormat = GL_RGBA;
            TextureFormat = GL_RGBA;
        }
        glTexImage2D(GL_TEXTURE_2D, 0, TextureFormat, Texture->Width, Texture->Height, 
                              0, ImageFormat, GL_UNSIGNED_BYTE, Texture->Pixels);
        glGenerateMipmap(Batch->TextureId);

        Batch->IsInitialized = true;
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else {

    }
}

void DrawBatchRectangle(batch* Batch, vec2 Position, vec2 Dimension, vec4 Color)
{
    if(Batch) {
        // TODO(rajat): Important map color values to 0.0f/1.0f

        glm::mat4 Model = glm::mat4(1.0f);

        Model = glm::translate(Model, glm::vec3(Position.x, Position.y, 0));
        Model = glm::scale(Model, glm::vec3(Dimension.x, Dimension.y, 0));

        glm::vec4 TransformVertexData[6] = {
            {0.0f, 0.0f, 0.0f, 1.0f},
            {0.0f, 1.0f, 0.0f, 1.0f},
            {1.0f, 1.0f, 0.0f, 1.0f},

            {1.0f, 1.0f, 0.0f, 1.0f},
            {1.0f, 0.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, 0.0f, 1.0f}
        };

        for(int i = 0; i < 6; ++i) {
            TransformVertexData[i] = Model * TransformVertexData[i];
        }

        vec2 TextureCord[6] = {
            {0.0f, 0.0f},
            {0.0f, 1.0f},
            {1.0f, 1.0f},

            {1.0f, 1.0f},
            {1.0f, 0.0f},
            {0.0f, 0.0f}
        };

        glm::vec4* tvd = TransformVertexData;
        vec2* tc = TextureCord;

        f32 VertexData[48] = {
            tvd[0].x, tvd[0].y, tc[0].x, tc[0].y, Color.r, Color.g, Color.b, Color.a,
            tvd[1].x, tvd[1].y, tc[1].x, tc[1].y, Color.r, Color.g, Color.b, Color.a,
            tvd[2].x, tvd[2].y, tc[2].x, tc[2].y, Color.r, Color.g, Color.b, Color.a,
            tvd[3].x, tvd[3].y, tc[3].x, tc[3].y, Color.r, Color.g, Color.b, Color.a,
            tvd[4].x, tvd[4].y, tc[4].x, tc[4].y, Color.r, Color.g, Color.b, Color.a,
            tvd[5].x, tvd[5].y, tc[5].x, tc[5].y, Color.r, Color.g, Color.b, Color.a,
        };

        for(int i = 0; i < 48; ++i) {
            Batch->VertexBufferData[Batch->VertexBufferCurrentPos + i] = VertexData[i];
        }

        Batch->VertexBufferCurrentPos += 48;
    }
};

void EndBatch(batch* Batch)
{
    if(Batch) {
        glBindVertexArray(Batch->VertexArray);
        glBindBuffer(GL_ARRAY_BUFFER, Batch->DynamicVertexBuffer);

        void* BufferData = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        memcpy(BufferData, Batch->VertexBufferData, sizeof(float) * Batch->VertexBufferCurrentPos);
        Assert(glUnmapBuffer(GL_ARRAY_BUFFER) == GL_TRUE);
        // NOTE(Rajat): Always set proper stride values otherwise go under a huge
        // Debugging sesssion.
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 8, 0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 4));

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);

        glBindTexture(GL_TEXTURE_2D, Batch->TextureId);
        glActiveTexture(GL_TEXTURE0);

        glUseProgram(Batch->ShaderProgram);
        glDrawArrays(GL_TRIANGLES, 0, Batch->VertexBufferCurrentPos/8);
        Batch->VertexBufferCurrentPos = 0;
    }
}

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
     glm::mat4 Projection;
     opengl_rect_common OpenGLRectangleData;
     bool32 IsInitialized;
};

global_variable opengl_context GlobalGLContext;

void OpenGLInitContext(vec2 WindowAttribs)
{
     if(!GlobalGLContext.IsInitialized)
     {
          gladLoadGL();
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
                 #version 420 core

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
                   #version 420 core

                   in vs_out {
                      vec2 TexCoords;
                   } FragmentShaderInput;

                   out vec4 OutColor;

                   uniform sampler2D Texture;
                   uniform uint Attrib;
                   uniform vec4 Color;

                   void main()
                   {
                      if(Attrib == 1)
                      {
                        OutColor = Color;
                      }
                      else if(Attrib == 2)
                      {
                        OutColor = texture(Texture, FragmentShaderInput.TexCoords);
                      }
                      else
                      {
                        OutColor = Color * texture(Texture, FragmentShaderInput.TexCoords);
                      }
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

          glEnable(GL_BLEND);
          glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
          glGenTextures(1, &RectData->Texture);
          glBindTexture(GL_TEXTURE_2D, RectData->Texture);
          // NOTE(Rajat): Don't ever forget to set texture parameters of the
          // textures your are using
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
          glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 

          glUseProgram(RectData->ShaderProgram);

          glEnableVertexAttribArray(0);
          glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);
     }
     GlobalGLContext.Projection = glm::ortho(0.0f, WindowAttribs.x, WindowAttribs.y, 0.0f, -1.0f, 1.0f);
}

void OpenGLDrawRectangle(rect_draw_attribs* DrawAttribs, uint32 DrawFlags)
{
    if(GlobalGLContext.IsInitialized)
    {
        opengl_rect_common* RectData = &GlobalGLContext.OpenGLRectangleData;
        glBindVertexArray(RectData->VertexArray);
        glm::mat4 Model = glm::mat4(1.0f);

        Model = glm::translate(Model, glm::vec3(DrawAttribs->Position.x, DrawAttribs->Position.y, 0.0f));
        Model = glm::scale(Model, glm::vec3(DrawAttribs->Dimensions.x, DrawAttribs->Dimensions.y, 1.0f));

        glm::mat4 Projection = GlobalGLContext.Projection * Model;

        uint8 MatrixLocation = glGetUniformLocation(RectData->ShaderProgram, "Projection");
        uint8 ColorLocation = glGetUniformLocation(RectData->ShaderProgram, "Color");
        int8 DrawFlagsLocation = glGetUniformLocation(RectData->ShaderProgram, "Attrib");

        glUniformMatrix4fv(MatrixLocation, 1, GL_FALSE, glm::value_ptr(Projection));
        glUniform1ui(DrawFlagsLocation, DrawFlags);
        glUniform4fv(ColorLocation, 1, (const GLfloat*)&DrawAttribs->Color.data);

        local_persist uint32 LastId = 700;
        if(DrawAttribs->Id != LastId) {
        if(DrawFlags == RECTANGLE_FILL_TEXTURE || DrawFlags == RECTANGLE_FILL_TEXCOLOR) {
          uint32 TextureFormat;
          uint32 ImageFormat;
          printf("hello");

          if(DrawAttribs->Texture->Channels == 4)
            {
              TextureFormat = GL_RGBA;
              ImageFormat = GL_RGBA;
            }
          else {
            TextureFormat = GL_RGB;
            ImageFormat = GL_RGB;
          }
                glTexImage2D(GL_TEXTURE_2D, 0, TextureFormat, DrawAttribs->Texture->Width, DrawAttribs->Texture->Height, 
                                    0, ImageFormat, GL_UNSIGNED_BYTE, DrawAttribs->Texture->Pixels);
                glGenerateMipmap(RectData->Texture);
        }
        }
        LastId = DrawAttribs->Id;
        glUseProgram(RectData->ShaderProgram);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
        glBindVertexArray(0);
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


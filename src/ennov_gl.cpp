#include "glad/glad.c"

#include "ennov.h"
#include "ennov_math.h"
#include "ennov_platform.h"
#include "ennov_gl.h"

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

void
QueryRenderData(renderer_data* RenderData)
{
    if(RenderData)
    {
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &RenderData->NumTextureSlots);
    }
    else
    {
        //TODO(rajat): Logging
    }
}

internal_ batch_data
CreateBatch(game_areana* Areana);

internal_ u32
CreateShaderProgram(char* VertexShaderSource, char* FragmentShaderSource)
{
    u32 VertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShader, 1, &VertexShaderSource, NULL);
    glCompileShader(VertexShader);

    u32 FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentShader, 1, (const char**)&FragmentShaderSource, NULL);
    int success;
    char infoLog[512];
    glCompileShader(FragmentShader);
    glGetProgramiv(FragmentShader, GL_COMPILE_STATUS, &success);
    // TODO(rajat): Clean up these temp checks after logging system built
    if(!success)
    {
        glGetProgramInfoLog(FragmentShader, 512, NULL, infoLog);
        printf("%s\n", infoLog);
    }

    u32 ShaderProgram = glCreateProgram();
    glAttachShader(ShaderProgram, VertexShader);
    glAttachShader(ShaderProgram, FragmentShader);
    glLinkProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(ShaderProgram, 512, NULL, infoLog);
        printf("%s\n", infoLog);
    }
    glValidateProgram(ShaderProgram);
    glDeleteShader(VertexShader);
    glDeleteShader(FragmentShader);

    glUseProgram(ShaderProgram);

    return ShaderProgram;
}

void
InitRenderer(renderer_data* RenderData, game_areana* Areana)
{
    if(RenderData)
    {
        // TODO(rajat): Use Game Memory to do this
        // TODO(rajat): Create texture allocating api. And cached textures
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &RenderData->NumTextureSlots);
        RenderData->Batch = CreateBatch(Areana);
        char* VertexShaderSource = {R"(
             #version 420 core

             layout(location = 0) in vec4 Position;
             layout(location = 1) in vec4 Color;
             layout(location = 2) in float TexIndex;

             out vec4 FragColor;
             out vec2 TextureCoord;
             out float OutTexIndex;

             uniform mat4 Projection;

             void main()
             {
                 gl_Position = Projection * vec4(Position.xy, 0.0f, 1.0f);
                 FragColor = Color;
                 TextureCoord = Position.zw;
                 OutTexIndex = TexIndex;
             }
        )"
        };

        // NOTE(rajat): Be careful with names in shaders make sure to have same names

        char* FragmentShaderSource = {R"(
             #version 420 core

             out vec4 OutputColor;

             in vec4 FragColor;
             in vec2 TextureCoord;
             in float OutTexIndex;

             uniform sampler2D Textures[16];

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
        )"};

        RenderData->Batch.ShaderProgram = CreateShaderProgram(VertexShaderSource, FragmentShaderSource);

        RenderData->ShaderProgram = RenderData->Batch.ShaderProgram;

        u32 ProjectionLocation = glGetUniformLocation(RenderData->Batch.ShaderProgram, "Projection");
        glUniformMatrix4fv(ProjectionLocation, 1, GL_FALSE, glm::value_ptr(RenderData->Projection));

        u32 SamplerLocation = glGetUniformLocation(RenderData->Batch.ShaderProgram, "Textures");
        s32 Samplers[32];

        for(int i = 0; i < 32; ++i)
        {
            Samplers[i] = i;
        }

        glUniform1iv(SamplerLocation, RenderData->NumTextureSlots, Samplers);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        RenderData->Areana = Areana;

        glGenVertexArrays(1, &RenderData->VertexArray);
        glBindVertexArray(RenderData->VertexArray);

        glGenBuffers(1, &RenderData->DynamicVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, RenderData->DynamicVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, 256 * sizeof(float) * 48, NULL, GL_DYNAMIC_DRAW);

// NOTE(Rajat): Always set proper stride values otherwise go under a huge
        // Debugging sesssion.
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 9, 0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void*)(sizeof(float) * 4));
        glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void*)(sizeof(float) * 8));

        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
    }
    else
    {
        //TODO(rajat): Logging
    }
};

internal_ texture
CreateTextureEx(u8* Pixels, u32 PixelFormat, u32 Width, u32 Height, u32 TextureFormat)
{
    u32 TextureId;
    glGenTextures(1, &TextureId);
    glBindTexture(GL_TEXTURE_2D, TextureId);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, PixelFormat, Width, Height,
                 0, TextureFormat, GL_UNSIGNED_BYTE, Pixels);
    glGenerateMipmap(TextureId);

    texture NewTexture = {};
    NewTexture.Width = Width;
    NewTexture.Height = Height;
    NewTexture.Id = TextureId;

    return NewTexture;
}

internal_ texture
CreateTexture(loaded_bitmap* Texture)
{
    u32 TextureFormat = GL_RGB;
    u32 ImageFormat = GL_RGB;
    if(Texture->Channels == 4) {
        ImageFormat = GL_RGBA;
        TextureFormat = GL_RGBA;
    }
    return CreateTextureEx(Texture->Pixels, ImageFormat, Texture->Width,
                           Texture->Height, TextureFormat);
}

internal_ batch_data
CreateBatch(game_areana* Areana)
{
    local_persist u32 LastId = 1;
    batch_data Batch = {};
    Batch.Id = LastId;
    // TODO(rajat): Move heap allocations to Game memory allocations
    Batch.VertexBufferData = (float*)PushStruct_(Areana, sizeof(float) * 48 * 256);
    Batch.VertexBufferSize = 48 * 256;
    LastId++;

    return Batch;
}

void FlushBatch(batch_data* Batch, u32 ShaderProgram, u32 VertexArray, u32 VertexBuffer);

void
BatchRenderRectangleDx(batch_data* Batch, texture *Texture, vec4 Color,
                       f32 x, f32 y, f32 w, f32 h, f32 s0, f32 t0, f32 s1, f32 t1)
{
    if(Batch)
    {
        f32 Index = -1.0f;
        if(Texture)
        {
            for(int i = 0; i < Batch->NumBindTextureSlots; ++i)
            {
                if(Texture->Id == Batch->TextureSlots[i])
                {
                    Index = i;
                    break;
                }
            }

            if(Index < 0)
            {
                Batch->TextureSlots[Batch->NumBindTextureSlots] = Texture->Id;
                Index = Batch->NumBindTextureSlots;
                Batch->NumBindTextureSlots++;
            }
        }

        batch_data* BackBatch = Batch;

        glm::mat4 Model = glm::mat4(1.0f);

        Model = glm::translate(Model, glm::vec3(x, y, 0));
        Model = glm::scale(Model, glm::vec3(w, h, 0));

        glm::vec4 tvd[6] = {
            {0.0f, 1.0f, 0.0f, 1.0f},
            {1.0f, 0.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, 0.0f, 1.0f},

            {0.0f, 1.0f, 0.0f, 1.0f},
            {1.0f, 1.0f, 0.0f, 1.0f},
            {1.0f, 0.0f, 0.0f, 1.0f}
        };

        for(int i = 0; i < 6; ++i) {
            tvd[i] = Model * tvd[i];
        }

        f32 VertexData[54] = {
            tvd[0].x, tvd[0].y, s0, t1, Color.r, Color.g, Color.b, Color.a, Index,
            tvd[1].x, tvd[1].y, s1, t0, Color.r, Color.g, Color.b, Color.a, Index,
            tvd[2].x, tvd[2].y, s0, t0, Color.r, Color.g, Color.b, Color.a, Index,
            tvd[3].x, tvd[3].y, s0, t1, Color.r, Color.g, Color.b, Color.a, Index,
            tvd[4].x, tvd[4].y, s1, t1, Color.r, Color.g, Color.b, Color.a, Index,
            tvd[5].x, tvd[5].y, s1, t0, Color.r, Color.g, Color.b, Color.a, Index
        };

        for(int i = 0; i < 54; ++i) {
            BackBatch->VertexBufferData[BackBatch->VertexBufferCurrentPos + i] = VertexData[i];
        }

        BackBatch->VertexBufferCurrentPos += 54;
    }
}

void
DrawBatchRectangleDx(renderer_data* RenderData, texture* Texture, vec4 Color,
                     f32 x, f32 y, f32 w, f32 h, f32 s0, f32 t0, f32 s1, f32 t1)
{
    if(RenderData) {
        // TODO(rajat): Important map color values to 0.0f/1.0f
        // NOTE(rajat): Always try to minimize work postpone, do it now to decrease complexity

        if(RenderData->Batch.NumBindTextureSlots == RenderData->NumTextureSlots ||
           RenderData->Batch.VertexBufferCurrentPos + 54 >= RenderData->Batch.VertexBufferSize)
        {
            FlushBatch(&RenderData->Batch, RenderData->Batch.ShaderProgram, RenderData->VertexArray, RenderData->DynamicVertexBuffer);
            RenderData->Batch.NumBindTextureSlots = 0;
            RenderData->Batch.VertexBufferCurrentPos = 0;

            for(int i = 0; i < 32; i++)
            {
                RenderData->Batch.TextureSlots[i] = -1;
            }
        }
        BatchRenderRectangleDx(&RenderData->Batch, Texture, Color, x, y, w, h, s0, t0, s1, t1);

    }
}

void
BatchRenderRectangle(batch_data* Batch, texture *Texture, vec4 Color,
                     rect *SrcClip, vec2 Position, vec2 Dimension)
{
    if(Batch)
    {
        if(Texture && SrcClip)
        {
            rect r = *SrcClip;
            vec2 d = {Texture->Width, Texture->Height};
            BatchRenderRectangleDx(Batch, Texture, Color, Position.x, Position.y,
                                   Dimension.x, Dimension.y, r.x/d.x, r.y/r.y,
                                   (r.x + r.w)/d.x, (r.y + r.h)/d.y);
            return;
        }

        BatchRenderRectangleDx(Batch, Texture, Color, Position.x, Position.y,
                               Dimension.x, Dimension.y, 0, 0, 1, 1);
    }
}

void
DrawBatchRectangleEx(renderer_data *RenderData, texture *Texture, vec4 Color,
                     rect *SrcClip, vec2 Position, vec2 Dimension, u32 ShaderProgram)
{
    if(RenderData) {
        // TODO(rajat): Important map color values to 0.0f/1.0f
        // NOTE(rajat): Always try to minimize work postpone, do it now to decrease complexity

        if(RenderData->Batch.NumBindTextureSlots == RenderData->NumTextureSlots ||
           RenderData->Batch.VertexBufferCurrentPos + 54 >= RenderData->Batch.VertexBufferSize ||
           RenderData->ShaderProgram != ShaderProgram)
        {
            FlushBatch(&RenderData->Batch, RenderData->Batch.ShaderProgram, RenderData->VertexArray, RenderData->DynamicVertexBuffer);
            RenderData->Batch.ShaderProgram = ShaderProgram;
            RenderData->Batch.NumBindTextureSlots = 0;
            RenderData->Batch.VertexBufferCurrentPos = 0;

            for(int i = 0; i < 32; i++)
            {
                RenderData->Batch.TextureSlots[i] = -1;
            }
        }

        BatchRenderRectangle(&RenderData->Batch, Texture, Color, SrcClip, Position, Dimension);

       }
}

void
DrawBatchRectangle(renderer_data *RenderData, texture *Texture, vec4 Color, rect *SrcClip, vec2 Position, vec2 Dimension)
{
    DrawBatchRectangleEx(RenderData, Texture, Color, SrcClip, Position, Dimension, RenderData->Batch.ShaderProgram);
};

void
FlushBatch(batch_data* Batch, u32 ShaderProgram, u32 VertexArray, u32 VertexBuffer)
{
    if(Batch) {
        for(int i = 0; i < Batch->NumBindTextureSlots; ++i)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, Batch->TextureSlots[i]);
        }

        glUseProgram(ShaderProgram);

        glBindVertexArray(VertexArray);
        glBindBuffer(GL_ARRAY_BUFFER, VertexBuffer);

        // TODO(rajat): Should use persistent mapping for supported platforms
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * Batch->VertexBufferCurrentPos, Batch->VertexBufferData);
        glDrawArrays(GL_TRIANGLES, 0, Batch->VertexBufferCurrentPos/9);
        Batch->VertexBufferCurrentPos = 0;

        Batch->NumBindTextureSlots = 0;
    }
}

void
FlushRenderer(renderer_data* RenderData)
{
    if(RenderData)
    {
        FlushBatch(&RenderData->Batch, RenderData->Batch.ShaderProgram, RenderData->VertexArray, RenderData->DynamicVertexBuffer);

        RenderData->Batch.ShaderProgram = RenderData->ShaderProgram;
    }
    else
    {
    }
}

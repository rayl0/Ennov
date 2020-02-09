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

struct batch_data
{
    u32 DynamicVertexBuffer;
    u32 VertexArray;
    u32 NumBindTextureSlots;
    u32 TextureSlots[32]; // TODO(rajat): There is no point in dynamically allocating here
    f32 *VertexBufferData;
    u32 VertexBufferSize;
    u32 VertexBufferCurrentPos;
};

struct renderer_data
{
    s32 NumTextureSlots;
    batch_data BackBatch;
    batch_data *Batches;
    u32 NumBatchCount;
    u32 ShaderProgram;
    game_areana *Areana;
    glm::mat4 Projection;
};

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

internal batch_data
CreateBatch(game_areana* Areana);

void
InitRenderer(renderer_data* RenderData, game_areana* Areana)
{
    if(RenderData)
    {
        // TODO(rajat): Use Game Memory to do this
        // TODO(rajat): Create texture allocating api. And cached textures
        RenderData->Batches = (batch_data*)PushStruct_(Areana, sizeof(batch_data) * 50);
        glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &RenderData->NumTextureSlots);
        RenderData->BackBatch = CreateBatch(Areana);
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

        u32 VertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(VertexShader, 1, &VertexShaderSource, NULL);
        glCompileShader(VertexShader);

        u32 FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(FragmentShader, 1, (const char**)&FragmentShaderSource, NULL);
        int success;
        char infoLog[512];
        glCompileShader(FragmentShader);
        glGetProgramiv(FragmentShader, GL_COMPILE_STATUS, &success);
        if(!success)
        {
            glGetProgramInfoLog(FragmentShader, 512, NULL, infoLog);
            printf("%s\n", infoLog);
        }


        RenderData->ShaderProgram = glCreateProgram();
        glAttachShader(RenderData->ShaderProgram, VertexShader);
        glAttachShader(RenderData->ShaderProgram, FragmentShader);
        glLinkProgram(RenderData->ShaderProgram);
        glGetProgramiv(RenderData->ShaderProgram, GL_LINK_STATUS, &success);
        if(!success)
        {
            glGetProgramInfoLog(RenderData->ShaderProgram, 512, NULL, infoLog);
            printf("%s\n", infoLog);
        }
        glValidateProgram(RenderData->ShaderProgram);

        glDeleteShader(VertexShader);
        glDeleteShader(FragmentShader);

        glUseProgram(RenderData->ShaderProgram);

        u32 ProjectionLocation = glGetUniformLocation(RenderData->ShaderProgram, "Projection");
        glUniformMatrix4fv(ProjectionLocation, 1, GL_FALSE, glm::value_ptr(RenderData->Projection));

        u32 SamplerLocation = glGetUniformLocation(RenderData->ShaderProgram, "Textures");
        s32 Samplers[32];

        for(int i = 0; i < 32; ++i)
        {
            Samplers[i] = i;
        }

        glUniform1iv(SamplerLocation, RenderData->NumTextureSlots, Samplers);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        //TODO(rajat): Logging
    }
};

struct texture
{
    u32 Id;
    u32 Width, Height;
};

internal texture
CreateTexture(loaded_bitmap* Texture)
{
    u32 TextureId;
    glGenTextures(1, &TextureId);
    glBindTexture(GL_TEXTURE_2D, TextureId);

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
    glGenerateMipmap(TextureId);

    texture NewTexture = {};
    NewTexture.Width = Texture->Width;
    NewTexture.Height = Texture->Height;
    NewTexture.Id = TextureId;

    return NewTexture;
}

internal batch_data
CreateBatch(game_areana* Areana)
{
    batch_data Batch = {};
    glGenVertexArrays(1, &Batch.VertexArray);
    glBindVertexArray(Batch.VertexArray);

    glGenBuffers(1, &Batch.DynamicVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, Batch.DynamicVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, 256 * sizeof(float) * 48, NULL, GL_DYNAMIC_DRAW);

    // TODO(rajat): Move heap allocations to Game memory allocations
    Batch.VertexBufferData = (float*)PushStruct_(Areana, sizeof(float) * 48 * 256);
    Batch.VertexBufferSize = 48 * 256;

    // NOTE(Rajat): Always set proper stride values otherwise go under a huge
    // Debugging sesssion.
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 9, 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void*)(sizeof(float) * 4));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float) * 9, (void*)(sizeof(float) * 8));

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    return Batch;
}

void
DrawBatchRectangle(renderer_data* RenderData, texture *Texture, vec4 Color, rect *SrcClip, vec2 Position, vec2 Dimension)
{
    if(RenderData) {
        // TODO(rajat): Important map color values to 0.0f/1.0f
        Assert(RenderData->BackBatch.VertexBufferCurrentPos + 48 < RenderData->BackBatch.VertexBufferSize);

        if(RenderData->BackBatch.NumBindTextureSlots == RenderData->NumTextureSlots)
        {
            RenderData->Batches[RenderData->NumBatchCount] = RenderData->BackBatch;
            RenderData->BackBatch = CreateBatch(RenderData->Areana);
            RenderData->NumBatchCount++;
        }

        f32 Index = -1.0f;
        if(Texture)
        {
            for(int i = 0; i < RenderData->NumTextureSlots; ++i)
            {
                if(Texture->Id == RenderData->BackBatch.TextureSlots[i])
                {
                    Index = i;
                    break;
                }
            }

            if(Index < 0)
            {
                RenderData->BackBatch.TextureSlots[RenderData->BackBatch.NumBindTextureSlots] = Texture->Id;
                Index = RenderData->BackBatch.NumBindTextureSlots;
                RenderData->BackBatch.NumBindTextureSlots++;
            }
        }

        batch_data* BackBatch = &RenderData->BackBatch;

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

        f32 VertexData[54] = {
            tvd[0].x, tvd[0].y, tc[0].x, tc[0].y, Color.r, Color.g, Color.b, Color.a, Index,
            tvd[1].x, tvd[1].y, tc[1].x, tc[1].y, Color.r, Color.g, Color.b, Color.a, Index,
            tvd[2].x, tvd[2].y, tc[2].x, tc[2].y, Color.r, Color.g, Color.b, Color.a, Index,
            tvd[3].x, tvd[3].y, tc[3].x, tc[3].y, Color.r, Color.g, Color.b, Color.a, Index,
            tvd[4].x, tvd[4].y, tc[4].x, tc[4].y, Color.r, Color.g, Color.b, Color.a, Index,
            tvd[5].x, tvd[5].y, tc[5].x, tc[5].y, Color.r, Color.g, Color.b, Color.a, Index
        };

        for(int i = 0; i < 54; ++i) {
            BackBatch->VertexBufferData[BackBatch->VertexBufferCurrentPos + i] = VertexData[i];
        }

        BackBatch->VertexBufferCurrentPos += 54;

    }
};

void FlushBatch(batch_data* Batch, u32 ShaderProgram)
{
    if(Batch) {
        glBindVertexArray(Batch->VertexArray);
        glBindBuffer(GL_ARRAY_BUFFER, Batch->DynamicVertexBuffer);

        void* BufferData = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        memcpy(BufferData, Batch->VertexBufferData, sizeof(float) * Batch->VertexBufferCurrentPos);
        Assert(glUnmapBuffer(GL_ARRAY_BUFFER) == GL_TRUE);

        if(Batch->NumBindTextureSlots != 0)
        {
            for(int i = 0; i < Batch->NumBindTextureSlots; ++i)
            {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, Batch->TextureSlots[i]);
            }
        }

        glUseProgram(ShaderProgram);
        glDrawArrays(GL_TRIANGLES, 0, Batch->VertexBufferCurrentPos/9);
        Batch->VertexBufferCurrentPos = 0;
    }

}

void
FlushRenderer(renderer_data* RenderData)
{
    if(RenderData)
    {
        for(int i = 0; i < RenderData->NumBatchCount; ++i)
        {
            FlushBatch(&RenderData->Batches[i], RenderData->ShaderProgram);
        }

        FlushBatch(&RenderData->BackBatch, RenderData->ShaderProgram);
    }
    else
    {
    }
}

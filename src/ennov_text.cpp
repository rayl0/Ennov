#include "ennov.h"
#include "ennov_gl.h"
#include "glad/glad.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <string.h>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

FT_Library FtLib;
FT_Face FtFace;

struct text_rendering_data
{
    b32 IsInitialized;
    u32 TextShaderProgram;
    u32 NumTextureSlots;
    character_glyph* Characters;
    batch_data Batch;
    u32 VertexArray;
    u32 DynamicVertexBuffer;
};

void
InitializeFreeType()
{
    FT_Init_FreeType(&FtLib);
}

void
InitializeAreana(game_areana* Areana, void* BaseAddress, u32 Size)
{
    Areana->BaseAddress = BaseAddress;
    Areana->Size = Size;
    Areana->Used = 0;
}

u32
CalculatePadding(u64 Address, u32 Align)
{
    u32 Padding = (Address % Align);
    return (Padding == 0) ? 0 : (Align - Padding);
}

void*
PushStruct_(game_areana* Areana, memory_index Size)
{
    Assert(Areana->Used + Size <= Areana->Size);
    void* NewStruct = (Areana->BaseAddress + Areana->Used);
    u64 Address = ((u64)(Areana->BaseAddress) + Areana->Used);
    u32 Padding = CalculatePadding(Address, 8);
    Areana->Used += Size + Padding;
    return NewStruct;
}

character_glyph*
LoadTTF(char* File, u32 Size, game_areana* Areana)
{
    if(FT_New_Face(FtLib, File, 0, &FtFace)) Assert(0);
    FT_Set_Pixel_Sizes(FtFace, 0, Size);

    character_glyph* Characters = (character_glyph*)PushStruct_(Areana, 128 * sizeof(character_glyph));

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for(int i = 0; i < 128; i++)
    {
        if(FT_Load_Char(FtFace, i, FT_LOAD_RENDER) == 0)
        {
            Characters[i].TextureId = CreateTextureEx(FtFace->glyph->bitmap.buffer, GL_RED, FtFace->glyph->bitmap.width, FtFace->glyph->bitmap.rows, GL_RED);
            Characters[i].Width = FtFace->glyph->bitmap.width;
            Characters[i].Height = FtFace->glyph->bitmap.rows;
            Characters[i].Bearing = {FtFace->glyph->bitmap_left, FtFace->glyph->bitmap_top};
            Characters[i].Advance = FtFace->glyph->advance.x;
        }
        // TODO(rajat): Store num character extracted
    }

    FT_Done_Face(FtFace);
    return Characters;
}

void
BeginText(text_rendering_data* TextData, character_glyph* Characters, glm::mat4 *Projection, game_areana* Areana)
{
    if(TextData->IsInitialized)
        return;

    if(TextData)
    {
        TextData->IsInitialized = true;

        Assert(Characters != NULL);
        TextData->Characters = Characters; // TODO(rajat): Make sure characters are read only

        TextData->Batch = CreateBatch(Areana);
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
                   OutputColor = FragColor * vec4(1.0f, 1.0f, 1.0f, texture(Textures[SamplerIndex], TextureCoord).r);
               }
             }
        )"};

        TextData->TextShaderProgram = CreateShaderProgram(VertexShaderSource, FragmentShaderSource);

        // TODO(rajat): This must be queried from opengl
        TextData->NumTextureSlots = 16;

        u32 ProjectionLocation = glGetUniformLocation(TextData->TextShaderProgram, "Projection");
        glUniformMatrix4fv(ProjectionLocation, 1, GL_FALSE, glm::value_ptr(*Projection));

        u32 SamplerLocation = glGetUniformLocation(TextData->TextShaderProgram, "Textures");
        s32 Samplers[32];

        for(int i = 0; i < 32; ++i)
        {
            Samplers[i] = i;
        }

        // TODO(rajat): There should be proper num texture slots assigned
        // glUniform1iv(SamplerLocation, Data->RendererData->NumTextureSlots, Samplers);
        glUniform1iv(SamplerLocation, 16, Samplers);

        glGenVertexArrays(1, &TextData->VertexArray);
        glBindVertexArray(TextData->VertexArray);

        glGenBuffers(1, &TextData->DynamicVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, TextData->DynamicVertexBuffer);
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
        // TODO(rajat): logging
    }
}

void EndText(text_rendering_data *Data);

void
RenderText(text_rendering_data* Data, const char* String, vec2 Position, f32 Scale, vec4 Color)
{
    Assert(String != NULL);
    Assert(Data != NULL);

    if(Data->IsInitialized)
    {
        for(int i = 0; i < strlen(String); i++)
        {
            f32 x = Position.x + Data->Characters[String[i]].Bearing.x * Scale;
            f32 y = Position.y +
                 (Data->Characters[String['H']].Bearing.y - Data->Characters[String[i]].Bearing.y) * Scale;
            f32 w = Data->Characters[String[i]].Width * Scale;
            f32 h = Data->Characters[String[i]].Height * Scale;

            if(Data->Batch.NumBindTextureSlots == Data->NumTextureSlots || Data->Batch.VertexBufferCurrentPos + 54 >
                Data->Batch.VertexBufferSize)
            {
                EndText(Data);
            }

            BatchRenderRectangle(&Data->Batch, &Data->Characters[String[i]].TextureId, Color, NULL, {x, y}, {w, h});

            Position.x += (Data->Characters[String[i]].Advance >> 6) * Scale;
        }
    }
    else
    {
        // TODO(rajat): Logging!
    }
}

void
EndText(text_rendering_data *Data)
{
    glUseProgram(Data->TextShaderProgram);
    FlushBatch(&Data->Batch, Data->TextShaderProgram, Data->VertexArray, Data->DynamicVertexBuffer);
}

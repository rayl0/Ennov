#include "ennov.h"
#include "ennov_gl.h"
#include "glad/glad.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include <string.h>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

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

global_variable texture FontTexture;
global_variable b32 IsInitialized = 0;
global_variable text_rendering_data *Renderer = NULL;

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
    void* NewStruct = ((char*)Areana->BaseAddress + Areana->Used);
    u64 Address = ((u64)(Areana->BaseAddress) + Areana->Used);
    u32 Padding = CalculatePadding(Address, 8);
    Areana->Used += Size + Padding;
    return NewStruct;
}

// STUDY(rajat): stb_truetype and stb_textedit

u8 TTFBuffer[2048 * 2048];
stbtt_packedchar CharacterData[96];

texture
LoadttfTexture(u8* FileMemory, f32 Size)
{
    stbtt_pack_context PackCtx;
    if(stbtt_PackBegin(&PackCtx, TTFBuffer, 2048, 2048, 0, 0, NULL)) printf("hurrah!");
    stbtt_PackSetOversampling(&PackCtx, 4, 4);
    stbtt_PackFontRange(&PackCtx, FileMemory, 0, Size, 32, 96, CharacterData);
    stbtt_PackEnd(&PackCtx);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    texture NewTexture = CreateTextureEx(TTFBuffer, GL_RED, 2048, 2048, GL_RED);
    return NewTexture;
}

void
BeginText(text_rendering_data *RenderData, u8 *FontFileMemory, f32 Size)
{
    if(!IsInitialized)
    {
        Renderer = RenderData;
        FontTexture = LoadttfTexture(FontFileMemory, Size);
        IsInitialized = true;
    }
}

void EndText(text_rendering_data *Data);

void
DrawString(char *String, f32 x, f32 y, f32 Scale, vec4 Color)
{
    Assert(IsInitialized != NULL);

    while(*String)
    {
        if(*String >= 32 && *String < 128)
        {
            // TODO(rajat): Store and calculate them when loading fonts and create character mapping to reterive correct one
            stbtt_aligned_quad q;
            stbtt_GetPackedQuad(CharacterData, 2048, 2048, *String - 32, &x, &y, &q, 0);

            if(Renderer->Batch.NumBindTextureSlots == Renderer->NumTextureSlots || Renderer->Batch.VertexBufferCurrentPos + 54 >
               Renderer->Batch.VertexBufferSize)
            {
                EndText(Renderer);
            }

            // TODO(rajat): Scaling of characters, Scale value is ignored
            BatchRenderRectangleDx(&Renderer->Batch, &FontTexture, Color,
                                   q.x0, q.y0, (q.x1 - q.x0), (q.y1 - q.y0), q.s0, q.t0, q.s1, q.t1);
        }

        ++String;
    }
}

void
EndText()
{
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

void
EndText(text_rendering_data *Data)
{
    glUseProgram(Data->TextShaderProgram);
    FlushBatch(&Data->Batch, Data->TextShaderProgram, Data->VertexArray, Data->DynamicVertexBuffer);
}

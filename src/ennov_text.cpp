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

global_variable texture FontTexture;
global_variable b32 IsInitialized = 0;

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
    stbtt_PackBegin(&PackCtx, TTFBuffer, 2048, 2048, 0, 0, NULL);
    stbtt_PackSetOversampling(&PackCtx, 4, 4);
    stbtt_PackFontRange(&PackCtx, FileMemory, 0, Size, 32, 96, CharacterData);
    stbtt_PackEnd(&PackCtx);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    texture NewTexture = CreateTextureEx(TTFBuffer, GL_RED, 2048, 2048, GL_RED);
    return NewTexture;
}

void
CreateTextFonts(u8 *FontFileMemory, f32 Size)
{
    FontTexture = LoadttfTexture(FontFileMemory, Size);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, FontTexture.Id);

    IsInitialized = true;
}

void
DrawString(const char *String, f32 x, f32 y, f32 Scale, u32 Color)
{
    Assert(IsInitialized != false);

    while(*String)
    {
        if(*String >= 32 && (int)*String < 128)
        {
            // TODO(rajat): Store and calculate them when loading fonts and create character mapping to reterive correct one
            stbtt_aligned_quad q;
            stbtt_GetPackedQuad(CharacterData, 2048, 2048, *String - 32, &x, &y, &q, 0);
            // TODO(rajat): Scaling of characters, Scale value is ignored
            // FillQuad(q.x0, q.y0, q.x1 - q.x0, q.y1 - q.y0, Color);
            FillTexQuadClippedReserved(q.x0, q.y0 , q.x1 - q.x0, q.y1 - q.y0, Color, &FontTexture, q.s0, q.t0, q.s1, q.t1);
        }

        ++String;
    }
}

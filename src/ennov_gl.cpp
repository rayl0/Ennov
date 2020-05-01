#include "ennov_platform.h"

#if ENNOV_PLATFORM_LINUX
#include "glad/glad.h"
#else
#include <GLES2/gl2.h>
#endif

#include "ennov.h"
#include "ennov_math.h"
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

// TODO(rajat): IMPORTANT: Change render commits to out of memory assertions

// NOTE(rajat): Be careful with names in shaders make sure to have same names

// NOTE(rajat): Trying to be less generic here
static render_context RenderContexts[10] = {};
static u32 ContextCount = {};
static b32 IsCtxInitialized = false;

static s32 CurrentId = -1;

void
CreateRenderContext(u32* Id, const char* VertexShader, const char* FragmentShader)
{
    // TODO(rajat): Update it after updating NumContexts
    Assert(ContextCount <= 9);

    *Id = ContextCount++;
    render_context& rctx = RenderContexts[*Id];

    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &rctx.NumTextureSlots);

    glGenVertexArrays(1, &rctx.VertexArray);
    glBindVertexArray(rctx.VertexArray);

    glGenBuffers(1, &rctx.VertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, rctx.VertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * VERTEX_BUFFER_SIZE, NULL, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &rctx.IndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rctx.IndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(u32) * INDEX_BUFFER_SIZE, NULL, GL_DYNAMIC_DRAW);

    char TempBuffer[4048];
    sprintf(TempBuffer, FragmentShader, rctx.NumTextureSlots);

    rctx.TexQuadShader = CreateShaderProgram(VertexShader, TempBuffer);
    glUseProgram(rctx.TexQuadShader);

    rctx.ViewProj = glm::ortho(0.0f, 1200.0f, 675.0f, 0.0f, -1.0f, 1.0f);

    GLuint ViewProjLocation = glGetUniformLocation(rctx.TexQuadShader, "ViewProj");
    glUniformMatrix4fv(ViewProjLocation, 1, GL_FALSE, glm::value_ptr(rctx.ViewProj));

    u32 SamplerLocation = glGetUniformLocation(rctx.TexQuadShader, "Textures");
    s32 Samplers[32];

    for(int i = 0; i < 32; ++i)
    {
        Samplers[i] = i;
    }

    glUniform1iv(SamplerLocation, rctx.NumTextureSlots, Samplers);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 9, 0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(f32) * 9, (void*)(sizeof(f32) * 4));
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(f32) * 9, (void*)(sizeof(f32) * 8));

    rctx.VertexBufferCurrentPos = 0;
    rctx.NumBindTextureSlots = 0;

    IsCtxInitialized = true;
}

void
BindRenderContext(u32 Id)
{
    Assert(Id <= 10);

    CurrentId = Id;
}

static glm::vec4 qd[4] = {
    {1.0f, 0.0f, 0.0f, 1.0f},
    {0.0f, 0.0f, 0.0f, 1.0f},
    {0.0f, 1.0f, 0.0f, 1.0f},
    {1.0f, 1.0f, 0.0f, 1.0f}
};

void
FillQuad(f32 x, f32 y, f32 w, f32 h, u32 Color)
{
    Assert(CurrentId != -1);

    render_context& rctx = RenderContexts[CurrentId];

    if(rctx.VertexBufferCurrentPos + 36 >= VERTEX_BUFFER_SIZE)
        RenderCommit();

    f32 Index = -1.0f;

    glm::mat4 Model = glm::mat4(1.0f);

    Model = glm::translate(Model, glm::vec3(x, y, 0));
    Model = glm::scale(Model, glm::vec3(w, h, 0));

    glm::vec4 tvd[4];

    u8 r, g, b, a;
    DecodeRGBA(Color, &r, &g, &b, &a);

    for(int i = 0; i < 4; ++i) {
        tvd[i] = Model * qd[i];
    }

    f32 VertexData[4 * 9] = {
        tvd[0].x, tvd[0].y, 0, 0, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f, Index,
        tvd[1].x, tvd[1].y, 0, 0, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f, Index,
        tvd[2].x, tvd[2].y, 0, 0, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f, Index,
        tvd[3].x, tvd[3].y, 0, 0, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f, Index
    };

    for(int i = 0; i < 36; i++)
    {
        rctx.VertexBufferData[rctx.VertexBufferCurrentPos + i] = VertexData[i];
    }

    u32 IndexData[6] = {0, 1, 2, 2, 3, 0};

    for(int i = 0; i < 6; ++i)
    {
        rctx.IndexBufferData[rctx.IndexBufferCurrentPos + i] = IndexData[i] + rctx.BufferIndex;
    }

    rctx.IndexBufferCurrentPos += 6;
    rctx.BufferIndex += 4;

    rctx.VertexBufferCurrentPos += 36;
}

void
FillTexQuad(f32 x, f32 y, f32 w, f32 h, u32 Color, texture *Texture)
{
    Assert(CurrentId != -1);

    render_context& rctx = RenderContexts[CurrentId];

    if(rctx.VertexBufferCurrentPos + 36 >= VERTEX_BUFFER_SIZE)
        RenderCommit();

    f32 Index = -1.0f;
    if(rctx.NumBindTextureSlots == rctx.NumTextureSlots)
        RenderCommit();

    for(int i = 0; i < rctx.NumBindTextureSlots; ++i)
    {
        if(Texture->Id == rctx.TextureMap[i])
        {
            Index = i;
        }
    }

    if(Index == -1.0f)
    {
        rctx.TextureMap[rctx.NumBindTextureSlots] = Texture->Id;
        Index = rctx.NumBindTextureSlots;
        rctx.NumBindTextureSlots++;
    }

    glm::mat4 Model = glm::mat4(1.0f);

    Model = glm::translate(Model, glm::vec3(x, y, 0));
    Model = glm::scale(Model, glm::vec3(w, h, 0));

    glm::vec4 tvd[4];

    u8 r, g, b, a;
    DecodeRGBA(Color, &r, &g, &b, &a);

    for(int i = 0; i < 4; ++i) {
        tvd[i] = Model * qd[i];
    }

    f32 VertexData[36] = {
        tvd[0].x, tvd[0].y, qd[0].x, qd[0].y, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f, Index,
        tvd[1].x, tvd[1].y, qd[1].x, qd[1].y, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f, Index,
        tvd[2].x, tvd[2].y, qd[2].x, qd[2].y, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f, Index,
        tvd[3].x, tvd[3].y, qd[3].x, qd[3].y, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f, Index
    };

    for(int i = 0; i < 36; i++)
    {
        rctx.VertexBufferData[rctx.VertexBufferCurrentPos + i] = VertexData[i];
    }

    u32 IndexData[6] = {0, 1, 2, 2, 3, 0};

    for(int i = 0; i < 6; ++i)
    {
        rctx.IndexBufferData[rctx.IndexBufferCurrentPos + i] = IndexData[i] + rctx.BufferIndex;
    }

    rctx.IndexBufferCurrentPos += 6;
    rctx.BufferIndex += 4;

    rctx.VertexBufferCurrentPos += 36;
}

void
FillTexQuad(f32 x, f32 y, f32 w, f32 h, texture *Texture)
{
    FillTexQuad(x, y, w, h, 0xFFFFFFFF, Texture);
}

void
FillTexQuadClipped(f32 x, f32 y, f32 w, f32 h, u32 Color,
                   texture* Texture, f32 s0, f32 t0, f32 s1, f32 t1)
{
    Assert(CurrentId != -1);

    render_context& rctx = RenderContexts[CurrentId];

    if(rctx.VertexBufferCurrentPos + 36 >= VERTEX_BUFFER_SIZE)
        RenderCommit();

    f32 Index = -1.0f;
    if(rctx.NumBindTextureSlots == rctx.NumTextureSlots)
        RenderCommit();

    for(int i = 0; i < rctx.NumBindTextureSlots; ++i)
    {
        if(Texture->Id == rctx.TextureMap[i])
        {
            Index = i;
        }
    }

    if(Index == -1.0f)
    {
        rctx.TextureMap[rctx.NumBindTextureSlots] = Texture->Id;
        Index = rctx.NumBindTextureSlots;
        rctx.NumBindTextureSlots++;
    }

    glm::mat4 Model = glm::mat4(1.0f);

    Model = glm::translate(Model, glm::vec3(x, y, 0));
    Model = glm::scale(Model, glm::vec3(w, h, 0));

    glm::vec4 tvd[4];

    u8 r, g, b, a;
    DecodeRGBA(Color, &r, &g, &b, &a);

    for(int i = 0; i < 4; ++i) {
        tvd[i] = Model * qd[i];
    }

    f32 VertexData[36] = {
        tvd[0].x, tvd[0].y, s1, t0, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f, Index,
        tvd[1].x, tvd[1].y, s0, t0, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f, Index,
        tvd[2].x, tvd[2].y, s0, t1, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f, Index,
        tvd[3].x, tvd[3].y, s1, t1, r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f, Index
    };

    for(int i = 0; i < 36; i++)
    {
        rctx.VertexBufferData[rctx.VertexBufferCurrentPos + i] = VertexData[i];
    }

    u32 IndexData[6] = {0, 1, 2, 2, 3, 0};

    for(int i = 0; i < 6; ++i)
    {
        rctx.IndexBufferData[rctx.IndexBufferCurrentPos + i] = IndexData[i] + rctx.BufferIndex;
    }

    rctx.IndexBufferCurrentPos += 6;
    rctx.BufferIndex += 4;

    rctx.VertexBufferCurrentPos += 36;
}

void
RenderContextUpdateViewProj(u32 Id, f32 Width, f32 Height)
{
    Assert(Id != -1);

    render_context* rctx = &RenderContexts[Id];
    rctx->ViewProj = glm::ortho(0.0f, Width, Height, 0.0f, -1.0f, 1.0f);

    glUseProgram(rctx->TexQuadShader);

    GLuint ViewProjLocation = glGetUniformLocation(rctx->TexQuadShader, "ViewProj");
    glUniformMatrix4fv(ViewProjLocation, 1, GL_FALSE, glm::value_ptr(rctx->ViewProj));

    glUseProgram(0);
}


void
FillTexQuadClipped(f32 x, f32 y, f32 w, f32 h,
                   texture *Texture, f32 s0, f32 t0, f32 s1, f32 t1)
{
    FillTexQuadClipped(x, y, w, h, 0xFFFFFFFF, Texture, s0, t0, s1, t1);
}

void
FillTexQuadClipped(f32 x, f32 y, f32 w, f32 h, u32 Color, texture *Texture, vec4
                   r /*Clip*/)
{
    FillTexQuadClipped(x, y, w, h, Color, Texture, r.x/w, r.y/h,
                       (r.x + r.z)/w, (r.y + r.w)/h);
}

void
FillTexQuadClipped(f32 x, f32 y, f32 w, f32 h, texture *Texture, vec4
                   r /*Clip */)
{
    FillTexQuadClipped(x, y, w, h, Texture, r.x/w, r.y/h,
                       (r.x + r.z)/w, (r.y + r.w)/h);
}

void
RenderCommit()
{
    render_context& rctx = RenderContexts[CurrentId];

    render_cmd* cmd = &rctx.RenderCommands[rctx.NumCommands++];

    glBindBuffer(GL_ARRAY_BUFFER, rctx.VertexBuffer);
    glBufferSubData(GL_ARRAY_BUFFER, 0,
                    sizeof(float) * rctx.VertexBufferCurrentPos,
                    rctx.VertexBufferData);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rctx.IndexBuffer);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0,
                    sizeof(u32) * rctx.IndexBufferCurrentPos, rctx.IndexBufferData);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    for(int i = 0; i < rctx.NumBindTextureSlots; ++i)
    {
        cmd->TextureMap[i] = rctx.TextureMap[i];
    }

    cmd->NumVertices = rctx.IndexBufferCurrentPos;
    cmd->NumBindTextureSlots = rctx.NumBindTextureSlots;

    rctx.VertexBufferCurrentPos = 0;
    rctx.IndexBufferCurrentPos = 0;
    rctx.BufferIndex = 0;
    rctx.NumBindTextureSlots = 0;
}

void FlushRenderCommands(u32 Id)
{
    render_context& rctx = RenderContexts[Id];

    glBindVertexArray(rctx.VertexArray);
    glBindBuffer(GL_ARRAY_BUFFER, rctx.VertexBuffer);
    glUseProgram(rctx.TexQuadShader);

    for(int i = 0; i < rctx.NumCommands; ++i)
    {
        render_cmd* cmd = &rctx.RenderCommands[i];

        for(int i = 0; i < cmd->NumBindTextureSlots; ++i)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, cmd->TextureMap[i]);
        }

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rctx.IndexBuffer);
        glDrawElements(GL_TRIANGLES, cmd->NumVertices, GL_UNSIGNED_INT, NULL);
    }

    rctx.NumCommands = 0;
}

/* Extended API */
void
FillTexQuad(f32 *DestRect, texture *Texture)
{
    FillTexQuad(DestRect[0], DestRect[1], DestRect[2], DestRect[3], Texture);
}

internal_ u32
CreateShaderProgram(const char* VertexShaderSource, const char* FragmentShaderSource)
{
    u32 VertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShader, 1, &VertexShaderSource, NULL);
    glCompileShader(VertexShader);

    u32 FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentShader, 1, &FragmentShaderSource, NULL);

    int success;
    char infoLog[512];
    glCompileShader(FragmentShader);
    glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &success);

    // TODO(rajat): Clean up these temp checks after logging system built
    if(!success)
    {
        glGetShaderInfoLog(FragmentShader, 512, NULL, infoLog);
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

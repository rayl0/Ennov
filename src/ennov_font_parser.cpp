#include "ennov_platform.h"
#include "ennov.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>

// A single character spacing info
// STUDY(rajat): Look up alogoritms like hashing
struct fontface
{
    char Id; // Currently in Ascii

    f32 x, y; // Position of the glyph in the bitmap
    f32 w, h; // Width and height of the glyph in the bitmap

    // f32 s0, t0;
    // f32 s1, t1;

    f32 xoffset;
    f32 yoffset;

    f32 xadvance;
};

struct fontinfo
{
    u32 Size;
    u32 Padding[4];
    u32 PaddingWidth;
    u32 PaddingHeight;
    s32 SpacingX;
    s32 SpacingY;

    char FontName[50];
    loaded_bitmap* FontBitmap;

    u32 FontBitmapWidth;
    u32 FontBitmapHeight;

    u32 LineHeight;

    fontface Fonts[128]; // Only ascii for now

    u32 Count;
};

// This function may fail
int GetNextLine(char** Line, size_t* n, char* Stream)
{
    static u32 StreamSize = strlen(Stream) + 1;

    char* Start = Stream + *n;
    char* Out = *Line;

    s32 i = 0;

    if(*Start == '\n' || *Start == '\0')
        return -1;

    while(*Start != '\n' && *Start != '\0')
    {
        *Out = *Start;
        Start++;
        Out++;
        (*n)++;

        if(*n + 1 == StreamSize)
            return -1;
    }

    *Out = '\0';
    *n = *n + 1;

    return 0;
}


char* FindCharInString(char* s, char c)
{
    while(*s != c)
    {
        s++;

        if(*s == '\0')
            return NULL;
    }

    return s;
}

int Split(char* source, char** s1, char** s2, char c)
{
    char* main = source;
    while(*source != c)
    {
        source++;

        if(*source == '\0' || *source == '\n')
        {
            *s1 = main;
            *source = '\0';

            return -1;
        }
    }

    *s1 = main;

    *source = '\0';
    source++;

    *s2 = source;

    return 0;
}

char* Lskip(char* s)
{
    while(*s != '\0' && *s == ' ')
    {
        s++;
    }

    return s;
}

#define PAD_TOP 0
#define PAD_LEFT 1
#define PAD_BOTTOM 2
#define PAD_RIGHT 3

#define DESIRED_PADDING 8

// -1 when unsuccesful
int
GetFontInfo(const char *fntfile, fontinfo *FontInfo)
{
    struct stat FileState;
    int FileHandle;

    FileHandle = open(fntfile, O_RDONLY);
    fstat(FileHandle, &FileState);

    if(FileState.st_size == 0)
        return -1;

    char Buffer[200000];
    read(FileHandle, Buffer, FileState.st_size);

    char* Start;
    char* Cursor;
    char* Name;
    char* Value;

    char* Front;
    char* End;

    char* KeyFront;
    char* KeyEnd;

    char LineBuffer[1024] = {};
    Start = LineBuffer;
    size_t n = 0;

    while(GetNextLine(&Start, &n, Buffer) == 0)
    {
        Split(Start, &Name, &Value, ' ');

        if(!strcmp(Name, "info"))
        {
            while(!Split(Value, &Front, &End, ' '))
            {
                Split(Front, &KeyFront, &KeyEnd, '=');

                if(!strcmp(Front, "face"))
                {
                    strcpy(FontInfo->FontName, KeyEnd);
                }
                else if(!strcmp(Front, "size"))
                {
                    FontInfo->Size = atoi(KeyEnd);
                }
                else if(!strcmp(Front, "padding"))
                {
                    char *ArrayStart, *ArrayEnd;
                    s32 i = 0;
                    while(!Split(KeyEnd, &ArrayStart, &ArrayEnd, ','))
                    {
                        FontInfo->Padding[i] = atoi(ArrayStart);
                        KeyEnd = ArrayEnd;
                        i++;
                    }
                    FontInfo->Padding[i] = atoi(KeyEnd);

                    FontInfo->PaddingWidth =
                        FontInfo->Padding[PAD_LEFT] +
                        FontInfo->Padding[PAD_RIGHT];

                    FontInfo->PaddingHeight =
                        FontInfo->Padding[PAD_TOP] +
                        FontInfo->Padding[PAD_BOTTOM];
                }
                Value = Lskip(End);
            }
            Split(Front, &KeyFront, &KeyEnd, '=');

            if(!strcmp(KeyFront, "spacing"))
            {
                char *ArrayStart, *ArrayEnd;
                Split(KeyEnd, &ArrayStart, &ArrayEnd, ',');
                FontInfo->SpacingX = atoi(ArrayStart);
                FontInfo->SpacingY = atoi(ArrayEnd);
            }
        }
        else if(!strcmp(Name, "common"))
        {
            while(!Split(Value, &Front, &End, ' '))
            {
                Split(Front, &KeyFront, &KeyEnd, '=');

                if(!strcmp(KeyFront, "lineHeight"))
                {
                    FontInfo->LineHeight = (u32)atoi(KeyEnd);
                }
                else if(!strcmp(KeyFront, "scaleW"))
                {
                    FontInfo->FontBitmapWidth = (u32)atoi(KeyEnd);
                }
                else if(!strcmp(KeyFront, "scaleH"))
                {
                    FontInfo->FontBitmapHeight = (u32)atoi(KeyEnd);
                }

                Value = Lskip(End);
            }
        }
        else if(!strcmp(Name, "page"))
        {
            while(!Split(Value, &Front, &End, ' '))
            {
                Split(Front, &KeyFront, &KeyEnd, '=');
                Value = Lskip(End);
            }
            Split(Front, &KeyFront, &KeyEnd, '=');

            // TODO(rajat): Load font bitmap
            if(!strcmp(KeyFront, "file"))
            {
                // TODO(rajat): Relative path loading
                // NOTE(rajat): Only reterives default.png as a loaded bitmap
                FontInfo->FontBitmap = LoadPixelsFrom("./assets/fonts/default.png", &GameState->AssestStorage);
            }
        }
        else if(!strcmp(Name, "chars"))
        {
            Split(Value, &KeyFront, &KeyEnd, '=');

            if(!strcmp(KeyFront, "count"))
            {
                FontInfo->Count = atoi(KeyEnd);
            }
        }
        // TODO(rajat): Handle the edge case of the last character data
        else if(!strcmp(Name, "char"))
        {
            static int FontId = 0;
            while(!Split(Value, &Front, &End, ' '))
            {
                // printf("Front %s\n", Front);
                Split(Front, &KeyFront, &KeyEnd, '=');

                if(!strcmp(KeyFront, "id"))
                {
                    FontInfo->Fonts[atoi(KeyEnd)].Id = atoi(KeyEnd);
                    FontId = atoi(KeyEnd);
                }
                else if(!strcmp(KeyFront, "x"))
                {
                    FontInfo->Fonts[FontId].x = atoi(KeyEnd);
                }
                else if(!strcmp(KeyFront, "y"))
                {
                    FontInfo->Fonts[FontId].y = atoi(KeyEnd);
                }
                else if(!strcmp(KeyFront, "width"))
                {
                    FontInfo->Fonts[FontId].w = atoi(KeyEnd);
                }
                else if(!strcmp(KeyFront, "height"))
                {
                    FontInfo->Fonts[FontId].h = atoi(KeyEnd);
                }
                else if(!strcmp(KeyFront, "xoffset"))
                {
                    FontInfo->Fonts[FontId].xoffset = atoi(KeyEnd);
                }
                else if(!strcmp(KeyFront, "yoffset"))
                {
                    FontInfo->Fonts[FontId].yoffset = atoi(KeyEnd);
                }
                else if(!strcmp(KeyFront, "xadvance"))
                {
                    FontInfo->Fonts[FontId].xadvance = atoi(KeyEnd);
                }
                Value = Lskip(End);
            }
        }
    }
}

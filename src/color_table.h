#ifndef COLOR_TABLE

#ifdef ENNOV_MATH_H
#include "ennov_math.h"
#else
struct vec3
{
    union {
        struct {
            float r, g, b;
        };
        float data[3];
    };
};
#endif //


struct _colors
{
    vec3 ALICE_BLUE = {0.941f, 0.973f, 1.000f};
    vec3 ANTIQUE_WHITE = {0.980f, 0.922f, 0.843f};
    vec3 AQUA = {0.000f, 1.000f, 1.000f};
    vec3 AQUA_MARINE = {0.498f, 1.000f, 0.831f};
    vec3 AZURE = {0.941f, 1.000f, 1.000f};
    vec3 BEIGE = {0.961f, 0.961f, 0.863f};
    vec3 BISQUE = {1.000f, 0.894f, 0.769f};
    vec3 BLACK = {0.000f, 0.000f, 0.000f};
    vec3 BLANCHED_ALMOND = {1.000f, 0.922f, 0.804f};
    vec3 BLUE = {0.000f, 0.000f, 1.000f};
    vec3 BLUE_VIOLET = {0.541f, 0.169f, 0.886f};
    vec3 BROWN = {0.647f, 0.165f, 0.165f};
    vec3 BURLYWOOD = {0.871f, 0.722f, 0.529f};
    vec3 CADET_BLUE = {0.373f, 0.620f, 0.627f};
    vec3 CHARTREUSE = {0.498f, 1.000f, 0.000f};
    vec3 CHOCOLATE = {0.824f, 0.412f, 0.118f};
    vec3 CORAL = {1.000f, 0.498f, 0.314f};
    vec3 CORN_FLOWER_BLUE = {0.392f, 0.584f, 0.929f};
    vec3 CORN_SILK = {1.000f, 0.973f, 0.863f};
    vec3 CRIMSON = {0.863f, 0.078f, 0.235f};
    vec3 CYAN = {0.000f, 1.000f, 1.000f};
    vec3 DARK_BLUE = {0.000f, 0.000f, 0.545f};
    vec3 DARK_CYAN = {0.000f, 0.545f, 0.545f};
    vec3 DARK_GOLDENROD = {0.722f, 0.525f, 0.043f};
    vec3 DARK_GRAY = {0.663f, 0.663f, 0.663f};
    vec3 DARK_GREEN = {0.000f, 0.392f, 0.000f};
    vec3 DARK_GREY = {0.663f, 0.663f, 0.663f};
    vec3 DARK_KHAKI = {0.741f, 0.718f, 0.420f};
    vec3 DARK_MAGENTA = {0.545f, 0.000f, 0.545f};
    vec3 DARK_OLIVE_GREEN = {0.333f, 0.420f, 0.184f};
    vec3 DARK_ORANGE = {1.000f, 0.549f, 0.000f};
    vec3 DARK_ORCHID = {0.600f, 0.196f, 0.800f};
    vec3 DARK_RED = {0.545f, 0.000f, 0.000f};
    vec3 DARK_SALMON = {0.914f, 0.588f, 0.478f};
    vec3 DARK_SEAGREEN = {0.561f, 0.737f, 0.561f};
    vec3 DARK_SLATEBLUE = {0.282f, 0.239f, 0.545f};
    vec3 DARK_SLATEGRAY = {0.184f, 0.310f, 0.310f};
    vec3 DARK_SLATEGREY = {0.184f, 0.310f, 0.310f};
    vec3 DARK_TURQUOISE = {0.000f, 0.808f, 0.820f};
    vec3 DARK_VIOLET = {0.580f, 0.000f, 0.827f};
    vec3 DEEP_PINK = {1.000f, 0.078f, 0.576f};
    vec3 DEEP_SKYBLUE = {0.000f, 0.749f, 1.000f};
    vec3 DIM_GRAY = {0.412f, 0.412f, 0.412f};
    vec3 DIM_GREY = {0.412f, 0.412f, 0.412f};
    vec3 DODGER_BLUE = {0.118f, 0.565f, 1.000f};
    vec3 FIRE_BRICK = {0.698f, 0.133f, 0.133f};
    vec3 FLORAL_WHITE = {1.000f, 0.980f, 0.941f};
    vec3 FOREST_GREEN = {0.133f, 0.545f, 0.133f};
    vec3 FUCHSIA = {1.000f, 0.000f, 1.000f};
    vec3 GAINSBORO = {0.863f, 0.863f, 0.863f};
    vec3 GHOST_WHITE = {0.973f, 0.973f, 1.000f};
    vec3 GOLD = {1.000f, 0.843f, 0.000f};
    vec3 GOLDENROD = {0.855f, 0.647f, 0.125f};
    vec3 GRAY = {0.502f, 0.502f, 0.502f};
    vec3 GREEN = {0.000f, 0.502f, 0.000f};
    vec3 GREEN_YELLOW = {0.678f, 1.000f, 0.184f};
    vec3 GREY = {0.502f, 0.502f, 0.502f};
    vec3 HONEYDEW = {0.941f, 1.000f, 0.941f};
    vec3 HOT_PINK = {1.000f, 0.412f, 0.706f};
    vec3 INDIAN_RED = {0.804f, 0.361f, 0.361f};
    vec3 INDIGO = {0.294f, 0.000f, 0.510f};
    vec3 IVORY = {1.000f, 1.000f, 0.941f};
    vec3 KHAKI = {0.941f, 0.902f, 0.549f};
    vec3 LAVENDER = {0.902f, 0.902f, 0.980f};
    vec3 LAVENDER_BLUSH = {1.000f, 0.941f, 0.961f};
    vec3 LAWN_GREEN = {0.486f, 0.988f, 0.000f};
    vec3 LEMON_CHIFFON = {1.000f, 0.980f, 0.804f};
    vec3 LIGHT_BLUE = {0.678f, 0.847f, 0.902f};
    vec3 LIGHT_CORAL = {0.941f, 0.502f, 0.502f};
    vec3 LIGHT_CYAN = {0.878f, 1.000f, 1.000f};
    vec3 LIGHT_GOLDENROD_YELLOW = {0.980f, 0.980f, 0.824f};
    vec3 LIGHT_GRAY = {0.827f, 0.827f, 0.827f};
    vec3 LIGHT_GREEN = {0.565f, 0.933f, 0.565f};
    vec3 LIGHT_GREY = {0.827f, 0.827f, 0.827f};
    vec3 LIGHT_PINK = {1.000f, 0.714f, 0.757f};
    vec3 LIGHT_SALMON = {1.000f, 0.627f, 0.478f};
    vec3 LIGHT_SEAGREEN = {0.125f, 0.698f, 0.667f};
    vec3 LIGHT_SKYBLUE = {0.529f, 0.808f, 0.980f};
    vec3 LIGHT_SLATEGRAY = {0.467f, 0.533f, 0.600f};
    vec3 LIGHT_SLATEGREY = {0.467f, 0.533f, 0.600f};
    vec3 LIGHT_STEELBLUE = {0.690f, 0.769f, 0.871f};
    vec3 LIGHT_YELLOW = {1.000f, 1.000f, 0.878f};
    vec3 LIME = {0.000f, 1.000f, 0.000f};
    vec3 LIME_GREEN = {0.196f, 0.804f, 0.196f};
    vec3 LINEN = {0.980f, 0.941f, 0.902f};
    vec3 MAGENTA = {1.000f, 0.000f, 1.000f};
    vec3 MAROON = {0.502f, 0.000f, 0.000f};
    vec3 MEDIUM_AQUAMARINE = {0.400f, 0.804f, 0.667f};
    vec3 MEDIUM_BLUE = {0.000f, 0.000f, 0.804f};
    vec3 MEDIUM_ORCHID = {0.729f, 0.333f, 0.827f};
    vec3 MEDIUM_PURPLE = {0.576f, 0.439f, 0.859f};
    vec3 MEDIUM_SEAGREEN = {0.235f, 0.702f, 0.443f};
    vec3 MEDIUM_SLATEBLUE = {0.482f, 0.408f, 0.933f};
    vec3 MEDIUM_SPRING_GREEN = {0.000f, 0.980f, 0.604f};
    vec3 MEDIUM_TURQUOISE = {0.282f, 0.820f, 0.800f};
    vec3 MEDIUM_VIOLET_RED = {0.780f, 0.082f, 0.522f};
    vec3 MIDNIGHT_BLUE = {0.098f, 0.098f, 0.439f};
    vec3 MINT_CREAM = {0.961f, 1.000f, 0.980f};
    vec3 MISTYROSE = {1.000f, 0.894f, 0.882f};
    vec3 MOCCASIN = {1.000f, 0.894f, 0.710f};
    vec3 NAVAJO_WHITE = {1.000f, 0.871f, 0.678f};
    vec3 NAVY = {0.000f, 0.000f, 0.502f};
    vec3 OLDLACE = {0.992f, 0.961f, 0.902f};
    vec3 OLIVE = {0.502f, 0.502f, 0.000f};
    vec3 OLIVEDRAB = {0.420f, 0.557f, 0.137f};
    vec3 ORANGE = {1.000f, 0.647f, 0.000f};
    vec3 ORANGE_RED = {1.000f, 0.271f, 0.000f};
    vec3 ORCHID = {0.855f, 0.439f, 0.839f};
    vec3 PALE_GOLDENROD = {0.933f, 0.910f, 0.667f};
    vec3 PALE_GREEN = {0.596f, 0.984f, 0.596f};
    vec3 PALE_TURQUOISE = {0.686f, 0.933f, 0.933f};
    vec3 PALE_VIOLET_RED = {0.859f, 0.439f, 0.576f};
    vec3 PAPAYAWHIP = {1.000f, 0.937f, 0.835f};
    vec3 PEACHPUFF = {1.000f, 0.855f, 0.725f};
    vec3 PERU = {0.804f, 0.522f, 0.247f};
    vec3 PINK = {1.000f, 0.753f, 0.796f};
    vec3 PLUM = {0.867f, 0.627f, 0.867f};
    vec3 POWDER_BLUE = {0.690f, 0.878f, 0.902f};
    vec3 PURPLE = {0.502f, 0.000f, 0.502f};
    vec3 RED = {1.000f, 0.000f, 0.000f};
    vec3 ROSY_BROWN = {0.737f, 0.561f, 0.561f};
    vec3 ROYAL_BLUE = {0.255f, 0.412f, 0.882f};
    vec3 SADDLE_BROWN = {0.545f, 0.271f, 0.075f};
    vec3 SALMON = {0.980f, 0.502f, 0.447f};
    vec3 SANDY_BROWN = {0.957f, 0.643f, 0.376f};
    vec3 SEA_GREEN = {0.180f, 0.545f, 0.341f};
    vec3 SEA_SHELL = {1.000f, 0.961f, 0.933f};
    vec3 SIENNA = {0.627f, 0.322f, 0.176f};
    vec3 SILVER = {0.753f, 0.753f, 0.753f};
    vec3 SKYBLUE = {0.529f, 0.808f, 0.922f};
    vec3 SLATEBLUE = {0.416f, 0.353f, 0.804f};
    vec3 SLATEGRAY = {0.439f, 0.502f, 0.565f};
    vec3 SLATEGREY = {0.439f, 0.502f, 0.565f};
    vec3 SNOW = {1.000f, 0.980f, 0.980f};
    vec3 SPRINGGREEN = {0.000f, 1.000f, 0.498f};
    vec3 STEELBLUE = {0.275f, 0.510f, 0.706f};
    vec3 TAN = {0.824f, 0.706f, 0.549f};
    vec3 TEAL = {0.000f, 0.502f, 0.502f};
    vec3 THISTLE = {0.847f, 0.749f, 0.847f};
    vec3 TOMATO = {1.000f, 0.388f, 0.278f};
    vec3 TURQUOISE = {0.251f, 0.878f, 0.816f};
    vec3 VIOLET = {0.933f, 0.510f, 0.933f};
    vec3 WHEAT = {0.961f, 0.871f, 0.702f};
    vec3 WHITE = {1.000f, 1.000f, 1.000f};
    vec3 WHITE_SMOKE = {0.961f, 0.961f, 0.961f};
    vec3 YELLOW = {1.000f, 1.000f, 0.000f};
    vec3 YELLOW_GREEN = {0.604f, 0.804f, 0.196f};
}static colors;

#define COLOR_TABLE
#endif // COLOR_TABLE

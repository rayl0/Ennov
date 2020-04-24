#pragma once
#ifndef ENNOV_UTILS_H

inline f32
normf(f32 value, f32 min, f32 max)
{
    return (value - min) / (max - min);
}

inline f32
lerpf(f32 value, f32 min, f32 max)
{
    return (max - min) * value + min;
}

inline f32
clampf(f32 value, f32 min, f32 max)
{
    if(value < min)
        return min;
    if(value > max)
        return max;

    return value;
}

inline f32
minf(f32 a, f32 b)
{
    if(a > b)
        return b;
    else
        return a;
}

inline f32
maxf(f32 a, f32 b)
{
    if(a > b)
        return a;
    else
        return b;
}

inline f32
mapf(f32 value, f32 src_min, f32 src_max, f32 dest_min, f32 dest_max)
{
    f32 temp = normf(value, src_min, src_max);
    return lerpf(temp, dest_min, dest_max);
}

#define ENNOV_UTILS_H
#endif

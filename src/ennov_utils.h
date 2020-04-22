#pragma once
#ifndef ENNOV_UTILS_H

inline f32
normf(f32 min, f32 max, f32 value)
{
    return (value - min) / (max - min);
}

inline f32
lerpf(f32 min, f32 max, f32 value)
{
    return (max - min) * value;
}

inline f32
clampf(f32 min, f32 max, f32 value)
{
    if(value < min)
        return min;
    if(value > max)
        return max;

    return value;
}

inline f32
mapf(f32 src_min, f32 src_max, f32 dest_min, f32 dest_max, f32 value)
{
    f32 temp = normf(src_min, src_max, value);
    return lerpf(dest_min, dest_max, temp);
}

#define ENNOV_UTILS_H
#endif

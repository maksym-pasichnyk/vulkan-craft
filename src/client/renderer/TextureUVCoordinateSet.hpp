#pragma once

struct TextureUVCoordinateSet {
    float minU;
    float minV;
    float maxU;
    float maxV;

    inline constexpr float getInterpolatedU(float t) const {
        return (1 - t) * minU + t * maxU;
    }

    inline constexpr float getInterpolatedV(float t) const {
        return (1 - t) * minV + t * maxV;
    }
};

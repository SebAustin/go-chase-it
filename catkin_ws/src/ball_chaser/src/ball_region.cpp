#include "ball_chaser/ball_region.h"

namespace ball_chaser
{

namespace
{

constexpr std::uint32_t kBytesPerPixel = 3;
constexpr std::uint8_t kWhite = 255;

constexpr double kForwardSpeed = 0.5;  // m/s
constexpr double kTurnSpeed = 0.5;     // rad/s, positive turns left

bool isWhite(const std::vector<std::uint8_t>& data, std::size_t i)
{
    return data[i] == kWhite && data[i + 1] == kWhite && data[i + 2] == kWhite;
}

}  // namespace

BallRegion classifyBallRegion(const std::vector<std::uint8_t>& data,
                              std::uint32_t width,
                              std::uint32_t height,
                              std::uint32_t step)
{
    const std::uint64_t row_bytes = static_cast<std::uint64_t>(width) * kBytesPerPixel;
    if (width == 0 || height == 0 || step < row_bytes ||
        data.size() < static_cast<std::uint64_t>(step) * height)
    {
        return BallRegion::None;
    }

    std::uint64_t column_sum = 0;
    std::uint64_t white_count = 0;
    for (std::uint32_t row = 0; row < height; ++row)
    {
        const std::size_t row_start = static_cast<std::size_t>(row) * step;
        for (std::uint32_t col = 0; col < width; ++col)
        {
            if (isWhite(data, row_start + col * kBytesPerPixel))
            {
                column_sum += col;
                ++white_count;
            }
        }
    }

    if (white_count == 0)
    {
        return BallRegion::None;
    }

    // average = column_sum / white_count; compare 3 * average against width without
    // dividing, so the thirds are exact rather than truncated.
    const std::uint64_t scaled_average = 3 * column_sum;
    const std::uint64_t scaled_width = static_cast<std::uint64_t>(width) * white_count;
    if (scaled_average < scaled_width)
    {
        return BallRegion::Left;
    }
    if (scaled_average < 2 * scaled_width)
    {
        return BallRegion::Center;
    }
    return BallRegion::Right;
}

DriveCommand driveCommandFor(BallRegion region)
{
    switch (region)
    {
        case BallRegion::Left:
            return {0.0, kTurnSpeed};
        case BallRegion::Center:
            return {kForwardSpeed, 0.0};
        case BallRegion::Right:
            return {0.0, -kTurnSpeed};
        case BallRegion::None:
        default:
            return {0.0, 0.0};
    }
}

const char* toString(BallRegion region)
{
    switch (region)
    {
        case BallRegion::Left:
            return "left";
        case BallRegion::Center:
            return "center";
        case BallRegion::Right:
            return "right";
        case BallRegion::None:
        default:
            return "none";
    }
}

}  // namespace ball_chaser

#ifndef BALL_CHASER_BALL_REGION_H
#define BALL_CHASER_BALL_REGION_H

#include <cstdint>
#include <vector>

namespace ball_chaser
{

// Which third of the image the Ball's average column falls in.
enum class BallRegion
{
    None,
    Left,
    Center,
    Right
};

// The velocities that make up a Drive Command.
struct DriveCommand
{
    double linear_x;
    double angular_z;
};

// Classify a packed 3-bytes-per-pixel image (rgb8 or bgr8).
// `step` is the row length in bytes and may include padding past width * 3.
// Returns BallRegion::None when no pure-white pixel is found or the buffer is malformed.
BallRegion classifyBallRegion(const std::vector<std::uint8_t>& data,
                              std::uint32_t width,
                              std::uint32_t height,
                              std::uint32_t step);

DriveCommand driveCommandFor(BallRegion region);

const char* toString(BallRegion region);

}  // namespace ball_chaser

#endif  // BALL_CHASER_BALL_REGION_H

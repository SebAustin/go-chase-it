#include "ball_chaser/ball_region.h"

#include <gtest/gtest.h>

using ball_chaser::BallRegion;
using ball_chaser::classifyBallRegion;
using ball_chaser::driveCommandFor;

namespace
{

constexpr std::uint32_t kBytesPerPixel = 3;

// A gray image of the given size; `step` defaults to a tightly packed row.
struct TestImage
{
    std::uint32_t width;
    std::uint32_t height;
    std::uint32_t step;
    std::vector<std::uint8_t> data;

    TestImage(std::uint32_t w, std::uint32_t h, std::uint32_t row_padding = 0)
        : width(w), height(h), step(w * kBytesPerPixel + row_padding), data(step * h, 100)
    {
    }

    void paintWhite(std::uint32_t col, std::uint32_t row)
    {
        const std::size_t i = row * step + col * kBytesPerPixel;
        data[i] = data[i + 1] = data[i + 2] = 255;
    }

    BallRegion classify() const { return classifyBallRegion(data, width, height, step); }
};

}  // namespace

TEST(ClassifyBallRegion, ReturnsNoneWhenNoWhitePixels)
{
    TestImage img(9, 3);
    EXPECT_EQ(img.classify(), BallRegion::None);
}

TEST(ClassifyBallRegion, ReturnsLeftForWhiteInFirstThird)
{
    TestImage img(9, 3);
    img.paintWhite(1, 1);
    EXPECT_EQ(img.classify(), BallRegion::Left);
}

TEST(ClassifyBallRegion, ReturnsCenterForWhiteInMiddleThird)
{
    TestImage img(9, 3);
    img.paintWhite(4, 2);
    EXPECT_EQ(img.classify(), BallRegion::Center);
}

TEST(ClassifyBallRegion, ReturnsRightForWhiteInLastThird)
{
    TestImage img(9, 3);
    img.paintWhite(8, 0);
    EXPECT_EQ(img.classify(), BallRegion::Right);
}

TEST(ClassifyBallRegion, UsesAverageColumnNotFirstPixel)
{
    // First white pixel is on the left, but most of the Ball is on the right.
    TestImage img(9, 3);
    img.paintWhite(0, 0);
    img.paintWhite(8, 1);
    img.paintWhite(8, 2);
    img.paintWhite(7, 2);
    img.paintWhite(8, 0);
    // Columns 0, 7, 8, 8, 8 average to 6.2: in the right third (6..8).
    EXPECT_EQ(img.classify(), BallRegion::Right);
}

TEST(ClassifyBallRegion, ThirdBoundariesUseExactAverage)
{
    // Columns 5 and 6 average to 5.5, still the middle third of a 9-wide image.
    TestImage img(9, 1);
    img.paintWhite(5, 0);
    img.paintWhite(6, 0);
    EXPECT_EQ(img.classify(), BallRegion::Center);

    // Adding column 7 moves the average to 6.0, the first column of the right third.
    img.paintWhite(7, 0);
    EXPECT_EQ(img.classify(), BallRegion::Right);
}

TEST(ClassifyBallRegion, IgnoresNearlyWhitePixels)
{
    TestImage img(9, 3);
    const std::size_t i = 4 * kBytesPerPixel;
    img.data[i] = 255;
    img.data[i + 1] = 255;
    img.data[i + 2] = 254;
    EXPECT_EQ(img.classify(), BallRegion::None);
}

TEST(ClassifyBallRegion, IgnoresRowPaddingBytes)
{
    // White bytes in the padding area must not count as a pixel.
    TestImage img(9, 2, 3);
    const std::size_t padding_start = 9 * kBytesPerPixel;
    img.data[padding_start] = img.data[padding_start + 1] = img.data[padding_start + 2] = 255;
    EXPECT_EQ(img.classify(), BallRegion::None);

    img.paintWhite(0, 1);
    EXPECT_EQ(img.classify(), BallRegion::Left);
}

TEST(ClassifyBallRegion, ReturnsNoneForTruncatedBuffer)
{
    TestImage img(9, 3);
    img.paintWhite(4, 0);
    img.data.resize(img.data.size() - 1);
    EXPECT_EQ(img.classify(), BallRegion::None);
}

TEST(ClassifyBallRegion, ReturnsNoneWhenStepShorterThanRow)
{
    TestImage img(9, 3);
    img.paintWhite(4, 0);
    EXPECT_EQ(classifyBallRegion(img.data, img.width, img.height, img.width * kBytesPerPixel - 1),
              BallRegion::None);
}

TEST(ClassifyBallRegion, ReturnsNoneForEmptyImage)
{
    EXPECT_EQ(classifyBallRegion({}, 0, 0, 0), BallRegion::None);
}

TEST(DriveCommandFor, StopsWhenNoBall)
{
    const auto cmd = driveCommandFor(BallRegion::None);
    EXPECT_DOUBLE_EQ(cmd.linear_x, 0.0);
    EXPECT_DOUBLE_EQ(cmd.angular_z, 0.0);
}

TEST(DriveCommandFor, DrivesForwardWhenCentered)
{
    const auto cmd = driveCommandFor(BallRegion::Center);
    EXPECT_GT(cmd.linear_x, 0.0);
    EXPECT_DOUBLE_EQ(cmd.angular_z, 0.0);
}

TEST(DriveCommandFor, TurnsLeftWithPositiveAngularZ)
{
    const auto cmd = driveCommandFor(BallRegion::Left);
    EXPECT_DOUBLE_EQ(cmd.linear_x, 0.0);
    EXPECT_GT(cmd.angular_z, 0.0);
}

TEST(DriveCommandFor, TurnsRightWithNegativeAngularZ)
{
    const auto cmd = driveCommandFor(BallRegion::Right);
    EXPECT_DOUBLE_EQ(cmd.linear_x, 0.0);
    EXPECT_LT(cmd.angular_z, 0.0);
}

TEST(ToString, NamesEveryRegion)
{
    EXPECT_STREQ(ball_chaser::toString(BallRegion::None), "none");
    EXPECT_STREQ(ball_chaser::toString(BallRegion::Left), "left");
    EXPECT_STREQ(ball_chaser::toString(BallRegion::Center), "center");
    EXPECT_STREQ(ball_chaser::toString(BallRegion::Right), "right");
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

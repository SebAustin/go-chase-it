#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include "ball_chaser/ball_region.h"
#include <sensor_msgs/Image.h>
#include <sensor_msgs/image_encodings.h>

using ball_chaser::BallRegion;

// Define a global client that can request services
ros::ServiceClient client;

// Region of the last Drive Command that reached drive_bot; empty until the first one succeeds
bool has_sent_command = false;
BallRegion last_sent_region = BallRegion::None;

// This function calls the command_robot service to drive the robot in the specified direction
bool drive_robot(float lin_x, float ang_z)
{
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    if (!client.call(srv))
    {
        ROS_ERROR("Failed to call service /ball_chaser/command_robot");
        return false;
    }
    return true;
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::ImageConstPtr& img)
{
    // White is (255,255,255) in both orders, so rgb8 and bgr8 are equally fine
    if (img->encoding != sensor_msgs::image_encodings::RGB8 &&
        img->encoding != sensor_msgs::image_encodings::BGR8)
    {
        ROS_WARN_THROTTLE(5.0, "Unsupported image encoding '%s', expected rgb8", img->encoding.c_str());
        return;
    }

    const BallRegion region = ball_chaser::classifyBallRegion(img->data, img->width, img->height, img->step);

    // Only send a Drive Command when it changes; a failed call is retried on the next frame
    if (has_sent_command && region == last_sent_region)
    {
        return;
    }

    const ball_chaser::DriveCommand command = ball_chaser::driveCommandFor(region);
    ROS_INFO("Ball region: %s", ball_chaser::toString(region));
    if (drive_robot(command.linear_x, command.angular_z))
    {
        has_sent_command = true;
        last_sent_region = region;
    }
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Wait for drive_bot so the first commands are not lost
    ROS_INFO("Waiting for /ball_chaser/command_robot");
    ros::service::waitForService("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}

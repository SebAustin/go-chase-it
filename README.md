# Go Chase It

Udacity Robotics Software Engineer, project 2. A differential-drive robot with a camera and a Hokuyo lidar lives in a Gazebo world built with the Building Editor, and chases a white ball.

```
catkin_ws/src/
├── my_robot/                  robot model, world, launch files
│   ├── launch/                world.launch, robot_description.launch
│   ├── meshes/                hokuyo.dae (fetched in the VM, see below)
│   ├── rviz/                  my_robot.rviz
│   ├── urdf/                  my_robot.xacro, my_robot.gazebo
│   └── worlds/                the Building Editor world with my_ball
└── ball_chaser/               ball detection and driving
    ├── launch/                ball_chaser.launch
    ├── srv/                   DriveToTarget.srv
    ├── include/ball_chaser/   ball_region.h
    ├── src/                   drive_bot.cpp, process_image.cpp, ball_region.cpp
    └── test/                  test_ball_region.cpp (gtest)
```

## Robot

Changes from the lesson sample:

| | Lesson | This robot |
|---|---|---|
| Chassis | 0.4 × 0.2 × 0.1 m | 0.5 × 0.3 × 0.1 m |
| Wheel radius | 0.10 m | 0.12 m |
| Wheel separation | 0.30 m | 0.36 m |
| Color | default gray | blue chassis, black wheels, red camera |

Inertia comes from solid-body formulas, and the casters reach the ground exactly, so the robot sits level.

## Setup (Udacity workspace, ROS Noetic)

```bash
cp -r catkin_ws/src/my_robot catkin_ws/src/ball_chaser ~/catkin_ws/src/
wget https://s3-us-west-1.amazonaws.com/udacity-robotics/hokuyo.dae -P ~/catkin_ws/src/my_robot/meshes/
cd ~/catkin_ws && catkin_make && source devel/setup.bash
```

## Run

```bash
roslaunch my_robot world.launch          # terminal 1: Gazebo + RViz
roslaunch ball_chaser ball_chaser.launch # terminal 2: drive_bot + process_image
rosrun rqt_image_view rqt_image_view     # optional: watch the camera
```

Drag the ball in front of the robot in Gazebo; it turns toward it and drives up to it.

Test the drive service on its own:

```bash
rosservice call /ball_chaser/command_robot "linear_x: 0.5
angular_z: 0.0"
```

## How the chasing works

`process_image` averages the column of every pure-white (255,255,255) pixel and classifies the result into the left, center or right third of the image, or none. Each region maps to a fixed drive command: turn left, go forward, turn right, stop. A command is sent to `/ball_chaser/command_robot` only when it changes; if a call fails, it is retried on the next frame. `drive_bot` publishes the requested velocities on `/cmd_vel`.

Nothing else in the world may be pure white, or the robot would chase it.

## Tests

```bash
cd ~/catkin_ws && catkin_make run_tests_ball_chaser && catkin_test_results
```

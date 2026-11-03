# Go Chase It

A simulated differential-drive robot that sees a white ball through its camera and drives toward it.

## Language

**Ball**:
The single white sphere in the world; the only object the robot chases. Any pure-white (255,255,255) pixel in a camera image is treated as part of the Ball.
_Avoid_: target, object

**Ball Region**:
Which third of the camera image the Ball's average horizontal position falls in: left, center, or right — or none when no Ball pixels are visible.
_Avoid_: ball position, direction, side

**Drive Command**:
One of four fixed motions — turn left, go forward, turn right, stop — chosen from the Ball Region and sent to the robot only when it changes.
_Avoid_: velocity request, motor command

**Start Pose**:
Where the robot is placed in the world at launch; always inside the building.
_Avoid_: spawn point, initial position

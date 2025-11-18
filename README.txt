Group Members: Neel Shejwalkar, Jay Katyan

===========
Milestone 1
===========

Video: https://youtu.be/QCswykhS4P4

Jay Katyan: Procedural Terrain, Efficient Terrain Rendering and Chunking


Neel Shejwalkar:

Mouse movement:
I implemented smooth mouse movement by calculating the change in position between the center and the direction of the current movement, snapping the cursor back to the center after every mouse event. This performed poorly because recentering is somewhat expensive, so I changed the code to only recenter after the cursor moved beyond a certain threshold number of pixels away from the center, resulting in much smoother movement.

Placing/removing blocks:
This was implemented by performing grid marching from the camera's center through its forward vector. The desired block to remove was simply the one that the grid marching algorithm finds to be the closest hit. To know where to add a new block, however, I needed to modify the algorithm slightly to keep track of, and return, the (empty) block immediately before a hit.

Collision detection:
To detect a collision, I performed grid marching 3 times (one for each dimension) for all 8 vertices of the player's bounding box. If the expected distance to be travelled for that tick (calculated using the current velocity at that tick) for any direction is greater than the "allowed" distance (found using grid marching) for that direction, then the current velocity is zeroed out for that direction. Splitting up the logic for velocity into components like this lends to smooth sliding across walls, as well as allowing for jumping.

Other features:
I added a crosshair by buffering 2 small lines, and then drawing them in screen space using the same progFlat shader program.
I added a teleportation feature - by clicking t, one can aim and then teleport directly to a block up to 200 blocks' distance from the player's current position. This was implemented using the grid marching function used before.

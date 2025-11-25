Group Members: Neel Shejwalkar, Jay Katyan

===========
Milestone 1
===========

Video: https://youtu.be/QCswykhS4P4

Jay Katyan:

Procedural Terrain:
I implemented procedural terrain generation by creating two new classes, the "World" class and the "Noise" class. The noise class contains a number of helper functions/various noise generators, whereas the world class uses these noise generators to procedurally generate the terrain. Currently, the world class is capable of generating two distinct terrains, the "grassland" terrain and
the "mountains" terrain (each represented as an enum). The "grassland" terrain uses a combination of Worley noise and Fractal Perlin Noise to create a realistic, rolling terrain with subtle hills.
The "mountains" terrain uses Fractal Perlin Noise which is then post-processed to amplify its outputs/height to create more realistic mountains. Finally, I've implemented logic in terrain.cpp to
set blocks according to their world height. Throughout my work, I used various helper scripts/tools to visualize different noise functions and height maps before implementing them into the game itself.

Efficient Terrain Rendering and Chunking:
To implement efficient terrain rendering and chunking, I implemented the create() function such that the VBO data is interleaved (rather than instanced) and only contains face data (rather the all sides for every block, including faces that cannot be directly seen). This improved performance significantly, and allowed for significantly more efficient terrain rendering. I also implemented
terrain expansion logic, however rather than only doing checks for -16-+16 around the player, I decided to check a much larger range of -32-+32 on x and z. This allows for a signficantly improved
user experience when roaming the world and generating new chunks, as chunks are not generated in "strips" and rather as larger landmasses. I also increased the render distance significantly, allowing
for 1024x1024 blocks to be visible at once. In part, this was done to assist my debugging of the world generation. Simultaneously however, this improves the gameplay experience and makes the game
far more similar to Minecraft's official implementation.

Neel Shejwalkar:

Mouse movement:
I implemented smooth mouse movement by calculating the change in position between the center and the direction of the current movement, snapping the cursor back to the center after every mouse event. This performed poorly because recentering is somewhat expensive, so I changed the code to only recenter after the cursor moved beyond a certain threshold number of pixels away from the center, resulting in much smoother movement.

Placing/removing blocks:
This was implemented by performing grid marching from the camera's center through its forward vector. The desired block to remove was simply the one that the grid marching algorithm finds to be the closest hit. To know where to add a new block, however, I needed to modify the algorithm slightly to keep track of, and return, the (empty) block immediately before a hit.

Collision detection:
To detect a collision, I performed grid marching 3 times (one for each dimension) for all 8 vertices of the player's bounding box. If the expected distance to be travelled for that tick (calculated using the current velocity at that tick) for any direction is greater than the "allowed" distance (found using grid marching) for that direction, then the current velocity is zeroed out for that direction. Splitting up the logic for velocity into components like this lends to smooth sliding across walls and allows for jumping.

Other features:
I added a crosshair by buffering 2 small lines, and then drawing them in screen space using the same progFlat shader program.
I added a teleportation feature - by clicking t, one can aim and then teleport directly to a block up to 200 blocks' distance from the player's current position. This was implemented using the grid marching function used before.


===========
Milestone 2
===========

Video: https://youtu.be/IdP-l8xU5Pg

Jay Katyan:
Cave Generation: 
I implemented cave generation by extending the 2D procedural noise pipeline used for the terrain. Using a 3D Perlin Noise field, each (x, y, z) coordinate is evaluated against a cave threshold to determine whether that position should be carved out into a cave, which produces organically-shaped underground caves. I also added lava by assigning lava blocks to low altitude empty regions, ensuring that deeper caves naturally fill with lava while higher caves remain hollow.

Multithreading:
I implemented multithreading for the extended terrain generation setting up BlockTypeWorkers for internal data structure editing and VBOWorkers for adding to the VBO. I decided to have these workers subclass Qt's QRunnable and be managed by QThreadPool for this milestone over the C++ standard library multithreading utilities, as I already had prior experience with Qt multithreading and thought it would integrate better. To safely edit and share data between the workers and the main Terrain class, I used QMutex to lock and wait for completed chunk additions and VBO edits for each respective worker. 

Neel Shejwalkar:
Texturing and Texture Animation:
I added textures into our game by using the given texture atlas and appending the correct UV coordinates into the per-chunk VBOs. I added an "animate" flag for water and lava blocks in the z position of the vec4 that specified UV coordinates in the VBO, and updated the fragment shader to correctly animate these blocks. For each chunk, I created 2 pairs of VBOs, one for transparent and one for opaque blocks, and enabled alpha blending.

Other features:
I slightly restructured the terrain generation to make it easier to set up for multithreading by introducing the terrain generation zone primitive and reordering chunk loops.

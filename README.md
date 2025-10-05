### opengl-mischiefs engine

**API: OpenGL**

In-engine How-to:
+ Navigation: W-A-S-D=planar motion. Space=Up. LShift=Down. Q=Peek left. E=Peek right.
+ Zoom: mouse scroll.
+ Hit LeftCtrl to toggle axes

Homework 2 report:     
+ Tasks [1;2;3;4]:   

Below is an example of instanced rendering with multiple meshes and multiple textures.      
Cubes, pyramids and Bills are batched per mesh. Textures are bound bindlessly and passed within SSBO to the shader.    
Axes are also instanced in a sense that model matrices are passed as per-instance vertex attributes (not the most efficient approach, but I wanted to get more practice).      
The world plane is a huge squashed cube (to make sure the backface culling does not make it render from a single side), with texture coordinates determined based on the fragment's world position. In the picture, the plane uses the anisotropic filtering of level 8.

![instancing](./data/instancing.png)

+ Tasks [5]:
   
I've run the scene with instancing on and off for various numbers of cubes. Measurements were taken with `mangohud`.     
As expected, the instancing apporach demonstrated less steep increase in average frame time.      
Behold the numbers:     

![plot](./data/results.png)

![benchmark](./data/benchmarking.png)
![benchmark2](./data/benchmarking2.png)



Extra tasks implemented:     
+ ~~Quaternion-based camera (Q/E allow to peek like in r6s)~~
+ ~~Global and object-local coordinate axes (hit LeftCtrl)~~
+ ~~Dynamically moving light cude~~

**Late days **used**: 0/7**




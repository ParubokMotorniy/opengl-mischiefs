### opengl-mischiefs engine

**API: OpenGL**

# In-engine How-to:
+ Navigation: W-A-S-D=planar motion. Space=Up. LShift=Down. Q=Peek left. E=Peek right.
+ Zoom: mouse scroll.
+ Hit LeftCtrl to toggle axes

## Homework 2 report:     
### Tasks [1;2;3;4]:   

Below is an example of instanced rendering with multiple meshes and multiple textures.      
Cubes, pyramids and Bills are batched per mesh. Textures are bound bindlessly and passed within SSBO to the shader.    
Axes are also instanced in a sense that model matrices are passed as per-instance vertex attributes (not the most efficient approach, but I wanted to get more practice).      
The world plane is a huge squashed cube (to make sure the backface culling does not make it render from a single side), with texture coordinates determined based on the fragment's world position. In the picture, the plane uses the anisotropic filtering of level 8.

![instancing](./data/instancing.png)

### Tasks [5]:
   
I've run the scene with instancing on and off for various numbers of cubes. Measurements were taken with `mangohud`.     
As expected, the instancing apporach demonstrated less steep increase in average frame time.      
Behold the numbers:     

![plot](./data/results.png)

![benchmark](./data/benchmarking.png)
![benchmark2](./data/benchmarking2.png)

### Extra: Texture filtering approaches comparison

The advantage of anisotropic sampling is clear: instead of pixel mess, we observe more-less accurate lines that stratch into the distance.     
Similarly, the trilinear filtering introduces gentle blur, which is surely more appealing.     

|   |   |
|---|---|
|*nearest*|*bilinear*|
|![nearest](./data/nearest.png)|![bilinear](./data/bilinear.png)   |
|*trilinear*|*aniso8*|
| ![trilinear](./data/trilinear.png)|![aniso8](./data/aniso8.png)   |
|*aniso16*||
|![aniso16](./data/aniso16.png)   |   |

And here is the numeric data (obtained with that same `mangohub`). Note that difference starts to become evident only for large numbers of cubes. Nonetheless, even then it's ... unpredictable. The frame time is chracterized with lots of spikes, while, on average, we can state that anisotropic filtering is more expensive.  

![results](./data/filtering_results.png)

### Extra: Arbitrary mode loading

You've seen example with bill, so here is an example with a different model: 

![mayhem](./data/tank_mayhem.png)


Extra tasks implemented:    
+ Add support for bindless textures (instanced shader class internally pass textures handles to the shader via SSBO)     
+ Allow instancing of arbitrary model (almost arbitrary: only .obj files have been tested and are known to be processed correctly. The attached 'tank' and 'bill' models work, for example).
+ Performance and visual comparsion of various texture filtering algorithms (the textures expose interface that allows setting of filtering mode).
+ ~~Quaternion-based camera (Q/E allow to peek like in r6s)~~
+ ~~Global and object-local coordinate axes (hit LeftCtrl)~~
+ ~~Dynamically moving light cude~~

Fixes of the homework #1: 
+ singletons
+ resources paths

**Late days **used**: 0/7**




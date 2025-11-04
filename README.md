### opengl-mischiefs engine

**Name/Surname**:      
Oles Yarish

**API: OpenGL**

**Late days**: 0/7

## How to build

```
mkdir build && cd build
cmake ..
cmake --build . 
```

# In-engine How-to:
+ Navigation: W-A-S-D=planar motion. Space=Up. LShift=Down. Q=Peek left. E=Peek right.
+ Zoom: mouse scroll.
+ Hit LeftCtrl to toggle axes
+ Hit RightCtrl to toggle world plane

## Homework 3 report:

### Tasks [1;2]

Multiple light sources of each kind are possible. They are passed to shaders, which implement standard Blinn-Phong lighting, through uniform buffers.    
The light sources have their own shaders and are represented with pointy spheres (which visualize the directions of spot&directional lights).       
In the scene there are two point ligths and two directional lights.     
The effect of individual lights can be verified by commenting out the lightint application loops in `fragment_standard.fs`

### Tasks [3]

I tested my engine, besides the beloved Linux, on Windows machine with Intel (R) UHD 770 graphics (which does support bindless textures), compiled with MSVC 2022.  
Both compilation and rendering succeeded.    

I pray to all known gods the engine will compile with other versions of MSVC on different machines.


### Extras
1. Textured spotlight.          
As demonstrated by the projected texture of Bill, the spotlights can have textures represent their diffuse color.  
**Note:** the texturing only works with bindless textures. 

2. Skybox support    
The whole-new manager takes care of cubemaps now, and so skyboxes can be added to the scene. 
   
![extra-demo](./data/demo.png)

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




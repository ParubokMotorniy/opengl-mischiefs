### opengl-mischiefs engine

   This engine has been developed in the scope of "Introduction to Realtime Computer Graphics" university course.        
I believe my changelogs for individual homeworks are quite detailed, so I refer a reader to them to learn what this "engine" is capable of.       
   As my final project I've also added support for rendering of volumetirc fog. In particular, the defined volumes of fog are raymarched in a compute shader, while taking into account the influence of light. 

## DEMO

You can check out the demo of the engine [here](./data/demo.mp4).

## Most interesting features

+ The engine relies on bindless textures in pretty much everything, except for shadow maps      
+ Arbitrary models can be loaded and instanced, as part of either Blinn-Phong or PBR passes
+ HDR tonemapping
+ Tiny entity-component-system (ECS)
+ Raymarched volumetric fog rendering.
+ Quaternion-based camera (Q/E allow to peek like in r6s)
+ Global and object-local coordinate axes
+ Skybox

## How to build

```
mkdir build && cd build
cmake ..
cmake --build .
```

Should you find any assets missing, try running `git lfs pull` in the project's directory.


# In-engine How-to:
+ Navigation: W-A-S-D=planar motion. Space=Up. LShift=Down. Q=Peek left. E=Peek right.
+ Zoom: mouse scroll.
+ Hit RightCtrl to toggle world plane

*Important*: for proper out-of-the-box compilation, the `ENGINE_DISABLE_BINDLESS_TEXTURES` must be `OFF`.
*Note*: the engine demo can be found in the `data` folder. 

## Homework 5 changelog:

### Task [1] - HDR and gamma correction support

The engine keeps a stack of frame buffers that can be swapped at runtime. Among those is the HDR color buffer that most passes unknowingly render into until the HDR pass silently tonemaps that buffer into the standard LDR color buffer.  
Currently 5 tonemapping algorithms are supported.  

|||
|---|---|
|No tonemapping|Reinhard-Exposure|
|![nt](./data/no_tonemapping.png)|![re](./data/exposure.png)|
|Uncharted|Filmic|
|![uc](./data/uncharted.png)|![fc](./data/filmic.png)|
|ACES|ACES filmic|
|![as](./data/aces.png)|![af](./data/aces_filmic.png)|

### Task [2] - PBR support

The models are loaded with a simple material by default, plus the PBR material if explicitly requested. Thus, one can allocate that model to either standard or PBR passes. 
Moreover, the PBR models are instanced and rely on bindless textures, the same way the standard models do.

### Task [3] - Normal mapping support

The PBR pipeline supports normal mapping (as long as the normal map is provided with the model!). Not much to say on that account.

## Homework 4 changelog:

### Tasks [1] - Shadow rendering support
Shadow biasing is achieved by slightly shifting fragments along their normals prior to shadow testing.      
Currently only directional lights support shadow mapping.

### Tasks [2] - PCF and comparison sampling support

|||
|---|---|
|No PCF, No Comparison sampler|PCF, No Comparison sampler |
|![no-pcf](./data/no_pcf_shadow.png)|![sw-pcf](./data/sf_pcf_shadow.png)|
|No PCF, Comparison sampler|PCF, Comparison Sampler|
|![hw-pcf](./data/hw_pcf_shadow.png)|![double-pcf](./data/double_pcf_shadow.png)|

### Tasks [3] - Basic UI with ImGUI
The current UI allows to fine-tune the shadow bias and adjust the gizmos.

### Tasks [4] - Transparency support
The transparent objects are rendered in a separate pass, similar to standard objects and shadow maps.  

## Homework 3 report:

### Tasks [1;2] - Point/Directional/Spot lights support

Multiple light sources of each kind are possible. They are passed to shaders, which implement standard Blinn-Phong lighting, through uniform buffers.    
The light sources have their own shaders and are represented with pointy spheres (which visualize the directions of spot&directional lights).       
In the scene there are two point ligths and two directional lights.     
The effect of individual lights can be verified by commenting out the lighting application loops in `fragment_standard.fs`

### Extra features
1. Textured spotlight.          
As demonstrated by the projected texture of Bill Cipher, the spotlights can have textures represent their diffuse color.  
**Note:** the texturing only works with bindless textures. 

2. Skybox support    
The whole-new manager takes care of cubemaps now, and so skyboxes can be added to the scene. 
   
![extra-demo](./data/demo.png)

## Homework 2 report:     
### Tasks [1;2;3;4]: - Model loading&instancing support

Below is an example of instanced rendering with multiple meshes and multiple textures.      
Cubes, pyramids and Bills are batched per mesh. Textures are bound bindlessly and passed within SSBO to the shader.    
Axes are also instanced in a sense that model matrices are passed as per-instance vertex attributes (not the most efficient approach, but I wanted to get more practice).      
The world plane is a huge squashed cube (to make sure the backface culling does not make it render from a single side), with texture coordinates determined based on the fragment's world position. In the picture, the plane uses the anisotropic filtering of level 8.

![instancing](./data/instancing.png)

### Task [5]: - Performance test
   
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

And here is the numeric data (obtained with that same `mangohud`). Note that difference starts to become evident only for large numbers of cubes. Nonetheless, even then it's ... unpredictable. The frame time is chracterized with lots of spikes, while, on average, we can state that anisotropic filtering is more expensive.  

![results](./data/filtering_results.png)

### Extra: Arbitrary model loading

You've seen example with bill, so here is an example with a different model: 

![mayhem](./data/tank_mayhem.png)

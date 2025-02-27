# Graphics Programming 1 - Raytracer | Jelle Rubbens - 2DAE20N

## Added feature

Small feature called Mutli-Sample Anti-Alisiasing (MSAA). In each pixel, several samples are taken with a [Grid uniform distribution pattern](https://en.wikipedia.org/wiki/Supersampling#Supersampling_patterns).  
The current amount of samples can now be adjusted with the **up/down arrow keys**, within a range of [1, 16] with increments of 4. (*Sample amount avaible: {1, 4, 16}*)

## Controls

### Toggle Modes

- **F2** -> Toggle Shadows 
- **F3** -> Toggle Lighting Mode

### Camera 

- **WASD** -> Move camera
- **RMB (hold)** -> Rotate Camera (Pitch & Yawn)

### Switch scenes

*(The scenes are ordered based on the weeks of class. The first scene that loads up on start is the ReferenceScene from Week 4. The previous scene is Week 3, Next is Week 4 Bunny Scene)*

- **Left Arrow** -> Load previous Scene
- **Right Arrow** -> Load next Scene

### Multi-Sample Anti-Aliasing (MSAA)

- **Up Arrow** -> Increase sample count
    - (*currentSampleAmount x 4*)
- **Up Arrow** -> Decrease sample count
    - (*currentSampleAmount / 4*)


## Shading Mode Comparison

| **Shading Mode**   | **Reference Scene**                                                                                                                                               | **lowpoly_bunny.obj**                                                                                                                                            |
|---------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------|
| **Combined**        | <img src="./GitMedia/ReferenceScene/RayTracing_Buffer_Reference_Combined.png" alt="drawing" width="600"/>                                                        | <img src="./GitMedia/Bunny/RayTracing_Buffer_Bunny_Combined.png" alt="drawing" width="600"/>                                                                    |
| **Observed Area**   | <img src="./GitMedia/ReferenceScene/RayTracing_Buffer_Reference_ObservedArea.png" alt="drawing" width="600"/>                                                     | <img src="./GitMedia/Bunny/RayTracing_Buffer_Bunny_ObservedArea.png" alt="drawing" width="600"/>                                                                |
| **Radiance**        | <img src="./GitMedia/ReferenceScene/RayTracing_Buffer_Reference_Radiance.png" alt="drawing" width="600"/>                                                         | <img src="./GitMedia/Bunny/RayTracing_Buffer_Bunny_Radiance.png" alt="drawing" width="600"/>                                                                    |
| **Raw BRDF Data**   | <img src="./GitMedia/ReferenceScene/RayTracing_Buffer_Reference_RawBRDF.png" alt="drawing" width="600"/>                                                          | <img src="./GitMedia/Bunny/RayTracing_Buffer_Bunny_RawBRDF.png" alt="drawing" width="600"/>                                                                     |

# Dx11SDL
Test application combining Dx11 rendering with an SDL2 window

## Features to add

* [DONE] Fix strange view distorsion
    * Tune near and far values
    * Normalize directions used to build transform matrices
    * Make sure we provide the FOV angle properly

* [DONE] Check and make sure the winding order is being use properly
    * Also check to make sure we are culling the back faces

* [DONE] Add a directionnal light source position to send to shader
    * Apply simple shading

* [DONE] Better OBJ file loading for multiple objets in one file
    * Handle indices and only store unique vertices
    * Use indexed draws with the new indices for better storage perf.

* [IN PROGRESS] Implement simple shadow mapping technique
    * Depth only pass from the light's POV
    * Send this depth texture to lighting shader
    * Reconstruct 3d position of fragment to test if the fragment is in shadow or not

* [IN PROGRESS] Binding and sampling from textures defined in the material files
    * Reading the texture files needed
    * Loading the texture data and upload it to the GPU memory
    * Create a texture array with all the textures needed
    * Bind the texture array to the base pass PROGRESS
    * Tag vertices with the right texture index

* Add a plane object under the main OBJ file
    * Test simple shadow mapping techniques

* Look into changing the renderer to use deferred rendering
    * Add more lights to stress test deferred
    * Add logic to handle point lights

# Dx11SDL
Test application combining Dx11 rendering with an SDL2 window

## Features to add

* Fix strange view distorsion
    * Tune near and far values
    * Normalize directions used to build transform matrices
    * Make sure we provide the FOV angle properly

* Check and make sure the winding order is being use properly
    * Also check to make sure we are culling the back faces

* Add a directionnal light source position to send to shader
    * Apply simple shading

* Add a plane object under the main OBJ file
    * Test simple shadow mapping techniques

* Look into changing the renderer to use deferred rendering
    * Add more lights to stress test deferred
    * Add logic to handle point lights
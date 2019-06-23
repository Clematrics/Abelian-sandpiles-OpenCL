# Abelian sandpiles OpenCL
An abelian sandpiles simulator, which uses OpenCL to improve performances

This project is similar to my other Abelian sandpiles simulator [here](https://github.com/Clematrics/Abelian-sandpiles), except it uses OpenCL to dispatch compute tasks rather than forcing to use the CPU. This greatly improves performances when using a GPU for example.

Please note that the current version contains bugs on some hardware, especially when changing the resolution of the image. The simulation then doesn't work properly. My machine, on which everything works, is a Surface Pro 3 with an i5 processor. However it doesn't always work on a GTX 1070ti. This could be an issue in my code or an issue in the driver, as I am not using the latest version of OpenCL (1.2 rather than 2.2).

# To do :

 * Implement a way to change the current rule (because)
 * Use `restrict` keyword in the kernel
 * Improve cache coherency on the device by tiling grids
 * Use the kernel to compute the image to display
 * Transfer directly the image computed by the kernel to an OpenGL image so it can be displayed without using the CPU
 * Add a way to choose on which device(s?) the simulation runs
 * Use an import-export file system to save a simulation and continue it later, or to import a starting pattern.
 * Implement addition or substraction of grids to fully support operations of the abelian sandpiles group
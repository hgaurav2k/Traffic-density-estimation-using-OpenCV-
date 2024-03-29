# Traffic Density estimation using Open CV functions

## Camera angle correction and frame cropping

- On the command line, type `make all` to compile the source code.
- To run the C++ executable, type `make run args=<filename>` where filename is either `empty` or `traffic`.
- On running the executable file, the original image appears in a window, choose the points that form the boundary of the required portion of the road. Points can be choosen in any order. `Ctrl+Left Mouse Button Click` resets the selection and points can be chosen again.
- Press any key to view the projected image and thereafter the cropped image.
- Size of the projected image is `1920x1080` and that of cropped image is `544x867` (irrespective of size of input image).
- To delete the C++ executable and the modified photographs, type `make clean`.


## Dynamic and queue density
- Create the C++ executable by typing 'make main2'.
- To run the C++ executable, type `make run args=<filename>` where filename is either `empty` or `traffic`.
- To delete the C++ executable and the modified photographs, type `make clean`.


## Trade-off analysis for software design choices 
- Compare runtime and error over various design and implementation choices such as frame-skipping, concurrency and resolution modification.

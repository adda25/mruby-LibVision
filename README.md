#LibVision
*LibVision* is an mruby gem for computer vision operations
on frames grabs from the Raspberry camera.

##Compile the C++ shared library 
```sh
cd lib; make
sudo make install
make clean
```
##Building mruby 
```sh
./run_test.rb
```

##Run from mruby
In order to work on static image loaded from memory:
```ruby
lb = LibVision.new()
lb.set_value4key(["imagePath", *"yourAbsolutePathToImage"*])
lb.execute(["loadImageFromMem", "preprocessingADPT", "detectSquares", "saveCandidates"])
lb.get_value4key(["polygonsFounds"])
```
or in order to use the Raspberry camera:

```ruby
lb = LibVision.new()
lb.execute(["openCamera","acquireFrame", "preprocessingADPT", "detectCircles", "holdOnlyRightColored", "saveCandidates"])
lb.get_value4key(["polygonsFounds"])
```

type *lb.execute(["printMethods"])* for a list of the available functions.

##Dependencies
This gem require OpenCV and the Raspicam library from: http://www.uco.es/investiga/grupos/ava/node/40
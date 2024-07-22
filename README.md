# NeuVC
Neural based vst plugin for VoiceConversion. 
Supports RVC and RVC2 trained models in pytorch .pth (python) and .pt (c++ traced) formats. 

# Video Presentation 

[![IMAGE ALT TEXT HERE](https://img.youtube.com/vi/bhtOwvWZOy0/0.jpg)](https://www.youtube.com/watch?v=bhtOwvWZOy0)



# build with XCode

put JUCE folder on the Thirdparty folder, then run
```
$ mkdir build 
$ cd build
```
then create an Xcode project with 
```
$ make -G "Xcode" ./build
```

you can launch the .XcodeProj and build it yourselg

# Install and run 

You can download the pre-built MacOS-Arm release on the releases page. 

# Python setup 
If you want to use the python mode to directly convert with .pth RVC models you should 
- Install python setting up a conda/miniconda version 3.0+
- Install using pip the requirements in requirements.txt 
`conda activate base`
`conda install pip¡
move to cloned NeuVC directory and launch 
`pip install -r requirements.txt`

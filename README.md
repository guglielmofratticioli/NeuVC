# NeuVC
Neural based vst plugin for VoiceConversion 

# build 
$ make -G "Xcode" ./build
# install 
put JUCE folder on the Thirdparty folder, then run
```
$ mkdir build 
$ cd build 
$ cmake .. 
```

## important (Env setup) 

you need to clone the RVC repository -> [https://github.com/RVC-Project/Retrieval-based-Voice-Conversion-WebUI], with pretrained models downloaded, in NeuVC working directory: 
- MacOS -> '' ..user_name/Library/NeuVC/ ''

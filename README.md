# RedBack's Video/Audio Telemetry Server  

-----------------------------------------------------------
## Introduction:

- A Telemetry server built in C++ to broadcast video/audio feed from the new RB20 formula one racing car to clients connected
on the browser. The source code uses multiple thirdparty technologies and libraries such as [Google's WebRTC](https://webrtc.org/) as well
as a networking library that I have developed to allow for event based network programming using [Boost](https://www.boost.org/) websockets which can be found
[here](https://github.com/Yousif-CS/eventsocketcpp). 

-----------------------------------------------------------
## To build and run:

1) run the BUILD.sh script, this will ensure that the required libraries and api's are installed
   as well as build the library for it 

2) You can open the root repository with vscode:
	- To generate the required build files: ALT + t then choose "Run tasks", after that choose the build type: Debug/Release, or
	
	- To run the code: CTRL + SHIFT + B

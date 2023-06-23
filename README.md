# Spatial Transcriptome Visualizer

## To-do

- ~~Support universal binary for Mac (x64 and arm64)~~
- Port to Windows and Linux
- Sign the app

## How to install

Please download the program from the release page to the right. 

The first time running the program might fail due to macOS security measures. You may manually allow the program to run by going to the System Settings -> Privacy and Security -> Security and click on Allow. 

## How to compile

First make sure you have the following tools installed. We recommend that you use the same version we specified. 

- macOS 13.4 with the latest Xcode Command Line Tools
- clang 14.0.3
- make 3.81
- node.js 20.2.0

Clone the repository to wherever you prefer on your computer and compile it with the following command. 

```
git clone -b yin https://github.com/amcilraithwustl/st-visualizer.git 
cd st-visualizer
make
```

Alternatively, you can try to compile with cmake. 

## How to use

~~Why don't you ask the Magic Conch, Squidward?~~
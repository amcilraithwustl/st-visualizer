#!/bin/zsh

echo $(uname -a) >> time.txt

{time ./bin/st-visualizer 0 0; } 2>> time.txt

{time ./bin/st-visualizer 0 0; } 2>> time.txt

echo -en '\n' >> time.txt
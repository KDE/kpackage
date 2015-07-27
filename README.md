# Package Framework

Installation and loading of additional content (ex: scripts, images...) as packages

## Introduction
The Package framework lets the user install and load packages of non binary content such as scripted extensions or graphic assets, as if they were traditional plugins.

## Note for packagers
After a package gets installed, its index should be regenerated. Index files CANNOT be shipped directly in packages. However they should be regenerated as post install script for the package itself.
In the cmake build directory of applications with packages there will be the file regenerateindex.sh that will contain all the commands that have to be executed in post install scripts to regenerate the indexes.


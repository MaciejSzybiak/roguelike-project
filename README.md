# Introduction

[![Build Status](https://travis-ci.com/MaciejSzybiak/roguelike-project.svg?branch=master)](https://travis-ci.com/MaciejSzybiak/roguelike-project)

This project is a simple turn-based roguelike game.
Its goal is to provide the basic functionality of this game type while keeping it as simple as possible.

![game screenshot](docs/images/screenshot1.png)

The game uses [freeglut library](http://freeglut.sourceforge.net/) for simplified OpenGL implementation and
stb_image.h file from [stb library](https://github.com/nothings/stb) for PNG file support.

## Compiling

#### Windows
The code includes a VS2017 project file which can be used to easily compile a Windows executable. All required dependencies are included
in this repository.

#### Linux
A Linux executable can be created using the provided Makefile.

Before compiling install the freeglut library using *apt-get install freeglut3-dev* command.

## Running the game
After building the game using Visual Studio or the Makefile you should have all required files set up correctly in the *bin/* directory.

If you are running the game on **Linux** make sure the *freeglut3* package is installed on your system.

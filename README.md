Apoapsis
========

A game about the solitude and survival in a high-tech lump 400 thousand meters
above the earth.

The project is still very WIP!

This repository contains the engine and the core package.
... which is pretty useless for itself.  Take a look at the example package
at https://github.com/henry4k/apoapsis-example to see how it can be used.


## Dependencies

Runtime dependencies:

- PhysFS 2.0+
- Lua 5.2
- Open GL
- GLFW 3
- Open AL
- ALURE
- Bullet


Compile time depencencies:

- GLM
- Python 3 (needed by flextGL)
- Python 3 package wheezy.template (needed by flextGL)


Ideally install pkg-config and use one of the provided sample
tup.config files.


## Licence and copyright

Copyright © Henry Kielmann

Apoapsis is licensed under the MIT license, which can be found in the
`LICENCE` file.

The project comes bundled with a number of dependencies, each with its own licence.
See their respective readme and licence files in the `third-party` folder
for details.

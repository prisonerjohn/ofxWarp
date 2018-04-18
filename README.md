## ofxWarp
An openFrameworks port of the [Cinder-Warping](https://github.com/paulhoux/Cinder-Warping) block by @paulhoux.
Enables you to easily create editable bi-linear and perspective warps, or a combination of the two.

[![Build status](https://travis-ci.org/prisonerjohn/ofxWarp.svg?branch=master)](https://travis-ci.org/prisonerjohn/ofxWarp)
[![Build status](https://ci.appveyor.com/api/projects/status/2f3a7456lo33gt9c/branch/master?svg=true)](https://ci.appveyor.com/project/prisonerjohn/ofxwarp/branch/master)

#### From the [Cinder-Warping](https://github.com/paulhoux/Cinder-Warping) README:

Image warping, as used in this addon, is the process of manipulating an image so that it can be projected onto flat or curved surfaces without distortion. There are three types of warps available:
* **Perspective warp**: performs image correction for projecting onto flat surfaces in case the projector is not horizontally and/or vertically aligned with the wall. Use it to exactly project your content on a rectangular area on a wall or building, regardless of how your projector is setup. For best results, disable any keystone correction on the projector. Then simply drag the four corners of your content where you want them to be on the wall.
* **Bilinear warp**: inferior to perspective warping on flat surfaces, but allows projecting onto curved surfaces. Simply add control points to the warp and drag them to where you want your content to be. 
* **Perspective-bilinear warp**: a combination of both techniques, where you first drag the four corners of your content to the desired location on the wall, then adjust for curvatures using the additional control points. If you (accidentally) move your projector later, all you need to do is adjust the four corners and the projection should perfectly fit on the curved wall again.
 
#### Installation

* Drop the addon folder into your `openFrameworks/addons` directory, and add to project as you would any other addon.
* Copy the shaders found in the example to your project's `bin/data/shaders/ofxWarp` folder.

#### Compatibility

* openFrameworks 0.9 and up
* OpenGL 3 and up (programmable pipeline)
* The included shaders only work with normalized textures (`GL_TEXTURE_2D`) but can be easily modified to work with rectangle textures

#### Controls
You can use `ofxWarp::Controller` to adjust your warps:
* `w` to toggle editing on all warps
* Use mouse or cursor keys to move the currently selected control point
* `TAB` to select the next control point
* `-` or `+` to change brightness
* `r` to reset the warp to its default settings
* `F11` to flip content horizontally
* `F12` to flip content vertically

For Perspective warps only:
* `F9` to rotate content counter-clockwise
* `F10` to rotate content clockwise

For Bilinear warps only:
* `m` to toggle between linear and curved mapping
* `F1` to reduce the number of horizontal control points
* `F2` to increase the number of horizontal control points
* `F3` to reduce the number of vertical control points
* `F4` to increase the number of vertical control points
* `F5` to decrease the mesh resolution
* `F6` to increase the mesh resolution
* `F7` to toggle adaptive mesh resolution

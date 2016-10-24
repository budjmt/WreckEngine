# WreckEngine

Originally conceived to support a wrecking-ball crane game, WreckEngine has more-or-less become 
my engine implementation playground. Someday that game will be the demo.

General Goals:
- Explore collision physics to the best of my ability
- Implement interesting functionality found in other engines while trying to streamline it
- Take advantage of the modern C++ toolkit
- Learn advanced graphics programming concepts, as needed
- Make the engine simple to use and reuse, so it might actually be taken advantage of

Stuff I've Done:
- Arbitrary Mesh Colliders
- Transform system with hierarchy and as-needed, on-demand computation
- Memory control using smart pointers
- Safe, low-impact CPU-side wrappers for GPU-side OpenGL constructs (gl_structs.h)
- Graphical debug primitives, using streamed buffer data and instanced rendering
- Properties (a la C#, property.h)
- Dynamic Event System

Stuff I Want to Do:
- Physically accurate collision resolution (in progress!!)
- Constraints
- Flexible Material System
- Better Lighing System
- Lua JIT scripting system, allowing for dynamic rebuilds
- Terrain system

Stuff I Might Want to Do:
- Component System?
- More advanced rendering techniques?
	- Sorting non-opaque objects to enable accurate alpha blending
	- Deferred Rendering
	- PBR
- Fluid Dynamics/Soft-body Physics?
- Scene Loader/Editor (stretch)
- Make it more cross-platform

...

- Make the wrecking-ball demo

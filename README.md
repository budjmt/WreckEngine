# WreckEngine

Originally conceived to support a wrecking-ball crane game, WreckEngine has more-or-less become 
an engine implementation playground. Someday that game will be the demo.

General Goals:
- Explore collision physics
- Implement/attempt to streamline interesting functionality found in other engines
- Take advantage of the modern C++ toolkit
- Learn advanced graphics programming concepts
- Make the engine simple to use and reuse

Stuff I've Done:
- Arbitrary Mesh Colliders
- Transform system with hierarchy and as-needed, on-demand computation
- Memory control using smart pointers
- Safe, low-impact CPU-side wrappers for GPU-side OpenGL constructs (gl_structs.h)
- Graphical debug primitives, using streamed buffer data and instanced rendering
- Properties (a la C#, property.h)
- Dynamic Event System
- Multithreading the various update loops
- Flexible renderer that can manage post-process chains, render groups, and full render pass chains
- Dynamic lighting, supporting deferred and forward lighting

Stuff I Want to Do:
- Physically accurate collision resolution (in progress!!)
- Constraints
- Lua JIT scripting system, allowing for dynamic rebuilds
- Terrain system

Stuff I Might Want to Do:
- Component System?
- More advanced rendering techniques?
	- Sorting non-opaque objects to enable accurate alpha blending
	- Visibility Buffer
	- PBR
- Fluid Dynamics/Soft-body Physics?
- Scene Loader/Editor (stretch)
- Make it more cross-platform

...

- Make the wrecking-ball demo

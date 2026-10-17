# fr-030: candytron in the browser

[Farbrausch](https://www.farbrausch.de/)'s 64k intro *candytron* (2nd at
Breakpoint 2003; this is the final version), built from the original source code to WebAssembly and
WebGL 2. Watch it at **https://kampvogn.dk/demos/candytron/**.

This is a fork of [farbrausch/fr_public](https://github.com/farbrausch/fr_public),
where the source of the intro's engine (GenThree) and synthesizer (V2) was
published in 2012. The intro is generated at start-up exactly as in 2003:
textures, meshes and the scene come from the same 74 KB script and 13 KB mesh
(packed with the code into a 64 KB executable back then), and the music is
synthesized while it plays by V2 from a 39 KB song file.

## Building

```sh
git clone --recursive https://github.com/kampvogn/candytron-web
cd candytron-web
web/build.sh             # needs Docker; uses emscripten/emsdk:4.0.10
```

Serve `web/build/` with any web server and open `index.html`.

## What was changed

The player was written for 32-bit Windows, Direct3D 9 and MSVC; the changes
are kept small and marked `kampvogn.dk`.

- **Direct3D 9** comes from [d3d9-webgl](https://github.com/LostMyCode/d3d9-webgl)
  (submodule `web/d3d9-webgl`, branch `candytron` of
  [our fork](https://github.com/kampvogn/d3d9-webgl)). The intro needed a
  complete fixed-function pipeline there: generated shaders with up to four
  texture stages and all their operations, directional, point and spot lights,
  texture coordinate generation and texture transforms, plus render targets
  that sample and are rendered to like on Direct3D.
- **Inline assembly** (x87, MMX) is rewritten in C: the bitmap operators in
  `genbitmap.cpp`, the FPU-mode dependent rounding in `_types.hpp`
  (`sFtol` rounds down, as the player sets the FPU to), V2's player timing.
- **The script runtime** called operators with a dword array put on the stack;
  WebAssembly checks call signatures, so each operator gets a typed thunk
  (`scriptthunk.hpp`).
- **x86 stack layout**: several operators read past one parameter into the
  next (`InitSRT(&s.x)` reads s, r and t) or walk varargs on the stack
  (`RecAddScale`, `sSPrintF`). These now build the arrays they need.
- **Music**: the 2003 song file is converted once to the current V2M format
  (`web/tools/convv2m.cpp`, using fr_public's `v2mconv`) and played by V2's
  portable `synth_core.cpp` through SDL audio.
- **Fonts**: the GDI text calls draw with a 2D canvas instead.
- **Platform**: `_startintro.cpp` gets a browser main loop (the audio clock
  drives the timing, as in the original), start-up progress and the canvas.

## Licenses

GenThree and V2 are under the BSD license (`genthree/LICENSE.txt`,
`v2/LICENSE.txt`); the intro's data (`genthree/data`) is CC-BY 3.0 by
Farbrausch; d3d9-webgl is MIT. Credits for candytron: chaos (code), ryg
(code), gizmo (graphics), kb (music).

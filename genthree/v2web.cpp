// This file is distributed under a BSD license. See LICENSE.txt for details.
// kampvogn.dk: V2 for the web player.
//
// The x86 intro plays its V2M with the 2003 player (_viruz2.cpp) on the V2
// assembler synth (_viruz2a.asm). fr_public has the C++ port of the synth
// (v2/synth_core.cpp, bugs included) and the matching newer player
// (v2/v2mplayer.cpp); the tune is converted to that V2M version once at build
// time (tools/convv2m). This file uses only V2's own headers (its types clash
// with GenThree's) and offers two C functions to viruz2web.cpp.

#include <string.h>
#include "v2mplayer.cpp"   // the synth itself is in v2synth.cpp (synth.h and libv2.h clash)

static V2MPlayer Player;

extern "C" void __stdcall OutputDebugStringA(const char *) {}

extern "C" int v2web_init(const void *v2m)
{
  Player.Init();
  if (!Player.Open(v2m, 44100))
    return 0;
  Player.Play();
  return 1;
}

extern "C" void v2web_render(float *buf, int samples)
{
  Player.Render(buf, samples);
}

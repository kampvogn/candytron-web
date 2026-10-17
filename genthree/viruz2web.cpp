// This file is distributed under a BSD license. See LICENSE.txt for details.
// kampvogn.dk: sViruz2 (the intro's music player) for the web build, on top
// of v2web.cpp; Render() converts to 16 bits exactly as _viruz2.cpp does.
#include "_viruz2.hpp"

extern "C" int v2web_init(const void *v2m);
extern "C" void v2web_render(float *buf, int samples);

sBool sViruz2::Init(sInt songnr)
{
  Buffer = new sF32[16384];
  return v2web_init(Stream) != 0;
}

sInt sViruz2::Render(sS16 *d,sInt samples)
{
  sInt result;
  sInt count;
  sInt i;

  result = 0;
  while(samples>0)
  {
    count = sMin(samples,4096);
    v2web_render(Buffer,count);
    for(i=0;i<count*2;i++)
      *d++ = sFtol(sRange<sF32>(Buffer[i],1,-1)*0x7fff);
    samples-=count;
    result += count;
  }
  return result;
}

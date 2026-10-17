// Convert an old V2M (e.g. Candytron's 2003 josie.v2m) to the newest V2M version
// with fr_public's v2mconv, so synth_core.cpp / v2mplayer.cpp can play it.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "v2mconv.h"
#include "sounddef.h"

// v2mconv.cpp needs these from sounddef.cpp's sdInit(); built the same way
// here, so the converter does not need sounddef.cpp (Win32 file handling)
int v2version;
int *v2vsizes;
int *v2gsizes;
static void InitVersions()
{
  v2version = 0;
  for (int i = 0; i < v2nparms; i++) if (v2parms[i].version > v2version) v2version = v2parms[i].version;
  for (int i = 0; i < v2ngparms; i++) if (v2gparms[i].version > v2version) v2version = v2gparms[i].version;
  v2vsizes = new int[v2version + 1]; v2gsizes = new int[v2version + 1];
  memset(v2vsizes, 0, (v2version + 1) * sizeof(int)); memset(v2gsizes, 0, (v2version + 1) * sizeof(int));
  for (int i = 0; i < v2nparms; i++) for (int j = v2version; j >= v2parms[i].version; j--) v2vsizes[j]++;
  for (int i = 0; i < v2ngparms; i++) for (int j = v2version; j >= v2gparms[i].version; j--) v2gsizes[j]++;
  for (int i = 0; i <= v2version; i++) v2vsizes[i] += 1 + 255 * 3;
}
int main(int argc, char **argv)
{
  InitVersions();
  if (argc != 3) { fprintf(stderr, "usage: convv2m in.v2m out.v2m\n"); return 1; }
  FILE *f = fopen(argv[1], "rb");
  if (!f) { perror(argv[1]); return 1; }
  fseek(f, 0, SEEK_END); long n = ftell(f); fseek(f, 0, SEEK_SET);
  unsigned char *in = (unsigned char *)malloc(n);
  fread(in, 1, n, f); fclose(f);
  int v = CheckV2MVersion(in, (int)n);
  printf("version delta %d\n", v);
  if (v < 0) { fprintf(stderr, "not a V2M I can convert (%s)\n", v2mconv_errors[-v]); return 1; }
  unsigned char *out = 0; int outlen = 0;
  ConvertV2M(in, (int)n, &out, &outlen);
  if (!out) { fprintf(stderr, "conversion failed\n"); return 1; }
  f = fopen(argv[2], "wb"); fwrite(out, 1, outlen, f); fclose(f);
  printf("%ld -> %d bytes\n", n, outlen);
  return 0;
}

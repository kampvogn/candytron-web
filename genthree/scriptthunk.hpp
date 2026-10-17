// This file is distributed under a BSD license. See LICENSE.txt for details.
// kampvogn.dk: typed calls for the script runtime under WebAssembly.
//
// The x86 player calls every script function through CallCode() in
// cslrt.cpp: it copies `count` dwords (objects, then ints; floats and small
// structs such as sF323 as their raw bits) onto the stack and calls the
// pointer. WebAssembly checks the exact signature of indirect calls, so each
// function in ScriptFunctions[] gets a thunk instead: it takes the dword array
// and reads the real parameters from it in order, each sizeof(T) bytes rounded
// up to whole dwords, then calls the function with the right types.
#ifndef __SCRIPTTHUNK_HPP__
#define __SCRIPTTHUNK_HPP__

#ifdef __EMSCRIPTEN__
#include <string.h>
#include <tuple>
#include <type_traits>

typedef sInt (*ScriptThunkFunc)(const sInt *para);

// Note: the operators may not read past one parameter into the next (as
// InitSRT(&s.x) did with s, r and t on the x86 stack); those places build
// the array they need themselves.
template<typename T> struct ScriptThunkArg
{
  static T Get(const sInt *&p)
  {
    T v;
    memcpy(&v,p,sizeof(T));
    p += (sizeof(T)+3)/4;
    return v;
  }
};

template<typename F, F f> struct ScriptThunk;

template<typename R, typename... A, R (*f)(A...)> struct ScriptThunk<R (*)(A...), f>
{
  static sInt Call(const sInt *p)
  {
    // a braced list is evaluated left to right, so the reads stay in order
    std::tuple<A...> args{ScriptThunkArg<A>::Get(p)...};
    return Result(args, std::is_void<R>());
  }
  static sInt Result(std::tuple<A...> &args, std::true_type)
  {
    std::apply(f, args);
    return 0;
  }
  static sInt Result(std::tuple<A...> &args, std::false_type)
  {
    R r = std::apply(f, args);
    sInt i = 0;
    memcpy(&i, &r, sizeof(r) < sizeof(i) ? sizeof(r) : sizeof(i));
    return i;
  }
};

#define SCRIPT_FN(f) ((void *)&ScriptThunk<decltype(&f), &f>::Call)
#else
#define SCRIPT_FN(f) ((void *)f)
#endif

#endif

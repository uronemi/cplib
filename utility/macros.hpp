#pragma once
#include "Template.hpp"

#define INT(...) int __VA_ARGS__; input(__VA_ARGS__)
#define LL(...) long long __VA_ARGS__; input(__VA_ARGS__)
#define UINT(...) unsigned int __VA_ARGS__; input(__VA_ARGS__)
#define ULL(...) unsigned long long __VA_ARGS__; input(__VA_ARGS__)
#define MINT(m, ...) Modint<m> __VA_ARGS__; input(__VA_ARGS__)
#define MINT998(...) Mint998 __VA_ARGS__; input(__VA_ARGS__)
#define STR(...) std::string __VA_ARGS__; input(__VA_ARGS__)
#define CHR(...) char __VA_ARGS__; input(__VA_ARGS__)
#define FRT(...) float __VA_ARGS__; input(__VA_ARGS__)
#define DBL(...) double __VA_ARGS__; input(__VA_ARGS__)
#define LD(...) long double __VA_ARGS__; input(__VA_ARGS__)
#define VEC3(type, name, N) std::vector<type> name(N); input(name)
#define VEC4(type, name, N, l) std::vector<type> name(N); for (std::size_t i = (l); i < (N); ++i) input(name[i])
#define VEC5(type, name, N, l, r) std::vector<type> name(N); for (std::size_t i = (l); i < (r); ++i) input(name[i]);
#define GET_VEC_MACRO(_1, _2, _3, _4, _5, NAME, ...) NAME
#define VEC(...) GET_VEC_MACRO(__VA_ARGS__, VEC5, VEC4, VEC3)(__VA_ARGS__)
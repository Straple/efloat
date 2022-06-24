#pragma once

#include <random>
using namespace std;
using ld = long double;

mt19937_64 rnd(42);

// [0, 1]
ld get_urnd() {
	return (ld)rnd() / ULLONG_MAX;
}

// [-1, 1]
ld get_rnd() {
	return ((ld)rnd() - LLONG_MAX) / ULLONG_MAX * 2;
}

// [left, right]
ld get_rnd_range(ld left, ld right) {
	ld len = right - left;
	return left + len * get_urnd();
}
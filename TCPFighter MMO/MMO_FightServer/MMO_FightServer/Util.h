#pragma once

inline
	int rangeRand(int start, int end)
{
	return ((rand() % (end - start)) + start);
}

inline
	int align(int target, int boundary)
{
	return (target + (boundary - 1)) & ~(boundary - 1);
}

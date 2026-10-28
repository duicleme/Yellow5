#ifndef __GRID_H__
#define __GRID_H__

#include <d3dx9.h>


struct LineVertex
{
	float x, y, z;
	DWORD color;

	static const DWORD FVF =
		D3DFVF_XYZ | D3DFVF_DIFFUSE;
};



void DrawGrid(IDirect3DDevice9* Device);

#endif

#ifndef __WARP_H__
#define __WARP_H__

#include <d3dx9.h>
#include <d3dx9shape.h>
#include "Planet.h"




class Warp
{

	struct LineVertex
	{
		float x, y, z;
		DWORD color;

		static const DWORD FVF =
			D3DFVF_XYZ | D3DFVF_DIFFUSE;
	};

	
public:
	Warp();
	
	virtual ~Warp();

public:

	ID3DXMesh* SunLineMesh;
	HRESULT Init(IDirect3DDevice9* Device);

	void DrawSunLine(IDirect3DDevice9* Device,
		Planet& sun1, Planet& sun2);

};




#endif
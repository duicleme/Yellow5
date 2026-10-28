#ifndef __PLANET_H__
#define __PLANET_H__

#include <Windows.h>
#include <d3dx9.h>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "sqlite/sqlite3.h"

class Planet
{
public:

	struct Vertex
	{
		float _x, _y, _z;
		DWORD _color;

		Vertex()
			: _x(0.0f), _y(0.0f), _z(0.0f),
			_color(0)
		{}

		Vertex(float x, float y, float z, DWORD color)
			: _x(x), _y(y), _z(z), _color(color)
		{}

		static constexpr DWORD FVF =
			D3DFVF_XYZ | D3DFVF_DIFFUSE;
	};

private:

	IDirect3DVertexBuffer9* VB;
	IDirect3DIndexBuffer9* IB;

public:

	int id;
	std::string name;

	// 공전 중심
	float cx, cy, cz;

	// 현재 위치
	float x, y, z;

	// 자전 및 공전 각도
	float selfAngle;
	float orbitAngleY;
	float orbitAngleX;

	float orbitRadius;
	float tiltX;

	float orbitSpeed;
	

	bool stopped;
	bool positionInitialized;

	
public:
	D3DXMATRIX World;

public:

	Planet();
	virtual ~Planet();

	bool InitDatabase(sqlite3* db);

	void SavePlanet(sqlite3* db, const char* name);
	void LoadPlanet(sqlite3* db, const char* name);

	void StopPlanet(sqlite3* db, const char* name);
	void StartPlanet(sqlite3* db, const char* name);

	void Init(IDirect3DDevice9* Device);

	void UpdatePlanet(
		float timeDelta,
		float scale,
		float tx,
		float ty,
		float tz,
		float orbitRadius,
		float tiltX,
		float orbitSpeed
	);

	void DrawPlanet(IDirect3DDevice9* Device);


	void PrintPosition(const char* message);
};

#endif
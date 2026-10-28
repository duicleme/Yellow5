#ifndef __PLANETMESH_H__
#define __PLANETMESH_H__


#include <Windows.h>
#include <d3dx9.h>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <iostream>

#include "sqlite/sqlite3.h"

class PlanetMesh
{
public:
	ID3DXMesh* Object;

public:
	int id;
	std::string name;

	float cx, cy, cz;
	float x, y, z;

	float selfAngle;
	float orbitAngleY;
	float orbitAngleX;         // 이것 사용하는지 모름..

	float orbitRadius;
	float tiltX;

	float orbitSpeed;

	bool stopped;
	bool positionInitialized;

public:
	D3DXMATRIX World;

public:
	PlanetMesh();
	virtual ~PlanetMesh();

public:
	bool InitDatabase(sqlite3* db);

	void SavePlanet(sqlite3* db, const char* name);
	void LoadPlanet(sqlite3* db, const char* name);

	void StopPlanet(sqlite3* db, const char* name);
	void StartPlanet(sqlite3* db, const char* name);


public:

	void Init(IDirect3DDevice9* Device, float Width, float Height, float Depth);

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

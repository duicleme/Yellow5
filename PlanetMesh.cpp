#include "PlanetMesh.h"

using namespace std;


PlanetMesh::PlanetMesh()
{
	Object = nullptr;

	cx = cy = cz = 0.0f;
	x = y = z = 0.0f;

	selfAngle = 0.0f;

	orbitAngleY = 0.0f;    // 이 2개는 사용하는지 모르겠음.
	orbitAngleX = 0.0f;   

	orbitRadius = 0.0f;
	tiltX = 0.0f;

	orbitSpeed = 0.0f;

	stopped = false;
	positionInitialized = false;

	D3DXMatrixIdentity(&World);

}

PlanetMesh::~PlanetMesh()
{
	if (Object) {
		Object->Release();
		Object = nullptr;
	}
}


bool PlanetMesh::InitDatabase(sqlite3* db)
{
	if (!db)
		return false;

	const char* sql =
		"CREATE TABLE IF NOT EXISTS Planets ("
		"ID INTEGER PRIMARY KEY AUTOINCREMENT, "
		"NAME TEXT NOT NULL UNIQUE, "
		"CX REAL, "
		"CY REAL, "
		"CZ REAL, "
		"X REAL, "
		"Y REAL, "
		"Z REAL, "
		"SELFANGLE REAL, "
		"ORBITANGLEY REAL,"
		"ORBITANGLEX REAL, "
		"ORBITRADIUS REAL, "
		"TILTX REAL, "
		"ORBITSPEED REAL "
		");";

	char* errMsg = nullptr;

	int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);

	if (rc != SQLITE_OK)
	{
		printf("CREATE Error: %s\n",
			errMsg ? errMsg : sqlite3_errmsg(db));

		sqlite3_free(errMsg);
		return false;
	}

	return true;


}




void PlanetMesh::SavePlanet(sqlite3* db, const char* name)
{
	if (!db || !name)
		return;

	this->name = name;

	const char* sql =
		"INSERT INTO Planets "
		"(NAME, CX, CY, CZ, X, Y, Z, SELFANGLE, "
		"ORBITANGLEY, ORBITANGLEX, "
		"ORBITRADIUS, TILTX, ORBITSPEED ) "
		"VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?) "
		"ON CONFLICT(NAME) DO UPDATE SET "
		"CX=excluded.CX, CY=excluded.CY, CZ=excluded.CZ, "
		"X=excluded.X, Y=excluded.Y, Z=excluded.Z, "
		"SELFANGLE=excluded.SELFANGLE, "
		"ORBITANGLEY=excluded.ORBITANGLEY, "
		"ORBITANGLEX=excluded.ORBITANGLEX, "
		"ORBITRADIUS=excluded.ORBITRADIUS, "
		"TILTX=excluded.TILTX, "
		"ORBITSPEED=excluded.ORBITSPEED;";

	sqlite3_stmt* stmt = nullptr;

	int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

	if (rc != SQLITE_OK)
	{
		printf("SAVE PREPARE ERROR: %s\n", sqlite3_errmsg(db));
		return;
	}

	sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
	sqlite3_bind_double(stmt, 2, cx);
	sqlite3_bind_double(stmt, 3, cy);
	sqlite3_bind_double(stmt, 4, cz);
	sqlite3_bind_double(stmt, 5, x);
	sqlite3_bind_double(stmt, 6, y);
	sqlite3_bind_double(stmt, 7, z);
	sqlite3_bind_double(stmt, 8, selfAngle);
	sqlite3_bind_double(stmt, 9, orbitAngleY);
	sqlite3_bind_double(stmt, 10, orbitAngleX);
	sqlite3_bind_double(stmt, 11, orbitRadius);
	sqlite3_bind_double(stmt, 12, tiltX);
	sqlite3_bind_double(stmt, 13, orbitSpeed);

	rc = sqlite3_step(stmt);

	if (rc != SQLITE_DONE)
		printf("SAVE ERROR: %s\n", sqlite3_errmsg(db));
	else
		printf("SAVE OK: %s\n", name);

	sqlite3_finalize(stmt);
}



void PlanetMesh::LoadPlanet(sqlite3* db, const char* name)
{
	char sqlSelect[1024];

	sprintf_s(sqlSelect, sizeof(sqlSelect),
		"SELECT ID, NAME, CX, CY, CZ, X, Y, Z, "
		"SELFANGLE, ORBITANGLEY, ORBITANGLEX, "
		"ORBITRADIUS, TILTX, ORBITSPEED "
		"FROM Planets WHERE NAME = '%s';",
		name
	);

	auto callback = [](void* data, int argc,
		char** argv, char** azColName) -> int
		{
			PlanetMesh* p = (PlanetMesh*)data;

			p->id = argv[0] ? atoi(argv[0]) : 0;
			p->name = argv[1] ? argv[1] : "";

			p->cx = argv[2] ? (float)atof(argv[2]) : 0.0f;
			p->cy = argv[3] ? (float)atof(argv[3]) : 0.0f;
			p->cz = argv[4] ? (float)atof(argv[4]) : 0.0f;

			p->x = argv[5] ? (float)atof(argv[5]) : 0.0f;
			p->y = argv[6] ? (float)atof(argv[6]) : 0.0f;
			p->z = argv[7] ? (float)atof(argv[7]) : 0.0f;

			p->selfAngle = argv[8] ? (float)atof(argv[8]) : 0.0f;
			p->orbitAngleY = argv[9] ? (float)atof(argv[9]) : 0.0f;
			p->orbitAngleX = argv[10] ? (float)atof(argv[10]) : 0.0f;

			p->orbitRadius = argv[11] ? (float)atof(argv[11]) : 0.0f;
			p->tiltX = argv[12] ? (float)atof(argv[12]) : 0.0f;
			p->orbitSpeed = argv[13] ? (float)atof(argv[13]) : 0.0f;


			// 저장된 공전 중심 좌표 복원 완료
			p->positionInitialized = true;

			return 0;
		};

	char* errMsg = nullptr;

	int rc = sqlite3_exec(db, sqlSelect, callback, this, &errMsg);

	if (rc != SQLITE_OK)
	{
		printf("SELECT Error: %s\n",
			errMsg ? errMsg : sqlite3_errmsg(db));

		sqlite3_free(errMsg);
	}
}




void PlanetMesh::StartPlanet(sqlite3* db, const char* name)
{
	LoadPlanet(db, name);
	stopped = false;
}

void PlanetMesh::StopPlanet(sqlite3* db, const char* name)
{
	stopped = true;
	SavePlanet(db, name);
}


void PlanetMesh::Init(IDirect3DDevice9* Device, float Width, float Height, float Depth)
{

	//D3DXCreateBox(Device,Width,Height,Depth,  &Object, 0);
	D3DXCreateSphere(Device, Width, Height, Depth, &Object, 0);

}






void PlanetMesh::UpdatePlanet(
	float timeDelta,
	float scale,
	float tx,
	float ty,
	float tz,
	float orbitRadius,
	float tiltX,
	float orbitSpeed )
{
	D3DXMATRIX S;
	D3DXMATRIX RSelf;
	D3DXMATRIX TOrbit;
	D3DXMATRIX ROrbitY;
	D3DXMATRIX RTiltX;
	D3DXMATRIX TCenter;

	if (!positionInitialized)
	{
		
		this->orbitRadius = orbitRadius;
		this->tiltX = tiltX;

		this->orbitSpeed = orbitSpeed;

		positionInitialized = true;
	}

	cx = tx;
	cy = ty;
	cz = tz;

	

	if (!stopped)
	{
		selfAngle += timeDelta * 2.0f;
//		orbitAngleY += timeDelta * 0.5f;
		orbitAngleY += timeDelta * this->orbitSpeed;

		if (selfAngle >= D3DX_PI * 2.0f)
			selfAngle -= D3DX_PI * 2.0f;

		if (orbitAngleY >= D3DX_PI * 2.0f)
			orbitAngleY -= D3DX_PI * 2.0f;
	}

	D3DXMatrixScaling(&S, scale, scale, scale);
	D3DXMatrixRotationY(&RSelf, selfAngle);

	D3DXMatrixTranslation(
		&TOrbit, this->orbitRadius, 0.0f, 0.0f);

	D3DXMatrixRotationY(&ROrbitY, orbitAngleY);

	// 공전 후 궤도 전체를 고정 각도만큼 기울임
	D3DXMatrixRotationX(&RTiltX, this->tiltX);

	D3DXMatrixTranslation(&TCenter, cx, cy, cz);

	World = S * RSelf * TOrbit
		* ROrbitY * RTiltX * TCenter;

	x = World._41;
	y = World._42;
	z = World._43;
}


void PlanetMesh::DrawPlanet(IDirect3DDevice9* Device)
{
	if (!Device || !Object )
		return;

	Device->SetRenderState(D3DRS_LIGHTING, TRUE);
//	Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	Device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);

	D3DMATERIAL9 mtrl = {};
	mtrl.Diffuse = D3DXCOLOR(1, 0, 0, 1);
	mtrl.Ambient = mtrl.Diffuse;
	Device->SetMaterial(&mtrl);

	Device->SetRenderState(D3DRS_COLORVERTEX, FALSE);
	Device->SetRenderState(D3DRS_DIFFUSEMATERIALSOURCE, D3DMCS_MATERIAL);
	Device->SetRenderState(D3DRS_AMBIENTMATERIALSOURCE, D3DMCS_MATERIAL);


	// 정점 색상 또는 조명 결과를 다시 사용
	Device->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	Device->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	Device->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

	
	mtrl.Diffuse = D3DXCOLOR(0, 0, 1, 1);
	mtrl.Ambient = mtrl.Diffuse;
	Device->SetMaterial(&mtrl);


	Device->SetTransform(D3DTS_WORLD, &World);

	Object->DrawSubset(0);
}


void PlanetMesh::PrintPosition(const char* message)
{
	if (!stopped)
	{
		printf("%s : X=%7.2f Y=%7.2f Z=%7.2f\n", message, x, y, z);
		printf("cx=%7.2f cy=%7.2f cz=%7.2f sa=%7.2f oay=%7.2f oax=%7.2f\n", cx, cy, cz, selfAngle, orbitAngleY, orbitAngleX);
	}
}

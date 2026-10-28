#include "d3dUtility.h"

#include <stdio.h>
#include <Windows.h>
#include <iostream>
#include <cmath>

#include ".\sqlite\sqlite3.h"

#include "Planet.h"
#include "PlanetMesh.h"

#include "camera.h"
#include "Grid.h"
#include "Warp.h"

using namespace std;

IDirect3DDevice9* Device = 0;

const int Width		= 1920;
const int Height	= 1080;


Camera TheCamera(Camera::AIRCRAFT);


Planet Sun, Earth;
PlanetMesh Moon;

Planet Sun2;


sqlite3* db = nullptr;
char* errMessage = nullptr;

bool showPlanet = true;



Warp WarpLine;

bool showSunLine = false;
bool tKeyPressed = false;

bool cameraTravel = false;
float travelTime = 0.0f;
float travelDuration = 10.0f;  // 10초 동안 이동

D3DXVECTOR3 travelStart;
D3DXVECTOR3 travelEnd;

const float travelStartDistance = 10.0f;

LPD3DXMESH mesh = nullptr;
LPDIRECT3DTEXTURE9 wingTexture = nullptr;




void UpdateCameraTravel(float timeDelta)
{
	if (!cameraTravel)
		return;

	travelTime += timeDelta;

	float t = travelTime / travelDuration;

	if (t >= 1.0f)
		t = 1.0f;

	// Sun1 → Sun2 이동 방향
	D3DXVECTOR3 direction = travelEnd - travelStart;

	if (D3DXVec3LengthSq(&direction) < 0.000001f)
	{
		cameraTravel = false;
		return;
	}

	D3DXVec3Normalize(&direction, &direction);

	// Sun2에 너무 가까이 접근하지 않도록
	// 목표 위치를 Sun2보다 3만큼 앞에 설정
	D3DXVECTOR3 finalPosition =
		travelEnd - direction * 3.0f;

	// Sun1에서 최종 위치까지 이동
	D3DXVECTOR3 position =
		travelStart + (finalPosition - travelStart) * t;

	// 빨간 원기둥보다 2만큼 위쪽
	position.y += 2.0f;

	// 카메라가 이동 방향을 바라봄
	D3DXVECTOR3 target = position + direction * 10.0f;

	D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);

	// 카메라의 실제 위치와 방향 변경
	TheCamera.lookAt(&position, &target, &up);

	if (t >= 1.0f)
	{
		cameraTravel = false;
		showSunLine = false;  // 빨간 연결선 제거

		printf("Warp Completed!\n");
	}
}

bool Setup()
{


	if (sqlite3_open("test.db", &db) != SQLITE_OK)
	{
		printf("DB Open Error: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		db = nullptr;
	}
	
	
	Sun.InitDatabase(db);
	Earth.InitDatabase(db);
	
	Moon.InitDatabase(db);
	Sun2.InitDatabase(db);





	Device->SetRenderState(D3DRS_LIGHTING, true);				// LIGHTING FALSE.

	D3DXVECTOR3 direction(0.0f, -1.0f, 1.0f);

	D3DLIGHT9 light = {};
	light.Type = D3DLIGHT_DIRECTIONAL;
	light.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	light.Direction = direction;

	Device->SetLight(0, &light);
	Device->LightEnable(0, TRUE);

	Device->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(80, 80, 80));





	D3DXMATRIX proj;
	D3DXMatrixPerspectiveFovLH(
		&proj,
		D3DX_PI * 0.5f,					 
		(float)Width / (float)Height,	// aspect ratio
		1.0f,							// near
		1000.f);						// far 

	Device->SetTransform(D3DTS_PROJECTION, &proj);

	


	Sun.Init(Device);
	Earth.Init(Device);
	Moon.Init(Device,1.0f,10.0f,10.0f);
	
	Sun2.Init(Device);


	WarpLine.Init(Device);


	//===========================================================================
	
	// 비행기 메시 로드
	HRESULT hr = D3DXLoadMeshFromXA(
		"X-Wing.x",
		D3DXMESH_MANAGED,
		Device,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		&mesh
	);

	if (FAILED(hr))
		return false;

	// 비행기 텍스처 로드
	hr = D3DXCreateTextureFromFileA(
		Device,
		"X-Wing_BaseColor.png",
		&wingTexture
	);

	if (FAILED(hr))
		return false;


	//===================================================================================




	return true;
}

void Cleanup()
{

	
	if (wingTexture)
	{
		wingTexture->Release();
		wingTexture = nullptr;
	}

	if (mesh)
	{
		mesh->Release();
		mesh = nullptr;
	}






	if (db)
	{
		sqlite3_close(db);
		db = nullptr;
	}
}





bool Display(float timeDelta)
{
	if (Device)
	{

/*
		// 뷰 위치 고정
		D3DXVECTOR3 position(0.0f, 3.0f, -10.0f);
		// 뷰가 바라보는 위치
		D3DXVECTOR3 target(0.0f, 0.0f, 0.0f);
		// 뷰의 위쪽 방향 (Y축)
		D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);
		D3DXMATRIX V;
		D3DXMatrixLookAtLH(&V, &position, &target, &up);
		Device->SetTransform(D3DTS_VIEW, &V);
*/

		if (::GetAsyncKeyState('A') & 0x8000f) TheCamera.walk(4.0f * timeDelta);
		if (::GetAsyncKeyState('Z') & 0x8000f) TheCamera.walk(-4.0f * timeDelta);
		if (::GetAsyncKeyState(VK_UP) & 0x8000f) TheCamera.pitch(1.0f * timeDelta);
		if (::GetAsyncKeyState(VK_DOWN) & 0x8000f) TheCamera.pitch(-1.0f * timeDelta);
		if (::GetAsyncKeyState(VK_LEFT) & 0x8000f) TheCamera.roll(1.0f * timeDelta);
		if (::GetAsyncKeyState(VK_RIGHT) & 0x8000f) TheCamera.roll(-1.0f * timeDelta);



		bool tDown = (::GetAsyncKeyState('T') & 0x8000) != 0;

		if (tDown && !tKeyPressed && !cameraTravel)
		{
			// 현재 카메라 위치
			D3DXVECTOR3 cameraPos;
			TheCamera.getPosition(&cameraPos);

			// 첫 번째 태양 위치
			D3DXVECTOR3 sunPos(Sun.x, Sun.y, Sun.z);

			// 카메라와 태양 사이의 거리 계산
			D3DXVECTOR3 diff = cameraPos - sunPos;
			float distance = D3DXVec3Length(&diff);

			if (distance <= travelStartDistance)
			{
				// 가까이 있을 때만 여행 시작
				showSunLine = true;
				cameraTravel = true;
				travelTime = 0.0f;

				travelStart = sunPos;
				travelEnd = D3DXVECTOR3(Sun2.x, Sun2.y, Sun2.z);

				printf("Warp Start! Distance: %.2f\n", distance);
			}
			else
			{
				printf("Too far from Sun! Distance: %.2f\n", distance);
			}
		}

		tKeyPressed = tDown;

		

//		D3DXMATRIX V;
//		TheCamera.getViewMatrix(&V);
//		Device->SetTransform(D3DTS_VIEW, &V);

		D3DXMATRIX V;
		TheCamera.getViewMatrix(&V);
		Device->SetTransform(D3DTS_VIEW, &V);

		// 카메라 이동 중에는 여행용 View 사용
		UpdateCameraTravel(timeDelta);


		
		static float angle = (3.0f * D3DX_PI) / 2.0f;
		

		



		angle += timeDelta;

		if (angle >= D3DX_PI * 2.0f)
		{
			angle = fmodf(angle, D3DX_PI * 2.0f);
		}



		Device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0xffffffff, 1.0f, 0);
		Device->BeginScene();


		// 지평선 및 바닥 격자
		DrawGrid(Device);

		
		Sun.UpdatePlanet( timeDelta, 0.7f, 0.0f, 10.0f, 0.0f, 0.0f,0.0f,0.5f);
		Earth.UpdatePlanet(timeDelta, 0.2f, 0.0f, 10.0f, 0.0f, 5.0f, 0.0f,0.5f);
		Moon.UpdatePlanet(timeDelta, 0.1f, Earth.x, Earth.y, Earth.z, 2.0f, D3DXToRadian(30.0f),2.0f);

		Sun2.UpdatePlanet(timeDelta, 0.7f, -80.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f);

		if (showPlanet)
		{
			Sun.DrawPlanet(Device);
			Earth.DrawPlanet(Device);
			Moon.DrawPlanet(Device);

			Sun2.DrawPlanet(Device);
		}

		if (showSunLine)
		{
			WarpLine.DrawSunLine(Device, Sun, Sun2);
		}

		





		if (mesh && wingTexture)
		{
			D3DXMATRIX S, T, World;

			// 크기
			D3DXMatrixScaling(&S, 0.5f, 0.5f, 0.5f);

			// 위치
			D3DXMatrixTranslation(&T, 0.0f, 10.0f, 0.0f);

			World = S * T;

			Device->SetTransform(D3DTS_WORLD, &World);

			// 렌더링 설정
			Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
			Device->SetRenderState(D3DRS_LIGHTING, FALSE);

			Device->SetTextureStageState(
				0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

			Device->SetTextureStageState(
				0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

			Device->SetTextureStageState(
				0, D3DTSS_ALPHAOP, D3DTOP_DISABLE);

			Device->SetTexture(0, wingTexture);

			// 비행기 그리기
			mesh->DrawSubset(0);

			Device->SetTexture(0, nullptr);

			// 다른 행성 렌더링을 위한 상태 복구
			Device->SetTextureStageState(
				0, D3DTSS_COLOROP, D3DTOP_MODULATE);

			Device->SetTextureStageState(
				0, D3DTSS_COLORARG1, D3DTA_TEXTURE);

			Device->SetTextureStageState(
				0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

			Device->SetTextureStageState(
				0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

			Device->SetTextureStageState(
				0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

			Device->SetTextureStageState(
				0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

			Device->SetRenderState(D3DRS_LIGHTING, TRUE);
		}


		







		Device->EndScene();
		Device->Present(0, 0, 0, 0);

	}
	return true;
}

LRESULT CALLBACK d3d::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_DESTROY:
		::PostQuitMessage(0);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			::DestroyWindow(hwnd);

		if (wParam == 'S') {
			Sun.StopPlanet(db, "Sun");
			Earth.StopPlanet(db, "Earth");
			Moon.StopPlanet(db, "Moon");

			Sun2.StopPlanet(db, "Sun2");
		}
		if (wParam == 'L') {
			Sun.StartPlanet(db, "Sun");
			Earth.StartPlanet(db, "Earth");
			Moon.StartPlanet(db, "Moon");

			Sun2.StartPlanet(db, "Sun2");
		}

		if (wParam == 'V') showPlanet = !showPlanet;


		break;

	}
	return ::DefWindowProc(hwnd, msg, wParam, lParam);

}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR cmdLine, int showCmd)
{

	AllocConsole();

	FILE* fp = nullptr;
	freopen_s(&fp, "CONOUT$", "w", stdout);

	printf("Console Ready!\n");

	if (!d3d::InitD3D(hInstance, Width, Height, true, D3DDEVTYPE_HAL, &Device))
	{
		::MessageBox(0, "InitD3D() - FAILED", 0, 0);
		return 0;
	}

	if (!Setup())
	{
		::MessageBox(0, "Setup() - FAILED", 0, 0);
		return 0;
	}

	d3d::EnterMsgLoop(Display);

	Cleanup();

	Device->Release();

	return 0;
}
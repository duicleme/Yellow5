#include "Warp.h"



Warp::Warp()
{
	SunLineMesh = nullptr;
}

Warp::~Warp()
{
	if (SunLineMesh)
	{
		SunLineMesh->Release();
		SunLineMesh = nullptr;
	}
}

HRESULT Warp::Init(IDirect3DDevice9* Device)
{
	/*
	HRESULT hr = D3DXCreateCylinder(
		Device,        // 1. Direct3D 장치
		0.15f,         // 2. 아래쪽 반지름
		0.15f,         // 3. 위쪽 반지름
		1.0f,          // 4. 길이
		12,            // 5. 원 둘레 분할 수
		1,             // 6. 높이 분할 수
		&SunLineMesh,  // 7. 생성된 메시
		nullptr        // 8. 인접 정보
	);
	*/

	HRESULT hr = D3DXCreateCylinder(Device, 0.05f, 0.05f, 1.0f, 12, 1, &SunLineMesh, nullptr);

	return hr;
}


void Warp::DrawSunLine(
	IDirect3DDevice9* Device,
	Planet& sun1,
	Planet& sun2)
{
	if (!Device || !SunLineMesh)
		return;

	D3DXVECTOR3 start(sun1.x, sun1.y, sun1.z);
	D3DXVECTOR3 end(sun2.x, sun2.y, sun2.z);

	D3DXVECTOR3 direction = end - start;

	float length = D3DXVec3Length(&direction);

	if (length < 0.0001f)
		return;

	D3DXVec3Normalize(&direction, &direction);

	// 두 태양 사이의 중간 위치
	D3DXVECTOR3 center = (start + end) * 0.5f;

	// 원기둥의 기본 방향은 Z축
//	D3DXVECTOR3 zAxis(0.0f, 0.0f, 1.0f);

	D3DXVECTOR3 zAxis(0.0f, 0.0f, 1.0f);

	// Z축에서 두 태양을 연결하는 방향으로 회전
	D3DXQUATERNION rotation;

	D3DXVECTOR3 axis;
	D3DXVec3Cross(&axis, &zAxis, &direction);

	float dot = D3DXVec3Dot(&zAxis, &direction);

	if (dot > 1.0f) dot = 1.0f;
	if (dot < -1.0f) dot = -1.0f;

	if (D3DXVec3LengthSq(&axis) < 0.000001f)
	{
		if (dot >= 0.0f)
			D3DXQuaternionIdentity(&rotation);
		else
		{
			D3DXVECTOR3 xAxis(1.0f, 0.0f, 0.0f);

			D3DXQuaternionRotationAxis(
				&rotation,
				&xAxis,
				D3DX_PI
			);
		}
			
	}
	else
	{
		D3DXVec3Normalize(&axis, &axis);
		D3DXQuaternionRotationAxis(
			&rotation,
			&axis,
			acosf(dot)
		);
	}

	D3DXMATRIX S, R, T, World;

	// 원기둥 길이를 두 태양 사이의 거리로 설정
	D3DXMatrixScaling(&S, 1.0f, 1.0f, length);
	D3DXMatrixRotationQuaternion(&R, &rotation);
	D3DXMatrixTranslation(
		&T, center.x, center.y, center.z);

	World = S * R * T;

	// 기존 렌더링 상태 보관
	IDirect3DStateBlock9* state = nullptr;

	if (FAILED(Device->CreateStateBlock(
		D3DSBT_ALL, &state)))
		return;

	D3DXMATRIX oldWorld;
	Device->GetTransform(D3DTS_WORLD, &oldWorld);

	Device->SetTransform(D3DTS_WORLD, &World);

	// 빨간색 고정
	Device->SetRenderState(D3DRS_LIGHTING, FALSE);
	Device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	Device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	Device->SetRenderState(D3DRS_FOGENABLE, FALSE);

	Device->SetTexture(0, nullptr);

	Device->SetTextureStageState(
		0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);

	Device->SetTextureStageState(
		0, D3DTSS_COLORARG1, D3DTA_TFACTOR);

	Device->SetRenderState(
		D3DRS_TEXTUREFACTOR,
		D3DCOLOR_XRGB(255, 0, 0));

	// 빨간색 원기둥 출력
	SunLineMesh->DrawSubset(0);

	// 이전 상태 복원
	state->Apply();
	Device->SetTransform(D3DTS_WORLD, &oldWorld);
	state->Release();
}
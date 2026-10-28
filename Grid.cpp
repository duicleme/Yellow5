#include "Grid.h"



void DrawGrid(IDirect3DDevice9* Device)
{
	const int GRID_SIZE = 50;
	const float STEP = 1.0f;

	Device->SetTexture(0, NULL);
	Device->SetRenderState(D3DRS_LIGHTING, FALSE);
	Device->SetRenderState(D3DRS_ZENABLE, TRUE);

	Device->SetFVF(LineVertex::FVF);

	D3DXMATRIX I;
	D3DXMatrixIdentity(&I);
	Device->SetTransform(D3DTS_WORLD, &I);

	for (int i = -GRID_SIZE; i <= GRID_SIZE; i++)
	{
		float p = i * STEP;
		float extent = GRID_SIZE * STEP;

		// X축 방향
		LineVertex xLine[2] =
		{
			{-extent, 0.0f, p, 0xffaaaaaa},
			{ extent, 0.0f, p, 0xffaaaaaa}
		};

		Device->DrawPrimitiveUP(
			D3DPT_LINELIST,
			1,
			xLine,
			sizeof(LineVertex)
		);

		// Z축 방향
		LineVertex zLine[2] =
		{
			{p, 0.0f, -extent, 0xffaaaaaa},
			{p, 0.0f,  extent, 0xffaaaaaa}
		};

		Device->DrawPrimitiveUP(
			D3DPT_LINELIST,
			1,
			zLine,
			sizeof(LineVertex)
		);
	}
}

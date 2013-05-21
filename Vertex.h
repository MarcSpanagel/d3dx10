#pragma once

struct Vertex
{
	Vertex(){}
	Vertex(float x, float y, float z,
		float u, float v, float nx, float ny, float nz) : 
			Pos(x,y,z), Tex(u,v), normal(nx, ny, nz){}
	D3DXVECTOR3 Pos;
	D3DXVECTOR2 Tex;
	D3DXVECTOR3 normal;
};
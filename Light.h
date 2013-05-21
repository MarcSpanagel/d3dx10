struct Light
{
	Light()
	{
		ZeroMemory(this, sizeof(Light));
	}
	D3DXVECTOR3 dir;
	float pad;
	D3DXCOLOR ambient;
	D3DXCOLOR diffuse;
};
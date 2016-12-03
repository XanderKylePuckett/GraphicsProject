#pragma once

namespace DX11UWA
{
	// Constant buffer used to send MVP matrices to the vertex shader.
	struct ModelViewProjectionConstantBuffer
	{
		DirectX::XMFLOAT4X4 model;
		DirectX::XMFLOAT4X4 view;
		DirectX::XMFLOAT4X4 projection;
	};

	struct LightingConstantBuffer
	{
		DirectX::XMFLOAT4 lightState;
		DirectX::XMFLOAT4 dLightDirection;
		DirectX::XMFLOAT4 pLightPos0;
		DirectX::XMFLOAT4 pLightPos1;
		DirectX::XMFLOAT4 pLightPos2;
		DirectX::XMFLOAT4 pLightColorRadius0;
		DirectX::XMFLOAT4 pLightColorRadius1;
		DirectX::XMFLOAT4 pLightColorRadius2;
	};

	struct Vertex
	{
		DirectX::XMFLOAT4 pos;
		DirectX::XMFLOAT4 uv;
		DirectX::XMFLOAT4 normal;
	};
}
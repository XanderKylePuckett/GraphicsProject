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

	struct LightingBuffer
	{
		DirectX::XMFLOAT4 lightState;
		DirectX::XMFLOAT4 dLightDirection;
		DirectX::XMFLOAT4 pLightPos[ 3 ];
		DirectX::XMFLOAT4 pLightColorRadius[ 3 ];
	};

	struct Vertex
	{
		DirectX::XMFLOAT4 position;
		DirectX::XMFLOAT4 texcoord;
		DirectX::XMFLOAT4 normal;
	};
}
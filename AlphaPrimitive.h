#pragma once
#include "BasePrimitive.h"
#include "FireEffect.h"
class AlphaPrimitive final : public BasePrimitive
{
public:
	AlphaPrimitive(ID3D11Device* pDevice, Elite::FVector3 position, FireEffect* pEffect, Texture* pTexture, std::vector<Vertex_Input>& vertices, std::vector<uint32_t>& indices);
	~AlphaPrimitive();

	AlphaPrimitive(const AlphaPrimitive& other) = delete;
	AlphaPrimitive& operator=(const AlphaPrimitive& other) = delete;
	AlphaPrimitive(AlphaPrimitive&& other) = delete;
	AlphaPrimitive& operator=(AlphaPrimitive&& other) = delete;

	virtual void Render(ID3D11DeviceContext* pDeviceContext) override;

private:
	FireEffect* m_pEffect;
};
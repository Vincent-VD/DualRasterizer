#pragma once
#include "BasePrimitive.h"

class Primitive final : public BasePrimitive
{
public:
	Primitive(ID3D11Device* pDevice, Elite::FVector3 position, Effect* pEffect, std::vector<Texture*> pTextures, std::vector<Vertex_Input>& vertices, std::vector<uint32_t>& indices);
	~Primitive();

	Primitive(const Primitive& other) = delete;
	Primitive& operator=(const Primitive& other) = delete;
	Primitive(Primitive&& other) = delete;
	Primitive& operator=(Primitive&& other) = delete;

	virtual void Render(ID3D11DeviceContext* pDeviceContext) override;

private:
	Effect* m_pEffect;
	Texture* m_pNormalTexture;
	Texture* m_pSpecularTexture;
	Texture* m_pGlossTexture;
};


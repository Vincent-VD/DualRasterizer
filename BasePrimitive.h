#pragma once
//#include "ERenderer.h"
#include "pch.h"
#include "EMath.h"
#include "ERGBColor.h"
#include "Effect.h"
#include "Texture.h"
#include <vector>

struct Vertex_Input
{
	Elite::FPoint4 Position;
	Elite::FPoint3 Color;
	Elite::FVector3 Normal;
	Elite::FVector3 Tangent;
	Elite::FVector2 UV;
};

class BasePrimitive
{
public:
	BasePrimitive(ID3D11Device* pDevice, Elite::FVector3 position, Texture* pTexture, std::vector<Vertex_Input>& vertices, std::vector<uint32_t>& indices);
	~BasePrimitive();

	BasePrimitive(const BasePrimitive& other) = delete;
	BasePrimitive& operator=(const BasePrimitive& other) = delete;
	BasePrimitive(BasePrimitive&& other) = delete;
	BasePrimitive& operator=(BasePrimitive&& other) = delete;

	virtual void Render(ID3D11DeviceContext* pDeviceContext) = 0;

	std::vector<Vertex_Input> GetVertices() const { return m_Vertices; };
	std::vector<uint32_t> GetIndices() const { return m_Indices; };
	Elite::FMatrix4 GetWorldPosition() { return m_WorldMatrix; };
	void SetIsRotating();
	void UpdateRotation(float elapsedSec);

protected:
	bool m_IsRotating{ true };
	float m_RotSpeed{ 50.f };
	Elite::FMatrix4 m_WorldMatrix;
	Elite::FMatrix4 m_RotationMatrix;
	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;
	std::vector<Vertex_Input> m_Vertices;
	std::vector<uint32_t> m_Indices;
	Texture* m_pDiffuseTexture;
	ID3D11InputLayout* m_pVertexLayout;
	uint32_t m_AmountIndices;

	HRESULT InitializeMesh(ID3D11Device* pDevice, BaseEffect* pEffect, std::vector<Vertex_Input>& vertices, std::vector<uint32_t>& indices);
};


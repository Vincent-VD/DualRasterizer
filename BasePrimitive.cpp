#include "pch.h"
#include "BasePrimitive.h"

BasePrimitive::BasePrimitive(ID3D11Device* pDevice, Elite::FVector3 position, Texture* pTexture, std::vector<Vertex_Input>& vertices, std::vector<uint32_t>& indices)
{
	m_pDiffuseTexture = pTexture;
	m_AmountIndices = 0;
	m_pIndexBuffer = nullptr;
	m_pVertexBuffer = nullptr;
	m_pVertexBuffer = nullptr;
	m_Vertices = vertices;
	m_Indices = indices;
	m_WorldMatrix = Elite::FMatrix4::Identity();
	m_WorldMatrix[3] = Elite::FVector4{ position, 1.f };
}

BasePrimitive::~BasePrimitive()
{
	if (m_pVertexBuffer)
	{
		m_pVertexBuffer->Release();
		m_pVertexBuffer = nullptr;
	}
	if (m_pIndexBuffer)
	{
		m_pIndexBuffer->Release();
		m_pIndexBuffer = nullptr;
	}
	if (m_pVertexLayout)
	{
		m_pVertexLayout->Release();
		m_pVertexLayout = nullptr;
	}
}


HRESULT BasePrimitive::InitializeMesh(ID3D11Device* pDevice, BaseEffect* pEffect, std::vector<Vertex_Input>& vertices, std::vector<uint32_t>& indices) {
	//Create Vertex layout
	HRESULT result{ S_OK };
	static const uint32_t numElements{ 5 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};
	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "COLOR";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 16;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "NORMAL";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[2].AlignedByteOffset = 28;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "TANGENT";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[3].AlignedByteOffset = 40;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[4].SemanticName = "TEXCOORD";
	vertexDesc[4].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[4].AlignedByteOffset = 52;
	vertexDesc[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	//Create Vertex buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Vertex_Input) * (uint32_t)vertices.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	D3D11_SUBRESOURCE_DATA initData = { 0 };
	initData.pSysMem = vertices.data();
	result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	if (FAILED(result))
		return result;

	//Create Input layout
	D3DX11_PASS_DESC passDesc{};
	pEffect->GetTechnique()->GetPassByIndex(0)->GetDesc(&passDesc);
	result = pDevice->CreateInputLayout(vertexDesc,
										numElements,
										passDesc.pIAInputSignature,
										passDesc.IAInputSignatureSize,
										&m_pVertexLayout);
	if (FAILED(result))
		return result;

	//Create Index Buffer
	m_AmountIndices = (uint32_t)indices.size();
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_AmountIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	initData.pSysMem = indices.data();
	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	if (FAILED(result))
		return result;

	return result;
}

void BasePrimitive::SetIsRotating()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, 12);
	std::cout << "TOGGLING ROTATION: ";
	if (m_IsRotating)
	{
		m_IsRotating = false;
		std::cout << "off\n";
	}
	else
	{
		m_IsRotating = true;
		std::cout << "on\n";
	}
	SetConsoleTextAttribute(hConsole, 15);
}

void BasePrimitive::UpdateRotation(float elapsedSec)
{
	if (m_IsRotating)
	{
		Elite::FMatrix4 rotMatrix{ Elite::MakeRotationY(elapsedSec * Elite::ToRadians(m_RotSpeed)) };
		m_RotationMatrix = rotMatrix;
		m_WorldMatrix *= m_RotationMatrix;
	}
}
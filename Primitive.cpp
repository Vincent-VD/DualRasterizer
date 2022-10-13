#include "pch.h"
#include "Primitive.h"

Primitive::Primitive(ID3D11Device* pDevice, Elite::FVector3 position, Effect* pEffect, std::vector<Texture*> pTextures, std::vector<Vertex_Input>& vertices, std::vector<uint32_t>& indices)
	: BasePrimitive(pDevice, position, pTextures[0], vertices, indices)
{
	m_pEffect = pEffect;
	m_pNormalTexture = pTextures[1];
	m_pSpecularTexture = pTextures[2];
	m_pGlossTexture = pTextures[3];

	HRESULT result = InitializeMesh(pDevice, pEffect, vertices, indices);
}

Primitive::~Primitive()
{

}

void Primitive::Render(ID3D11DeviceContext* pDeviceContext)
{
	//Set Vertex buffer
	UINT stride = sizeof(Vertex_Input);
	UINT offset{};
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//Set Index buffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//Set Input layout
	pDeviceContext->IASetInputLayout(m_pVertexLayout);

	//Set primitive topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	m_pEffect->SetDiffuseMap(m_pDiffuseTexture->GetResourceView());
	m_pEffect->SetNormalMap(m_pNormalTexture->GetResourceView());
	m_pEffect->SetSpecularMap(m_pSpecularTexture->GetResourceView());
	m_pEffect->SetGlossMap(m_pGlossTexture->GetResourceView());
	m_pEffect->SetRasterizerState(pDeviceContext);

	//Render a triangle
	D3DX11_TECHNIQUE_DESC techDesc{};
	m_pEffect->GetTechnique()->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pEffect->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_AmountIndices, 0, 0);
	}
}
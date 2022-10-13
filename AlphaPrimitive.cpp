#include "pch.h"
#include "AlphaPrimitive.h"

AlphaPrimitive::AlphaPrimitive(ID3D11Device* pDevice, Elite::FVector3 position, FireEffect* pEffect, Texture* pTexture, std::vector<Vertex_Input>& vertices, std::vector<uint32_t>& indices)
	: BasePrimitive{ pDevice, position, pTexture, vertices, indices }
{
	m_pEffect = pEffect;
	InitializeMesh(pDevice, pEffect, vertices, indices);
}

AlphaPrimitive::~AlphaPrimitive()
{

}

void AlphaPrimitive::Render(ID3D11DeviceContext* pDeviceContext)
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
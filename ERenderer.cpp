#include "pch.h"

#ifdef _DEBUG
#include <vld.h>
#endif

//Project includes
#include "ERenderer.h"
#include "ECamera.h"
#include "OBJParser.h"
#include "BRDF.h"

using namespace Elite;

std::vector<SDL_Scancode> gKonamiCode{ SDL_SCANCODE_UP, SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, SDL_SCANCODE_B, SDL_SCANCODE_A};

template <typename T>
void VertexTransform(const std::vector<T>& originalVertices,
					 std::vector<T>& transformedVertices,
					 FMatrix4 worldPosition,
					 FMatrix4 worldViewProjectionMatrix,
					 uint32_t width, uint32_t height);

//FMatrix4 Elite::Renderer::m_WorldMatrix{ FMatrix4::Identity() };

Elite::Renderer::Renderer(SDL_Window * pWindow)
	: m_pWindow{ pWindow }
	, m_Width{}
	, m_Height{}
	, m_IsInitialized{ false }
{
	int width, height = 0;
	m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
	SDL_GetWindowSize(pWindow, &width, &height);
	m_Width = static_cast<uint32_t>(width);
	m_Height = static_cast<uint32_t>(height);
	//m_RotationMatrix = Elite::FMatrix4::Identity();
	m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
	m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

	//Initialize DirectX pipeline
	HRESULT result = InitializeDirectX();
	if (!FAILED(result))
	{
		m_pEffect = new Effect{ m_pDevice, m_pDeviceContext, L"Resources/PosCol3D.fx" };
		m_pEffect->SetLightDirection(m_LightDirection);
		m_pEffect->SetPhongSettings(FVector3{ float(M_PI), 7.f, 25.f });
		//Texture vector order must ALWAYS be: diffuse, normal, specular, gloss
		m_pTextures = { new Texture{ m_pDevice, "Resources/vehicle_diffuse.png" },
						new Texture{ m_pDevice, "Resources/vehicle_normal.png" },
						new Texture{ m_pDevice, "Resources/vehicle_specular.png" },
						new Texture{ m_pDevice, "Resources/vehicle_gloss.png" } };
		m_pFireEffect = new FireEffect{ m_pDevice, m_pDeviceContext, L"Resources/PosCol3D.fx" };
		m_pFireTexture = new Texture{ m_pDevice, "Resources/fireFX_diffuse.png" };
		m_pRastaRizing = new Texture{ m_pDevice, "Resources/Rasta_Rising2.png" };
	}

	m_Vertices = { {FPoint3(0.5f, 0.5f, 0.5f), {}, {}, {}, FVector2(1.f, 0.f) },
				   {FPoint3(0.5f, -0.5f, 0.5f), {}, {}, {}, FVector2(1.f, 1.f) },
				   {FPoint3(-0.5f, -0.5f, 0.5f), {}, {}, {}, FVector2(0.f, 1.f) },
				   {FPoint3(-0.5f, 0.5f, 0.5f), {}, {}, {}, FVector2(0.f, 0.f) } };
	m_Indices = { 0, 1, 2, 3, 0, 2 };
	//Renderer::m_WorldMatrix[3][2] = 30.f;

	LoadPrimitives();

	m_IsInitialized = true;
}

Elite::Renderer::~Renderer()
{
	if (m_pEffect)
	{
		delete m_pEffect;
		m_pEffect = nullptr;
	}
	if (m_pFireEffect)
	{
		delete m_pFireEffect;
		m_pFireEffect = nullptr;
	}
	if (m_pFireTexture)
	{
		delete m_pFireTexture;
		m_pFireTexture = nullptr;
	}
	if (m_pRastaRizing)
	{
		delete m_pRastaRizing;
		m_pRastaRizing = nullptr;
	}
	for (size_t iter = 0; iter < m_pTextures.size(); ++iter)
	{
		delete m_pTextures[iter];
		m_pTextures[iter] = nullptr;
	}
	for (size_t iter = 0; iter < m_pPrimitives.size(); ++iter)
	{
		delete m_pPrimitives[iter];
		m_pPrimitives[iter] = nullptr;
	}
	if (m_pRenderTargetView)
	{
		m_pRenderTargetView->Release();
		m_pRenderTargetView = nullptr;
	}
	if (m_pRenderTargetBuffer)
	{
		m_pRenderTargetBuffer->Release();
		m_pRenderTargetBuffer = nullptr;
	}
	if (m_pDepthStencilView)
	{
		m_pDepthStencilView->Release();
		m_pDepthStencilView = nullptr;
	}
	if (m_pDepthStencilBuffer)
	{
		m_pDepthStencilBuffer->Release();
		m_pDepthStencilBuffer = nullptr;
	}
	if (m_pSwapChain)
	{
		m_pSwapChain->Release();
		m_pSwapChain = nullptr;
	}
	if (m_pDebug)
	{
		m_pDebug->Release();
		m_pDebug = nullptr;
	}
	if (m_pDeviceContext)
	{
		m_pDeviceContext->ClearState();
		m_pDeviceContext->Flush();
		m_pDeviceContext->Release();
		m_pDeviceContext = nullptr;
	}
	if (m_pDXGIFactory)
	{
		m_pDXGIFactory->Release();
		m_pDXGIFactory = nullptr;
	}
	if (m_pDevice)
	{
		m_pDevice->Release();
		m_pDevice = nullptr;
	}

}

void Elite::Renderer::RenderHardware()
{
	if (!m_IsInitialized) 
		return;

	Camera* sCamera{ Camera::GetInstance() };

	//Clear buffers
	RGBColor clearColor{ RGBColor{0.f, 0.f, 0.3f} };
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

	
	//Render
	for (size_t iter = 0; iter < m_pPrimitives.size(); ++iter)
	{
		if (iter == (m_pPrimitives.size() - 1) && !m_DisplayEasterEgg)
		{
			continue;
		}
		FMatrix4 worldViewProjectionMatrix{ sCamera->GetProjectionMatrix() * sCamera->GetWorldToView() * m_pPrimitives[iter]->GetWorldPosition() };
		m_pEffect->SetWorldViewProjMatrix(worldViewProjectionMatrix);
		m_pEffect->SetWorldMatrix(m_pPrimitives[iter]->GetWorldPosition());
		m_pEffect->SetViewInverseMatrix(sCamera->GetViewToWorld());
		m_pFireEffect->SetWorldViewProjMatrix(worldViewProjectionMatrix);

		if (iter == 1 && !m_IsRenderFireEffect)
		{
			continue;
		}
		m_pPrimitives[iter]->Render(m_pDeviceContext);
	}

	//Present
	m_pSwapChain->Present(m_IsEnabledVSync, 0);
}

void Elite::Renderer::RenderSoftware()
{
	SDL_LockSurface(m_pBackBuffer);
	const float res{ float(m_Width) * float(m_Height) };
	std::vector<float> depthBuffer(uint64_t(res), FLT_MAX);
	std::vector<Vertex_Input> transformedVertices{};

	SDL_FillRect(m_pBackBuffer, nullptr, SDL_MapRGB(m_pBackBuffer->format, static_cast<Uint8>(0.5f * 255.f), static_cast<Uint8>(0.5f * 255.f), static_cast<Uint8>(0.5f * 255.f)));

	Camera* sCamera{ Camera::GetInstance() };

	for (size_t iter = 0; iter < m_pPrimitives.size(); iter += 2)
	{
		if (iter == (m_pPrimitives.size() - 1) && !m_DisplayEasterEgg)
		{
			continue;
		}
		FMatrix4 worldViewProjectionMatrix{ sCamera->GetProjectionMatrix() * sCamera->GetWorldToView() * m_pPrimitives[iter]->GetWorldPosition() };

		std::vector<Vertex_Input> vertices{ m_pPrimitives[iter]->GetVertices() };
		std::vector<uint32_t> indices{ m_pPrimitives[iter]->GetIndices() };

		VertexTransform(vertices, transformedVertices, m_pPrimitives[iter]->GetWorldPosition(), worldViewProjectionMatrix, m_Width, m_Height);

		RGBColor finalColor{};
		for (size_t iter = 0; iter < indices.size(); iter += 3)
		{
			FPoint4 v0{}, v1{}, v2{};
			v0 = transformedVertices[indices[iter]].Position;
			v1 = transformedVertices[indices[iter + 1]].Position;
			v2 = transformedVertices[indices[iter + 2]].Position;

			//Calculate bounding box
			IPoint2 topLeft{ INT_MAX, INT_MAX };
			IPoint2 bottomRight{ 0 , 0 };

			//Clamp top left between 0 and screen width/height
			topLeft.x = static_cast<int>(Clamp(std::min(v0.x, std::min(v1.x, v2.x)), 0.f, static_cast<float>(m_Width - 1)));
			topLeft.y = static_cast<int>(Clamp(std::min(v0.y, std::min(v1.y, v2.y)), 0.f, static_cast<float>(m_Height - 1)));
			bottomRight.x = static_cast<int>(std::ceil(Clamp(std::max(v0.x, std::max(v1.x, v2.x)), 0.f, static_cast<float>(m_Width - 1))));
			bottomRight.y = static_cast<int>(std::ceil(Clamp(std::max(v0.y, std::max(v1.y, v2.y)), 0.f, static_cast<float>(m_Height - 1))));

			for (uint32_t r = topLeft.y; r < uint32_t(bottomRight.y); ++r)
			{
				for (uint32_t c = topLeft.x; c < uint32_t(bottomRight.x); ++c)
				{
					FPoint2 currPixelPoint{ static_cast<float>(c), static_cast<float>(r) };

					float v0Cross{}, v1Cross{}, v2Cross{};
					const float triangleArea{ Cross(FVector2{v0 - v1}, FVector2{v0 - v2}) };

					//edge A
					const FVector2 edgeA{ v1 - v0 };
					const FVector2 pointTo0{ currPixelPoint - FPoint2(v0) };
					v2Cross = Cross(edgeA, pointTo0);
					//edge B
					const FVector2 edgeB{ v2 - v1 };
					const FVector2 pointTo1{ currPixelPoint - FPoint2(v1) };
					v0Cross = Cross(edgeB, pointTo1);
					//edge C
					const FVector2 edgeC{ v0 - v2 };
					const FVector2 pointTo2{ currPixelPoint - FPoint2(v2) };
					v1Cross = Cross(edgeC, pointTo2);

					//If totalWeight is 1, point is inside triangle
					if (CullModeTest(v0Cross, v1Cross, v2Cross))
					{
						const float weight0{ v0Cross / triangleArea };
						const float weight1{ v1Cross / triangleArea };
						const float weight2{ v2Cross / triangleArea };

						float zBufferValue{ 1.f / (weight0 * (1.f / v0.z) + weight1 * (1.f / v1.z) + weight2 * (1.f / v2.z)) };
						const float wInterpolated{ 1.f / (weight0 * (1.f / v0.w) + weight1 * (1.f / v1.w) + weight2 * (1.f / v2.w)) };
						if (zBufferValue < depthBuffer[size_t(c + (r * m_Width))])
						{
							depthBuffer[size_t(c + (r * m_Width))] = zBufferValue;
							FVector2 finalUV{};
							FVector3 newNormal{}, newTangent{};
							FVector3 newWPos{};
							finalUV = (transformedVertices[indices[iter]].UV / v0.w) * weight0 +
								(transformedVertices[indices[iter + 1]].UV / v1.w) * weight1 +
								(transformedVertices[indices[iter + 2]].UV / v2.w) * weight2;
							finalUV *= wInterpolated;

							newNormal = (transformedVertices[indices[iter]].Normal / v0.w) * weight0 +
								(transformedVertices[indices[iter + 1]].Normal / v1.w) * weight1 +
								(transformedVertices[indices[iter + 2]].Normal / v2.w) * weight2;
							newNormal *= wInterpolated;

							newTangent = (transformedVertices[indices[iter]].Tangent / v0.w) * weight0 +
								(transformedVertices[indices[iter + 1]].Tangent / v1.w) * weight1 +
								(transformedVertices[indices[iter + 2]].Tangent / v2.w) * weight2;
							newTangent *= wInterpolated;

							newWPos = (FVector3(transformedVertices[indices[iter]].Color) / v0.w) * weight0 +
								(FVector3(transformedVertices[indices[iter + 1]].Color) / v1.w) * weight1 +
								(FVector3(transformedVertices[indices[iter + 2]].Color) / v2.w) * weight2;
							newWPos *= wInterpolated;

							FPoint3 worldPos{ newWPos };
							const Vertex_Input tempVert{ {}, {}, GetNormalized(newNormal), newTangent, finalUV };
							finalColor = PixelShader(tempVert, sCamera->GetPosition(), worldPos);
							if (finalColor.a < 0)
							{
								continue;
							}

							finalColor.MaxToOne();
							m_pBackBufferPixels[c + (r * m_Width)] = SDL_MapRGB(m_pBackBuffer->format,
								static_cast<uint8_t>(finalColor.r * 255.f),
								static_cast<uint8_t>(finalColor.g * 255.f),
								static_cast<uint8_t>(finalColor.b * 255.f));
						}
					}
				}
			}
		}
		SDL_UnlockSurface(m_pBackBuffer);
		SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
		SDL_UpdateWindowSurface(m_pWindow);

	}
}

HRESULT Elite::Renderer::InitializeDirectX()
{
	//Create device and device context, using hardware acceleration
	D3D_FEATURE_LEVEL featureLevel{ D3D_FEATURE_LEVEL_11_0 };
	uint32_t createDeviceFlags{};
#if defined(DEBUG) || defined(_DEBUG)
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	HRESULT result = D3D11CreateDevice(0,
									 D3D_DRIVER_TYPE_HARDWARE,
									 0,
									 createDeviceFlags,
									 0, 0,
									 D3D11_SDK_VERSION,
									 &m_pDevice,
									 &featureLevel,
									 &m_pDeviceContext);
	if (FAILED(result))
		return result;

	result = m_pDevice->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&m_pDebug));
	if (!FAILED(result))
	{
		result = m_pDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	}

	//Create DXGI Factory to create SwapChain based on hardware
	result = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&m_pDXGIFactory));
	if (FAILED(result))
		return result;

	//Create Swapchain Descriptor
	DXGI_SWAP_CHAIN_DESC swapChainDescr{};
	swapChainDescr.BufferDesc.Width = m_Width;
	swapChainDescr.BufferDesc.Height = m_Height;
	swapChainDescr.BufferDesc.RefreshRate.Numerator = 1;
	swapChainDescr.BufferDesc.RefreshRate.Denominator = 60;
	swapChainDescr.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDescr.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDescr.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swapChainDescr.SampleDesc.Count = 1;
	swapChainDescr.SampleDesc.Quality = 0;
	swapChainDescr.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDescr.BufferCount = 1;
	swapChainDescr.Windowed = true;
	swapChainDescr.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapChainDescr.Flags = 0;

	//Get the handle HWND from the SDL backbuffer
	SDL_SysWMinfo sysWMinfo{};
	SDL_VERSION(&sysWMinfo.version);
	SDL_GetWindowWMInfo(m_pWindow, &sysWMinfo);
	swapChainDescr.OutputWindow = sysWMinfo.info.win.window;

	//Create swapchain and hook it up to the handle of the SDL window
	result = m_pDXGIFactory->CreateSwapChain(m_pDevice, &swapChainDescr, &m_pSwapChain);
	if (FAILED(result))
		return result;

	//Create depth/stencil buffer and view
	D3D11_TEXTURE2D_DESC depthStencilDesc{};
	depthStencilDesc.Width = m_Width;
	depthStencilDesc.Height = m_Height;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	//Describe the resource view for the depth/stencil buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = depthStencilDesc.Format;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	result = m_pDevice->CreateTexture2D(&depthStencilDesc, 0, &m_pDepthStencilBuffer);
	if (FAILED(result))
		return result;

	result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
	if (FAILED(result))
		return result;

	//Create the RenderTargetView
	result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
	if (FAILED(result))
		return result;
	result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, 0, &m_pRenderTargetView);
	if (FAILED(result))
		return result;

	//Bind the Views to the Output Merger stage
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

	//Set the viewPort
	D3D11_VIEWPORT viewPort{};
	viewPort.Width = static_cast<float>(m_Width);
	viewPort.Height = static_cast<float>(m_Height);
	viewPort.TopLeftX = 0.f;
	viewPort.TopLeftY = 0.f;
	viewPort.MinDepth = 0.f;
	viewPort.MaxDepth = 1.f;
	m_pDeviceContext->RSSetViewports(1, &viewPort);

	return result;
}

void Elite::Renderer::SetIsRotating()
{
	for (size_t iter = 0; iter < m_pPrimitives.size() - 1; ++iter)
	{
		m_pPrimitives[iter]->SetIsRotating();
	}
}

void Elite::Renderer::UpdateRotation(float elapsedSec)
{
	for (size_t iter = 0; iter < m_pPrimitives.size() - 1; ++iter)
	{
		m_pPrimitives[iter]->UpdateRotation(elapsedSec);
	}
}

void Elite::Renderer::LoadPrimitives()
{
	ParseOBJ("Resources/vehicle.obj", m_Vertices, m_Indices);
	BasePrimitive* prim{ new Primitive(m_pDevice, Elite::FVector3{0.f, 0.f, 30.f}, m_pEffect, m_pTextures, m_Vertices, m_Indices) };
	m_pPrimitives.push_back(prim);

	ParseOBJ("Resources/fireFX.obj", m_Vertices, m_Indices);
	prim = new AlphaPrimitive(m_pDevice, Elite::FVector3{ 0.f, 0.f, 30.f }, m_pFireEffect, m_pFireTexture, m_Vertices, m_Indices);
	m_pPrimitives.push_back(prim);

	m_Vertices = { {FPoint3(5.f, 5.f, 5.f), {}, {}, {}, FVector2(1.f, 0.f) },
				   {FPoint3(5.f, -5.f, 5.f), {}, {}, {}, FVector2(1.f, 1.f) },
				   {FPoint3(-5.f, -5.f, 5.f), {}, {}, {}, FVector2(0.f, 1.f) },
				   {FPoint3(-5.f, 5.f, 5.f), {}, {}, {}, FVector2(0.f, 0.f) } };
	m_Indices = { 0, 1, 2, 3, 0, 2 };
	prim = new AlphaPrimitive(m_pDevice, Elite::FVector3{ 10.f, 10.f, 30.f }, m_pFireEffect, m_pRastaRizing, m_Vertices, m_Indices);
	m_pPrimitives.push_back(prim);
}

void Elite::Renderer::ChangeTechnique()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, 12);
	std::cout << "CHANGING TECHNIQUE:\n";
	m_pEffect->ChangeTechnique();
	for (Texture* txt : m_pTextures)
	{
		txt->ChangeTechnique();
	}
	SetConsoleTextAttribute(hConsole, 15);
}

void Elite::Renderer::ChangeCullMode()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, 12);
	std::cout << "CHANGING CULL MODE: ";
	m_CullMode = D3D11_CULL_MODE(int(m_CullMode) % 3 + 1);
	m_pEffect->ChangeRasterizerState(m_pDevice, m_pDeviceContext, m_CullMode);

	switch (m_CullMode)
	{
	case D3D10_CULL_NONE:
		std::cout << "none\n";
		break;
	case D3D10_CULL_FRONT:
		std::cout << "front\n";
		break;
	case D3D10_CULL_BACK:
		std::cout << "back\n";
		break;
	}
	SetConsoleTextAttribute(hConsole, 15);
}

void Elite::Renderer::ChangeRenderFireEffect()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, 12);
	std::cout << "TOGGLING FIRE EFFECT: ";
	if (m_IsRenderFireEffect)
	{
		m_IsRenderFireEffect = false;
		std::cout << "off\n";
	}
	else
	{
		m_IsRenderFireEffect = true;
		std::cout << "on\n";
	}
	SetConsoleTextAttribute(hConsole, 15);
}

bool Elite::Renderer::CullModeTest(float v0Cross, float v1Cross, float v2Cross)
{
	bool res{ false };
	switch (m_CullMode)
	{
	case D3D10_CULL_NONE:
		if ((v0Cross <= 0.f) && (v1Cross <= 0.f) && (v2Cross <= 0.f) || (v0Cross > 0.f) && (v1Cross > 0.f) && (v2Cross > 0.f))
		{
			res = true;
		}
		break;
	case D3D10_CULL_FRONT:
		if ((v0Cross > 0.f) && (v1Cross > 0.f) && (v2Cross > 0.f))
		{
			res = true;
		}
		break;
	case D3D10_CULL_BACK:
		if ((v0Cross <= 0.f) && (v1Cross <= 0.f) && (v2Cross <= 0.f))
		{
			res = true;
		}
		break;
	}
	return res;
}

template <typename T>
void VertexTransform(const std::vector<T>& originalVertices,
					 std::vector<T>& transformedVertices,
					 FMatrix4 worldPosition,
					 FMatrix4 worldViewProjectionMatrix,
					 uint32_t width, uint32_t height)
{
	transformedVertices = originalVertices;
	for (size_t iter = 0; iter < originalVertices.size(); ++iter)
	{
		FPoint4 vert{ originalVertices[iter].Position };
		FPoint4 transformedVert{ worldViewProjectionMatrix * vert };
		transformedVert.x /= transformedVert.w;
		transformedVert.y /= transformedVert.w;
		transformedVert.z /= transformedVert.w;
		FVector4 newNormal{ GetNormalized(worldPosition * FVector4(originalVertices[iter].Normal)) };
		FVector4 newTangent{ GetNormalized(worldPosition * FVector4(originalVertices[iter].Tangent)) };
		FPoint4 ssp{ ((transformedVert.x + 1) / 2) * width, ((1 - transformedVert.y) / 2) * height, transformedVert.z, -transformedVert.w };
		transformedVertices[iter].Position = ssp;
		transformedVertices[iter].Normal = newNormal.xyz;
		transformedVertices[iter].Tangent = newTangent.xyz;
		transformedVertices[iter].Color = (worldPosition * vert).xyz;
	}
}

void Elite::Renderer::ChangeRenderMode()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, 12);
	std::cout << "------------------------------" << std::endl;
	std::cout << "CHANGING RENDER MODE: ";
	switch (m_RenderMode)
	{
	case RenderMode::HARDWARE:
		m_RenderMode = RenderMode::SOFTWARE;
		m_LightDirection = FVector3{ 0.577f, -0.577f, 0.577f };
		std::cout << "software\n";
		break;
	case RenderMode::SOFTWARE:
		m_RenderMode = RenderMode::HARDWARE;
		m_LightDirection = FVector3{ 0.577f, -0.577f, 0.577f };
		std::cout << "hardware\n";
		break;
	}
	std::cout << "------------------------------" << std::endl;
	SetConsoleTextAttribute(hConsole, 15);
	Camera* sCamera{ Camera::GetInstance() };
	sCamera->SwitchCameraMode(m_RenderMode);
}

RenderMode Elite::Renderer::GetRenderMode()
{
	return m_RenderMode;
}

RGBColor Elite::Renderer::PixelShader(const Vertex_Input& v, const FPoint3& viewMatrix, const FPoint3& pixel)
{
	const FVector3 viewDirection{ GetNormalized(pixel - viewMatrix) };
	FVector3 binormal = Cross(v.Tangent, v.Normal);
	FMatrix3 tangentSpaceAxis = FMatrix3(v.Tangent, binormal, v.Normal);

	RGBColor normalMapSample{ m_pTextures[1]->Sample(v.UV) };
	//normalMapSample /= 255.f;
	normalMapSample.r = 2.f * normalMapSample.r - 1.f;
	normalMapSample.g = 2.f * normalMapSample.g - 1.f;
	normalMapSample.b = 2.f * normalMapSample.b - 1.f;

	FVector3 newNormal{ tangentSpaceAxis * FVector3(normalMapSample.r, normalMapSample.g, normalMapSample.b) };
	float observedArea{};
	observedArea = Dot(-newNormal, m_LightDirection);
	observedArea = Clamp(observedArea, 0.f, 1.f);

	float exp{ m_pTextures[3]->Sample(v.UV).r };

	RGBColor lambert{ BRDF::Lambert(m_pTextures[0]->Sample(v.UV), RGBColor{1, 1 ,1}) };

	RGBColor phong{ BRDF::Phong(m_pTextures[2]->Sample(v.UV), exp * 25.f, m_LightDirection, viewDirection, newNormal) };

	RGBColor colorVec{ RGBColor{1.f, 1.f, 1.f} * 7.f *observedArea * lambert + phong };

	return colorVec;
}

void Elite::Renderer::RegisterKeyPress(SDL_Scancode key)
{
	if (!m_DisplayEasterEgg)
	{
		if (key == gKonamiCode[m_Iter])
		{
			++m_Iter;
			std::cout << "Next\n";
		}
		else
		{
			m_Iter = 0;
		}

		if (m_Iter == gKonamiCode.size())
		{
			HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
			SetConsoleTextAttribute(hConsole, 118);
			std::cout << "*****************************************************" << std::endl;
			std::cout << "KONAMI CODE ACTIVATED: press 'k' to toggle easter egg\n";
			std::cout << "*****************************************************" << std::endl;
			SetConsoleTextAttribute(hConsole, 15);
			m_DisplayEasterEgg = true;
		}
	}
}

void Elite::Renderer::ToggleVSync() {
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole, 18);
	std::cout << "TOGGLING V-Sync: ";
	if (m_IsEnabledVSync)
	{
		m_IsEnabledVSync = false;
		std::cout << "off\n";
	}
	else
	{
		m_IsEnabledVSync = true;
		std::cout << "on\n";
	}
	SetConsoleTextAttribute(hConsole, 15);
}
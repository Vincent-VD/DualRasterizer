//------------------------
//  Global Variables
//------------------------

float4x4 gWorldViewProj : WorldViewProjection;
Texture2D gDiffuseMap : DiffuseMap;
Texture2D gNormalMap : NormalMap;
Texture2D gSpecularMap : SpecularMap;
Texture2D gGlossMap : GlossinessMap;
float4x4 gWorldMatrix : WorldMatrix;
float4x4 gONB : ViewInverseMatrix;
float3 gLightDirection : LightDirection;
float3 gPhongSettings : PhongSettings;


//------------------------
//  Input/Output Structs
//------------------------

struct VS_INPUT
{
	float3 Position : POSITION;
	float3 Color : COLOR;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 UV : TEXCOORD;
};

struct VS_OUTPUT
{
	float4 Position : SV_POSITION;
	float3 WorldPosition : COLOR;
	float3 Normal : NORMAL;
	float3 Tangent : TANGENT;
	float2 UV : TEXCOORD;
};


//------------------------
//  Depth stencil state
//------------------------

DepthStencilState gDepthStencilStateAlpha
{
	DepthEnable = true;
	DepthWriteMask = zero;
	DepthFunc = less;
	StencilEnable = false;
};

DepthStencilState gDepthStencilState
{
	DepthEnable = true;
	DepthWriteMask = all;
	DepthFunc = less;
};


//------------------------
//  Blend state
//------------------------

BlendState gBlendStateAlpha
{
	BlendEnable[0] = true;
	SrcBlend = src_alpha;
	DestBlend = inv_src_alpha;
	SrcBlendAlpha = zero;
	RenderTargetWriteMask[0] = 0x0F;
};

BlendState gBlendState
{
	BlendEnable[0] = false;
};


//------------------------
//  Sample point
//------------------------

SamplerState samPoint
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Border;
	AddressV = Clamp;
	BorderColor = float4(0.f, 0.f, 1.f, 1.f);
};

SamplerState samLin
{
	Filter = MIN_MAG_POINT_MIP_LINEAR;
	AddressU = Border;
	AddressV = Clamp;
	BorderColor = float4(0.f, 0.f, 1.f, 1.f);
};

SamplerState samAX
{
	Filter = ANISOTROPIC;
	AddressU = Border;
	AddressV = Clamp;
	BorderColor = float4(0.f, 0.f, 1.f, 1.f);
};


//------------------------
//  Rasterizer State
//------------------------

RasterizerState gRasterizerStateBack
{
	CullMode = back;
	FrontCounterClockwise = true;
};

RasterizerState gRasterizerStateFront
{
	CullMode = front;
	FrontCounterClockwise = true;
};

RasterizerState gRasterizerStateNone
{
	CullMode = none;
	FrontCounterClockwise = true;
};


//------------------------
//  Vertex Shader
//------------------------
VS_OUTPUT VS1(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
	output.WorldPosition = input.Position;
	output.Normal = input.Normal;
	output.Tangent = input.Tangent;
	output.UV = input.UV;
	return output;
}


VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Position = mul(float4(input.Position, 1.f), gWorldViewProj);
	output.WorldPosition = mul(float4(input.Position, 1.f), gWorldMatrix);
	output.Normal = normalize(mul(input.Normal, (float3x3)gWorldMatrix));
	output.Tangent = normalize(mul(input.Tangent, (float3x3)gWorldMatrix));
	output.UV = input.UV;
	return output;
}


//----------------------------
//  Shader Helper Functions
//----------------------------

float4 LambertPhongShader(VS_OUTPUT input, SamplerState samState)
{
	float3 viewDirection = normalize(input.WorldPosition.xyz - transpose(gONB)[3].xyz);
	float3 binormal = cross(input.Tangent, input.Normal);
	float3x3 tangentSpaceAxis = { input.Tangent,
								  binormal,
								  input.Normal };
	float3 normalMapSample = (float3)gNormalMap.Sample(samState, input.UV);
	normalMapSample = (normalMapSample * 2) - float3(1, 1, 1);
	float3 newNormal = mul(transpose(tangentSpaceAxis), normalMapSample);
	float observedArea = dot(newNormal, -gLightDirection);
	observedArea = saturate(observedArea);

	float exp = gGlossMap.Sample(samState, input.UV).r * gPhongSettings.z;

	const float3 reflect = gLightDirection - (2 * dot(newNormal, gLightDirection)) * newNormal;
	const float cosAlpha = saturate(dot(reflect, -viewDirection));
	const float4 phongValue = gSpecularMap.Sample(samState, input.UV) * pow(cosAlpha, exp);

	const float4 diffuseColor = gDiffuseMap.Sample(samState, input.UV) * gPhongSettings.y * observedArea / gPhongSettings.x;
	const float4 finalColor = phongValue + diffuseColor;
	return finalColor;
}

//------------------------
//  Pixel Shader
//------------------------
float4 PS(VS_OUTPUT input) : SV_TARGET
{
	return gDiffuseMap.Sample(samPoint, input.UV);
}

float4 PSPoint(VS_OUTPUT input) : SV_TARGET
{
	float4 res = LambertPhongShader(input, samPoint);
	return res;
}

float4 PSLinear(VS_OUTPUT input) : SV_TARGET
{
	float4 res = LambertPhongShader(input, samLin);
	return res;
}

float4 PSAnisotropic(VS_OUTPUT input) : SV_TARGET
{
	float4 res = LambertPhongShader(input, samAX);
	return res;
}


//------------------------
//  Technique
//------------------------
technique11 FireTechnique
{
	pass P0
	{
		//SetRasterizerState(gRasterizerStateNone);
		SetDepthStencilState(gDepthStencilStateAlpha, 0);
		SetBlendState(gBlendStateAlpha, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS1()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
};

technique11 PointTechnique
{
	pass P0
	{
		//SetRasterizerState(gRasterizerStateBack);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader( CompileShader( vs_5_0, VS() ) );
		SetGeometryShader( NULL );
		SetPixelShader( CompileShader( ps_5_0, PSPoint() ) );
	}
};

technique11 LinearTechnique
{
	pass P0
	{
		//SetRasterizerState(gRasterizerStateBack);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSLinear()));
	}
};

technique11 AnisotropicTechnique
{
	pass P0
	{
		//SetRasterizerState(gRasterizerStateBack);
		SetDepthStencilState(gDepthStencilState, 0);
		SetBlendState(gBlendState, float4(0.f, 0.f, 0.f, 0.f), 0xFFFFFFFF);
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_5_0, PSAnisotropic()));
	}
};
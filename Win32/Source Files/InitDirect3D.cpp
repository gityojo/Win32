#pragma once
#pragma comment(lib, "WinMM")

#include "d3dApp.h"

using namespace std;
using namespace DirectX;

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT4 Color;
};

struct ConstantBuffer
{
	XMFLOAT4X4 WorldViewProj;
};

class InitDirect3DApp : public D3DApp
{
public:
	InitDirect3DApp(HINSTANCE hInstance);
	~InitDirect3DApp();

	bool Init();
	void OnResize();
	void UpdateScene(float dt);
	void DrawScene();

private:
	void InputAssembler();
	ID3DBlob* LoadShader(const string& filename);

private:
	ID3D11Buffer* mVertexBuffer;
	ID3D11Buffer* mIndexBuffer;
	ID3D11Buffer* mConstantBuffer;

	ID3D11VertexShader* mVertexShader;
	ID3D11PixelShader* mPixelShader;

	ID3D11InputLayout* mInputLayout;

	XMFLOAT4X4 mWorld;
	XMFLOAT4X4 mView;
	XMFLOAT4X4 mProj;

	float mTheta = XM_PI * 1.5f;
	float mPhi = XM_PIDIV4;
	float mRadius = 5.0f;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
				   PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	InitDirect3DApp theApp(hInstance);
	
	if( !theApp.Init() )
		return 0;

	// PlaySound(TEXT("Resource Files/Sweet Devil.wav"), NULL, SND_FILENAME | SND_ASYNC | SND_LOOP);
	PlaySound(
		MAKEINTRESOURCE(IDR_WAVE),
		GetModuleHandle(NULL),
		SND_RESOURCE | SND_ASYNC | SND_LOOP);

	return theApp.Run();
}

InitDirect3DApp::InitDirect3DApp(HINSTANCE hInstance)
: D3DApp(hInstance) 
{
	XMMATRIX I = XMMatrixIdentity();

	XMStoreFloat4x4(&mWorld, I);
	XMStoreFloat4x4(&mView, I);
	XMStoreFloat4x4(&mProj, I);
}

InitDirect3DApp::~InitDirect3DApp()
{
}

bool InitDirect3DApp::Init()
{
	if(!D3DApp::Init())
		return false;

	InputAssembler();

	return true;
}

void InitDirect3DApp::OnResize()
{
	D3DApp::OnResize();
}

void InitDirect3DApp::UpdateScene(float dt)
{
	// Convert Spherical to Cartesian coordinates.
	float x = mRadius * sinf(mPhi) * cosf(mTheta);
	float z = mRadius * sinf(mPhi) * sinf(mTheta);
	float y = mRadius * cosf(mPhi);

	// Build the view matrix.
	XMVECTOR pos = XMVectorSet(x, y, z, 1.0f);
	XMVECTOR target = XMVectorZero();
	XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, view);

	XMMATRIX world = XMLoadFloat4x4(&mWorld);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);

	XMMATRIX worldViewProj = world * view * proj;

	// Update the constant buffer with the latest worldViewProj matrix.
	ConstantBuffer cb;
	XMStoreFloat4x4(&cb.WorldViewProj, XMMatrixTranspose(worldViewProj));
	md3dDeviceContext->UpdateSubresource(mConstantBuffer, 0, nullptr, &cb, 0, 0);
}

void InitDirect3DApp::DrawScene()
{
	md3dDeviceContext->ClearRenderTargetView(mRenderTargetView, Colors::Black);
	md3dDeviceContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	md3dDeviceContext->IASetInputLayout(mInputLayout);

	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	md3dDeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);

	md3dDeviceContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	md3dDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	md3dDeviceContext->VSSetShader(mVertexShader, nullptr, 0);

	md3dDeviceContext->VSSetConstantBuffers(0, 1, &mConstantBuffer);

	md3dDeviceContext->PSSetShader(mPixelShader, nullptr, 0);

	md3dDeviceContext->DrawIndexed(36, 0, 0);

	// Present the rendered image to the window.  Because the maximum frame latency is set to 1,
	// the render loop will generally be throttled to the screen refresh rate, typically around
	// 60 Hz, by sleeping the application on Present until the screen is refreshed.
	mSwapChain->Present(1, 0);
}

void InitDirect3DApp::InputAssembler()
{
	Vertex vertices[8] =
	{
		{XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White)},
		{XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Black)},
		{XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red)},
		{XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green)},
		{XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue)},
		{XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow)},
		{XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan)},
		{XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta)}
	};

	D3D11_BUFFER_DESC vbd;
	vbd.ByteWidth = sizeof(vertices);
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vsd;
	vsd.pSysMem = vertices;
	vsd.SysMemPitch = 0;
	vsd.SysMemSlicePitch = 0;

	md3dDevice->CreateBuffer(&vbd, &vsd, &mVertexBuffer);

	uint16_t indices[36] =
	{
		// front face
		0, 1, 2,
		0, 2, 3,

		// back face
		4, 6, 5,
		4, 7, 6,

		// left face
		4, 5, 1,
		4, 1, 0,

		// right face
		3, 2, 6,
		3, 6, 7,

		// top face
		1, 5, 6,
		1, 6, 2,

		// bottom face
		4, 0, 3,
		4, 3, 7
	};

	D3D11_BUFFER_DESC ibd;
	ibd.ByteWidth = sizeof(indices);
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA isd;
	isd.pSysMem = indices;
	isd.SysMemPitch = 0;
	isd.SysMemSlicePitch = 0;
	
	md3dDevice->CreateBuffer(&ibd, &isd, &mIndexBuffer);

	D3D11_BUFFER_DESC cbd;
	cbd.ByteWidth = sizeof(ConstantBuffer);
	cbd.Usage = D3D11_USAGE_DEFAULT;
	cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbd.CPUAccessFlags = 0;
	cbd.MiscFlags = 0;
	cbd.StructureByteStride = 0;

	md3dDevice->CreateBuffer(&cbd, nullptr, &mConstantBuffer);

	ID3DBlob* vertexShader = LoadShader("Shaders/VertexShader.cso");
	ID3DBlob* pixelShader = LoadShader("Shaders/PixelShader.cso");

	md3dDevice->CreateVertexShader(vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), nullptr, &mVertexShader);

	D3D11_INPUT_ELEMENT_DESC vertexDesc[2] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	md3dDevice->CreateInputLayout(vertexDesc, 2, vertexShader->GetBufferPointer(), vertexShader->GetBufferSize(), &mInputLayout);

	md3dDevice->CreatePixelShader(pixelShader->GetBufferPointer(), pixelShader->GetBufferSize(), nullptr, &mPixelShader);
}

ID3DBlob* InitDirect3DApp::LoadShader(const string& filename)
{
	ifstream ifs(filename, ios::binary);

	ifs.seekg(0, ios::end);
	ifstream::pos_type size = ifs.tellg();
	ifs.seekg(0, ios::beg);

	ID3DBlob* pBlob;
	D3DCreateBlob(size, &pBlob);

	ifs.read((char*)pBlob->GetBufferPointer(), size);
	ifs.close();

	return pBlob;
}

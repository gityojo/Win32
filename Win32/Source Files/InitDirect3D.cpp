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
	ID3D11RasterizerState* mRasterizerState;

	XMMATRIX mBoxWorld;
	XMMATRIX mSphereWorld;
	XMMATRIX mFbx1World;
	XMMATRIX mFbx2World;

	XMMATRIX mView;
	XMMATRIX mProj;

	UINT mBoxVertexOffset;
	UINT mSphereVertexOffset;
	UINT mFbxVertexOffset;

	UINT mBoxIndexOffset;
	UINT mSphereIndexOffset;
	UINT mFbxIndexOffset;

	UINT mBoxIndexCount;
	UINT mSphereIndexCount;
	UINT mFbxIndexCount;

	float angle = 0.0f;
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

	mBoxWorld = I;
	mSphereWorld = I;
	mFbx1World = I;
	mFbx2World = I;

	mView = I;
	mProj = I;
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

	mProj = XMMatrixPerspectiveFovLH(0.25f * XM_PI, AspectRatio(), 1.0f, 1000.0f);
}

void InitDirect3DApp::UpdateScene(float dt)
{
	// Build the view matrix.
	XMVECTOR eyePosition = XMVectorSet(0.0f, 0.0f, -7.0f, 1.0f);
	XMVECTOR focusPosition = XMVectorZero();
	XMVECTOR upDirection = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

	mView = XMMatrixLookAtLH(eyePosition, focusPosition, upDirection);
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

	md3dDeviceContext->PSSetShader(mPixelShader, nullptr, 0);

	md3dDeviceContext->RSSetState(mRasterizerState);

	ConstantBuffer cb;
	XMMATRIX worldViewProj;

	angle += 0.01f;

	mBoxWorld = XMMatrixTranslation(0.0f, -1.0f, 0.0f) * XMMatrixRotationY(-angle);
	worldViewProj = mBoxWorld * mView * mProj;

	// Update the constant buffer with the latest worldViewProj matrix.
	XMStoreFloat4x4(&cb.WorldViewProj, XMMatrixTranspose(worldViewProj));
	md3dDeviceContext->UpdateSubresource(mConstantBuffer, 0, nullptr, &cb, 0, 0);
	md3dDeviceContext->VSSetConstantBuffers(0, 1, &mConstantBuffer);

	md3dDeviceContext->DrawIndexed(mBoxIndexCount, mBoxIndexOffset, mBoxVertexOffset);
	
	mSphereWorld = XMMatrixTranslation(0.0f, 1.0f, 0.0f);
	worldViewProj = mSphereWorld * mView * mProj;

	// Update the constant buffer with the latest worldViewProj matrix.
	XMStoreFloat4x4(&cb.WorldViewProj, XMMatrixTranspose(worldViewProj));
	md3dDeviceContext->UpdateSubresource(mConstantBuffer, 0, nullptr, &cb, 0, 0);
	md3dDeviceContext->VSSetConstantBuffers(0, 1, &mConstantBuffer);

	md3dDeviceContext->DrawIndexed(mSphereIndexCount, mSphereIndexOffset, mSphereVertexOffset);

	// Present the rendered image to the window.  Because the maximum frame latency is set to 1,
	// the render loop will generally be throttled to the screen refresh rate, typically around
	// 60 Hz, by sleeping the application on Present until the screen is refreshed.
	mSwapChain->Present(1, 0);
}

void InitDirect3DApp::InputAssembler()
{
	vector<Vertex> vertices =
	{
		{XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::White)},
		{XMFLOAT3(-1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Violet)},
		{XMFLOAT3(+1.0f, +1.0f, -1.0f), XMFLOAT4(Colors::Red)},
		{XMFLOAT3(+1.0f, -1.0f, -1.0f), XMFLOAT4(Colors::Green)},
		{XMFLOAT3(-1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Blue)},
		{XMFLOAT3(-1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Yellow)},
		{XMFLOAT3(+1.0f, +1.0f, +1.0f), XMFLOAT4(Colors::Cyan)},
		{XMFLOAT3(+1.0f, -1.0f, +1.0f), XMFLOAT4(Colors::Magenta)}
	};

	vector<uint16_t> indices =
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

	GeometryGenerator generator;
	GeometryGenerator::MeshData mesh = generator.CreateSphere(1.0f, 60, 60);

	for (uint32_t i = 0; i < mesh.Vertices.size(); i++)
	{
		vertices.push_back(Vertex({mesh.Vertices[i].Position, XMFLOAT4(Colors::LimeGreen)}));
	}

	indices.insert(indices.end(), mesh.GetIndices16().begin(), mesh.GetIndices16().end());

	FbxManager* manager = FbxManager::Create();
	FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
	manager->SetIOSettings(ios);
	FbxScene* scene = FbxScene::Create(manager, "");

	manager->Destroy();

	mBoxIndexCount = 36;
	mSphereIndexCount = (UINT)mesh.Indices32.size();
	mFbxIndexCount = 0;

	mBoxIndexOffset = 0;
	mSphereIndexOffset = mBoxIndexOffset + mBoxIndexCount;
	mFbxIndexOffset = mSphereIndexOffset + mSphereIndexCount;

	mBoxVertexOffset = 0;
	mSphereVertexOffset = mBoxVertexOffset + 8;
	mFbxVertexOffset = mSphereVertexOffset + (UINT)mesh.Vertices.size();

	D3D11_BUFFER_DESC vbd;
	vbd.ByteWidth = sizeof(Vertex) * (UINT)vertices.size();
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA vsd;
	vsd.pSysMem = vertices.data();
	vsd.SysMemPitch = 0;
	vsd.SysMemSlicePitch = 0;

	md3dDevice->CreateBuffer(&vbd, &vsd, &mVertexBuffer);

	D3D11_BUFFER_DESC ibd;
	ibd.ByteWidth = sizeof(uint16_t) * (UINT)indices.size();
	ibd.Usage = D3D11_USAGE_DEFAULT;
	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags = 0;
	ibd.MiscFlags = 0;
	ibd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA isd;
	isd.pSysMem = indices.data();
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

	D3D11_RASTERIZER_DESC rd;
	ZeroMemory(&rd, sizeof(D3D11_RASTERIZER_DESC));
	rd.FillMode = D3D11_FILL_SOLID;
	rd.CullMode = D3D11_CULL_BACK;
	rd.DepthClipEnable = TRUE;

	md3dDevice->CreateRasterizerState(&rd, &mRasterizerState);
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

#pragma once
#include "CompleteHeader.h"

class DirectXToy : public IGameApp
{
public:
	DirectXToy(void) {}

	virtual void Startup(void) override;
	virtual void Cleanup(void) override;
	virtual bool IsDone(void) override;

	virtual void Update(float deltaT) override;
	virtual void RenderScene(void) override;

public:
	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> GetStaticSamplers() const;
private:

	//Camera m_Camera;
	//std::unique_ptr<CameraController> m_CameraController;

	D3D12_VIEWPORT m_MainViewport;
	D3D12_RECT m_MainScissor;
	ComPtr<IDXGIFactory4> iDXGIFactory_;
	ComPtr<IDXGIAdapter3> iDXGIAdapter_;
	ComPtr<IDXGISwapChain4> iDXGISwapChain_;

	ComPtr<ID3D12Device> device_;
	ComPtr<ID3D12CommandAllocator> commandAllocator_;
	ComPtr<ID3D12CommandQueue> commandQueue_;
	ComPtr<ID3D12GraphicsCommandList> commandList_;
	ComPtr<ID3D12Fence> fence_;
	UINT64 fenceValue_{};
	static constexpr UINT64 InitialFenceValue = 0;

	struct DescriptorHandleAccesor
	{
		DescriptorHandleAccesor(ID3D12DescriptorHeap* descriptorHeap, UINT handleIncrementSize) : 
			source_{ descriptorHeap }, handleIncrementSize_{ handleIncrementSize } {}
		CD3DX12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(int index) const;
		CD3DX12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(int index) const;

		ID3D12DescriptorHeap* source_{ nullptr };
		UINT handleIncrementSize_;
	};
	std::map<ID3D12DescriptorHeap*, DescriptorHandleAccesor> descriptorHandleAccesors_;

	ComPtr<ID3D12DescriptorHeap> descriptorHeapRTV_;
	UINT32 rtvHandleIncrementSize_{};
	ComPtr<ID3D12DescriptorHeap> descriptorHeapDSV_;
	UINT32 dsvHandleIncrementSize_{};
	ComPtr<ID3D12DescriptorHeap> descriptorHeapCBVSRVUAV_;
	UINT32 cbvHandleIncrementSize_{};
	UINT descriptorHeapSize_{ 256 };

	ComPtr<ID3D12RootSignature> rootSignature1_;

	enum class PSO
	{
		StaticMesh,
		SkinnedMesh,
		StaticMeshShadow,
		SkinnedMeshShadow,
	};

	using GraphicsPSOMap = std::map<PSO, ComPtr<ID3D12PipelineState>>;
	using GraphicsPSODescMap = std::map<PSO, D3D12_GRAPHICS_PIPELINE_STATE_DESC>;
	GraphicsPSOMap psoMap_;
	GraphicsPSODescMap psoDescMap_;

	enum class Shader
	{
		StaticMeshVS,
		StaticMeshPS,
		SkinnedMeshVS,
		SkinnedMeshPS,

		TestVS,
		TestPS,
	};
	using ShaderMap = std::map<Shader, ComPtr<ID3DBlob>>;
	ShaderMap shaderMap_;

	enum class InputElement
	{
		StaticMesh,
		SkinnedMesh,
	};

	using InputElementsMap = std::map<InputElement, std::vector<D3D12_INPUT_ELEMENT_DESC>>;
	InputElementsMap inputElementsMap_;


	//ModelInstance m_ModelInst;
	//ShadowCamera m_SunShadowCamera;
	float elapsedTime_{};
	bool isDone_{ false };

	//for FPS Counting
	unsigned int frameCount_{};
	float frameTimeCount_{};

public:
	void LoadTexture(/*...*/);
	void LoadMesh(/*...*/);

	struct Texture
	{
		std::string name_;
		std::wstring filePath_;

		ComPtr<ID3D12Resource> resource_;
		ComPtr<ID3D12Resource> uploadHeap_;
	};
	std::map<std::string, Texture> textures_;
public:
	static ComPtr<ID3D12Resource> CreateDefaultBuffer(
		ID3D12Device* device,
		ID3D12GraphicsCommandList* cmdList,
		const void* initData,
		UINT64 byteSize,
		ComPtr<ID3D12Resource>& uploadBuffer);

	struct VertexBuffer
	{
		ComPtr<ID3D12Resource> defaultVertexBuffer_; //gpu
		ComPtr<ID3D12Resource> uploadVertexBuffer_; //gpu for upload
		ComPtr<ID3DBlob*> rawVertexBuffer_;//cpu
		
		ComPtr<ID3D12Resource> defaultIndexBuffer_; //gpu
		ComPtr<ID3D12Resource> uploadIndexBuffer_; //gpu for upload
		ComPtr<ID3DBlob*> rawVertexIndexBuffer_;//cpu

		UINT vertexByteStride_{};
		UINT vertexBufferByteSize_{};
		DXGI_FORMAT indexFormat_{ DXGI_FORMAT_R16_UINT };
		UINT indexBufferByteSize_{};
	};
	

	//왠만하면 InputLayout과 호환되도록.
	struct Vertex
	{
		Vertex() {}
		Vertex(
			const XMFLOAT3& p,
			const XMFLOAT3& n,
			const XMFLOAT3& t,
			const XMFLOAT2& uv) :
			Position(p),
			Normal(n),
			TangentU(t),
			TexC(uv) {}
		Vertex(
			float px, float py, float pz,
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v) :
			Position(px, py, pz),
			Normal(nx, ny, nz),
			TangentU(tx, ty, tz),
			TexC(u, v) {}

		XMFLOAT3 Position;
		XMFLOAT3 Normal;
		XMFLOAT3 TangentU;
		XMFLOAT2 TexC;
	};
};

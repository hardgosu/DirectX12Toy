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
		ComPtr<ID3D12Resource>& uploadBuffer,
		bool flushCommandList = false); //TODO : if (flushCommandList)

	//왠만하면 InputLayout과 호환되도록.
	struct Vertex
	{
		Vertex() {}
		Vertex(
			const XMFLOAT3& p,
			const XMFLOAT3& n,
			const XMFLOAT3& t,
			const XMFLOAT2& uv) :
			position_(p),
			normal_(n),
			tangentU_(t),
			texC_(uv) {}
		Vertex(
			float px, float py, float pz,
			float nx, float ny, float nz,
			float tx, float ty, float tz,
			float u, float v) :
			position_(px, py, pz),
			normal_(nx, ny, nz),
			tangentU_(tx, ty, tz),
			texC_(u, v) {}

		XMFLOAT3 position_;
		XMFLOAT3 normal_;
		XMFLOAT3 tangentU_;
		XMFLOAT2 texC_;
	};
	// skinnedVertex?

	//편의를 위해 VertexBuffer와 VertexBufferView가 1:1 관계라고 가정
	//원래 1:다 관계
	//모든 포맷의 정점데이터를 수용할수있다.
	struct VertexBuffer
	{
		using Byte = UINT8;
		//64는 임의의 정점 크기
		static constexpr unsigned ReservedVBSize = 64 * 1024 * 1024;
		static constexpr unsigned ReservedIBSIze = sizeof(UINT16) * 1024 * 1024;

		ComPtr<ID3D12Resource> defaultVertexBuffer_; //gpu
		ComPtr<ID3D12Resource> uploadVertexBuffer_; //gpu for upload
		std::unique_ptr<Byte[]> cpuVertexBuffer_{ nullptr };
		size_t vbSize_{};

		ComPtr<ID3D12Resource> defaultIndexBuffer_; //gpu
		ComPtr<ID3D12Resource> uploadIndexBuffer_; //gpu for upload
		std::unique_ptr<Byte[]> cpuIndexBuffer_{ nullptr };
		size_t ibSize_{};


		VertexBuffer()
		{
			cpuVertexBuffer_ = std::make_unique<Byte[]>(ReservedVBSize);
			cpuIndexBuffer_ = std::make_unique<Byte[]>(ReservedIBSIze);
		}

		void AddToVB(const void* pData, size_t byteSize)
		{
			ASSERT(ReservedVBSize >= byteSize + vbSize_);

			std::memcpy(cpuVertexBuffer_.get() + vbSize_, pData, byteSize);
			vbSize_ += byteSize;
		}

		void AddToIB(const void* pData, size_t byteSize)
		{
			ASSERT(ReservedIBSIze >= byteSize + ibSize_);

			std::memcpy(cpuVertexBuffer_.get() + ibSize_, pData, byteSize);
			ibSize_ += byteSize;
		}

		void Confirm(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList, bool clearData = false);
	};
	std::map<std::string, VertexBuffer> vertexBufferPool_; //1~2개정도만 만들면 충분..
public:
	void LoadRenderItem();
	struct Mesh
	{
		D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const
		{
			D3D12_VERTEX_BUFFER_VIEW vbv;
			vbv.BufferLocation = desc_.sourceBuffer_->defaultVertexBuffer_->GetGPUVirtualAddress();
			vbv.StrideInBytes = desc_.vertexByteSize_;
			vbv.SizeInBytes = desc_.vertexBufferByteSize_;

			return vbv;
		}

		D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const
		{
			D3D12_INDEX_BUFFER_VIEW ibv;
			ibv.BufferLocation = desc_.sourceBuffer_->defaultIndexBuffer_->GetGPUVirtualAddress();
			ibv.Format = desc_.indexFormat_;
			ibv.SizeInBytes = desc_.indexBufferByteSize_;

			return ibv;
		}

		struct Desc
		{
			//My View
			UINT vertexByteSize_{};
			UINT vertexBufferByteSize_{};
			DXGI_FORMAT indexFormat_{ DXGI_FORMAT_R16_UINT };
			UINT indexBufferByteSize_{};

			UINT indexCount_{};
			UINT startIndexLocation_{};
			INT baseVertexLocation_{};

			VertexBuffer* sourceBuffer_{ nullptr };
		};
		Desc desc_;

		Mesh() {};
		Mesh(const Desc& desc) : desc_{ desc }
		{
		};
	};
	using MeshMap = std::map<std::string, Mesh>;
	MeshMap meshMap_;

public:
	struct Material
	{
		XMFLOAT4 diffuseAlbedo_{ 1.0f, 1.0f, 1.0f, 1.0f };
		XMFLOAT3 fresnelR0_{ 0.01f, 0.01f, 0.01f };
		float roughness_{ 0.5f };
		XMFLOAT4X4 materialTransform_ = MathHelper::Identity4x4();
		UINT normalMapIndex_{};
		UINT diffuseMapIndex_{};
		UINT specularMapIndex_{};
		UINT roughnesMapIndex_{};
	};
	enum MaterialKind
	{
		Standard,
		Glossy,
		Metalic,
	};
	using MaterialMap = std::map<MaterialKind, Material>;
	MaterialMap materialMapCPU_;

	struct InstanceData
	{
		XMFLOAT4X4 worldMatrix_ = MathHelper::Identity4x4();
		XMFLOAT4X4 texTransform_ = MathHelper::Identity4x4();
		UINT materialIndex_{};
		UINT pad_{};
		UINT pad2_{};
		UINT pad3_{};
	};

	struct Light
	{
		XMFLOAT3 strength_{ 0.5f, 0.5f, 0.5f };
		float falloffStart_{ 1.0f };					// point/spot light only
		XMFLOAT3 direction_{ 0.0f, -1.0f, 0.0f };		// directional/spot light only
		float falloffEnd_{ 10.0f };						// point/spot light only
		XMFLOAT3 position_{ 0.0f, 0.0f, 0.0f };			// point/spot light only
		float spotPower_{ 64.0f };						// spot light only
	};

	static constexpr unsigned MaxLights = 16;

	struct ConstantBuffer1
	{
		XMFLOAT4X4 viewMatrix_;
		XMFLOAT4X4 projectionMatrix_;
		XMFLOAT4X4 shadowMatrix_;
		XMFLOAT4X4 textureMatrix_;
		XMFLOAT3 eyePosW_{ 0.0f, 0.0f, 0.0f };
		float pad_;
		float totalTime_;
		float deltaTime_;
		XMFLOAT2 pad2_;
		Light lights_[MaxLights];
		XMFLOAT4 ambientLight_{ 0.0f, 0.0f, 0.0f, 1.0f };
		XMFLOAT2 renderTargetSize_{ 0.0f, 0.0f };
		XMFLOAT2 inverseRenderTargetSize_{ 0.0f, 0.0f };
	};

	struct InstancingRenderItem
	{
		struct Desc
		{
			ID3D12PipelineState* pso_{ nullptr };
			Mesh* mesh_{ nullptr };
			UINT instanceBufferCount_{};
		};
		Desc desc_;
		std::vector<InstanceData> cpuInstanceBuffer_; //before to Set, write to GPU Mapped Data(UploadBufer)
		UINT visibleItemCount_{};

		InstancingRenderItem(const Desc& desc) : desc_{ desc }
		{
			ASSERT(desc_.pso_ != nullptr);
			ASSERT(desc_.mesh_ != nullptr);
			ASSERT(desc_.instanceBufferCount_ >= 0);
			cpuInstanceBuffer_.resize(desc_.instanceBufferCount_);
		}
	};
	std::vector<InstancingRenderItem> renderItems_;

	struct PassData //FR
	{
		std::unique_ptr<UploadBuffer<Material>> materialBuffer_;
		std::unique_ptr<UploadBuffer<InstanceData>> instanceBuffer_;
		std::unique_ptr<UploadBuffer<ConstantBuffer1>> constantBuffer_;
		UINT64 fence_{};
	};
	PassData passData_;
public:
	struct Camera
	{
		//중심축
		enum class Axis
		{
			X,
			Y,
			Z,
		};

		void Rotate(Axis axis, float angle);
		void SetPosition(const XMFLOAT3& position);
		const XMFLOAT4X4& GetViewMatrix();
		const XMFLOAT4X4& GetProjMatrix();
		void SetProjMatrix(float fovY = MathHelper::Pi * 0.25f, float aspect = 1.0f, float nearZ = 1.0f, float farZ = 1000.0f);
	private:
		XMFLOAT3 right_{ 1.0f, 0, 0 };
		XMFLOAT3 up_{ 0, 1.0f, 0 };
		XMFLOAT3 look_{ 0, 0, 1.0f };
		XMFLOAT3 position_{ 0, 0, 0 };

		XMFLOAT4X4 view_ = MathHelper::Identity4x4();
		XMFLOAT4X4 proj_ = MathHelper::Identity4x4();

		float fovY_{ MathHelper::Pi * 0.25f };
		float aspect_{ 1.0f };
		float nearZ_{ 1.0f };
		float farZ_{ 1000.0f };

		bool viewDirty_{ false };
		bool projDirty_{ false };
	};
	Camera camera_;
};

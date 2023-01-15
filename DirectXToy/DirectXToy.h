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
	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers() const;
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
	};
	using ShaderMap = std::map<Shader, D3D12_SHADER_BYTECODE>;
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
};

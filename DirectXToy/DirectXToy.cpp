#include "DirectXToy.h"

extern HWND g_hWnd;
extern uint32_t g_DisplayWidth;
extern uint32_t g_DisplayHeight;

void DirectXToy::Startup()
{
	using Adapter = IDXGIAdapter1;
	using Factory = IDXGIFactory4;
	using SwapChain = IDXGISwapChain3;
	using Device = decltype(device_);

	auto createDeviceWithBestAdapter = [](Factory* factory, Device& outputDevice)
	{
		//enum adapters
		ComPtr<Adapter> bestAdapter;
		ComPtr<Adapter> iterAdapter;
		SIZE_T MaxSize = 0;
		for (UINT i{}; factory->EnumAdapters1(i, &iterAdapter) != DXGI_ERROR_NOT_FOUND; ++i)
		{
			DXGI_ADAPTER_DESC1 desc;
			iterAdapter->GetDesc1(&desc);
			//소프트웨어 어댑터인가?
			if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
			{
				continue;
			}

			if (desc.DedicatedVideoMemory < MaxSize)
			{
				continue;
			}

			MaxSize = desc.DedicatedVideoMemory;
			bestAdapter.Swap(iterAdapter.Get());
		}

		std::vector<D3D_FEATURE_LEVEL> featureLevels
		{
			D3D_FEATURE_LEVEL_12_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
		};
		for (auto feature : featureLevels)
		{
			if (auto result = D3D12CreateDevice(bestAdapter.Get(), feature, IID_PPV_ARGS(&outputDevice)); result == S_OK)
			{
				return;
			}
		}

		factory->EnumWarpAdapter(IID_PPV_ARGS(&bestAdapter));
		for (auto feature : featureLevels)
		{
			if (auto result = D3D12CreateDevice(bestAdapter.Get(), feature, IID_PPV_ARGS(&outputDevice)); result == S_OK)
			{
				return;
			}
		}
	};
	//init DXGI Factory
	ASSERT_SUCCEEDED(CreateDXGIFactory(IID_PPV_ARGS(&iDXGIFactory_)));

	createDeviceWithBestAdapter(iDXGIFactory_.Get(), device_);
	ASSERT(device_ != nullptr);

	auto commandListType = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT;
	HRESULT result;
	ASSERT_SUCCEEDED(device_->CreateCommandAllocator(commandListType, IID_PPV_ARGS(&commandAllocator_)));
	D3D12_COMMAND_QUEUE_DESC queueDesc{};
	queueDesc.NodeMask = 1;
	queueDesc.Type = commandListType;
	ASSERT_SUCCEEDED(device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue_)));
	ASSERT_SUCCEEDED(device_->CreateCommandList(0, commandListType, commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_)));
	ASSERT_SUCCEEDED(device_->CreateFence(InitialFenceValue, D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_)));

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = descriptorHeapSize_;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	ASSERT_SUCCEEDED(device_->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeapDSV_)));
	dsvHandleIncrementSize_ = device_->GetDescriptorHandleIncrementSize(heapDesc.Type);

	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	ASSERT_SUCCEEDED(device_->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeapRTV_)));
	rtvHandleIncrementSize_ = device_->GetDescriptorHandleIncrementSize(heapDesc.Type);

	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	ASSERT_SUCCEEDED(device_->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeapCBVSRVUAV_)));
	cbvHandleIncrementSize_ = device_->GetDescriptorHandleIncrementSize(heapDesc.Type);

	using RootParameter = CD3DX12_ROOT_PARAMETER; //signature 1.0
	constexpr unsigned NumRootParameter = 4;
	std::array<RootParameter, NumRootParameter> parameters;

	parameters[0].InitAsConstantBufferView(0);
	parameters[1].InitAsShaderResourceView(0);
	parameters[2].InitAsShaderResourceView(1);
	parameters[3].InitAsConstants(32, 1);

	auto staticSamplers = GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(NumRootParameter, parameters.data(), staticSamplers.size(), staticSamplers.data());
	ComPtr<ID3DBlob> pOutBlob, pErrorBlob;
	ASSERT_SUCCEEDED(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION::D3D_ROOT_SIGNATURE_VERSION_1_0, &pOutBlob, &pErrorBlob));
	ASSERT_SUCCEEDED(device_->CreateRootSignature(0, pOutBlob->GetBufferPointer(), pOutBlob->GetBufferSize(), IID_PPV_ARGS(&rootSignature1_)));

	auto buildInputElements = [](InputElementsMap& outputMap)
	{
		std::vector<D3D12_INPUT_ELEMENT_DESC> staticMesh
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		//TODO
		std::vector<D3D12_INPUT_ELEMENT_DESC> staticMeshWorld
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			//{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 0 },
		};
	};

	using RootSignature = ComPtr<ID3D12RootSignature>;
	auto buildPSODesc = [](GraphicsPSODescMap& outputMap, const RootSignature& rootSignature)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC staticMesh{};
		staticMesh.NumRenderTargets = 1;
		staticMesh.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		staticMesh.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		staticMesh.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		staticMesh.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		staticMesh.pRootSignature = rootSignature.Get();
		std::string staticMeshVS =
			"g\n"
			"g\n";
		//staticMesh.VS = D3D12_
		//staticMesh.PS
		CompileShader(L"Shader/shaders.hlsl", nullptr, "VSInstancing", "vs_5_1");

	};

	auto buildPSO = [](GraphicsPSOMap& outputMap)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC staticMesh;

	};

	buildInputElements(inputElementsMap_);
	buildPSODesc(psoDescMap_, rootSignature1_);
	buildPSO(psoMap_);

	//psoMap_[PSO::StaticMesh] = CD3DX12_PIPLINE_STATE_
	//device_->CreateGraphicsPipelineState()
}

void DirectXToy::Cleanup()
{

}

void DirectXToy::Update(float deltaT)
{
	//Begin
	elapsedTime_ = deltaT;
	auto calculateFPS = [this](float deltaT)
	{
		++frameCount_;

		frameTimeCount_ += deltaT;
		if (frameTimeCount_ >= 1.0f)
		{
			std::wstring fpsText = L"FPS : ";
			fpsText += std::to_wstring(frameCount_);

			SetWindowText(g_hWnd, fpsText.c_str());

			frameTimeCount_ = 0;
			frameCount_ = 0;
		}
	};
	calculateFPS(deltaT);

	//ProcessInput(Interrupt)
	//Logic
	//Render
	RenderScene();
}

void DirectXToy::RenderScene()
{

}

bool DirectXToy::IsDone()
{
	return isDone_;
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> DirectXToy::GetStaticSamplers() const
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp };
}

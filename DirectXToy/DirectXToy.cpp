#include "DirectXToy.h"
#include "Dependent/GeometryGenerator.h"

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
	descriptorHandleAccesors_.insert(std::make_pair(descriptorHeapDSV_.Get(), 
		DescriptorHandleAccesor(descriptorHeapDSV_.Get(), dsvHandleIncrementSize_)));

	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	ASSERT_SUCCEEDED(device_->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeapRTV_)));
	rtvHandleIncrementSize_ = device_->GetDescriptorHandleIncrementSize(heapDesc.Type);
	descriptorHandleAccesors_.insert(std::make_pair(descriptorHeapRTV_.Get(),
		DescriptorHandleAccesor(descriptorHeapRTV_.Get(), rtvHandleIncrementSize_)));

	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	ASSERT_SUCCEEDED(device_->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeapCBVSRVUAV_)));
	cbvHandleIncrementSize_ = device_->GetDescriptorHandleIncrementSize(heapDesc.Type);
	descriptorHandleAccesors_.insert(std::make_pair(descriptorHeapCBVSRVUAV_.Get(),
		DescriptorHandleAccesor(descriptorHeapCBVSRVUAV_.Get(), cbvHandleIncrementSize_)));

	using RootParameter = CD3DX12_ROOT_PARAMETER; //signature 1.0
	constexpr unsigned NumRootParameter = 10;
	std::array<RootParameter, NumRootParameter> parameters;
	//성능 팁 : 자주 쓰이는 순으로 파라미터 구성
	parameters[0].InitAsConstantBufferView(0);
	parameters[1].InitAsConstantBufferView(1);
	parameters[2].InitAsConstantBufferView(2);
	parameters[3].InitAsConstantBufferView(3);
	parameters[4].InitAsShaderResourceView(0, 1);
	parameters[5].InitAsShaderResourceView(1, 1);

	CD3DX12_DESCRIPTOR_RANGE texTable1(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0);
	CD3DX12_DESCRIPTOR_RANGE texTable2(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1, 0);
	CD3DX12_DESCRIPTOR_RANGE texTable3(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 2, 0);
	CD3DX12_DESCRIPTOR_RANGE texTable4(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 40, 3, 0);
	parameters[6].InitAsDescriptorTable(1, &texTable1, D3D12_SHADER_VISIBILITY_PIXEL);
	parameters[7].InitAsDescriptorTable(1, &texTable2, D3D12_SHADER_VISIBILITY_PIXEL);
	parameters[8].InitAsDescriptorTable(1, &texTable3, D3D12_SHADER_VISIBILITY_PIXEL);
	parameters[9].InitAsDescriptorTable(1, &texTable4, D3D12_SHADER_VISIBILITY_PIXEL);

	auto staticSamplers = GetStaticSamplers();

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc(NumRootParameter, parameters.data(), staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
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
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};
		outputMap[InputElement::StaticMesh] = std::move(staticMesh);
	};

	auto buildShader = [](ShaderMap& outputMap)
	{
		outputMap[Shader::StaticMeshVS] = CompileShader(L"SamplePath.hlsl", nullptr, "VSMain", "vs_5_1");
		outputMap[Shader::StaticMeshPS] = CompileShader(L"SamplePath.hlsl", nullptr, "PSMain", "ps_5_1");
	};

	using RootSignature = ComPtr<ID3D12RootSignature>;
	auto buildPSODesc = [](GraphicsPSODescMap& outputMap, RootSignature& rootSignature, ShaderMap& shaderMap, InputElementsMap& inputElementsMap)
	{
		D3D12_GRAPHICS_PIPELINE_STATE_DESC staticMesh{};
		staticMesh.NumRenderTargets = 1;
		staticMesh.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		staticMesh.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		staticMesh.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		staticMesh.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		staticMesh.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		staticMesh.pRootSignature = rootSignature.Get();
		staticMesh.SampleMask = 0xFFFFFFFF;
		staticMesh.SampleDesc.Count = 1;
		staticMesh.SampleDesc.Quality = 0;
		staticMesh.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;

		staticMesh.VS = { shaderMap[Shader::StaticMeshVS]->GetBufferPointer(), shaderMap[Shader::StaticMeshVS]->GetBufferSize() };
		staticMesh.PS = { shaderMap[Shader::StaticMeshPS]->GetBufferPointer(), shaderMap[Shader::StaticMeshPS]->GetBufferSize() };

		staticMesh.InputLayout = { inputElementsMap[InputElement::StaticMesh].data(), 
			static_cast<UINT>(inputElementsMap[InputElement::StaticMesh].size()) };

		outputMap[PSO::StaticMesh] = staticMesh;
	};

	auto buildPSO = [](GraphicsPSOMap& outputMap, const GraphicsPSODescMap& psoDescMap, Device& device)
	{
		HRESULT hr = device->CreateGraphicsPipelineState(&psoDescMap.find(PSO::StaticMesh)->second, IID_PPV_ARGS(&outputMap[PSO::StaticMesh]));
		ASSERT_SUCCEEDED(hr);
	};

	buildInputElements(inputElementsMap_);
	buildShader(shaderMap_);
	buildPSODesc(psoDescMap_, rootSignature1_, shaderMap_, inputElementsMap_);
	buildPSO(psoMap_, psoDescMap_, device_);
	LoadTexture(); //어차피 재사용 불가능하면 람다화
	LoadMesh(); //어차피 재사용 불가능하면 람다화
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

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> DirectXToy::GetStaticSamplers() const
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

	const CD3DX12_STATIC_SAMPLER_DESC shadow(
		6, // shaderRegister
		D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
		0.0f,                               // mipLODBias
		16,                                 // maxAnisotropy
		D3D12_COMPARISON_FUNC_LESS_EQUAL,
		D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);

	return {
		pointWrap, pointClamp,
		linearWrap, linearClamp,
		anisotropicWrap, anisotropicClamp,
		shadow
	};
}

void DirectXToy::LoadTexture(/*...*/)
{
	std::vector<std::string> texNames =
	{
		"bricksDiffuseMap",
		"bricksNormalMap",
		"tileDiffuseMap",
		"tileNormalMap",
		"defaultDiffuseMap",
		"defaultNormalMap",
		"skyCubeMap"
	};

	std::vector<std::wstring> texFilenames =
	{
		L"Textures/bricks2.dds",
		L"Textures/bricks2_nmap.dds",
		L"Textures/tile.dds",
		L"Textures/tile_nmap.dds",
		L"Textures/white1x1.dds",
		L"Textures/default_nmap.dds",
		L"Textures/desertcube1024.dds"
	};

	ASSERT(texFilenames.size() == texNames.size());

	for (int i = 0; i < (int)texNames.size(); ++i)
	{
		auto& texture = textures_[texNames[i]] = Texture();
		texture.name_ = texNames[i];
		texture.filePath_ = texFilenames[i];
		ASSERT_SUCCEEDED(CreateDDSTextureFromFile12(device_.Get(),
			commandList_.Get(), texture.filePath_.c_str(),
			texture.resource_, texture.uploadHeap_));
	}
}

void DirectXToy::LoadMesh(/*...*/)
{
	GeometryGenerator generator;
	auto meshData = generator.CreateGeosphere(10.0f, 16);
	auto convertToMyBuffer = [this](const GeometryGenerator::MeshData& meshData, const std::string& name)
	{
		auto emplaced = meshDatas_.emplace(name);
		ASSERT(emplaced.second);
		auto& vertexBuffer = emplaced.first->second;
		vertexBuffer.cpuVertexBuffer_.resize(meshData.Vertices.size());
		vertexBuffer.cpuIndexBuffer_.resize(meshData.Indices32.size());

		for (size_t i{}; i < meshData.Vertices.size(); ++i)
		{
			auto& vertex = vertexBuffer.cpuVertexBuffer_[i];
			const auto& externVertex = meshData.Vertices[i];

			vertex.position_ = externVertex.Position;
			vertex.normal_ = externVertex.Normal;
			vertex.tangentU_ = externVertex.TangentU;
			vertex.texC_ = externVertex.TexC;
		}
		for (size_t i{}; i < meshData.Indices32.size(); ++i)
		{
			vertexBuffer.cpuIndexBuffer_[i] = meshData.Indices32[i];
		}
	};
	convertToMyBuffer(meshData, "GeoSphere");
}

CD3DX12_CPU_DESCRIPTOR_HANDLE DirectXToy::DescriptorHandleAccesor::GetCPUHandle(int index) const
{
	auto srv = CD3DX12_CPU_DESCRIPTOR_HANDLE(source_->GetCPUDescriptorHandleForHeapStart());
	srv.Offset(index, handleIncrementSize_);
	return srv;
}

CD3DX12_GPU_DESCRIPTOR_HANDLE DirectXToy::DescriptorHandleAccesor::GetGPUHandle(int index) const
{
	auto srv = CD3DX12_GPU_DESCRIPTOR_HANDLE(source_->GetGPUDescriptorHandleForHeapStart());
	srv.Offset(index, handleIncrementSize_);
	return srv;
}

static ComPtr<ID3D12Resource> CreateDefaultBuffer(
	ID3D12Device* device,
	ID3D12GraphicsCommandList* cmdList,
	const void* initData,
	UINT64 byteSize,
	ComPtr<ID3D12Resource>& uploadBuffer,
	bool flushCommandList /*false*/)
{
	ComPtr<ID3D12Resource> defaultBuffer;

	// 거의 바뀔일이 없는 데이터는 디폴트 힙에 만든다.
	ASSERT_SUCCEEDED(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(defaultBuffer.GetAddressOf())));

	// 하지만 처음 만들때는 업로드를 위해 업로드 힙이 필요한것이다.
	ASSERT_SUCCEEDED(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(uploadBuffer.GetAddressOf())));


	// 업로드 버퍼 리소스를 매핑해서 기록하거나 서브리소스 데이터를 서술해서 기록하는 방법
	// 두가지가 있다. 그중 서브리소스를 활용하는 방법이다.
	D3D12_SUBRESOURCE_DATA subResourceData = {};
	subResourceData.pData = initData;
	subResourceData.RowPitch = byteSize;
	subResourceData.SlicePitch = subResourceData.RowPitch;
	// 모든 리소스는 상태전이와 함께 복사든, 상태변경이든지 이루어져야 한다.
	// 즉, 복사 가능 상태(복사 Destination)가 되어야 복사가 가능한것이다. 복사 이후 원래의 상태 혹은
	// 아래와 같이 Read상태로 놓는다.
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST));
	//d3dx12.h 헬퍼
	UpdateSubresources<1>(cmdList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
	cmdList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
		D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	// 참고로 비동기임
	// 이후의 코드에서 커맨드리스트를 Flush해야 한다.

	return defaultBuffer;
}

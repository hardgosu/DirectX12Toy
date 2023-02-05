#include "DirectXToy.h"
#include "Independent/GeometryGenerator.h"
#include <d3d12sdklayers.h>

extern HWND g_hWnd;
extern uint32_t g_DisplayWidth;
extern uint32_t g_DisplayHeight;

void DirectXToy::Startup()
{
	ComPtr<ID3D12Debug> debugController;
	auto hrr = (D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
	debugController->EnableDebugLayer();

	auto createDeviceWithBestAdapter = [](Factory& factory, Device& outputDevice)
	{
		//enum adapters
		ComPtr<IDXGIAdapter1> bestAdapter;
		ComPtr<IDXGIAdapter1> iterAdapter;
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
	ASSERT_SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&iDXGIFactory_)));

	createDeviceWithBestAdapter(iDXGIFactory_, device_);
	ASSERT(device_ != nullptr);

	auto commandListType = D3D12_COMMAND_LIST_TYPE::D3D12_COMMAND_LIST_TYPE_DIRECT;
	HRESULT result;
	ASSERT_SUCCEEDED(device_->CreateCommandAllocator(commandListType, IID_PPV_ARGS(&mainCommandAllocator_)));
	D3D12_COMMAND_QUEUE_DESC queueDesc{};
	queueDesc.NodeMask = 1;
	queueDesc.Type = commandListType;
	ASSERT_SUCCEEDED(device_->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue_)));
	ASSERT_SUCCEEDED(device_->CreateCommandList(0, commandListType, mainCommandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_)));
	ASSERT_SUCCEEDED(device_->CreateFence(InitialFenceValue, D3D12_FENCE_FLAGS::D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_)));
	commandList_->Close();

	D3D12_DESCRIPTOR_HEAP_DESC heapDesc{};
	heapDesc.NodeMask = 0;
	heapDesc.NumDescriptors = 2;  //필요하다면 더 만드세요 : ex 거울반사, 그림자 매핑 외
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAGS::D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	ASSERT_SUCCEEDED(device_->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeapDSV_)));
	dsvHandleIncrementSize_ = device_->GetDescriptorHandleIncrementSize(heapDesc.Type);
	descriptorHandleAccesors_.insert(std::make_pair(descriptorHeapDSV_.Get(), 
		DescriptorHandleAccesor(descriptorHeapDSV_.Get(), dsvHandleIncrementSize_)));

	heapDesc.NumDescriptors = SwapChainCount; //필요하다면 더 만드세요. : ex 큐브매핑, 지연 렌더링
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	ASSERT_SUCCEEDED(device_->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&descriptorHeapRTV_)));
	rtvHandleIncrementSize_ = device_->GetDescriptorHandleIncrementSize(heapDesc.Type);
	descriptorHandleAccesors_.insert(std::make_pair(descriptorHeapRTV_.Get(),
		DescriptorHandleAccesor(descriptorHeapRTV_.Get(), rtvHandleIncrementSize_)));

	heapDesc.NumDescriptors = srvDescriptorHeapSize_;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE::D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
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
		outputMap[Shader::StaticMeshVS] = CompileShader(L"Shaders/SamplePath.hlsl", nullptr, "VSMain", "vs_5_1");
		outputMap[Shader::StaticMeshPS] = CompileShader(L"Shaders/SamplePath.hlsl", nullptr, "PSMain", "ps_5_1");
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
	ResetSwapChain(commandList_.Get(), mainCommandAllocator_.Get());
	LoadTexture(commandList_.Get(), mainCommandAllocator_.Get()); //어차피 재사용 불가능하면 람다화
	LoadMesh(commandList_.Get(), mainCommandAllocator_.Get()); //어차피 재사용 불가능하면 람다화
	LoadRenderItem(); //어차피 재사용 불가능하면 람다화

	auto prepareScene = [this]()
	{
		camera_.SetPosition(XMFLOAT3(0, -3.0f, -10.0f));
	};
	prepareScene();
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
	currentPassDataIndex_ = (currentPassDataIndex_ + 1) % NumFrameResource;
	auto& currentPassData = passData_[currentPassDataIndex_];
	if (currentPassData.fence_ > fence_->GetCompletedValue())
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ASSERT_SUCCEEDED(fence_->SetEventOnCompletion(currentPassData.fence_, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	auto beginRenderPass = [this, &currentPassData]()
	{
		ASSERT_SUCCEEDED(currentPassData.commandAllocator_->Reset());
		std::vector<ID3D12GraphicsCommandList*> commandLists
		{
			commandList_.Get(),
		};
		std::for_each(commandLists.begin(), commandLists.end(), [this, &currentPassData](auto& elem) { elem->Reset(currentPassData.commandAllocator_.Get(), nullptr); });
	};
	beginRenderPass();

	std::vector<std::function<void()>> renderPasses
	{
		[this, &currentPassData]() //Sample pass
		{
			std::vector <ID3D12DescriptorHeap*> descriptorHeaps
			{
				descriptorHeapCBVSRVUAV_.Get(),
			};
			commandList_->SetDescriptorHeaps(descriptorHeaps.size(), descriptorHeaps.data());
			commandList_->SetGraphicsRootSignature(rootSignature1_.Get());


			commandList_->SetGraphicsRootConstantBufferView(0, currentPassData.constantBuffer_->Resource()->GetGPUVirtualAddress());
			commandList_->SetGraphicsRootConstantBufferView(1, currentPassData.instanceBuffer_->Resource()->GetGPUVirtualAddress());
			//commandList_->SetGraphicsRootConstantBufferView(2, )
			//commandList_->SetGraphicsRootConstantBufferView(3, )
			commandList_->SetGraphicsRootShaderResourceView(4, currentPassData.materialBuffer_->Resource()->GetGPUVirtualAddress());
			//commandList_->SetGraphicsRootDescriptorTable(6)
			//commandList_->SetGraphicsRootDescriptorTable(7)
			commandList_->SetGraphicsRootDescriptorTable(8, 
				descriptorHandleAccesors_[descriptorHeapCBVSRVUAV_.Get()].GetGPUHandle(commonPassData_.cubemapSRVIndex_));
			commandList_->SetGraphicsRootDescriptorTable(9, 
				descriptorHandleAccesors_[descriptorHeapCBVSRVUAV_.Get()].GetGPUHandle(0));
			// 드로우 콜
		},

		[this]() //Shadow pass
		{

		},

		[this, &currentPassData]() //Test
		{
			commandList_->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentSwapChainBuffer(),
				D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
			commandList_->RSSetViewports(1, &mainViewport_);
			commandList_->RSSetScissorRects(1, &mainScissor_);
			auto currentRenderTargetView = descriptorHandleAccesors_[descriptorHeapRTV_.Get()].GetCPUHandle(currentBackBufferIndex_);
			auto depthStencilView = descriptorHandleAccesors_[descriptorHeapDSV_.Get()].GetCPUHandle(0);
			commandList_->ClearRenderTargetView(currentRenderTargetView, Colors::LightPink, 0, nullptr);
			commandList_->ClearDepthStencilView(depthStencilView, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
			
			//Do Draw Call

			commandList_->OMSetRenderTargets(1, &currentRenderTargetView, true, &depthStencilView);
			commandList_->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentSwapChainBuffer(),
				D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
		},
	};
	std::for_each(renderPasses.begin(), renderPasses.end(), [this](auto& elem) { elem(); });

	auto endRenderPass = [this, &currentPassData]()
	{
		//backbuffer transition
		//execute
		std::vector<ID3D12GraphicsCommandList*> commandLists
		{
			commandList_.Get(),
		};
		ExecuteCommandList(commandLists, fence_.Get(), commandQueue_.Get(), mainFenceValue_, false);
		currentPassData.fence_ = mainFenceValue_;
		iDXGISwapChain_->Present(0, 0);
		currentBackBufferIndex_ = (currentBackBufferIndex_ + 1) % SwapChainCount;
	};
	endRenderPass();
}

bool DirectXToy::IsDone()
{
	return isDone_;
}

void DirectXToy::LoadRenderItem()
{
	static constexpr int NumRenderItems = 4;
	static constexpr int InstanceBufferSize = 30;

	auto buildRenderItem = [this]()
	{
		renderItems_.reserve(NumRenderItems);

		InstancingRenderItem::Desc desc;
		desc.pso_ = psoMap_[PSO::StaticMesh].Get();
		desc.instanceBufferCount_ = InstanceBufferSize;

		desc.mesh_ = &meshMap_["GeoSphere"];
		renderItems_.push_back(desc);
		desc.mesh_ = &meshMap_["GeoSphere"];
		renderItems_.push_back(desc);
		desc.mesh_ = &meshMap_["Box"];
		renderItems_.push_back(desc);
		desc.mesh_ = &meshMap_["Box"];
		renderItems_.push_back(desc);
	};
	buildRenderItem();

	auto buildMaterial = [this]()
	{
		//아직 별 의미없음.
		materialMapCPU_[MaterialKind::Standard].diffuseAlbedo_ = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		materialMapCPU_[MaterialKind::Standard].fresnelR0_ = XMFLOAT3(0.02f, 0.02f, 0.02f);
		materialMapCPU_[MaterialKind::Standard].roughness_ = 0.3f;
		materialMapCPU_[MaterialKind::Standard].diffuseMapIndex_ = 0;


		materialMapCPU_[MaterialKind::Glossy].diffuseAlbedo_ = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		materialMapCPU_[MaterialKind::Glossy].fresnelR0_ = XMFLOAT3(0.05f, 0.05f, 0.05f);
		materialMapCPU_[MaterialKind::Glossy].roughness_ = 0.3f;
		materialMapCPU_[MaterialKind::Glossy].diffuseMapIndex_ = 1;
	};
	buildMaterial();
	std::for_each(passData_.begin(), passData_.end(), [this](auto& elem)
		{
			device_->CreateCommandAllocator(
				D3D12_COMMAND_LIST_TYPE_DIRECT,
				IID_PPV_ARGS(elem.commandAllocator_.GetAddressOf()));

			elem.instanceBuffer_ = std::make_unique<UploadBuffer<InstanceData>>(device_.Get(), InstanceBufferSize * NumRenderItems, false);
			elem.materialBuffer_ = std::make_unique<UploadBuffer<Material>>(device_.Get(), materialMapCPU_.size(), false);
			elem.constantBuffer_ = std::make_unique<UploadBuffer<ConstantBuffer1>>(device_.Get(), 1, true);
		});
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

void DirectXToy::LoadTexture(ID3D12GraphicsCommandList* commandList, ID3D12CommandAllocator* allocator)
{
	ASSERT_SUCCEEDED(allocator->Reset());
	ASSERT_SUCCEEDED(commandList->Reset(allocator, nullptr));

	std::vector<std::string> texNames =
	{
		"bricksDiffuseMap",
		"bricksNormalMap",
		"tileDiffuseMap",
		"tileNormalMap",
		"defaultDiffuseMap",
		"defaultNormalMap",
		"skyCubeMap",
	};

	std::vector<std::wstring> texFilenames =
	{
		L"Textures/bricks2.dds",
		L"Textures/bricks2_nmap.dds",
		L"Textures/tile.dds",
		L"Textures/tile_nmap.dds",
		L"Textures/white1x1.dds",
		L"Textures/default_nmap.dds",
		L"Textures/desertcube1024.dds",
	};

	ASSERT(texFilenames.size() == texNames.size());
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(descriptorHeapCBVSRVUAV_->GetCPUDescriptorHandleForHeapStart());
	auto srvDescriptorSize = descriptorHandleAccesors_[descriptorHeapCBVSRVUAV_.Get()].handleIncrementSize_;
	
	for (size_t i{}; i < texNames.size(); ++i)
	{
		auto& texture = textures_[texNames[i]] = Texture();
		texture.name_ = texNames[i];
		texture.filePath_ = texFilenames[i];
		ASSERT_SUCCEEDED(CreateDDSTextureFromFile12(device_.Get(),
			commandList_.Get(), texture.filePath_.c_str(),
			texture.resource_, texture.uploadHeap_));

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		srvDesc.Format = texture.resource_->GetDesc().Format;
		srvDesc.Texture2D.MipLevels = texture.resource_->GetDesc().MipLevels;

		device_->CreateShaderResourceView(texture.resource_.Get(), &srvDesc, hDescriptor);

		// next descriptor
		hDescriptor.Offset(1, srvDescriptorSize);
	}

	commonPassData_.cubemapSRVIndex_ = texNames.size();

	auto& skyCubeMap = textures_["skyCubeMap"].resource_;
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = skyCubeMap->GetDesc().MipLevels;
	srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
	srvDesc.Format = skyCubeMap->GetDesc().Format;
	device_->CreateShaderResourceView(skyCubeMap.Get(), &srvDesc, hDescriptor);

	ExecuteCommandList(commandList, fence_.Get(), commandQueue_.Get(), mainFenceValue_);
}

void DirectXToy::LoadMesh(ID3D12GraphicsCommandList* commandList, ID3D12CommandAllocator* allocator)
{
	ASSERT_SUCCEEDED(allocator->Reset());
	ASSERT_SUCCEEDED(commandList->Reset(allocator, nullptr));

	//1버퍼는 1메시에 대응하지 않는다.
	auto buildMesh = [](const GeometryGenerator::MeshData& meshData, VertexBuffer& vertexBuffer)
		-> Mesh
	{
		std::vector<Vertex> vertexContainer;
		std::vector<UINT16> indexContainer;

		vertexContainer.resize(meshData.Vertices.size());
		indexContainer.resize(meshData.Indices32.size());

		for (size_t i{}; i < meshData.Vertices.size(); ++i)
		{
			auto& vertex = vertexContainer[i];
			const auto& externVertex = meshData.Vertices[i];

			vertex.position_ = externVertex.Position;
			vertex.normal_ = externVertex.Normal;
			vertex.tangentU_ = externVertex.TangentU;
			vertex.texC_ = externVertex.TexC;
		}
		for (size_t i{}; i < meshData.Indices32.size(); ++i)
		{
			indexContainer[i] = static_cast<UINT16>(meshData.Indices32[i]);
		}

		std::optional<std::vector<UINT16>> indices;
		if (indexContainer.empty() == false)
		{
			indices.emplace(std::move(indexContainer));
		}
		return vertexBuffer.AddToVB(vertexContainer, indices);
	};

	auto& vertexBuffer1 = vertexBufferPool_["Main"];

	GeometryGenerator generator;
	auto meshData = generator.CreateGeosphere(10.0f, 16);
	auto meshData2 = generator.CreateBox(10.0f, 10.0f, 10.0f, 16);

	struct MeshData
	{
		std::string name_;
		const GeometryGenerator::MeshData* pData_;
	};
	using MeshDataList = std::map<std::string, const GeometryGenerator::MeshData*>;
	MeshDataList meshDataList
	{
		{"GeoSphere", &meshData },
		{"Box", &meshData2 },
	};

	for (const auto& [meshName, pMeshData] : meshDataList)
	{
		meshMap_[meshName] = buildMesh(*pMeshData, vertexBuffer1);
	}

	vertexBuffer1.Confirm(device_.Get(), commandList_.Get());
	ExecuteCommandList(commandList, fence_.Get(), commandQueue_.Get(), mainFenceValue_);
}
//TODO : Signature 추가 및 개선
void DirectXToy::ExecuteCommandList(std::vector<ID3D12GraphicsCommandList*>& commandLists, ID3D12Fence* fence,
	ID3D12CommandQueue* commandQueue, UINT64& fenceValue, bool sync/*true*/) const
{
	std::for_each(commandLists.begin(), commandLists.end(), [](auto& elem)
		{
			elem->Close();
		});
	commandQueue->ExecuteCommandLists(commandLists.size(), reinterpret_cast<ID3D12CommandList* const*>(commandLists.data()));
	ASSERT_SUCCEEDED(commandQueue->Signal(fence, ++fenceValue));
	if (sync && fence->GetCompletedValue() < fenceValue)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ASSERT_SUCCEEDED(fence->SetEventOnCompletion(fenceValue, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void DirectXToy::ExecuteCommandList(ID3D12GraphicsCommandList* commandList, ID3D12Fence* fence,
	ID3D12CommandQueue* commandQueue, UINT64& fenceValue, bool sync/*true*/) const
{
	commandList->Close();

	std::vector<ID3D12GraphicsCommandList*> commandLists
	{
		commandList,
	};

	commandQueue->ExecuteCommandLists(commandLists.size(), reinterpret_cast<ID3D12CommandList* const*>(commandLists.data()));
	ASSERT_SUCCEEDED(commandQueue->Signal(fence, ++fenceValue));
	if (sync && fence->GetCompletedValue() < fenceValue)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ASSERT_SUCCEEDED(fence->SetEventOnCompletion(fenceValue, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
}

void DirectXToy::ResetSwapChain(ID3D12GraphicsCommandList* commandList, ID3D12CommandAllocator* allocator)
{
	/*
	DXGI_SWAP_CHAIN_DESC desc;
	desc.BufferDesc.Width = g_DisplayWidth;
	desc.BufferDesc.Height = g_DisplayHeight;
	desc.BufferDesc.RefreshRate.Numerator = 60;
	desc.BufferDesc.RefreshRate.Denominator = 1;
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = SwapChainCount;
	desc.OutputWindow = g_hWnd;
	desc.Windowed = true;
	desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	*/
	
	/*
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC desc2;
	desc2.RefreshRate.Denominator = 1;
	desc2.RefreshRate.Numerator = 60;
	desc2.Scaling = DXGI_MODE_SCALING_STRETCHED;
	desc2.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	desc2.Windowed = FALSE; //창모드 아님
	*/

	//hr = factory->CreateSwapChain(commandQueue.Get(), &desc, reinterpret_cast<IDXGISwapChain**>(output.GetAddressOf()));

	ASSERT_SUCCEEDED(allocator->Reset());
	ASSERT_SUCCEEDED(commandList->Reset(allocator, nullptr));

	for (auto& buffer : swapChainBuffers_)
	{
		buffer.Reset();
	}
	depthStencilBuffer_.Reset();

	if (iDXGISwapChain_ == nullptr)
	{
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
		swapChainDesc.BufferCount = SwapChainCount;
		swapChainDesc.Width = g_DisplayWidth;
		swapChainDesc.Height = g_DisplayHeight;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.SampleDesc.Count = 1;

		auto hr = iDXGIFactory_->CreateSwapChainForHwnd(commandQueue_.Get(), g_hWnd, &swapChainDesc, nullptr, nullptr,
			reinterpret_cast<IDXGISwapChain1**>(iDXGISwapChain_.GetAddressOf()));
	}
	currentBackBufferIndex_ = 0;
	iDXGISwapChain_->ResizeBuffers(SwapChainCount, g_DisplayWidth, g_DisplayHeight, DXGI_FORMAT_R8G8B8A8_UNORM,
		DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH);
	
	int i{};
	for (auto& buffer : swapChainBuffers_)
	{
		ASSERT_SUCCEEDED(iDXGISwapChain_->GetBuffer(i, IID_PPV_ARGS(&buffer)));
		auto handle = descriptorHandleAccesors_[descriptorHeapRTV_.Get()].GetCPUHandle(i++);
		device_->CreateRenderTargetView(buffer.Get(), nullptr, handle);
	}

	CD3DX12_RESOURCE_DESC dsResourceDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D24_UNORM_S8_UINT, g_DisplayWidth, g_DisplayHeight);
	dsResourceDesc.DepthOrArraySize = 1;
	dsResourceDesc.MipLevels = 1;
	dsResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	ASSERT_SUCCEEDED(device_->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE,
		&dsResourceDesc, D3D12_RESOURCE_STATE_COMMON,
		&clearValue,IID_PPV_ARGS(depthStencilBuffer_.GetAddressOf())));

	D3D12_DEPTH_STENCIL_VIEW_DESC depthSencilViewDesc;
	depthSencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;
	depthSencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthSencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthSencilViewDesc.Texture2D.MipSlice = 0;

	device_->CreateDepthStencilView(depthStencilBuffer_.Get(), &depthSencilViewDesc,
		descriptorHandleAccesors_[descriptorHeapDSV_.Get()].GetCPUHandle(0));

	// 뎁스버퍼로 사용될려면 D3D12_RESOURCE_STATE_DEPTH_WRITE 상태로의 전이가 일어나야..
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(depthStencilBuffer_.Get(),
		D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE));

	std::vector<ID3D12GraphicsCommandList*> commandLists
	{
		commandList,
	};
	ExecuteCommandList(commandLists, fence_.Get(), commandQueue_.Get(), mainFenceValue_, true);

	//Update Viewport, Scissor rect

	mainViewport_.TopLeftX = 0;
	mainViewport_.TopLeftY = 0;
	mainViewport_.Width = static_cast<float>(g_DisplayWidth);
	mainViewport_.Height = static_cast<float>(g_DisplayHeight);
	mainViewport_.MinDepth = 0.0f;
	mainViewport_.MaxDepth = 1.0f;

	mainScissor_ = { 0, 0, static_cast<long>(g_DisplayWidth), static_cast<long>(g_DisplayHeight) };
}

ID3D12Resource* DirectXToy::CurrentSwapChainBuffer() const
{
	return swapChainBuffers_[currentBackBufferIndex_].Get();
}

void DirectXToy::VertexBuffer::Confirm(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList, bool clearData /*false*/)
{
	defaultVertexBuffer_ = CreateDefaultBuffer(pDevice, pCommandList, cpuVertexBuffer_.get(), vbSize_, uploadVertexBuffer_, clearData);
	ASSERT(defaultVertexBuffer_ != nullptr);

	defaultIndexBuffer_ = CreateDefaultBuffer(pDevice, pCommandList, cpuIndexBuffer_.get(), ibSize_, uploadIndexBuffer_, clearData);
	ASSERT(defaultIndexBuffer_ != nullptr);
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

ComPtr<ID3D12Resource> DirectXToy::CreateDefaultBuffer(
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

void DirectXToy::Camera::Rotate(Axis axis, float angle)
{
	XMMATRIX rotationMatrix;
	switch (axis)
	{
		case Axis::X:
		{
			rotationMatrix = XMMatrixRotationX(angle);
		}
		break;
		case Axis::Y:
		{
			rotationMatrix = XMMatrixRotationY(angle);
		}
		break;
		case Axis::Z:
		{
			rotationMatrix = XMMatrixRotationZ(angle);
		}
		break;
	}
	XMStoreFloat3(&right_, XMVector3TransformNormal(XMLoadFloat3(&right_), rotationMatrix));
	XMStoreFloat3(&up_, XMVector3TransformNormal(XMLoadFloat3(&up_), rotationMatrix));
	XMStoreFloat3(&look_, XMVector3TransformNormal(XMLoadFloat3(&look_), rotationMatrix));
	viewDirty_ = true;
}

void DirectXToy::Camera::SetPosition(const XMFLOAT3& position)
{
	position_ = position;
	viewDirty_ = true;
}

const XMFLOAT4X4& DirectXToy::Camera::GetProjMatrix()
{
	if (!projDirty_)
	{
		return proj_;
	}

	XMStoreFloat4x4(&proj_, XMMatrixPerspectiveFovLH(fovY_, aspect_, nearZ_, farZ_));
	projDirty_ = false;
	return proj_;
}

const XMFLOAT4X4& DirectXToy::Camera::GetViewMatrix()
{
	if (!viewDirty_)
	{
		return view_;
	}

	auto look = XMVector3Normalize(XMLoadFloat3(&look_));
	auto up = XMVector3Normalize(XMVector3Cross(look, XMLoadFloat3(&right_)));
	auto right = XMVector3Normalize(XMVector3Cross(up, look));

	XMStoreFloat3(&look_, look);
	XMStoreFloat3(&up_, up);
	XMStoreFloat3(&right_, right);

	float x = -XMVectorGetX(XMVector3Dot(XMLoadFloat3(&position_), right));
	float y = -XMVectorGetX(XMVector3Dot(XMLoadFloat3(&position_), up));
	float z = -XMVectorGetX(XMVector3Dot(XMLoadFloat3(&position_), look));

	view_ = XMFLOAT4X4
	{
		right_.x, up_.x, look_.x, 0,
		right_.y, up_.y, look_.y, 0,
		right_.z, up_.z, look_.z, 0,
		x, y, z, 1.0f,
	};
	viewDirty_ = false;
	return view_;
}

void DirectXToy::Camera::SetProjMatrix(float fovY/* PI * 0.25 */, float aspect/* 1.0f */, float nearZ/* 1.0f */, float farZ/* 1000.0f */)
{
	fovY_ = fovY;
	aspect_ = aspect;
	nearZ_ = nearZ;
	farZ_ = farZ;

	projDirty_ = true;
}

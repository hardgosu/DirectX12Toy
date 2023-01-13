#include "DirectXToy.h"

extern HWND g_hWnd;
extern uint32_t g_DisplayWidth;
extern uint32_t g_DisplayHeight;

void DirectXToy::Startup()
{
	using Adapter = IDXGIAdapter1;
	using Factory = ComPtr<IDXGIFactory4>;
	using SwapChain = IDXGISwapChain3;
	using Device = ComPtr<ID3D12Device>;

	auto createDeviceWithBestAdapter = [](const Factory& factory, Device& outputDevice)
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

}

void DirectXToy::Cleanup()
{

}

void DirectXToy::Update(float deltaT)
{
	elapsedTime_ = deltaT;

	//Tuning, Tool, ETC..
	fpsViewer_.Show(g_hWnd, elapsedTime_);
	//Input
	//Logic(Interrupt)
	RenderScene();
}

void DirectXToy::RenderScene()
{

}

bool DirectXToy::IsDone()
{
	return isDone_;
}

void DirectXToy::FPSViewer::Show(HWND hWnd, float elapsedTime)
{
	++counter_;
	timer_ += elapsedTime;
	if (timer_ >= Period)
	{
		std::wstring fpsStr = L"FPS : " + std::to_wstring(counter_);

		SetWindowText(hWnd, fpsStr.c_str());

		timer_ = 0;
		counter_ = 0;
	}
}
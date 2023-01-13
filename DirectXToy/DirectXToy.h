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

private:

	//Camera m_Camera;
	//std::unique_ptr<CameraController> m_CameraController;

	D3D12_VIEWPORT m_MainViewport;
	D3D12_RECT m_MainScissor;
	ComPtr<IDXGIFactory4> iDXGIFactory_;
	ComPtr<IDXGIAdapter3> iDXGIAdapter_;
	ComPtr<IDXGISwapChain3> iDXGISwapChain_;

	ComPtr<ID3D12Device> device_;
	//ModelInstance m_ModelInst;
	//ShadowCamera m_SunShadowCamera;
	float elapsedTime_{};
	bool isDone_{ false };
};

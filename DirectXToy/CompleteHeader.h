#pragma once
#include <windows.h>
#include <Windows.Foundation.h>
#include <wrl\wrappers\corewrappers.h>
#include <wrl\client.h>
#include <stdio.h>
#include <dxgi1_5.h>
#include <d3d12.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>
#include <ppl.h>
#include <iostream>
#include <string>
#include <memory>
#include <algorithm>
#include <vector>
#include <array>
#include <map>
#include <set>
#include <unordered_map>
#include <optional>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <cassert>
#include <chrono>
#include <functional>
#include <thread>
#include <any>
#include "Independent/IGameApp.h"
#include "Independent/d3dx12.h"
#include "Independent/DDSTextureLoader.h"
#include "Independent/MathHelper.h"
#include "Independent/UploadBuffer.h"
#include "Independent/StateMachine.h"
#include "Independent/DefaultBuffer.h"
#include "Utility.h"

#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "runtimeobject.lib")


using Microsoft::WRL::ComPtr;
using namespace DirectX;


ComPtr<ID3DBlob> CompileShader(
	const std::wstring& filename,
	const D3D_SHADER_MACRO* defines,
	const std::string& entrypoint,
	const std::string& target);

using WndProcT = std::function<void(HWND, UINT, WPARAM, LPARAM)>;
using ProcedureMap = std::unordered_multimap<int, WndProcT>;

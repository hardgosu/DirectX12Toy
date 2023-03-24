#pragma once

#include <Windows.h>
#include <d3d12.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

template<typename T>
class UploadBuffer
{
public:
	UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer) :
		isConstantBuffer_{ isConstantBuffer }
	{
		elementByteSize_ = sizeof(T);

		// Constant buffer elements need to be multiples of 256 bytes.
		// This is because the hardware can only view constant data 
		// at m*256 byte offsets and of n*256 byte lengths. 
		// typedef struct D3D12_CONSTANT_BUFFER_VIEW_DESC {
		// UINT64 OffsetInBytes; // multiple of 256
		// UINT   SizeInBytes;   // multiple of 256
		// } D3D12_CONSTANT_BUFFER_VIEW_DESC;
		if (isConstantBuffer)
		{
			//constexpr unsigned MaxConstantBufferSize = 65536;
			elementByteSize_ = CalcConstantBufferByteSize(sizeof(T));
			//assert(elementByteSize_ * elementCount <= MaxConstantBufferSize = 65536);
		}

		auto hResult = device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(elementByteSize_ * elementCount),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&uploadBuffer_));

		hResult = uploadBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&pMappedData_));

		// We do not need to unmap until we are done with the resource.  However, we must not write to
		// the resource while it is in use by the GPU (so we must use synchronization techniques).
	}

	UploadBuffer(const UploadBuffer& rhs) = delete;
	UploadBuffer& operator=(const UploadBuffer& rhs) = delete;
	~UploadBuffer()
	{
		if (uploadBuffer_ != nullptr)
			uploadBuffer_->Unmap(0, nullptr);

		pMappedData_ = nullptr;
	}

	ID3D12Resource* Resource()const
	{
		return uploadBuffer_.Get();
	}

	void CopyData(int elementIndex, const T& data)
	{
		memcpy(&pMappedData_[elementIndex * elementByteSize_], &data, sizeof(T));
	}

	void CopyData(const std::vector<T>& datas)
	{
		std::memcpy(pMappedData_, datas.data(), sizeof(T) * datas.size());
	}

	void MoveData(std::vector<T>&& datas)
	{
		std::move(datas.begin(), datas.end(), pMappedData_);
	}

	D3D12_GPU_VIRTUAL_ADDRESS GetElemAddress(int elemIndex)
	{
		return elementByteSize_ * elemIndex + uploadBuffer_->GetGPUVirtualAddress();
	}

	//상수버퍼의 경우 그래픽 하드웨어는 256바이트 정렬된 버퍼를 기대한다.
	//임의의 바이트 크기를 256배수로 align시켜준다.
	//300을 넘긴다면 512를 리턴하게된다.
	static UINT CalcConstantBufferByteSize(UINT byteSize)
	{
		return (byteSize + 255) & ~255;
	}
	BYTE* pMappedData_{ nullptr };
private:
	ComPtr<ID3D12Resource> uploadBuffer_;

	UINT elementByteSize_{};
	bool isConstantBuffer_{ false };
};
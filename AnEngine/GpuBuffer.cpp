#include "GpuBuffer.h"
#include "RenderCore.h"
#include"DescriptorHeap.hpp"
#include"DMath.hpp"
using namespace AnEngine::RenderCore;
using namespace AnEngine::RenderCore::Heap;

namespace AnEngine::RenderCore::Resource
{
	D3D12_RESOURCE_DESC GpuBuffer::DescribeBuffer(void)
	{
		D3D12_RESOURCE_DESC desc = {};
		desc.Alignment = 0;
		desc.DepthOrArraySize = 1;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Flags = m_resourceFlags;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.Height = 1;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Width = (UINT64)m_bufferSize;
		return desc;
	}

	GpuBuffer::GpuBuffer(const std::wstring& name, uint32_t numElements, uint32_t elementSize,
		const void* initialData) : GpuResource(), m_bufferSize(numElements * elementSize), 
		m_elementCount(numElements), m_elementSize(elementSize)
	{
		m_resourceFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		m_uav.ptr = GpuVirtualAddressUnknown;
		m_srv.ptr = GpuVirtualAddressUnknown;

		var device = r_graphicsCard[0]->GetDevice();
		D3D12_RESOURCE_DESC ResourceDesc = DescribeBuffer();
		m_usageState = D3D12_RESOURCE_STATE_COMMON;

		/*D3D12_HEAP_PROPERTIES heapProps;
		heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProps.CreationNodeMask = 1;
		heapProps.VisibleNodeMask = 1;*/

		//device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
		//	&ResourceDesc, m_usageState, nullptr, IID_PPV_ARGS(&m_resource_cp));
		var desc = DescribeBuffer();
		D3D12_RESOURCE_ALLOCATION_INFO allocationInfo = device->GetResourceAllocationInfo(1, 1, &desc);
		D3D12_HEAP_DESC heapDesc;
		heapDesc.SizeInBytes = allocationInfo.SizeInBytes;
		heapDesc.Properties.Type = D3D12_HEAP_TYPE_UPLOAD;
		heapDesc.Properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapDesc.Properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapDesc.Properties.CreationNodeMask = r_graphicsCard[0]->GetNodeNum();
		heapDesc.Properties.VisibleNodeMask = r_graphicsCard[0]->GetNodeNum();
		heapDesc.Alignment = allocationInfo.Alignment;
		heapDesc.Flags = D3D12_HEAP_FLAGS::D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES;
		ThrowIfFailed(device->CreateHeap(&heapDesc, IID_PPV_ARGS(&m_heap_cp)));

		ThrowIfFailed(device->CreatePlacedResource(m_heap_cp.Get(), 0, &desc, m_usageState,
			nullptr, IID_PPV_ARGS(&m_resource_cp)));

		m_gpuVirtualAddress = m_resource_cp->GetGPUVirtualAddress();

		if (initialData != nullptr)
		{
			std::byte* pVertexDataBegin;
			CD3DX12_RANGE readRange(0, 0);		//We do not intend to read from this resource on the CPU.
			m_resource_cp->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin));
			memcpy(pVertexDataBegin, initialData, sizeof(m_bufferSize));
			m_resource_cp->Unmap(0, nullptr);
		}
#if defined _DEBUG || defined DEBUG
		m_resource_cp->SetName(name.c_str());
#else
		(name);
#endif

		//CreateDerivedViews();
	}

	GpuBuffer::~GpuBuffer()
	{
	}

	const D3D12_CPU_DESCRIPTOR_HANDLE & GpuBuffer::GetUav(void) const
	{
		return m_uav;
	}

	const D3D12_CPU_DESCRIPTOR_HANDLE & GpuBuffer::GetSrv(void) const
	{
		return m_srv;
	}

	D3D12_GPU_VIRTUAL_ADDRESS GpuBuffer::RootConstantBufferView(void) const
	{
		return m_gpuVirtualAddress;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE GpuBuffer::CreateConstantBufferView(uint32_t offset, uint32_t size) const
	{
		var device = r_graphicsCard[0]->GetDevice();
		size = DMath::AlignUp(size, 16);

		D3D12_CONSTANT_BUFFER_VIEW_DESC CBVDesc;
		CBVDesc.BufferLocation = m_gpuVirtualAddress + (size_t)offset;
		CBVDesc.SizeInBytes = size;

		D3D12_CPU_DESCRIPTOR_HANDLE hCbv = DescriptorHeapAllocator::GetInstance()->Allocate(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		device->CreateConstantBufferView(&CBVDesc, hCbv);
		return hCbv;
	}

	D3D12_VERTEX_BUFFER_VIEW GpuBuffer::VertexBufferView(size_t offset, uint32_t size, uint32_t stride) const
	{
		D3D12_VERTEX_BUFFER_VIEW vbView;
		vbView.BufferLocation = m_gpuVirtualAddress + offset;
		vbView.SizeInBytes = size;
		vbView.StrideInBytes = stride;
		return vbView;
	}

	D3D12_VERTEX_BUFFER_VIEW GpuBuffer::VertexBufferView(size_t baseVertexIndex) const
	{
		size_t offset = baseVertexIndex * m_elementSize;
		return VertexBufferView(offset, (uint32_t)(m_bufferSize - offset), m_elementSize);
	}

	D3D12_INDEX_BUFFER_VIEW GpuBuffer::IndexBufferView(size_t offset, uint32_t size, bool b32Bit) const
	{
		D3D12_INDEX_BUFFER_VIEW ibView;
		ibView.BufferLocation = m_gpuVirtualAddress + offset;
		ibView.Format = b32Bit ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
		ibView.SizeInBytes = size;
		return ibView;
	}

	D3D12_INDEX_BUFFER_VIEW GpuBuffer::IndexBufferView(size_t startIndex) const
	{
		size_t offset = startIndex * m_elementSize;
		return IndexBufferView(offset, (uint32_t)(m_bufferSize - offset), m_elementSize == 4);
	}

}

namespace AnEngine::RenderCore::Resource
{
	ByteAddressBuffer::ByteAddressBuffer(const std::wstring & name, uint32_t numElements, uint32_t elementSize,
		const void* initialData) : GpuBuffer(name, numElements, elementSize, initialData)
	{
		CreateDerivedViews();
	}

	void ByteAddressBuffer::CreateDerivedViews()
	{
		var device = r_graphicsCard[0]->GetDevice();

		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		SRVDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRVDesc.Buffer.NumElements = (UINT)m_bufferSize / 4;
		SRVDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

		if (m_srv.ptr == GpuVirtualAddressUnknown)
		{
			m_srv = DescriptorHeapAllocator::GetInstance()->Allocate(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		device->CreateShaderResourceView(m_resource_cp.Get(), &SRVDesc, m_uav);

		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
		UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		UAVDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		UAVDesc.Buffer.NumElements = (UINT)m_bufferSize / 4;
		UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

		if (m_uav.ptr == GpuVirtualAddressUnknown)
			m_uav = DescriptorHeapAllocator::GetInstance()->Allocate(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		device->CreateUnorderedAccessView(m_resource_cp.Get(), nullptr, &UAVDesc, m_uav);
	}
}
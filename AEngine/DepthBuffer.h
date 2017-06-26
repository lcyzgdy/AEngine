#pragma once
#ifndef __DEPTHBUFFER_H__
#define __DEPTHBUFFER_H__
#include"DX.h"
#include"PixelBuffer.h"
#include"DescriptorHeap.h"
using namespace std;
using namespace RenderCore::Resource;
namespace RenderCore
{
	namespace Resource
	{
		class DepthBuffer :public PixelBuffer
		{
		protected:
			float m_clearDepth;
			uint8_t m_clearStencil;
			D3D12_CPU_DESCRIPTOR_HANDLE m_dsvHandle[4];
			D3D12_CPU_DESCRIPTOR_HANDLE m_depthSrvHandle;
			D3D12_CPU_DESCRIPTOR_HANDLE m_stencilSrvHandle;

			void CreateDerviedViews(ID3D12Device* device, DXGI_FORMAT& format, 
				RenderCore::Heap::DescriptorAllocator* descAllocator);

		public:
			DepthBuffer() = delete;
			DepthBuffer(float clearDepth = 0.0f, uint8_t clearStencil = 0);
			~DepthBuffer() = default;

			// ������ɫ������������ṩ�˵�ַ�򲻻�����ڴ档�����ַ���������������������ڿ�Խ֡����ESRAM�ر����ã�������
			void Create(const wstring& name, uint32_t _width, uint32_t _height, DXGI_FORMAT& format,
				ID3D12Device* device, RenderCore::Heap::DescriptorAllocator* descAllocator, 
				uint32_t numSamples = 1,
				D3D12_GPU_VIRTUAL_ADDRESS vidMemPtr = RenderCore::Resource::GpuVirtualAddressUnknown);

			const D3D12_CPU_DESCRIPTOR_HANDLE& GetDsv() const;
			const D3D12_CPU_DESCRIPTOR_HANDLE& GetDsvDepthReadOnly() const;
			const D3D12_CPU_DESCRIPTOR_HANDLE& GetDsvStencilReadOnly() const;
			const D3D12_CPU_DESCRIPTOR_HANDLE& GetDsvReadOnly() const;
			const D3D12_CPU_DESCRIPTOR_HANDLE& GetDepthSrv() const;
			const D3D12_CPU_DESCRIPTOR_HANDLE& GetStencilSrv() const;

			float GetClearDepth() const;
			uint8_t GetClearStencil() const;
		};
	}
}

#endif // !__DEPTHBUFFER_H__
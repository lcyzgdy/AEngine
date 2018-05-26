#include "GraphicsCard2D.h"
#include "RenderCore.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "d3d11.lib")

using namespace Microsoft::WRL;

namespace AnEngine::RenderCore::UI
{
	procedure GraphicsCard2D::InitializeForText()
	{
		ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &m_dWriteFactory));

		ThrowIfFailed(m_d2dContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_textBrush));
		ThrowIfFailed(m_dWriteFactory->CreateTextFormat(L"Consola", nullptr, DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 50, L"zh-cn", &m_textFormat));
		ThrowIfFailed(m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
		ThrowIfFailed(m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));
	}

	GraphicsCard2D::GraphicsCard2D()
	{

	}

	void GraphicsCard2D::Initialize()
	{
		uint32_t dxgiFactoryFlags = 0;
		uint32_t d3d11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
		D2D1_FACTORY_OPTIONS d2dFactoryOptions = {};

		ComPtr<ID3D11Device> d3d11Device;
		ThrowIfFailed(D3D11On12CreateDevice(r_graphicsCard[0]->GetDevice(), d3d11DeviceFlags, nullptr, 0,
			reinterpret_cast<IUnknown**>(r_graphicsCard[0]->GetCommandQueueAddress()), 1, 0, &d3d11Device,
			&m_d3d11DeviceContext, nullptr));

		ThrowIfFailed(d3d11Device.As(&m_d3d11On12Device));

		D2D1_DEVICE_CONTEXT_OPTIONS deviceOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
		ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, &m_d2dFactory));
		ComPtr<IDXGIDevice> dxgiDevice;
		ThrowIfFailed(m_d3d11On12Device.As(&dxgiDevice));
		ThrowIfFailed(m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice));
		ThrowIfFailed(m_d2dDevice->CreateDeviceContext(deviceOptions, &m_d2dContext));

		float dpiX;
		float dpiY;
		m_d2dFactory->GetDesktopDpi(&dpiX, &dpiY);
		m_bitmapProperties = D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED), dpiX, dpiY);

		InitializeForText();
	}

	void GraphicsCard2D::Release()
	{
	}

	ID3D11On12Device* GraphicsCard2D::GetDevice11On12()
	{
		return m_d3d11On12Device.Get();
	}

	ID3D11Resource* GraphicsCard2D::GetWrappedBackBuffer(uint32_t index)
	{
		return m_wrappedBackBuffers[index].Get();
	}
}
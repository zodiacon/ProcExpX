#include "pch.h"
#include "ProcessInfoEx.h"
#include "Processes.h"
#include "colors.h"
#include "ProcessManager.h"
#include "Globals.h"
#include "Settings.h"
#include "ProcessColor.h"
#include <wincodec.h>
#include "resource.h"

std::pair<const ImVec4&, const ImVec4&> ProcessInfoEx::GetColors(WinSys::ProcessManager& pm) const {
	using namespace ImGui;
	auto& colors = Globals::Get().GetSettings().ProcessColors;

	if (colors[Settings::DeletedObjects].Enabled && IsTerminated())
		return { colors[Settings::DeletedObjects].Color, colors[Settings::DeletedObjects].TextColor };

	if (colors[Settings::NewObjects].Enabled && IsNew())
		return { colors[Settings::NewObjects].Color, colors[Settings::NewObjects].TextColor };

	auto attributes = GetAttributes(pm);
	if (colors[Settings::Manageed].Enabled && (attributes & ProcessAttributes::Managed) == ProcessAttributes::Managed) {
		return { colors[Settings::Manageed].Color, colors[Settings::Manageed].TextColor };
	}
	if (colors[Settings::Immersive].Enabled && (attributes & ProcessAttributes::Immersive) == ProcessAttributes::Immersive) {
		return { colors[Settings::Immersive].Color, colors[Settings::Immersive].TextColor };
	}
	if (colors[Settings::Secure].Enabled && (attributes & ProcessAttributes::Secure) == ProcessAttributes::Secure) {
		return { colors[Settings::Secure].Color, colors[Settings::Secure].TextColor };
	}
	if (colors[Settings::Protected].Enabled && (attributes & ProcessAttributes::Protected) == ProcessAttributes::Protected) {
		return { colors[Settings::Protected].Color, colors[Settings::Protected].TextColor };
	}
	if (colors[Settings::Services].Enabled && (attributes & ProcessAttributes::Service) == ProcessAttributes::Service) {
		return { colors[Settings::Services].Color, colors[Settings::Services].TextColor };
	}
	if (colors[Settings::InJob].Enabled && (attributes & ProcessAttributes::InJob) == ProcessAttributes::InJob) {
		return { colors[Settings::InJob].Color, colors[Settings::InJob].TextColor };
	}

	return { ImVec4(-1, 0, 0, 0), ImVec4() };
}

ProcessAttributes ProcessInfoEx::GetAttributes(WinSys::ProcessManager& pm) const {
	if (_attributes == ProcessAttributes::NotComputed) {
		_attributes = ProcessAttributes::None;
		auto process = WinSys::Process::OpenById(_pi->Id, WinSys::ProcessAccessMask::QueryLimitedInformation);
		if (process) {
			if (process->IsManaged())
				_attributes |= ProcessAttributes::Managed;
			if (process->IsProtected())
				_attributes |= ProcessAttributes::Protected;
			if (process->IsImmersive())
				_attributes |= ProcessAttributes::Immersive;
			if (process->IsSecure())
				_attributes |= ProcessAttributes::Secure;
			if (process->IsInJob())
				_attributes |= ProcessAttributes::InJob;
			auto parent = pm.GetProcessById(_pi->ParentId);
			if (parent && ::_wcsicmp(parent->GetImageName().c_str(), L"services.exe") == 0)
				_attributes |= ProcessAttributes::Service;
		}
	}
	return _attributes;
}

const std::wstring& ProcessInfoEx::UserName() const {
	if (_username.empty()) {
		if (_pi->Id <= 4)
			_username = L"NT AUTHORITY\\SYSTEM";
		else {
			auto process = WinSys::Process::OpenById(_pi->Id);
			if (process)
				_username = process->GetUserNameW();
			if(_username.empty())
				_username = L"<access denied>";
		}
	}
	return _username;
}

bool ProcessInfoEx::Update() {
	if (!_isNew && !_isTerminated)
		return false;

	bool term = _isTerminated;
	if (::GetTickCount() > _expiryTime) {
		_isTerminated = _isNew = false;
		return term;
	}
	return false;
}

void ProcessInfoEx::New(uint32_t ms) {
	_isNew = true;
	_expiryTime = ::GetTickCount64() + ms;
}

void ProcessInfoEx::Term(uint32_t ms) {
	_isNew = false;
	_isTerminated = true;
	_expiryTime = ::GetTickCount64() + ms;
}

const std::wstring& ProcessInfoEx::GetExecutablePath() const {
	if (_executablePath.empty() && _pi->Id != 0) {
		const auto& path = _pi->GetNativeImagePath();
		if (path[0] == L'\\') {
			auto process = WinSys::Process::OpenById(_pi->Id, WinSys::ProcessAccessMask::QueryLimitedInformation);
			if (process)
				_executablePath = process->GetFullImageName();
		}
		else {
			_executablePath = path;
		}
	}
	return _executablePath;
}

ID3D11ShaderResourceView* ProcessInfoEx::Icon() const {
	if (m_spIcon == nullptr) {
		static HICON hAppIcon = ::LoadIcon(nullptr, IDI_APPLICATION);
		extern ID3D11Device* g_pd3dDevice;
		extern ID3D11DeviceContext* g_pd3dDeviceContext;

		UINT width, height;
		CComPtr<IWICImagingFactory> spFactory;
		spFactory.CoCreateInstance(CLSID_WICImagingFactory);
		CComPtr<IWICBitmap> spBitmap;
		auto& path = GetExecutablePath();
		auto hIcon = ::ExtractIcon(nullptr, path.c_str(), 0);
		if (!hIcon)
			hIcon = hAppIcon;
		ATLASSERT(hIcon);
		spFactory->CreateBitmapFromHICON(hIcon, &spBitmap);
		if (hIcon != hAppIcon)
			::DestroyIcon(hIcon);
		spBitmap->GetSize(&width, &height);

		// Create texture
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; //DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;

		CComPtr<ID3D11Texture2D> pTexture;
		D3D11_SUBRESOURCE_DATA subResource;
		CComPtr<IWICBitmapLock> spLock;
		spBitmap->Lock(nullptr, WICBitmapLockRead, &spLock);
		UINT size;
		WICInProcPointer ptr;
		spLock->GetDataPointer(&size, &ptr);
		subResource.pSysMem = ptr;
		subResource.SysMemPitch = desc.Width * 4;
		subResource.SysMemSlicePitch = 0;
		g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

		// Create texture view
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(srvDesc));
		srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;	//DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &m_spIcon);
	}
	return m_spIcon.p;
}

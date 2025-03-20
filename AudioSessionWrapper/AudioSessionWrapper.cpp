#include "pch.h"
#include "framework.h"
#include "AudioSessionWrapper.h"


extern "C" __declspec(dllexport) CAudioSessionWrapper* CreateAudioSessionWrapper() {
    std::cout << "init" << std::endl;
    return new CAudioSessionWrapper();
}

extern "C" __declspec(dllexport) void DeleteAudioSessionWrapper(CAudioSessionWrapper* wrapper) {
    delete wrapper;
}

extern "C" __declspec(dllexport) BSTR GetProcessName(CAudioSessionWrapper* wrapper, int index) {
    return _com_util::ConvertStringToBSTR(wrapper->GetProcessName(index).c_str());//SysAllocString(L"HELLO");//
}

extern "C" __declspec(dllexport) void UpdateSessions(CAudioSessionWrapper* wrapper) {
    wrapper->UpdateSessions();
}

extern "C" __declspec(dllexport) int SessionCount(CAudioSessionWrapper* wrapper) {
    return wrapper->SessionCount();
}

extern "C" __declspec(dllexport) float GetVolume(CAudioSessionWrapper* wrapper, int index) {
    return wrapper->GetVolume(index);
}

extern "C" __declspec(dllexport) void SetVolume(CAudioSessionWrapper* wrapper, int index, float volume) {
    wrapper->SetVolume(index, volume);
}

extern "C" __declspec(dllexport) bool GetMute(CAudioSessionWrapper* wrapper, int index) {
    return wrapper->GetMute(index);
}

extern "C" __declspec(dllexport) void SetMute(CAudioSessionWrapper* wrapper, int index, bool mute) {
    wrapper->SetMute(index, mute);
}

extern "C" __declspec(dllexport) int GetProcessIcon(CAudioSessionWrapper* wrapper, int index, BYTE** buffer, DWORD* size) {
    if (!wrapper) {
        std::cout << "wrapper is NULL!" << std::endl;
        return -1;
    }

    return wrapper->GetProcessIcon(index, buffer, size);
}

extern "C" __declspec(dllexport) int ReleaseMemory(BYTE* pArray) {
    delete[] pArray;
    return 0;
}

CAudioSessionWrapper::CAudioSessionWrapper()
{
    this->initialized = false;
    HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    std::cout << "CoInitializeEx returned: " << std::hex << hr << std::endl;

    if (FAILED(hr)) throw std::runtime_error("Failed to initialize COM library.");

    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator),
        nullptr,
        CLSCTX_INPROC_SERVER,
        __uuidof(IMMDeviceEnumerator),
        reinterpret_cast<void**>(deviceEnumerator.GetAddressOf())
    );

    if (FAILED(hr)) throw std::runtime_error("Failed to create MMDeviceEnumerator.");

    hr = deviceEnumerator->GetDefaultAudioEndpoint(eRender, eMultimedia, device.GetAddressOf());
    if (FAILED(hr)) throw std::runtime_error("Failed to get default audio endpoint.");

    hr = device->Activate(
        __uuidof(IAudioSessionManager2),
        CLSCTX_ALL,
        nullptr,
        reinterpret_cast<void**>(sessionManager.GetAddressOf())
    );

    if (FAILED(hr)) throw std::runtime_error("Failed to activate IAudioSessionManager2.");

    hr = sessionManager->GetSessionEnumerator(sessionEnumerator.GetAddressOf());

    if (FAILED(hr)) throw std::runtime_error("Failed to get audio session enumerator.");

    this->UpdateSessions();

    this->initialized = true;
    std::cout << "initialized" << std::endl;
}

CAudioSessionWrapper::~CAudioSessionWrapper()
{
    if (initialized) {
        CoUninitialize();
        initialized = false;
    }
}

std::string CAudioSessionWrapper::GetProcessName(int index) const
{
    if (index < 0 || index >= static_cast<int>(sessions.size())) {
        throw std::out_of_range("Session index out of range.");
    }
    if (!initialized) {
        throw std::runtime_error("Object is not initialized");
    }
    return sessions[index].IcoName.processName;
}

void CAudioSessionWrapper::UpdateSessions()
{

    HRESULT hr = sessionManager->GetSessionEnumerator(&this->sessionEnumerator);
    if (FAILED(hr)) {
        std::cerr << "Failed to get IAudioSessionEnumerator" << std::endl;
        return;
    }

    sessions.clear();
    int sessionCount = 0;
    hr = sessionEnumerator->GetCount(&sessionCount);
    std::cout << "new session coutn:" << sessionCount<<std::endl;
    if (FAILED(hr)) throw std::runtime_error("Failed to get session count.");

    for (int i = 0; i < sessionCount; ++i) {
        std::string DisplayName = "";
        Microsoft::WRL::ComPtr<IAudioSessionControl> sessionControl;


        hr = sessionEnumerator->GetSession(i, sessionControl.GetAddressOf());

        if (FAILED(hr)) continue;

        AudioSessionState state;
        hr = sessionControl->GetState(&state);
        if (FAILED(hr) || state == AudioSessionStateExpired) {
            std::cout << "Skipping expired session " << i << std::endl;
            continue; // Pomijamy wygasłe sesje
        }

        Microsoft::WRL::ComPtr<IAudioSessionControl2> sessionControl2;
        hr = sessionControl.As(&sessionControl2);

        if (FAILED(hr)) continue;

        

        DWORD processId = 0;
        hr = sessionControl2->GetProcessId(&processId);
        if (FAILED(hr)) continue;

        Microsoft::WRL::ComPtr<ISimpleAudioVolume> audioVolume;
        hr = sessionControl.As(&audioVolume);
        if (FAILED(hr)) continue;

        this->sessions.push_back({
            sessionControl,
            audioVolume,
            processId,
            GetProcessIcoAndNameById(processId),
            });
    }
}

int CAudioSessionWrapper::SessionCount() const
{
    return static_cast<int>(sessions.size());
}

float CAudioSessionWrapper::GetVolume(int index) const
{
    if (index < 0 || index >= static_cast<int>(sessions.size())) {
        throw std::out_of_range("Session index out of range.");
    }
    if (!initialized) {
        throw std::runtime_error("Object is not initialized");
    }
    float volume = 0.0f;
    HRESULT hr = sessions[index].audioVolume->GetMasterVolume(&volume);
    if (FAILED(hr)) throw std::runtime_error("Failed to get volume.");
    return volume;
}

void CAudioSessionWrapper::SetVolume(int index, float volume)
{
    if (index < 0 || index >= static_cast<int>(sessions.size())) {
        throw std::out_of_range("Session index out of range.");
    }
    if (!initialized) {
        throw std::runtime_error("Object is not initialized");
    }
    HRESULT hr = sessions[index].audioVolume->SetMasterVolume(volume, NULL);
    if (FAILED(hr)) throw std::runtime_error("Failed to set volume.");
}

bool CAudioSessionWrapper::GetMute(int index)
{
    if (index < 0 || index >= static_cast<int>(sessions.size())) {
        throw std::out_of_range("Session index out of range.");
    }
    if (!initialized) {
        throw std::runtime_error("Object is not initialized");
    }
    BOOL is_mute = false;
    HRESULT hr = sessions[index].audioVolume->GetMute(&is_mute);
    if (FAILED(hr)) throw std::runtime_error("Failed to get mute state.");
    return is_mute == TRUE;
}

void CAudioSessionWrapper::SetMute(int index, bool mute)
{
    if (index < 0 || index >= static_cast<int>(sessions.size())) {
        throw std::out_of_range("Session index out of range.");
    }
    if (!initialized) {
        throw std::runtime_error("Object is not initialized");
    }
    HRESULT hr = sessions[index].audioVolume->SetMute(mute, NULL);
    if (FAILED(hr)) throw std::runtime_error("Failed to set mute state.");
}

CAudioSessionWrapper::ProcessIcoName CAudioSessionWrapper::GetProcessIcoAndNameById(DWORD processId) const
{
    //std::cout << "GetProcessIcoAndNameById\n";
    HICON hIcon = nullptr;
    char processName[MAX_PATH] = "<unknown>";
    char processPath[MAX_PATH] = "<unknown>";
    WCHAR exeNameBuf[MAX_PATH];
    DWORD exeNameBufLen = sizeof(exeNameBuf) / sizeof(exeNameBuf[0]);
    HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (processHandle == nullptr) {
        std::cout << "OpenProcess failed for PID: " << processId << " Error: " << GetLastError() << "\n";
        return { "<unknown>", LoadIcon(NULL, IDI_APPLICATION) };
    }

    if (processHandle != nullptr) {
        HMODULE module;
        DWORD bytesNeeded;
        

        if (EnumProcessModules(processHandle, &module, sizeof(module), &bytesNeeded)) {
            GetModuleBaseNameA(processHandle, module, processName, sizeof(processName) / sizeof(char));
            GetModuleFileNameExA(processHandle, module, processPath, sizeof(processPath) / sizeof(char));


            QueryFullProcessImageNameW(processHandle, 0, exeNameBuf, &exeNameBufLen);
            UINT iconCount = ExtractIconExW(exeNameBuf, 0, &hIcon, NULL, 1);
            if (iconCount == 0 || hIcon == NULL) {
                std::wcout << "No ico in " << exeNameBuf << std::endl;
            }
            else {
                std::wcout << "ico found\n";
            }
            int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, exeNameBuf, -1, nullptr, 0, nullptr, nullptr);
            if (sizeNeeded > 0) {
                std::string result(sizeNeeded, 0);
                WideCharToMultiByte(CP_UTF8, 0, exeNameBuf, -1, &result[0], sizeNeeded, nullptr, nullptr);
                

                std::cout << "Valid icon path: " << result << std::endl;
            }
        }
        CloseHandle(processHandle);
    }

    return { std::string(processName),hIcon };
}

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
    UINT  num = 0;          // number of image encoders
    UINT  size = 0;         // size of the image encoder array in bytes

    Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;  // Failure

    pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
    if (pImageCodecInfo == NULL)
        return -1;  // Failure

    GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT j = 0; j < num; ++j)
    {
        if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
        {
            *pClsid = pImageCodecInfo[j].Clsid;
            free(pImageCodecInfo);
            return j;  // Success
        }
    }

    free(pImageCodecInfo);
    return -1;  // Failure
}

int CAudioSessionWrapper::GetProcessIcon(int index, BYTE** buffer, DWORD* size)
{
    if (index < 0 || index >= sessions.size()) {
        return -1;
    }

    HICON hIcon = sessions[index].IcoName.Ico;
    if (!hIcon) return -1;

    // init GDI+
    Gdiplus::GdiplusStartupInput gdiPlusStartupInput;
    ULONG_PTR gdiPlusToken;
    Gdiplus::GdiplusStartup(&gdiPlusToken, &gdiPlusStartupInput, NULL);


    // HICON to HBITMAP
    ICONINFO iconInfo;
    if (!GetIconInfo(hIcon, &iconInfo)) {
        Gdiplus::GdiplusShutdown(gdiPlusToken);
        return -1;
    }
    //Gdiplus::Bitmap bitmap(iconInfo.hbmColor, NULL);
    Gdiplus::Bitmap* bitmap = Gdiplus::Bitmap::FromHICON(hIcon);

    if (bitmap == nullptr) {
        std::cout << "bitmap fail" << std::endl;
        Gdiplus::GdiplusShutdown(gdiPlusToken);
        return -1;
    }


    IStream* pStream = NULL;
    if (FAILED(CreateStreamOnHGlobal(NULL, TRUE, &pStream))) {
        Gdiplus::GdiplusShutdown(gdiPlusToken);
        return -1;
    }

    // get CLSID for PNG
    CLSID pngClsid;
    if (GetEncoderClsid(L"image/png", &pngClsid) == -1) {
        pStream->Release();
        Gdiplus::GdiplusShutdown(gdiPlusToken);
        return -1;
    }

    //how to save icon
    //int stat = bitmap->Save(L"Bird.png", &pngClsid, NULL);

    //if (stat == 0)
    //    printf("Bird.png was saved successfully\n");
    //else
    //    printf("Failure: stat = %d\n", stat);

    // save bitmap as PNG to stream
    if (bitmap->Save(pStream, &pngClsid, NULL) != Gdiplus::Ok) {
        pStream->Release();
        Gdiplus::GdiplusShutdown(gdiPlusToken);
        return -1;
    }
    std::cout << "7" << std::endl;

    // get size and content of the stream
    STATSTG statstg;
    if (FAILED(pStream->Stat(&statstg, STATFLAG_NONAME))) {
        pStream->Release();
        Gdiplus::GdiplusShutdown(gdiPlusToken);
        return -1;
    }
    *size = statstg.cbSize.LowPart;
    std::cout << "8" << std::endl;

    if (*size == 0) {
        pStream->Release();
        Gdiplus::GdiplusShutdown(gdiPlusToken);
        return -1;
    }

    HGLOBAL hGlobal = NULL;
    if (FAILED(GetHGlobalFromStream(pStream, &hGlobal))) {
        pStream->Release();
        Gdiplus::GdiplusShutdown(gdiPlusToken);
        return -1;
    }

    void* pData = GlobalLock(hGlobal);
    if (pData == nullptr) return -1;
    std::cout << "9" << std::endl;
    //*size = 4;

    *buffer = (BYTE*)CoTaskMemAlloc(*size);
    if (*buffer == NULL) {
        GlobalUnlock(hGlobal);
        pStream->Release();
        Gdiplus::GdiplusShutdown(gdiPlusToken);
        return -1;
    }

    memcpy(*buffer, pData, *size);

    GlobalUnlock(hGlobal);
    delete bitmap;
    pStream->Release();
    Gdiplus::GdiplusShutdown(gdiPlusToken);
    
    return 0;
}

// Następujący blok ifdef jest standardowym sposobem tworzenia makr, które powodują, że eksportowanie
// z biblioteki DLL jest prostsze. Wszystkie pliki w obrębie biblioteki DLL są kompilowane z AUDIOSESSIONWRAPPER_EXPORTS
// symbol zdefiniowany w wierszu polecenia. Symbol nie powinien być zdefiniowany w żadnym projekcie
// które korzysta z tej biblioteki DLL. W ten sposób każdy inny projekt, którego pliki źródłowe dołączają ten plik, widzi
// funkcje AUDIOSESSIONWRAPPER_API w postaci zaimportowanej z biblioteki DLL, podczas gdy biblioteka DLL widzi symbole
// zdefiniowane za pomocą tego makra jako wyeksportowane.
#pragma once


// Ta klasa została wyeksportowana z pliku dll
class CAudioSessionWrapper {
public:
	CAudioSessionWrapper();
    ~CAudioSessionWrapper();
    std::string GetProcessName(int index) const;
    void UpdateSessions();
    int SessionCount() const;
    float GetVolume(int index) const;
    void SetVolume(int index, float volume);
    bool GetMute(int index);
    void SetMute(int index, bool mute);
    std::string IcoPath(int index);

private:
    struct Session {
        Microsoft::WRL::ComPtr<IAudioSessionControl> sessionControl;
        Microsoft::WRL::ComPtr<ISimpleAudioVolume> audioVolume;
        std::string processName;
        DWORD processId;
        std::string icoPath;
    };

    bool initialized = false;
    std::vector<Session> sessions;

    Microsoft::WRL::ComPtr<IMMDeviceEnumerator> deviceEnumerator;
    Microsoft::WRL::ComPtr<IMMDevice> device;
    Microsoft::WRL::ComPtr<IAudioSessionManager2> sessionManager;
    Microsoft::WRL::ComPtr<IAudioSessionEnumerator> sessionEnumerator;

    std::string GetProcessNameById(DWORD processId) const;
};

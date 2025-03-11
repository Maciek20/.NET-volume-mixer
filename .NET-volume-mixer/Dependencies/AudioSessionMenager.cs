using System;
using System.Runtime.InteropServices;

namespace WebApplication3.Dependencies
{
    public class AudioSessionMenager
    {
        private const string dllPath = ".\\x64\\Debug\\AudioSessionWrapper.dll";

        private readonly IntPtr _WraperPointer;

        [DllImport(dllPath)]
        private static extern IntPtr CreateAudioSessionWrapper();

        [DllImport(dllPath)]
        private static extern void DeleteAudioSessionWrapper(IntPtr wrapperPointer);



        [DllImport(dllPath)]
        private static extern void UpdateSessions(IntPtr wrapperPointer);

        [DllImport(dllPath,EntryPoint = "SessionCount")]
        private static extern int _SessionCount(IntPtr wrapperPointer);



        [DllImport(dllPath)]
        [return: MarshalAs(UnmanagedType.BStr)]
        private static extern string GetProcessName(IntPtr wrapperPointer, int index);

        [DllImport(dllPath)]
        [return: MarshalAs(UnmanagedType.BStr)]
        private static extern string IcoPath(IntPtr wrapperPointer, int index);



        [DllImport(dllPath)]
        private static extern float GetVolume(IntPtr wrapperPointer, int index);

        [DllImport(dllPath)]
        private static extern void SetVolume(IntPtr wrapperPointer, int index, float volume);



        [DllImport(dllPath)]
        private static extern bool GetMute(IntPtr wrapperPointer, int index);

        [DllImport(dllPath)]
        private static extern void SetMute(IntPtr wrapperPointer, int index, bool mute);


        
        public int SessionCount
        {
            get
            {
                return _SessionCount(_WraperPointer);
            }
        }

        public AudioSessionMenager()
        {
            try
            {
                _WraperPointer = CreateAudioSessionWrapper();
            }
            catch (Exception e) {
                Console.WriteLine(e.ToString());
            }
        }

        ~AudioSessionMenager()
        {
            DeleteAudioSessionWrapper(_WraperPointer);
        }

        public void UpdateSessions()
        {
            UpdateSessions(_WraperPointer);
        }

        public string GetProcessName(int index)
        {
            return GetProcessName(_WraperPointer, index);
        }

        public string GetProcessIcoPath(int index)
        {
            return IcoPath(_WraperPointer, index);
        }

        public float GetVolume(int index)
        {
            return GetVolume(_WraperPointer, index);
        }

        public void SetVolume(int index, float volume)
        {
            SetVolume(_WraperPointer, index, volume);
        }

        public bool GetMute(int index)
        {
            return GetMute(_WraperPointer, index);
        }

        public void SetMute(int index, bool mute)
        {
            SetMute(_WraperPointer, index, mute);
        }
    }
}

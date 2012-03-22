/*
  xaudio2.hpp (2010-08-14)
  author: OV2

  ruby-specific header to provide mingw-friendly xaudio2 interfaces
*/

#include <windows.h>
#undef NULL
#define NULL 0

#include <guiddef.h>
#include <mmreg.h>

DEFINE_GUID(CLSID_XAudio2, 0xe21a7345, 0xeb21, 0x468e, 0xbe, 0x50, 0x80, 0x4d, 0xb9, 0x7c, 0xf7, 0x08);
DEFINE_GUID(CLSID_XAudio2_Debug, 0xf7a76c21, 0x53d4, 0x46bb, 0xac, 0x53, 0x8b, 0x45, 0x9c, 0xae, 0x46, 0xbd);
DEFINE_GUID(IID_IXAudio2, 0x8bcf1f58, 0x9fe7, 0x4583, 0x8a, 0xc6, 0xe2, 0xad, 0xc4, 0x65, 0xc8, 0xbb);

DECLARE_INTERFACE(IXAudio2Voice);

#define XAUDIO2_COMMIT_NOW         0
#define XAUDIO2_DEFAULT_CHANNELS   0
#define XAUDIO2_DEFAULT_SAMPLERATE 0
#define XAUDIO2_DEFAULT_FREQ_RATIO 4.0f
#define XAUDIO2_DEBUG_ENGINE       0x0001
#define XAUDIO2_VOICE_NOSRC        0x0004

enum XAUDIO2_DEVICE_ROLE
{
    NotDefaultDevice            = 0x0,
    DefaultConsoleDevice        = 0x1,
    DefaultMultimediaDevice     = 0x2,
    DefaultCommunicationsDevice = 0x4,
    DefaultGameDevice           = 0x8,
    GlobalDefaultDevice         = 0xf,
    InvalidDeviceRole = ~GlobalDefaultDevice
};

struct XAUDIO2_DEVICE_DETAILS
{
    WCHAR DeviceID[256];
    WCHAR DisplayName[256];
    XAUDIO2_DEVICE_ROLE Role;
    WAVEFORMATEXTENSIBLE OutputFormat;
};

struct XAUDIO2_VOICE_DETAILS
{
    UINT32 CreationFlags;
    UINT32 InputChannels;
    UINT32 InputSampleRate;
};

typedef enum XAUDIO2_WINDOWS_PROCESSOR_SPECIFIER
{
    Processor1  = 0x00000001,
    Processor2  = 0x00000002,
    Processor3  = 0x00000004,
    Processor4  = 0x00000008,
    Processor5  = 0x00000010,
    Processor6  = 0x00000020,
    Processor7  = 0x00000040,
    Processor8  = 0x00000080,
    Processor9  = 0x00000100,
    Processor10 = 0x00000200,
    Processor11 = 0x00000400,
    Processor12 = 0x00000800,
    Processor13 = 0x00001000,
    Processor14 = 0x00002000,
    Processor15 = 0x00004000,
    Processor16 = 0x00008000,
    Processor17 = 0x00010000,
    Processor18 = 0x00020000,
    Processor19 = 0x00040000,
    Processor20 = 0x00080000,
    Processor21 = 0x00100000,
    Processor22 = 0x00200000,
    Processor23 = 0x00400000,
    Processor24 = 0x00800000,
    Processor25 = 0x01000000,
    Processor26 = 0x02000000,
    Processor27 = 0x04000000,
    Processor28 = 0x08000000,
    Processor29 = 0x10000000,
    Processor30 = 0x20000000,
    Processor31 = 0x40000000,
    Processor32 = 0x80000000,
    XAUDIO2_ANY_PROCESSOR = 0xffffffff,
    XAUDIO2_DEFAULT_PROCESSOR = XAUDIO2_ANY_PROCESSOR
} XAUDIO2_WINDOWS_PROCESSOR_SPECIFIER, XAUDIO2_PROCESSOR;

struct XAUDIO2_VOICE_SENDS
{
    UINT32 OutputCount;
    IXAudio2Voice* *pOutputVoices;
};

struct XAUDIO2_EFFECT_DESCRIPTOR
{
    IUnknown *pEffect;
    BOOL InitialState;
    UINT32 OutputChannels;
};

struct XAUDIO2_EFFECT_CHAIN
{
    UINT32 EffectCount;
    const XAUDIO2_EFFECT_DESCRIPTOR *pEffectDescriptors;
};

enum XAUDIO2_FILTER_TYPE
{
    LowPassFilter,
    BandPassFilter,
    HighPassFilter
};

struct XAUDIO2_FILTER_PARAMETERS
{
    XAUDIO2_FILTER_TYPE Type;
    float Frequency;
    float OneOverQ;
};

struct XAUDIO2_BUFFER
{
    UINT32 Flags;
    UINT32 AudioBytes;
    const BYTE *pAudioData;
    UINT32 PlayBegin;
    UINT32 PlayLength;
    UINT32 LoopBegin;
    UINT32 LoopLength;
    UINT32 LoopCount;
    void *pContext;
};

struct XAUDIO2_BUFFER_WMA
{
    const UINT32 *pDecodedPacketCumulativeBytes;
    UINT32 PacketCount;
};

struct XAUDIO2_VOICE_STATE
{
    void *pCurrentBufferContext;
    UINT32 BuffersQueued;
    UINT64 SamplesPlayed;
};

struct XAUDIO2_PERFORMANCE_DATA
{
    UINT64 AudioCyclesSinceLastQuery;
    UINT64 TotalCyclesSinceLastQuery;
    UINT32 MinimumCyclesPerQuantum;
    UINT32 MaximumCyclesPerQuantum;
    UINT32 MemoryUsageInBytes;
    UINT32 CurrentLatencyInSamples;
    UINT32 GlitchesSinceEngineStarted;
    UINT32 ActiveSourceVoiceCount;
    UINT32 TotalSourceVoiceCount;
    UINT32 ActiveSubmixVoiceCount;
    UINT32 TotalSubmixVoiceCount;
    UINT32 ActiveXmaSourceVoices;
    UINT32 ActiveXmaStreams;
};

struct XAUDIO2_DEBUG_CONFIGURATION
{
    UINT32 TraceMask;
    UINT32 BreakMask;
    BOOL LogThreadID;
    BOOL LogFileline;
    BOOL LogFunctionName;
    BOOL LogTiming;
};

DECLARE_INTERFACE(IXAudio2EngineCallback)
{
    STDMETHOD_(void, OnProcessingPassStart) (void);
    STDMETHOD_(void, OnProcessingPassEnd) (void);
    STDMETHOD_(void, OnCriticalError) (HRESULT Error);
};

DECLARE_INTERFACE(IXAudio2VoiceCallback)
{
    STDMETHOD_(void, OnVoiceProcessingPassStart) (UINT32 BytesRequired);
    STDMETHOD_(void, OnVoiceProcessingPassEnd) (void);
    STDMETHOD_(void, OnStreamEnd) (void);
    STDMETHOD_(void, OnBufferStart) (void *pBufferContext);
    STDMETHOD_(void, OnBufferEnd) (void *pBufferContext);
    STDMETHOD_(void, OnLoopEnd) (void *pBufferContext);
    STDMETHOD_(void, OnVoiceError) (void *pBufferContext, HRESULT Error);
};

DECLARE_INTERFACE(IXAudio2Voice)
{
    STDMETHOD_(void, GetVoiceDetails) (XAUDIO2_VOICE_DETAILS *pVoiceDetails);
    STDMETHOD(SetOutputVoices) (const XAUDIO2_VOICE_SENDS *pSendList);
    STDMETHOD(SetEffectChain) (const XAUDIO2_EFFECT_CHAIN *pEffectChain);
    STDMETHOD(EnableEffect) (UINT32 EffectIndex, UINT32 OperationSet = XAUDIO2_COMMIT_NOW);
    STDMETHOD(DisableEffect) (UINT32 EffectIndex, UINT32 OperationSet = XAUDIO2_COMMIT_NOW);
    STDMETHOD_(void, GetEffectState) (UINT32 EffectIndex, BOOL *pEnabled);
    STDMETHOD(SetEffectParameters) (UINT32 EffectIndex, const void *pParameters, UINT32 ParametersByteSize,
        UINT32 OperationSet = XAUDIO2_COMMIT_NOW);
    STDMETHOD(GetEffectParameters) (UINT32 EffectIndex, void *pParameters, UINT32 ParametersByteSize);
    STDMETHOD(SetFilterParameters) (const XAUDIO2_FILTER_PARAMETERS *pParameters, UINT32 OperationSet = XAUDIO2_COMMIT_NOW);
    STDMETHOD_(void, GetFilterParameters) (XAUDIO2_FILTER_PARAMETERS *pParameters);
    STDMETHOD(SetVolume) (float Volume, UINT32 OperationSet = XAUDIO2_COMMIT_NOW);
    STDMETHOD_(void, GetVolume) (float *pVolume);
    STDMETHOD(SetChannelVolumes) (UINT32 Channels, const float *pVolumes, UINT32 OperationSet = XAUDIO2_COMMIT_NOW);
    STDMETHOD_(void, GetChannelVolumes) (UINT32 Channels, float *pVolumes);
    STDMETHOD(SetOutputMatrix) (IXAudio2Voice *pDestinationVoice, UINT32 SourceChannels, UINT32 DestinationChannels,
        const float *pLevelMatrix, UINT32 OperationSet = XAUDIO2_COMMIT_NOW);
    STDMETHOD_(void, GetOutputMatrix) (IXAudio2Voice *pDestinationVoice, UINT32 SourceChannels,
        UINT32 DestinationChannels, float *pLevelMatrix);
    STDMETHOD_(void, DestroyVoice) (void);
};

DECLARE_INTERFACE_(IXAudio2MasteringVoice, IXAudio2Voice){};

DECLARE_INTERFACE_(IXAudio2SubmixVoice, IXAudio2Voice){};

DECLARE_INTERFACE_(IXAudio2SourceVoice, IXAudio2Voice)
{
    STDMETHOD(Start) (UINT32 Flags, UINT32 OperationSet = XAUDIO2_COMMIT_NOW);
    STDMETHOD(Stop) (UINT32 Flags, UINT32 OperationSet = XAUDIO2_COMMIT_NOW);
    STDMETHOD(SubmitSourceBuffer) (const XAUDIO2_BUFFER *pBuffer, const XAUDIO2_BUFFER_WMA *pBufferWMA = NULL);
    STDMETHOD(FlushSourceBuffers) (void);
    STDMETHOD(Discontinuity) (void);
    STDMETHOD(ExitLoop) (UINT32 OperationSet = XAUDIO2_COMMIT_NOW);
    STDMETHOD_(void, GetState) (XAUDIO2_VOICE_STATE *pVoiceState);
    STDMETHOD(SetFrequencyRatio) (float Ratio, UINT32 OperationSet = XAUDIO2_COMMIT_NOW);
    STDMETHOD_(void, GetFrequencyRatio) (float *pRatio);
};

DECLARE_INTERFACE_(IXAudio2, IUnknown)
{
    STDMETHOD(GetDeviceCount) (UINT32 *pCount);
    STDMETHOD(GetDeviceDetails) (UINT32 Index, XAUDIO2_DEVICE_DETAILS *pDeviceDetails);
    STDMETHOD(Initialize) (UINT32 Flags = 0, XAUDIO2_PROCESSOR XAudio2Processor = XAUDIO2_DEFAULT_PROCESSOR);
    STDMETHOD(RegisterForCallbacks) (IXAudio2EngineCallback *pCallback);
    STDMETHOD_(void, UnregisterForCallbacks) (IXAudio2EngineCallback *pCallback);
    STDMETHOD(CreateSourceVoice) (IXAudio2SourceVoice* *ppSourceVoice, const WAVEFORMATEX *pSourceFormat, UINT32 Flags = 0,
        float MaxFrequencyRatio = XAUDIO2_DEFAULT_FREQ_RATIO, IXAudio2VoiceCallback *pCallback = NULL,
        const XAUDIO2_VOICE_SENDS *pSendList = NULL, const XAUDIO2_EFFECT_CHAIN *pEffectChain = NULL);
    STDMETHOD(CreateSubmixVoice) (IXAudio2SubmixVoice* *ppSubmixVoice, UINT32 InputChannels, UINT32 InputSampleRate,
        UINT32 Flags = 0, UINT32 ProcessingStage = 0, const XAUDIO2_VOICE_SENDS *pSendList = NULL,
        const XAUDIO2_EFFECT_CHAIN *pEffectChain = NULL);
    STDMETHOD(CreateMasteringVoice) (IXAudio2MasteringVoice* *ppMasteringVoice,
        UINT32 InputChannels = XAUDIO2_DEFAULT_CHANNELS, UINT32 InputSampleRate = XAUDIO2_DEFAULT_SAMPLERATE,
        UINT32 Flags = 0, UINT32 DeviceIndex = 0, const XAUDIO2_EFFECT_CHAIN *pEffectChain = NULL);
    STDMETHOD(StartEngine) (void);
    STDMETHOD_(void, StopEngine) (void);
    STDMETHOD(CommitChanges) (UINT32 OperationSet);
    STDMETHOD_(void, GetPerformanceData) (XAUDIO2_PERFORMANCE_DATA *pPerfData);
    STDMETHOD_(void, SetDebugConfiguration) (const XAUDIO2_DEBUG_CONFIGURATION *pDebugConfiguration, void *pReserved = NULL);
};

inline HRESULT XAudio2Create(IXAudio2* *ppXAudio2, UINT32 Flags = 0,
    XAUDIO2_PROCESSOR XAudio2Processor = XAUDIO2_DEFAULT_PROCESSOR)
{
    IXAudio2 *pXAudio2;
    HRESULT hr = CoCreateInstance((Flags & XAUDIO2_DEBUG_ENGINE) ? CLSID_XAudio2_Debug : CLSID_XAudio2, NULL,
        CLSCTX_INPROC_SERVER, IID_IXAudio2, (void**)&pXAudio2);
    if(SUCCEEDED(hr)){
        hr = pXAudio2->Initialize(Flags, XAudio2Processor);
        if(SUCCEEDED(hr))
            *ppXAudio2 = pXAudio2;
        else
            pXAudio2->Release();
    }
    return hr;
}
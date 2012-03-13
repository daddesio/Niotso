/*
    TSOSimulatorClient.hpp - Copyright (c) 2012 Fatbag <X-Fi6@phppoll.org>

    Permission to use, copy, modify, and/or distribute this software for any
    purpose with or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <basetyps.h>

DECLARE_INTERFACE(IMMDeviceCollection);
DECLARE_INTERFACE(IMMNotificationClient);
DECLARE_INTERFACE(IPropertyStore);

/*
** IMMDevice
*/

DECLARE_INTERFACE_(IMMDevice, IUnknown)
{
    STDMETHOD(Activate) (REFIID iid, DWORD dwClsCtx, PROPVARIANT *pActivationParams, void **ppInterface);
    STDMETHOD(OpenPropertyStore) (DWORD stgmAccess, IPropertyStore **ppProperties);
    STDMETHOD(GetId) (LPWSTR *ppstrId);
    STDMETHOD(GetState) (DWORD *pdwState);
};

/*
** IMMDeviceEnumerator
*/

enum EDataFlow
{
    eRender,
    eCapture,
    eAll
};

enum ERole
{
    eConsole,
    eMultimedia,
    eCommunications
};

DECLARE_INTERFACE_(IMMDeviceEnumerator, IUnknown)
{
    STDMETHOD(EnumAudioEndpoints) (EDataFlow dataFlow, DWORD dwStateMask, IMMDeviceCollection **ppDevices);
    STDMETHOD(GetDefaultAudioEndpoint) (EDataFlow dataFlow, ERole role, IMMDevice **ppEndpoint);
    STDMETHOD(GetDevice) (LPCWSTR pwstrId, IMMDevice **ppDevice);
    STDMETHOD(RegisterEndpointNotificationCallback) (IMMNotificationClient *pClient);
    STDMETHOD(UnregisterEndpointNotificationCallback) (IMMNotificationClient *pClient);
};

/*
** IAudioClient
*/

enum AUDCLNT_SHAREMODE
{
    AUDCLNT_SHAREMODE_SHARED,
    AUDCLNT_SHAREMODE_EXCLUSIVE
};

enum AUDCLNT_STREAMFLAGS
{
    AUDCLNT_STREAMFLAGS_CROSSPROCESS             = 0x00010000,
    AUDCLNT_STREAMFLAGS_LOOPBACK                 = 0x00020000,
    AUDCLNT_STREAMFLAGS_EVENTCALLBACK            = 0x00040000,
    AUDCLNT_STREAMFLAGS_NOPERSIST                = 0x00080000,
    AUDCLNT_STREAMFLAGS_RATEADJUST               = 0x00100000,
    AUDCLNT_SESSIONFLAGS_EXPIREWHENUNOWNED       = 0x10000000,
    AUDCLNT_SESSIONFLAGS_DISPLAY_HIDE            = 0x20000000,
    AUDCLNT_SESSIONFLAGS_DISPLAY_HIDEWHENEXPIRED = 0x40000000
};

DECLARE_INTERFACE_(IAudioClient, IUnknown)
{
    STDMETHOD(Initialize) (AUDCLNT_SHAREMODE ShareMode, DWORD StreamFlags, LONGLONG hnsBufferDuration,
        LONGLONG hnsPeriodicity, const WAVEFORMATEX *pFormat, LPCGUID AudioSessionGuid);
    STDMETHOD(GetBufferSize) (UINT32 *pNumBufferFrames);
    STDMETHOD(GetStreamLatency) (LONGLONG *phnsLatency);
    STDMETHOD(GetCurrentPadding) (UINT32 *pNumPaddingFrames);
    STDMETHOD(IsFormatSupported) (AUDCLNT_SHAREMODE ShareMode, const WAVEFORMATEX *pFormat, WAVEFORMATEX **ppClosestMatch);
    STDMETHOD(GetMixFormat) (WAVEFORMATEX **ppDeviceFormat);
    STDMETHOD(GetDevicePeriod) (LONGLONG *phnsDefaultDevicePeriod, LONGLONG *phnsMinimumDevicePeriod);
    STDMETHOD(Start) (void);
    STDMETHOD(Stop) (void);
    STDMETHOD(Reset) (void);
    STDMETHOD(SetEventHandle) (HANDLE eventHandle);
    STDMETHOD(GetService) (REFIID riid, void **ppv);
};

/*
** IAudioRenderClient
*/

enum AUDCLNT_BUFFERFLAGS
{
    AUDCLNT_BUFFERFLAGS_DATA_DISCONTINUITY = 0x1,
    AUDCLNT_BUFFERFLAGS_SILENT             = 0x2,
    AUDCLNT_BUFFERFLAGS_TIMESTAMP_ERROR    = 0x4
};

DECLARE_INTERFACE_(IAudioRenderClient, IUnknown)
{
    STDMETHOD(GetBuffer) (UINT32 NumFramesRequested, BYTE **ppData);
    STDMETHOD(ReleaseBuffer) (UINT32 NumFramesWritten, DWORD dwFlags);
};
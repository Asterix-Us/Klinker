#include "Enumerator.h"
#include "ObjectIDMap.h"
#include "Receiver.h"
#include "Sender.h"
#include "Unity/IUnityRenderingExtensions.h"
#include <consoleapi.h>

#pragma region Local functions

namespace
{
    // ID-receiver map
    klinker::ObjectIDMap<klinker::Receiver> receiverMap_;

    // Callback for texture update events
    void TextureUpdateCallback(int eventID, void* data)
    {
        auto event = static_cast<UnityRenderingExtEventType>(eventID);
        if (event == kUnityRenderingExtEventUpdateTextureBeginV2)
        {
            // UpdateTextureBegin
            auto params = reinterpret_cast<UnityRenderingExtTextureUpdateParamsV2*>(data);
            auto receiver = receiverMap_[params->userData];

            // Check if the size of the data matches.
            auto dataSize = params->width * params->height * params->bpp;
            if (receiver->CalculateFrameDataSize() != dataSize) return;

            // Lock the frame data for the update.
            params->texData = const_cast<uint8_t*>(receiver->LockFrameData());
        }
        else if (event == kUnityRenderingExtEventUpdateTextureEndV2)
        {
            // UpdateTextureEnd
            // Just unlock the frame data.
            auto params = reinterpret_cast<UnityRenderingExtTextureUpdateParamsV2*>(data);
            receiverMap_[params->userData]->UnlockFrameData();
        }
    }
}

#pragma endregion

#pragma region Plugin common functions

extern "C" void UNITY_INTERFACE_EXPORT
    UNITY_INTERFACE_API UnityPluginLoad(IUnityInterfaces* interfaces)
{
#if defined(_DEBUG)
    // Open a console for debug logging.
    AllocConsole();
    FILE* pConsole;
    freopen_s(&pConsole, "CONOUT$", "wb", stdout);
#endif
}

extern "C" UnityRenderingEventAndData
    UNITY_INTERFACE_EXPORT GetTextureUpdateCallback()
{
    return TextureUpdateCallback;
}

#pragma endregion

#pragma region Enumeration plugin functions

namespace { klinker::Enumerator enumerator_; }

extern "C" int UNITY_INTERFACE_EXPORT
    RetrieveDeviceNames(void* names[], int maxCount)
{
    enumerator_.ScanDeviceNames();
    return enumerator_.CopyStringPointers(names, maxCount);
}

extern "C" int UNITY_INTERFACE_EXPORT
    RetrieveInputFormatNames(int deviceIndex, void* names[], int maxCount)
{
    enumerator_.ScanOutputFormatNames(deviceIndex);
    return enumerator_.CopyStringPointers(names, maxCount);
}

#pragma endregion

#pragma region Receiver plugin functions

extern "C" void UNITY_INTERFACE_EXPORT * CreateReceiver(int device, int format)
{
    auto instance = new klinker::Receiver();
    receiverMap_.Add(instance);
    instance->StartReceiving(device, format);
    return instance;
}

extern "C" void UNITY_INTERFACE_EXPORT DestroyReceiver(void* receiver)
{
    auto instance = reinterpret_cast<klinker::Receiver*>(receiver);
    receiverMap_.Remove(instance);
    instance->StopReceiving();
    instance->Release();
}

extern "C" unsigned int UNITY_INTERFACE_EXPORT GetReceiverID(void* receiver)
{
    auto instance = reinterpret_cast<klinker::Receiver*>(receiver);
    return receiverMap_.GetID(instance);
}

extern "C" int UNITY_INTERFACE_EXPORT GetReceiverFrameWidth(void* receiver)
{
    auto instance = reinterpret_cast<klinker::Receiver*>(receiver);
    return std::get<0>(instance->GetFrameDimensions());
}

extern "C" int UNITY_INTERFACE_EXPORT GetReceiverFrameHeight(void* receiver)
{
    auto instance = reinterpret_cast<klinker::Receiver*>(receiver);
    return std::get<1>(instance->GetFrameDimensions());
}

extern "C" void UNITY_INTERFACE_EXPORT * GetReceiverFormatName(void* receiver)
{
    auto instance = reinterpret_cast<klinker::Receiver*>(receiver);
    static BSTR name = nullptr;
    if (name != nullptr) SysFreeString(name);
    name = instance->RetrieveFormatName();
    return name;
}

#pragma endregion

#pragma region Sender plugin functions

extern "C" void UNITY_INTERFACE_EXPORT * CreateSender(int device, int format, int preroll)
{
    auto instance = new klinker::Sender();
    instance->StartSending(device, format, preroll);
    return instance;
}

extern "C" void UNITY_INTERFACE_EXPORT DestroySender(void* sender)
{
    auto instance = reinterpret_cast<klinker::Sender*>(sender);
    instance->StopSending();
    instance->Release();
}

extern "C" int UNITY_INTERFACE_EXPORT GetSenderFrameWidth(void* sender)
{
    auto instance = reinterpret_cast<klinker::Sender*>(sender);
    return std::get<0>(instance->GetFrameDimensions());
}

extern "C" int UNITY_INTERFACE_EXPORT GetSenderFrameHeight(void* sender)
{
    auto instance = reinterpret_cast<klinker::Sender*>(sender);
    return std::get<1>(instance->GetFrameDimensions());
}

extern "C" int UNITY_INTERFACE_EXPORT IsSenderReferenceLocked(void* sender)
{
    auto instance = reinterpret_cast<klinker::Sender*>(sender);
    return instance->IsReferenceLocked() ? 1 : 0;
}

extern "C" void UNITY_INTERFACE_EXPORT UpdateSenderFrame(void* sender, void* data)
{
    auto instance = reinterpret_cast<klinker::Sender*>(sender);
    instance->UpdateFrame(data);
}

#pragma endregion

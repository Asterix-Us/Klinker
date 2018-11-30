#pragma once

#include "Common.h"
#include <algorithm>
#include <vector>

namespace klinker
{
    // Device/format enumerator class
    class Enumerator final
    {
    public:

        #pragma region Constructor/destructor

        ~Enumerator()
        {
            FreeStrings();
        }

        #pragma endregion

        #pragma region Accessor methods

        int CopyStringPointers(void* pointers[], int maxCount) const
        {
            auto count = std::min(maxCount, static_cast<int>(names_.size()));
            for (auto i = 0; i < count; i++) pointers[i] = names_[i];
            return count;
        }

        #pragma endregion

        #pragma region Enumeration methods

        void ScanDeviceNames()
        {
            // Invalidate previous enumeration.
            FreeStrings();

            // Device iterator
            IDeckLinkIterator* iterator;
            auto res = CoCreateInstance(
                CLSID_CDeckLinkIterator, nullptr, CLSCTX_ALL,
                IID_IDeckLinkIterator, reinterpret_cast<void**>(&iterator)
            );

            // If the driver is not found, return an empty list
            // without emitting any error.
            if (res != S_OK) return;

            // Device name enumeration
            IDeckLink* device;
            while (iterator->Next(&device) == S_OK)
            {
                BSTR name;
                ShouldOK(device->GetDisplayName(&name));
                names_.push_back(name);
                device->Release();
            }

            // Cleaning up
            iterator->Release();
        }

        void ScanOutputFormatNames(int deviceIndex)
        {
            // Invalidate previous enumeration.
            FreeStrings();

            // Device iterator
            IDeckLinkIterator* iterator;
            auto res = CoCreateInstance(
                CLSID_CDeckLinkIterator, nullptr, CLSCTX_ALL,
                IID_IDeckLinkIterator, reinterpret_cast<void**>(&iterator)
            );

            // If the driver is not found, return an empty list
            // without emitting any error.
            if (res != S_OK) return;

            // Iterate until reaching the specified index.
            IDeckLink* device = nullptr;
            for (auto i = 0; i <= deviceIndex; i++)
            {
                if (device !=  nullptr) device->Release();
                if (iterator->Next(&device) != S_OK)
                {
                    // Wrong device index: Return an empty list.
                    iterator->Release();
                    return;
                }
            }

            iterator->Release(); // The iterator is no longer needed.

            // Output interface of the specified device
            IDeckLinkOutput* output;
            ShouldOK(device->QueryInterface(
                IID_IDeckLinkOutput, reinterpret_cast<void**>(&output)
            ));

            device->Release(); // The device object is no longer needed.

            // Display mode iterator
            IDeckLinkDisplayModeIterator* dmIterator;
            ShouldOK(output->GetDisplayModeIterator(&dmIterator));

            output->Release(); // The output interface is no longer needed.

            // Display mode name enumeration
            IDeckLinkDisplayMode* mode;
            while (dmIterator->Next(&mode) == S_OK)
            {
                BSTR name;
                ShouldOK(mode->GetName(&name));
                names_.push_back(name);
                mode->Release();
            }

            // Cleaning up
            dmIterator->Release();
        }

        #pragma endregion

    private:

        #pragma region Private members

        std::vector<BSTR> names_;

        void FreeStrings()
        {
            for (auto s : names_) SysFreeString(s);
            names_.clear();
        }

        #pragma endregion
    };
}

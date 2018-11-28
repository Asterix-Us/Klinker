using Unity.Collections;
using Unity.Collections.LowLevel.Unsafe;
using UnityEngine;
using System;
using System.Runtime.InteropServices;

namespace Klinker
{
    sealed class SenderPlugin : IDisposable
    {
        #region Factory methods

        public static SenderPlugin CreateAsyncSender(int device, int format, int preroll)
        {
            return new SenderPlugin(_CreateAsyncSender(device, format, preroll));
        }

        public static SenderPlugin CreateManualSender(int device, int format)
        {
            return new SenderPlugin(_CreateManualSender(device, format));
        }

        #endregion

        #region Disposable pattern

        SenderPlugin(IntPtr plugin)
        {
            _plugin = plugin;
        }

        ~SenderPlugin()
        {
            if (_plugin != IntPtr.Zero)
                Debug.LogError("Sender instance should be disposed before finalization.");
        }

        public void Dispose()
        {
            if (_plugin != IntPtr.Zero)
            {
                DestroySender(_plugin);
                _plugin = IntPtr.Zero;
            }
        }

        #endregion

        #region Public properties

        public Vector2Int FrameDimensions { get {
            return new Vector2Int(
                GetSenderFrameWidth(_plugin),
                GetSenderFrameHeight(_plugin)
            );
        } }

        public float FrameRate { get {
            return GetSenderFrameRate(_plugin);
        } }

        public bool IsProgressive { get {
            return IsSenderProgressive(_plugin) != 0;
        } }

        public bool IsReferenceLocked { get {
            return IsSenderReferenceLocked(_plugin) != 0;
        } }

        #endregion

        #region Public methods

        public unsafe void FeedFrame<T>(NativeArray<T> data) where T : struct
        {
            FeedFrameToSender(_plugin, (IntPtr)data.GetUnsafeReadOnlyPtr());
        }

        public void WaitCompletion(ulong frameNumber)
        {
            WaitSenderCompletion(_plugin, frameNumber);
        }

        #endregion

        #region Unmanaged code entry points

        IntPtr _plugin;

        [DllImport("Klinker", EntryPoint="CreateAsyncSender")]
        static extern IntPtr _CreateAsyncSender(int device, int format, int preroll);

        [DllImport("Klinker", EntryPoint="CreateManualSender")]
        static extern IntPtr _CreateManualSender(int device, int format);

        [DllImport("Klinker")]
        static extern void DestroySender(IntPtr sender);

        [DllImport("Klinker")]
        static extern int GetSenderFrameWidth(IntPtr sender);

        [DllImport("Klinker")]
        static extern int GetSenderFrameHeight(IntPtr sender);

        [DllImport("Klinker")]
        static extern float GetSenderFrameRate(IntPtr sender);

        [DllImport("Klinker")]
        static extern int IsSenderProgressive(IntPtr sender);

        [DllImport("Klinker")]
        static extern int IsSenderReferenceLocked(IntPtr sender);

        [DllImport("Klinker")]
        static extern void FeedFrameToSender(IntPtr sender, IntPtr frameData);

        [DllImport("Klinker")]
        static extern void WaitSenderCompletion(IntPtr sender, ulong frameNumber);

        #endregion
    }
}

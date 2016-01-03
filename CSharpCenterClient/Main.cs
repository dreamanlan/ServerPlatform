using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace CSharpCenterClient
{
  public static class CenterClientApi
  {
    [DllImport("CenterClientLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern int ReloadConfigScript();

    [DllImport("CenterClientLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool GetConfig(string key, StringBuilder nameBuf, int len);

    [DllImport("CenterClientLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern int TargetHandle([MarshalAs(UnmanagedType.LPStr)]string name);

    [DllImport("CenterClientLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool TargetName(int handle, StringBuilder nameBuf, int len);

    [DllImport("CenterClientLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool SendByHandle(int handle, byte[] data, int len);

    [DllImport("CenterClientLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool SendByName(string name, byte[] data, int len);

    [DllImport("CenterClientLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool SendCommandByHandle(int handle, string command);

    [DllImport("CenterClientLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool SendCommandByName(string name, string command);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void HandleNameHandleChangedCallback(bool addOrUpdate, string name, int handle);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void HandleMessageCallback(uint seq, int src, int dest, IntPtr data, int len);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void HandleMessageResultCallback(uint seq, int src, int dest, int result);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void HandleCommandCallback(int src, int dest, string msg);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void CenterLogHandler(string msg, int len);

    [DllImport("CenterClientLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool IsRun();

    [DllImport("CenterClientLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern void Quit();

    [DllImport("CenterClientLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetCenterLogHandler(CenterLogHandler logHandler);

    [DllImport("CenterClientLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern void Init(string serverType, int argc, string[] argv, HandleNameHandleChangedCallback nameHandleCallback, HandleMessageCallback msgCallback, HandleMessageResultCallback msgResultCallback, HandleCommandCallback cmdResultCallback);

    [DllImport("CenterClientLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern void Tick();

    [DllImport("CenterClientLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool IsConnected();

    [DllImport("CenterClientLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern void Disconnect();

    [DllImport("CenterClientLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern void Release();
  }
  public static class CenterHubApi
  {
    [DllImport("CenterHubLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern int ReloadConfigScript();

    [DllImport("CenterHubLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool GetConfig(string key, StringBuilder nameBuf, int len);

    [DllImport("CenterHubLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern int TargetHandle(int worldId, [MarshalAs(UnmanagedType.LPStr)]string name);

    [DllImport("CenterHubLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool TargetName(int worldId, int handle, StringBuilder nameBuf, int len);

    [DllImport("CenterHubLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool SendByHandle(int worldId, int handle, byte[] data, int len);

    [DllImport("CenterHubLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool SendByName(int worldId, string name, byte[] data, int len);

    [DllImport("CenterHubLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool SendCommandByHandle(int worldId, int handle, string command);

    [DllImport("CenterHubLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool SendCommandByName(int worldId, string name, string command);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void HandleNameHandleChangedCallback(int worldId, bool addOrUpdate, string name, int handle);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void HandleMessageCallback(int worldId, uint seq, int src, int dest, IntPtr data, int len);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void HandleMessageResultCallback(int worldId, uint seq, int src, int dest, int result);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void HandleCommandCallback(int worldId, int src, int dest, string msg);

    [UnmanagedFunctionPointer(CallingConvention.Cdecl)]
    public delegate void CenterLogHandler(string msg, int len);

    [DllImport("CenterHubLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool IsRun();

    [DllImport("CenterHubLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern void Quit();

    [DllImport("CenterHubLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern void SetCenterLogHandler(CenterLogHandler logHandler);

    [DllImport("CenterHubLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern void Init(string serverType, int argc, string[] argv, HandleNameHandleChangedCallback nameHandleCallback, HandleMessageCallback msgCallback, HandleMessageResultCallback msgResultCallback, HandleCommandCallback cmdResultCallback);

    [DllImport("CenterHubLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern void Tick();

    [DllImport("CenterHubLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern bool IsConnected(int worldId);

    [DllImport("CenterHubLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern void Disconnect(int worldId);

    [DllImport("CenterHubLibrary", CallingConvention = CallingConvention.Cdecl)]
    public static extern void Release();
  }
}

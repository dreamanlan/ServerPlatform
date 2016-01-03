using System;
using System.Collections.Generic;
using System.Threading;
using System.Text;
using System.Runtime.InteropServices;
using CSharpCenterClient;

namespace CSharpClientTest
{
  class Program
  {
    public static void HandleNameHandleChanged(bool addOrUpdate, string name, int handle)
    {
      Console.WriteLine("NameHandle:{0} {1}->{2}", addOrUpdate, name, handle);
    }
    public static void HandleMessage(uint seq, int src, int dest, IntPtr data, int len)
    {
      byte[] bytes = new byte[len];
      Marshal.Copy(data, bytes, 0, len);
      string s = Encoding.ASCII.GetString(bytes);
      Console.WriteLine("Echo:{0}", s);
    }
    public static void HandleMessageResult(uint seq, int src, int dest, int result)
    {
        Console.WriteLine("Echo:{0} {1}", seq, result);
    }

    public static void HandleCommand(int src, int dest, string result)
    {
      Console.WriteLine("Command result {0}->{1}:{2}", src, dest, result);
    }
    public static void HandleNameHandleChanged2(int worldId, bool addOrUpdate, string name, int handle)
    {
      Console.WriteLine("NameHandle2:{0} {1} {2}->{3}", worldId, addOrUpdate, name, handle);
    }
    public static void HandleMessage2(int worldId, uint seq, int src, int dest, IntPtr data, int len)
    {
      byte[] bytes = new byte[len];
      Marshal.Copy(data, bytes, 0, len);
      string s = Encoding.ASCII.GetString(bytes);
      Console.WriteLine("Echo2:{0} {1}", worldId, s);
    }
    public static void HandleMessageResult2(int worldId, uint seq, int src, int dest, int result)
    {
        Console.WriteLine("Echo2 Result:{0} {1} {2}", worldId, seq, result);
    }

    public static void HandleCommand2(int worldId, int src, int dest, string result)
    {
      Console.WriteLine("Command2 result {0} {1}->{2}:{3}", worldId, src, dest, result);
    }
    static void Main(string[] args)
    {
#if TEST_CENTER_CLIENT
      CenterClientApi.HandleNameHandleChangedCallback cb1 = HandleNameHandleChanged;
      CenterClientApi.HandleMessageCallback cb2 = HandleMessage;
      CenterClientApi.HandleMessageResultCallback cb3 = HandleMessageResult;
      CenterClientApi.HandleCommandCallback cb4 = HandleCommand;
      byte[] bytes = Encoding.ASCII.GetBytes("test test test");
      CenterClientApi.Init("csharpclient", args.Length, args, cb1, cb2, cb3, cb4);

      CenterClientApi.ReloadConfigScript();
      CenterClientApi.ReloadConfigScript();

      for (; ; ) {
        CenterClientApi.Tick();
        Thread.Sleep(10);

        int handle = CenterClientApi.TargetHandle("ServerCenter");
        if (handle > 0) {
          bool ret = CenterClientApi.SendByHandle(handle, bytes, (ushort)bytes.Length);
          CenterClientApi.SendCommandByHandle(handle, "output('test test test');");
        }
      }
      //CenterClientApi.Release();
#else
        CenterHubApi.HandleNameHandleChangedCallback cb1 = HandleNameHandleChanged2;
      CenterHubApi.HandleMessageCallback cb2 = HandleMessage2;
      CenterHubApi.HandleMessageResultCallback cb3 = HandleMessageResult2;
      CenterHubApi.HandleCommandCallback cb4 = HandleCommand2;
      byte[] bytes = Encoding.ASCII.GetBytes("test test test");
      CenterHubApi.Init("centerhub", args.Length, args, cb1, cb2, cb3, cb4);

      CenterHubApi.ReloadConfigScript();
      CenterHubApi.ReloadConfigScript();

      for (; ; ) {
        CenterHubApi.Tick();
        Thread.Sleep(10);

        int handle = CenterHubApi.TargetHandle(0, "hub2world0");
        if (handle > 0) {
          bool ret = CenterHubApi.SendByHandle(0, handle, bytes, (ushort)bytes.Length);
          CenterHubApi.SendCommandByHandle(0, handle, "output('test test test');");
        }
        handle = CenterHubApi.TargetHandle(1, "hub2world1");
        if (handle > 0) {
          bool ret = CenterHubApi.SendByHandle(1, handle, bytes, (ushort)bytes.Length);
          CenterHubApi.SendCommandByHandle(1, handle, "output('test test test');");
        }
      }
      //CenterHubApi.Release();
#endif
    }
  }
}

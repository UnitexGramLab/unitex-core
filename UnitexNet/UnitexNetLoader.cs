using System;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;

namespace Evercontact4Outlook.Lib
{
    public class UnitexNetLoader: IDisposable
    {
        /// <summary>
        /// 64 bit dll location - defaults to <assemblydir>\UnitexNet64.dll
        /// </summary>
        public string DllLocation64Bit;

        /// <summary>
        /// 32 bit dll location - defaults to <assemblydir>\UnitexNet.dll
        /// </summary>
        public string DllLocation32Bit;

        /// <summary>
        /// Default constructor.
        /// 
        /// The UnitexNet.dll and UnitexNet64.dll will be looked for in the default running assembly directory.
        /// </summary>
        public UnitexNetLoader()
        {
            //default locations of the dlls

            //use CodeBase instead of Location because of Shadow Copy.
            var vUri = new UriBuilder(Assembly.GetExecutingAssembly().CodeBase);
            var vPath = Uri.UnescapeDataString(vUri.Path + vUri.Fragment);
            var directory = Path.GetDirectoryName(vPath);
            DllLocation64Bit = Path.Combine(directory, "UnitexNet64.dll");
            DllLocation32Bit = Path.Combine(directory, "UnitexNet.dll");
            LoadLibrary();
        }

        /// <summary>
        /// Constructor specifying the directory where to look for UnitexNet.dll and UnitexNet64.dll.
        /// </summary>
        /// <param name="unitexNetDirectory">The directory where to look for DLLs.</param>
        public UnitexNetLoader(string unitexNetDirectory)
        {
            DllLocation64Bit = Path.Combine(unitexNetDirectory, "UnitexNet64.dll");
            DllLocation32Bit = Path.Combine(unitexNetDirectory, "UnitexNet.dll");
            LoadLibrary();
        }

        ~UnitexNetLoader()
        {
            Dispose(false);
            GC.SuppressFinalize(this);
        }

        #region IDisposable

        public void Dispose()
        {
            Dispose(true);
        }

        private void Dispose(bool disposing)
        {
            if (disposing)
            {
                if (!_dllHandle.Equals(IntPtr.Zero))
                {
                    var dllCanUnloadNowPtr = Win32NativeMethods.GetProcAddress(_dllHandle, "DllCanUnloadNow");
                    if (!dllCanUnloadNowPtr.Equals(IntPtr.Zero))
                    {
                        var dllCanUnloadNow = (DllCanUnloadNow) Marshal.GetDelegateForFunctionPointer(dllCanUnloadNowPtr, typeof(DllCanUnloadNow));
                        if (dllCanUnloadNow() != 0) return; //there are still live objects returned by the dll, so we should not unload the dll
                    }
                    Win32NativeMethods.FreeLibrary(_dllHandle);
                    _dllHandle = IntPtr.Zero;
                }
            }
        }

        #endregion

        #region private methods

        [ComVisible(false)]
        [ComImport, InterfaceType(ComInterfaceType.InterfaceIsIUnknown), Guid("00000001-0000-0000-C000-000000000046")]
        private interface IClassFactory
        {
            void CreateInstance([MarshalAs(UnmanagedType.Interface)] object pUnkOuter, ref Guid refiid, [MarshalAs(UnmanagedType.Interface)] out object ppunk);
            void LockServer(bool fLock);
        }

        [ComVisible(false)]
        [ComImport, InterfaceType(ComInterfaceType.InterfaceIsIUnknown), Guid("00000000-0000-0000-C000-000000000046")]
        private interface IUnknown
        {
        }

        private delegate int DllGetClassObject(ref Guid ClassId, ref Guid InterfaceId, [Out, MarshalAs(UnmanagedType.Interface)] out object ppunk);
        private delegate int DllCanUnloadNow();

        //COM GUIDs
        private static Guid IID_IClassFactory = new Guid("00000001-0000-0000-C000-000000000046");
        private static Guid IID_IUnknown = new Guid("00000000-0000-0000-C000-000000000046");

        //win32 functions to load\unload dlls and get a function pointer 
        private static class Win32NativeMethods
        {
            [DllImport("kernel32.dll", CharSet = CharSet.Ansi)]
            public static extern IntPtr GetProcAddress(IntPtr hModule, string lpProcName);

            [DllImport("kernel32.dll")]
            public static extern bool FreeLibrary(IntPtr hModule);

            [DllImport("kernel32.dll", CharSet = CharSet.Unicode)]
            public static extern IntPtr LoadLibraryW(string lpFileName);
        }

        //private variables
        private IntPtr _dllHandle = IntPtr.Zero;
        private readonly object _criticalSection = new object();

        private void LoadLibrary()
        {
            lock (_criticalSection)
            {
                if (_dllHandle.Equals(IntPtr.Zero))
                {
                    var dllPath = IntPtr.Size == 8 ? DllLocation64Bit : DllLocation32Bit;
                    _dllHandle = Win32NativeMethods.LoadLibraryW(dllPath);
                    if (_dllHandle.Equals(IntPtr.Zero))
                        throw new Exception(string.Format("Could not load '{0}'\nMake sure the dll exists.", dllPath));
                }
            }
        }

        #endregion
    }
}

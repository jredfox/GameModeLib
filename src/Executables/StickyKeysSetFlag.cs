using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace DisableStickyKeys
{
    class Program
    {
        const uint SPI_GETSTICKYKEYS = 0x003A;
        const uint SPI_SETSTICKYKEYS = 0x003B;
        const uint SKF_HOTKEYACTIVE = 0x00000004;
        const uint SKF_CONFIRMHOTKEY = 0x00000008;

        [StructLayout(LayoutKind.Sequential)]
        public struct STICKYKEYS
        {
            public int cbSize;
            public uint dwFlags;
        }

        [DllImport("user32.dll", EntryPoint = "SystemParametersInfo", SetLastError = false)]
        static extern bool SystemParametersInfo(uint uiAction, uint uiParam, ref STICKYKEYS pvParam, uint fWinIni);

        static void Main(string[] args)
        {
            if (args.Length == 0)
                return;
            string strflag = args[0].ToLower();
            //Fetch the Current Flag From the Registry and sync it with the current in memory sticky keys
            if(strflag.Equals("sync") || strflag.Equals("update"))
            {
                string registryPath = @"HKEY_CURRENT_USER\Control Panel\Accessibility\StickyKeys";
                string valueName = "Flags";
                strflag = (string) Registry.GetValue(registryPath, valueName, null);
            }
            uint flag = Convert.ToUInt32(strflag, strflag.StartsWith("0x") ? 16 : 10);
            STICKYKEYS skOff = new STICKYKEYS { cbSize = Marshal.SizeOf(typeof(STICKYKEYS)), dwFlags = 0 };
            SystemParametersInfo(SPI_GETSTICKYKEYS, (uint)Marshal.SizeOf(typeof(STICKYKEYS)), ref skOff, 0);
            skOff.dwFlags = flag;
            SystemParametersInfo(SPI_SETSTICKYKEYS, (uint)Marshal.SizeOf(typeof(STICKYKEYS)), ref skOff, 0);
        }
    }
}

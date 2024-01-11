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
            STICKYKEYS skOff = new STICKYKEYS { cbSize = Marshal.SizeOf(typeof(STICKYKEYS)), dwFlags = 0 };
            SystemParametersInfo(SPI_GETSTICKYKEYS, (uint)Marshal.SizeOf(typeof(STICKYKEYS)), ref skOff, 0);
            skOff.dwFlags &= ~SKF_HOTKEYACTIVE;
            skOff.dwFlags &= ~SKF_CONFIRMHOTKEY;
            SystemParametersInfo(SPI_SETSTICKYKEYS, (uint)Marshal.SizeOf(typeof(STICKYKEYS)), ref skOff, 0);
        }
    }
}

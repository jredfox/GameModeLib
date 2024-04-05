using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GenSynaptics
{
    class Program
    {
        static void Main(string[] args)
        {
            //Restore Synaptics Devices Back to their Default States
            string a = args[0].ToLower();
            bool Gen = !(a.Equals("/restore") || a.Equals("/restoredefaults"));

            //Get Absolute File and add Extension if Required
            string str_file = Path.GetFullPath(args[Gen ? 0 : 1]);
            if (!str_file.ToUpper().EndsWith(".REG"))
            {
                str_file += ".reg";
            }
            //Create Dir if it doesn't exist
            string dir = Path.GetDirectoryName(str_file);
            if (!Directory.Exists(dir))
            {
                Directory.CreateDirectory(dir);
            }
            //Cleanup
            if (File.Exists(str_file))
                File.Delete(str_file);

            using (StreamWriter writer = new StreamWriter(str_file))
            {
                RegistryKey tree = RegistryKey.OpenBaseKey(RegistryHive.CurrentUser, Environment.Is64BitOperatingSystem ? RegistryView.Registry64 : RegistryView.Default);
                RegistryKey key_syntp = tree.OpenSubKey(@"SOFTWARE\Synaptics\SynTP", false);
                if (key_syntp == null)
                {
                    Console.WriteLine(@"Synaptics\SynTP Not Found");
                    System.Environment.Exit(404);
                    return;
                }
                writer.WriteLine("Windows Registry Editor Version 5.00");
                foreach (var device in key_syntp.GetSubKeyNames())
                {
                    try
                    {
                        RegistryKey key_device = key_syntp.OpenSubKey(device, false);
                        if(Gen)
                        {
                            writer.WriteLine("\r\n[" + key_device.Name + "]");
                            writer.WriteLine("\"PalmDetectConfig\"=dword:00000000");
                            writer.WriteLine("\"PalmRejectAlways\"=dword:00000000");
                            writer.WriteLine("\"PalmRT\"=dword:00000000");
                        }
                        else
                        {
                            writer.WriteLine("\r\n[-" + key_device.Name + "]");
                            writer.WriteLine("\r\n[" + key_device.Name + "]");
                            writer.WriteLine("\"NotifyDriverFirstLoadState\"=dword:00000000");
                        }
                        Close(key_device);
                    }
                    catch (Exception e)
                    {
                        Console.Error.WriteLine(e);
                    }
                }
                Close(key_syntp);
            }
        }
        public static void Close(RegistryKey key)
        {
            try
            {
                if (key != null)
                    key.Close();
            }
            catch (Exception e)
            {
                Console.Error.WriteLine(e);
            }
        }
    }
}

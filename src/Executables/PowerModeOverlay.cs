using Microsoft.Win32;
using System;
using System.Configuration;
using System.Runtime.InteropServices;
using System.Security.AccessControl;

namespace PowerMode
{
    class SetPowerMode
    {
        public static Guid PowerSaver = new Guid("961cc777-2547-4f9d-8174-7d86181b8a7a");
        public static Guid Balanced = new Guid("00000000-0000-0000-0000-000000000000");
        public static Guid Performance = new Guid("3af9B8d9-7c97-431d-ad78-34a8bfea439f");
        public static Guid HighPerformance = new Guid("ded574b5-45a0-4f42-8737-46345c09c238");

        [DllImportAttribute("powrprof.dll", EntryPoint = "PowerSetActiveOverlayScheme")]
        public static extern uint PowerSetActiveOverlayScheme(out Guid OverlaySchemeGuid);

        [DllImportAttribute("powrprof.dll", EntryPoint = "PowerGetEffectiveOverlayScheme")]
        private static extern uint PowerGetEffectiveOverlayScheme(out Guid EffectiveOverlayPolicyGuid);
        static int Main(string[] args)
        {
            if (args.Length == 0)
                return -1;

            string strguid = args[0].ToLower();
            if (strguid.Equals("get"))
            {
                uint result = PowerGetEffectiveOverlayScheme(out Guid currentMode);
                Console.WriteLine(currentMode);
                return result == 0 ? 0 : -1;
            }
            else if (strguid.Equals("getname"))
            {
                uint result = PowerGetEffectiveOverlayScheme(out Guid currentMode);
                if (currentMode.Equals(PowerSaver))
                {
                    Console.WriteLine("PowerSaver");
                }
                else if (currentMode.Equals(Balanced))
                {
                    Console.WriteLine("Balanced");
                }
                else if (currentMode.Equals(Performance))
                {
                    Console.WriteLine("Performance");
                }
                else if (currentMode.Equals(HighPerformance))
                {
                    Console.WriteLine("HighPerformance");
                }
                else
                {
                    Console.WriteLine(currentMode);
                }
                return result == 0 ? 0 : -1;
            }
            else if (strguid.Equals("update") || strguid.Equals("sync"))
            {
                try
                {
                    Guid regPowerMode;
                    try
                    {
                        RegistryKey pwr = Registry.LocalMachine.OpenSubKey(@"SYSTEM\CurrentControlSet\Control\Power\User\PowerSchemes", RegistryKeyPermissionCheck.ReadWriteSubTree, RegistryRights.QueryValues);
                        regPowerMode = new Guid((string)pwr.GetValue("ActiveOverlayAcPowerScheme"));
                        closeKey(pwr);
                    }
                    catch(Exception e)
                    {
                        Console.WriteLine("Failed to get ActiveOverlay PowerScheme from the registry Exception:" + e.GetType().Name);
                        return -1;
                    }

                    uint result = PowerSetActiveOverlayScheme(out regPowerMode);
                    if (result != 0)
                    {
                        Console.Error.WriteLine("Failed to set PowerModeOverlay GUID " + regPowerMode + "\n");
                        return 5;
                    }
                }
                catch (Exception exception)
                {
                    Console.Error.WriteLine("{0}: {1}\n{2}", exception.GetType(), exception.Message, exception.StackTrace);
                    Console.WriteLine();
                    return 1;
                }
                return 0;
            }
            else if (strguid.Equals("help") || strguid.Equals("/?") || strguid.Equals("/help"))
            {
                Console.WriteLine("");
                Console.WriteLine("PowerModeOverlay.exe <Options>");
                Console.WriteLine("Set Options {PowerSaver, Balanced, Performance, HighPerformance or <GUID>}");
                Console.WriteLine("Get (prints current guid)");
                Console.WriteLine("GetName (gets the readable name or guid of the current PowerModeOverlay)");
                Console.WriteLine("Update (updates the current power mode that is fetched from the registry)");
                Console.WriteLine("Sync (Same as Update)");
                Console.WriteLine("Help");
                Console.WriteLine("Examples:");
                Console.WriteLine("PowerModeOverlay.exe HighPerformance");
                Console.WriteLine("PowerModeOverlay.exe ded574b5-45a0-4f42-8737-46345c09c238");
                Console.WriteLine("PowerModeOverlay.exe Get");
                Console.WriteLine("PowerModeOverlay.exe GetName");
                Console.WriteLine("PowerModeOverlay.exe Sync");
                return 0;
            }
            Guid powerMode = HighPerformance;
            if (strguid.Equals("powersaver"))
            {
                powerMode = PowerSaver;
            }
            else if (strguid.Equals("balanced"))
            {
                powerMode = Balanced;
            }
            else if (strguid.Equals("performance"))
            {
                powerMode = Performance;
            }
            else if (strguid.Equals("highperformance"))
            {
                powerMode = HighPerformance;
            }
            else
            {
                try
                {
                    powerMode = new Guid(strguid);
                }
                catch (Exception)
                {
                    Console.Error.WriteLine("Failed to parse GUID.\n");
                    return -1;
                }
            }
            try
            {
                uint result = PowerSetActiveOverlayScheme(out powerMode);
                if (result != 0)
                {
                    Console.Error.WriteLine("Failed to set PowerModeOverlay GUID " + powerMode + "\n");
                    return 5;
                }
            }
            catch (Exception exception)
            {
                Console.Error.WriteLine("{0}: {1}\n{2}", exception.GetType(), exception.Message, exception.StackTrace);
                Console.WriteLine();
                return 1;
            }
            return 0;
        }

        public static void closeKey(RegistryKey key)
        {
            try
            {
                key.Close();
            }
            catch (Exception e)
            {
                Console.Error.WriteLine(e.Message);
            }
        }
    }
}

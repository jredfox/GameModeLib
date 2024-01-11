using System;
using System.Configuration;
using System.Runtime.InteropServices;

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
        static int Main(string[] args)
        {
            if (args.Length == 0)
                return -1;

            string strguid = args[0].ToLower();
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
    }
}

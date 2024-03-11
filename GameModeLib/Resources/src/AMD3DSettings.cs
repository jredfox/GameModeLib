using Microsoft.Win32;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Compression;
using System.Threading;
using System.Linq;
using System.Security.AccessControl;
using System.Text;
using System.Threading.Tasks;

namespace AMD3dSettings
{
    class AMD3dSettings
    {

        static void Main(string[] args)
        {
            string cncmd = GetCnCMD();
            if (cncmd is null)
            {
                Console.Error.WriteLine("AMD's CNCMD.exe Is Not Found");
                return;
            }

            //Export Current Settings and Extract them
            string AMD3D = (System.IO.Path.GetTempPath() + @"\AMD3DSettings.zip").Replace(@"\\", @"\");
            string AMD3DDir = (System.IO.Path.GetTempPath() + @"\AMD3DSettings").Replace(@"\\", @"\");
            string AMD3DJSON = (System.IO.Path.GetTempPath() + @"\AMD3DSettings\rssettings.json").Replace(@"\\", @"\");
            Run("\"" + cncmd + "\" export \"" + AMD3D + "\"");

            //Wait Until File is Created for some odd reason it doesn't create it from the first command
            long start = DateTimeOffset.UtcNow.ToUnixTimeMilliseconds();
            while (!File.Exists(AMD3D))
            {
                Thread.Sleep(1);
                long time = DateTimeOffset.UtcNow.ToUnixTimeMilliseconds();
                if (time < start || (time - start) >= 5500L)
                {
                    break;
                }
            }
            if (!File.Exists(AMD3D))
            {
                Console.Error.WriteLine("AMD3DSettings Export JSON Failed:" + AMD3D);
                return;
            }

            //Backup Current 3D Settings and Generate Uninstall Batch
            if (args.Length > 0)
            {
                string udir = args[0].Trim();
                if (!string.IsNullOrEmpty(udir))
                {
                    try
                    {
                        string bak = udir + "\\AMD3DSettings.zip";
                        string cmd = udir + "\\AMD3DSettings.bat";
                        if (!File.Exists(bak))
                        {
                            if (!Directory.Exists(udir))
                            {
                                Directory.CreateDirectory(udir);
                            }
                            File.Copy(AMD3D, bak);
                            File.WriteAllText(cmd, "@ECHO OFF\r\nsetlocal enableDelayedExpansion\r\ncall \"" + cncmd + "\" import \"%~dp0AMD3DSettings.zip\"\r\ntimeout /NOBREAK /t 3 >nul 2>&1\r\ncall \"" + cncmd + "\" import \"%~dp0AMD3DSettings.zip\"");
                        }
                    }
                    catch (Exception e)
                    {
                        Console.Error.WriteLine(e.Message);
                    }
                }
            }

            try
            {
                Directory.Delete(AMD3DDir, true);
            }
            catch (Exception) { }

            try
            {
                ZipFile.ExtractToDirectory(AMD3D, AMD3DDir);
            }
            catch (Exception e)
            {
                Console.Error.WriteLine(e.Message);
            }

            //Edit JSON Settings
            JObject root;
            using (StreamReader reader = File.OpenText(AMD3DJSON))
            {
                root = (JObject)JToken.ReadFrom(new JsonTextReader(reader));
                display(root);
                global(root);
                video(root);
                powersaver(root);
                sampling(root);
            }
            //Save the JSON
            File.WriteAllText(AMD3DJSON, root.ToString());
            //Re-Import the JSON
            File.Delete(AMD3D);
            ZipFile.CreateFromDirectory(AMD3DDir, AMD3D);
            Run("\"" + cncmd + "\" import \"" + AMD3D + "\"");
            //Run it a second time as Vari-Bright won't change the first time
            Thread.Sleep(2500);
            Run("\"" + cncmd + "\" import \"" + AMD3D + "\"");

            //ZIP the Archive Back up
            try
            {
                Directory.Delete(AMD3DDir, true);
            }
            catch (Exception) { }
            File.Delete(AMD3D);
        }

        public static void display(JObject root)
        {
            try
            {
                //Handle Display Settings for AMD
                JArray display = (JArray)GetJValue(root, "Display");
                if (display == null)
                {
                    Console.Error.WriteLine("Missing: Display Settings");
                    return;
                }
                foreach (var token in display)
                {
                    if (token is JObject)
                    {
                        JToken tokenset = GetJValue((JObject)token, "Settings");
                        if (tokenset is JArray)
                        {
                            JArray settings = (JArray)tokenset;
                            foreach (var settoken in settings)
                            {
                                if (settoken is JObject)
                                {
                                    JObject setjson = (JObject)settoken;
                                    string name = GetName(setjson);
                                    if (name == null)
                                    {
                                        Console.Error.WriteLine("Maulformed: Display Name");
                                        continue;
                                    }
                                    string strvalname = ((string)((JValue)name).Value).ToUpper();
                                    if (strvalname.Equals("GPU SCALING"))
                                    {
                                        SetValue((JValue)GetJValue(setjson, "Value"), 1);
                                    }
                                    else if (strvalname.Equals("SCALING MODE"))
                                    {
                                        SetValue((JValue)GetJValue(setjson, "Value"), 0);
                                    }
                                    else if (strvalname.Contains("VARI") && strvalname.Contains("BRIGHT"))
                                    {
                                        SetValue((JValue)GetJValue(setjson, "Value"), 0);
                                        SetValue((JValue)GetJValue(setjson, "State"), 0);
                                    }
                                    else if (strvalname.Contains("FREESYNC"))
                                    {
                                        SetValue((JValue)GetJValue(setjson, "Value"), 1);
                                    }
                                }
                            }
                        }
                        else
                        {
                            Console.Error.WriteLine("Maulformed: Display Settings");
                        }
                    }
                }
            }
            catch (Exception e)
            {
                Console.Error.WriteLine("Error: Display");
                Console.Error.WriteLine(e);
            }
        }

        public static void global(JObject root)
        {
            try
            {
                //Handle 3d Settings
                JArray global = (JArray)GetJValue(root, "Global Settings");
                if (global == null)
                {
                    Console.Error.WriteLine("Missing: Global 3D Settings");
                    return;
                }
                foreach (var obj in global)
                {
                    if (obj is JObject)
                    {
                        JObject json = (JObject)obj;
                        object objset = GetJValue(json, "Settings");
                        if (objset is JArray)
                        {
                            JArray set = (JArray)objset;
                            foreach (var setting in set)
                            {
                                if (setting is JObject)
                                {
                                    JObject index = (JObject)setting;
                                    string name = GetName(index);
                                    if (name == null)
                                    {
                                        Console.Error.WriteLine("Maulformed: Global 3D Settings Name");
                                        continue;
                                    }
                                    if (name.Contains("anti") && name.Contains("lag"))
                                    {
                                        SetValue((JValue)GetJValue(index, "State"), 0);
                                        SetValue((JValue)GetJValue(index, "Value"), 0);
                                    }
                                    else if (name.Contains("boost"))
                                    {
                                        SetValue((JValue)GetJValue(index, "State"), 0);
                                    }
                                    else if (name.Contains("chill"))
                                    {
                                        object subobj = GetJValue(index, "SubSettings");
                                        if (subobj is JArray)
                                        {
                                            JArray subsettings = (JArray)subobj;
                                            foreach (var subset in subsettings)
                                            {
                                                if (subset is JObject)
                                                {
                                                    JObject subindex = (JObject)subset;
                                                    string subname = ((string)((JValue)GetJValue(subindex, "Name")).Value).ToLower();
                                                    if (subname.Equals("toggle"))
                                                    {
                                                        SetValue((JValue)GetJValue(subindex, "Value"), 0);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    else if (name.Contains("image sharpening"))
                                    {
                                        SetValue((JValue)GetJValue(index, "State"), 1);
                                        SetValue((JValue)GetJValue(index, "Value"), 100);
                                    }
                                    else if (name.Equals("frame rate target control"))
                                    {
                                        SetValue((JValue)GetJValue(index, "State"), 0);
                                    }
                                    else if (name.Equals("turbosync"))
                                    {
                                        SetValue((JValue)GetJValue(index, "Value"), 1);
                                    }
                                    else if (name.Equals("vsynccontrol"))
                                    {
                                        SetValue((JValue)GetJValue(index, "Value"), 1);
                                    }
                                    //# Anti Aliasing Use Application Settings #
                                    else if (name.Equals("eqaa"))
                                    {
                                        SetValue((JValue)GetJValue(index, "Value"), 0);
                                    }
                                    //# Anti Aliasing Disable Override
                                    else if (name.Equals("antialias"))
                                    {
                                        SetValue((JValue)GetJValue(index, "Value"), 0);
                                    }
                                    //# Texture Filtering Quality High #
                                    else if (name.Equals("tfq"))
                                    {
                                        SetValue((JValue)GetJValue(index, "Value"), 0);
                                    }
                                    else if (name.Equals("surfaceformatreplacements"))
                                    {
                                        SetValue((JValue)GetJValue(index, "Value"), 1);
                                    }
                                    else if (name.Equals("tessellation_option"))
                                    {
                                        SetValue((JValue)GetJValue(index, "Value"), 0);
                                    }
                                }
                            }
                        }
                        else
                        {
                            Console.Error.WriteLine("MaulFormed: Global 3D Settings");
                            return;
                        }
                    }
                }
            }
            catch (Exception e)
            {
                Console.Error.WriteLine("Error: Global 3D");
                Console.Error.WriteLine(e);
            }
        }

        public static void video(JObject root)
        {
            try
            {
                //Handle Video Profile
                JArray videoarr = (JArray)GetJValue(root, "Video Settings");
                if (videoarr == null)
                {
                    Console.Error.WriteLine("Missing: Video Settings");
                    return;
                }
                foreach (var vi in videoarr)
                {
                    if (vi is JObject)
                    {
                        JObject ji = (JObject)vi;
                        JToken vset = GetJValue(ji, "Settings");
                        if (vset is JArray)
                        {
                            JArray videoset = (JArray)vset;
                            int videoid = 6;
                            foreach (var v in videoset)
                            {
                                if (v is JObject)
                                {
                                    JObject j = (JObject)v;
                                    string name = GetName(j);
                                    if (name == null)
                                    {
                                        Console.Error.WriteLine("Maulformed: Video Settings Name");
                                        continue;
                                    }
                                    if (name.Equals("AMD Fluid Motion Video".ToLower()))
                                    {
                                        videoid = int.Parse("" + ((JValue)GetJValue(j, "ID")).Value);
                                        break;
                                    }
                                }
                            }
                            foreach (var v in videoset)
                            {
                                if (v is JObject)
                                {
                                    JObject j = (JObject)v;
                                    string name = GetName(j);
                                    if (name == null)
                                    {
                                        continue;
                                    }
                                    if (name.ToLower().Equals("video profile"))
                                    {
                                        SetValue((JValue)GetJValue(j, "Value"), videoid);
                                    }
                                }
                            }
                        }
                        else
                        {
                            Console.Error.WriteLine("MaulFormed: Video Settings");
                        }
                    }
                }
            }
            catch (Exception e)
            {
                Console.Error.WriteLine("Error: Video");
                Console.Error.WriteLine(e);
            }
        }

        public static void powersaver(JObject root)
        {
            try
            {
                //Handle Power Saver Setting
                JArray wizz = (JArray)GetJValue(root, "Wizard Profile Settings");
                if (wizz == null)
                {
                    Console.Error.WriteLine("Missing: Wizard Profile Settings(Power Saver Settings)");
                    return;
                }
                foreach (var w in wizz)
                {
                    if (w is JObject)
                    {
                        JToken wizztoken = GetJValue((JObject)w, "Settings");
                        if (wizztoken is JArray)
                        {
                            JArray wizzset = (JArray)wizztoken;
                            foreach (var v in wizzset)
                            {
                                if (v is JObject)
                                {
                                    JObject j = (JObject)v;
                                    string name = GetName(j);
                                    if (name == null)
                                    {
                                        Console.Error.WriteLine("Maulformed: Wizard Profile Settings(Power Saver Settings) Name");
                                        continue;
                                    }
                                    if (name.Contains("power saver"))
                                    {
                                        SetValue((JValue)GetJValue(j, "Value"), 0);
                                    }
                                }
                            }
                        }
                        else
                        {
                            Console.Error.WriteLine("Maulformed: Wizard Profile Settings(Power Saver Settings)");
                        }
                    }
                }
            }
            catch (Exception e)
            {
                Console.Error.WriteLine("Error: Wizard Profile Settings(Power Saver Settings)");
                Console.Error.WriteLine(e);
            }
        }

        public static void sampling(JObject root)
        {
            try
            {
                //Handle Sampling Intervals to Prevent Performance Issues
                JArray perfroot = (JArray)GetJValue(root, "Performance Settings");
                if (perfroot == null)
                {
                    Console.Error.WriteLine("Missing: Sampling Settings");
                    return;
                }
                foreach (var v in perfroot)
                {
                    if (v is JObject)
                    {
                        JToken p = GetJValue((JObject)v, "Settings");
                        if (p is JArray)
                        {
                            JArray perfset = (JArray)p;
                            foreach (var s in perfset)
                            {
                                if (s is JObject)
                                {
                                    JObject j = (JObject)s;
                                    string name = GetName(j);
                                    if (name == null)
                                    {
                                        Console.Error.WriteLine("Maulformed: Sampling Settings Name");
                                        continue;
                                    }
                                    if (name.Equals("sampling interval"))
                                    {
                                        SetValue((JValue)GetJValue(j, "Value"), 500);
                                    }
                                }
                            }
                        }
                        else
                        {
                            Console.Error.WriteLine("Maulformed: Sampling Settings");
                        }
                    }
                }
            }
            catch (Exception e)
            {
                Console.Error.WriteLine("Error: Sampling");
                Console.Error.WriteLine(e);
            }
        }

        public static void Run(string command)
        {
            command = "\"" + command + "\"";

            System.Diagnostics.Process process = new System.Diagnostics.Process();
            System.Diagnostics.ProcessStartInfo startInfo = new System.Diagnostics.ProcessStartInfo();
            startInfo.CreateNoWindow = true;
            startInfo.UseShellExecute = false;
            startInfo.WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden;
            startInfo.FileName = @"cmd.exe";
            startInfo.Arguments = "/s /c " + command;
            startInfo.RedirectStandardOutput = true;
            startInfo.RedirectStandardError = true;
            startInfo.RedirectStandardInput = true;
            process.StartInfo = startInfo;
            process.Start();
            process.StandardOutput.ReadToEnd();
            process.WaitForExit();
        }

        //Gets JSON Value Name From Setting Index Safely. Returns NUll if Name key isn't a NON-NULL string
        static string GetName(JObject index)
        {
            try
            {
                string name = ((string)((JValue)GetJValue(index, "Name")).Value).ToLower();
                return name;
            }
            catch (Exception) { }
            return null;
        }

        //Gets JSON Value ignoring Casing
        static JToken GetJValue(JObject json, string name)
        {
            JToken value = null;
            json.TryGetValue(name, StringComparison.OrdinalIgnoreCase, out value);
            return value;
        }

        //Dynamically Set a JSON Value without knowing if the Number is a String or Integer
        public static void SetValue(JValue val, int num)
        {
            object o = val.Value;
            if (o is string)
                val.Value = "" + num;
            else
                val.Value = num;
        }

        /*
         * Reliably Get the AMD Software's CNCMD.exe Index
         */
        public static string GetCnCMD()
        {
            try
            {
                RegistryKey HKLM = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, RegistryView.Registry64);
                RegistryKey cnkey = HKLM.OpenSubKey(@"SOFTWARE\AMD\CN");
                string dir = (string)cnkey.GetValue("InstallDir");
                if (dir.EndsWith(@"\") || dir.EndsWith(@"/"))
                {
                    dir = dir.Substring(0, dir.Length - 1);
                }
                return dir + @"\cncmd.exe";
            }
            catch (Exception e)
            {
                Console.Error.WriteLine(e.Message + " " + e.GetType());
                string HOMEDRIVE = Environment.GetFolderPath(Environment.SpecialFolder.System).Substring(0, 1);
                string cncmd = HOMEDRIVE + @":\Program Files\AMD\CNext\CNext\cncmd.exe";
                if (File.Exists(cncmd))
                    return cncmd;
                cncmd = HOMEDRIVE + @":\Program Files (x86)\AMD\CNext\CNext\cncmd.exe";
                if (File.Exists(cncmd))
                    return cncmd;

                //Check older or newer likely paths
                cncmd = HOMEDRIVE + @":\Program Files\AMD\CNext\cncmd.exe";
                if (File.Exists(cncmd))
                    return cncmd;
                cncmd = HOMEDRIVE + @":\Program Files (x86)\AMD\CNext\cncmd.exe";
                if (File.Exists(cncmd))
                    return cncmd;
            }
            return null;
        }
    }
}

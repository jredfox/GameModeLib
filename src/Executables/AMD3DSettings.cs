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
    class Program
    {

        static void Main(string[] args)
        {
            string cncmd = GetCnCMD();
            if (cncmd is null)
            {
                Console.WriteLine("AMD's CNCMD.exe Is Not Found");
                return;
            }
            Console.WriteLine(cncmd);

            //Export Current Settings and Extract them
            string AMD3D = (System.IO.Path.GetTempPath() + @"\AMD3DSettings.zip").Replace(@"\\", @"\");
            string AMD3DDir = (System.IO.Path.GetTempPath() + @"\AMD3DSettings").Replace(@"\\", @"\");
            string AMD3DJSON = (System.IO.Path.GetTempPath() + @"\AMD3DSettings\rssettings.json").Replace(@"\\", @"\");
            Run("\"" + cncmd + "\" export \"" + AMD3D + "\"");
            
            //Wait Until File is Created for some odd reason it doesn't create it from the first command
            long ms = 0;
            while(!File.Exists(AMD3D))
            {
                Thread.Sleep(1);
                ms++;
                if (ms >= 5000)
                    break;
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
            catch(Exception e)
            {
                Console.Error.WriteLine(e.Message);
            }
            
            //Edit JSON Settings
            JObject root;
            using (StreamReader reader = File.OpenText(@"C:\Users\jredfox\Desktop\rssettings.json"))
            {
                root = (JObject)JToken.ReadFrom(new JsonTextReader(reader));
                
                //Handle Display Settings for AMD
                JArray display = (JArray) GetJValue(root, "Display");
                foreach(var token in display)
                {
                    JArray settings = (JArray) GetJValue((JObject)token, "Settings");
                    foreach (var settoken in settings)
                    {
                        if(settoken is JObject)
                        {
                            JObject setjson = (JObject)settoken;
                            JToken name = GetJValue(setjson, "Name");
                            if(name is JValue)
                            {
                                string strvalname = ((string)((JValue)name).Value).ToUpper();
                                if (strvalname.Equals("GPU SCALING"))
                                {
                                    SetValue((JValue) GetJValue(setjson, "Value"), 1);
                                }
                                else if (strvalname.Equals("SCALING MODE"))
                                {
                                    SetValue((JValue) GetJValue(setjson, "Value"), 0);
                                }
                                else if (strvalname.Contains("VARI") && strvalname.Contains("BRIGHT"))
                                {
                                    SetValue((JValue) GetJValue(setjson, "Value"), 0);
                                    SetValue((JValue) GetJValue(setjson, "State"), 0);
                                }
                                else if (strvalname.Contains("FREESYNC"))
                                {
                                    SetValue((JValue) GetJValue(setjson, "Value"), 1);
                                }
                            }
                        }
                    }
                }

                //Handle 3d Settings
                JArray global = (JArray) GetJValue(root, "Global Settings");
                foreach (var obj in global)
                {
                    if(obj is JObject)
                    {
                        JObject json = (JObject)obj;
                        object objset = GetJValue(json, "Settings");
                        if(objset is JArray)
                        {
                            JArray set = (JArray)objset;
                            foreach (var setting in set)
                            {
                                if(setting is JObject)
                                {
                                    JObject index = (JObject)setting;
                                    string name = ((string) ((JValue) GetJValue(index, "Name") ).Value).ToLower();
                                    if(name.Contains("anti") && name.Contains("lag"))
                                    {
                                        Console.WriteLine("Anit Lag");
                                        SetValue((JValue) GetJValue(index, "State"), 0);
                                        SetValue((JValue) GetJValue(index, "Value"), 0);
                                    }
                                    else if(name.Contains("boost"))
                                    {
                                        Console.WriteLine("Boost");
                                        SetValue((JValue) GetJValue(index, "State"), 0);
                                    }
                                    else if(name.Contains("chill"))
                                    {
                                        Console.WriteLine("Chill");
                                        object subobj = GetJValue(index, "SubSettings");
                                        if(subobj is JArray)
                                        {
                                            JArray subsettings = (JArray)subobj;
                                            foreach (var subset in subsettings)
                                            {
                                                if(subset is JObject)
                                                {
                                                    JObject subindex = (JObject)subset;
                                                    string subname = ((string) ((JValue) GetJValue(subindex, "Name")).Value).ToLower();
                                                    if(subname.Equals("toggle"))
                                                    {
                                                        Console.WriteLine("Chill Toggle Found");
                                                        SetValue((JValue) GetJValue(subindex, "Value"), 0);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    else if(name.Contains("image sharpening"))
                                    {
                                        Console.WriteLine("Image Sharpening");
                                        SetValue((JValue) GetJValue(index, "State"), 1);
                                        SetValue((JValue) GetJValue(index, "Value"), 100);
                                    }
                                    else if(name.Equals("frame rate target control"))
                                    {
                                        Console.WriteLine("Frame Rate Target Control");
                                        SetValue((JValue) GetJValue(index, "State"), 0);
                                    }
                                    else if(name.Equals("turbosync"))
                                    {
                                        Console.WriteLine("TurboSync");
                                        SetValue((JValue) GetJValue(index, "Value"), 1);
                                    }
                                    else if (name.Equals("vsynccontrol"))
                                    {
                                        Console.WriteLine("VSyncControl");
                                        SetValue((JValue) GetJValue(index, "Value"), 1);
                                    }
                                    //# Anti Aliasing Use Application Settings #
                                    else if (name.Equals("eqaa"))
                                    {
                                        Console.WriteLine("EQAA");
                                        SetValue((JValue) GetJValue(index, "Value"), 0);
                                    }
                                    //# Texture Filtering Quality High #
                                    else if (name.Equals("tfq"))
                                    {
                                        Console.WriteLine("TFQ");
                                        SetValue((JValue) GetJValue(index, "Value"), 0);
                                    }
                                    else if (name.Equals("surfaceformatreplacements"))
                                    {
                                        Console.WriteLine("SurfaceFormatReplacements");
                                        SetValue((JValue) GetJValue(index, "Value"), 1);
                                    }
                                    else if (name.Equals("tessellation_option"))
                                    {
                                        Console.WriteLine("Tessellation_OPTION");
                                        SetValue((JValue) GetJValue(index, "Value"), 0);
                                    }
                                }
                            }
                        }
                    }
                }

                //Handle Video Profile
                JArray videoarr = (JArray) GetJValue(root, "Video Settings");
                foreach (var vi in videoarr)
                {
                    if(vi is JObject)
                    {
                        JObject ji = (JObject)vi;
                        JArray videoset = (JArray) GetJValue(ji, "Settings");
                        int videoid = -1;
                        foreach (var v in videoset)
                        {
                            if (v is JObject)
                            {
                                JObject j = (JObject)v;
                                string name = ((string)((JValue) GetJValue(j, "Name")).Value).ToLower();
                                if (name.Equals("AMD Fluid Motion Video".ToLower()))
                                {
                                    videoid = int.Parse("" + ((JValue) GetJValue(j, "ID")).Value);
                                    break;
                                }
                            }
                        }
                        if (videoid != -1)
                        {
                            foreach (var v in videoset)
                            {
                                if (v is JObject)
                                {
                                    JObject j = (JObject)v;
                                    string name = (string)((JValue) GetJValue(j, "Name")).Value;
                                    if (name.ToLower().Equals("video profile"))
                                    {
                                        Console.WriteLine("Video Profile:" + videoid);
                                        SetValue((JValue) GetJValue(j, "Value"), videoid);
                                    }
                                }
                            }
                        }
                    }
                }

                //Handle Power Saver Setting
                JArray wizz = (JArray) GetJValue(root, "Wizard Profile Settings");
                foreach (var w in wizz)
                {
                    if(w is JObject)
                    {
                        JArray wizzset = (JArray) GetJValue((JObject)w, "Settings");
                        foreach (var v in wizzset)
                        {
                            if (v is JObject)
                            {
                                JObject j = (JObject)v;
                                string name = ((string)((JValue) GetJValue(j, "Name")).Value).ToLower();
                                if (name.Contains("power saver"))
                                {
                                    Console.WriteLine("Disabling AMD GPU Power Saver");
                                    SetValue((JValue) GetJValue(j, "Value"), 0);
                                }
                            }
                        }
                    }
                }

                //Handle Sampling Intervals to Prevent Performance Issues
                JArray perfroot = (JArray) GetJValue(root, "Performance Settings");
                foreach (var v in perfroot)
                {
                    if(v is JObject)
                    {
                        JArray perfset = (JArray) GetJValue((JObject)v, "Settings");
                        foreach (var s in perfset)
                        {
                            if (s is JObject)
                            {
                                JObject j = (JObject)s;
                                string name = ((string)((JValue) GetJValue(j, "Name")).Value).ToLower();
                                if (name.Equals("sampling interval"))
                                {
                                    Console.WriteLine("Sampling Interval Set to 500");
                                    SetValue((JValue) GetJValue(j, "Value"), 500);
                                }
                            }
                        }
                    }
                }
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

            process.StartInfo = startInfo;
            process.Start();
            process.WaitForExit();
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
                string dir = (string) cnkey.GetValue("InstallDir");
                if (dir.EndsWith(@"\") || dir.EndsWith(@"/"))
                {
                    dir = dir.Substring(0, dir.Length - 1);
                }
                return dir + @"\cncmd.exe";
            }
            catch(Exception e)
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

using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Security;
using System.Security.Principal;
using System.Text;
using System.Threading.Tasks;

namespace RegImport
{
    public class Pair<T1, T2>
    {
        public T1 First { get; set; }
        public T2 Second { get; set; }

        public Pair(T1 first, T2 second)
        {
            First = first;
            Second = second;
        }
    }

    class Program
    {
        public static bool IMPORT_GLOBAL, IMPORT_USER, UNINSTALL_GLOBAL, UNINSTALL_USER;
        public static string SID;
        static void Main(string[] args)
        {
            //HELP
            if(args.Length < 3 || args[0].Equals("/?") || args[0].ToLower().Equals("/help"))
            {
                Help();
            }
            //Fix Arguments Path Seperators
            for (int i = 0; i < args.Length; i++)
                args[i] = args[i].Replace(@"/", @"\");

            //Parse Flags
            string set = args[0].ToUpper();
            for (int i = set.Length; i < 4; i++)
            {
                set += "F";
            }
            IMPORT_GLOBAL = set[0] == 'T';
            IMPORT_USER = set[1] == 'T';
            UNINSTALL_GLOBAL = set[2] == 'T';
            UNINSTALL_USER = set[3] == 'T';
            
            //Parse SID
            SID = args[1].Trim();
            if(SID.Equals("") || SID.ToUpper().Equals("NULL"))
            {
                SID = WindowsIdentity.GetCurrent().User.Value;
            }

            //Parse BaseDir and RegFiles to go through
            string[] dirs = args[2].Split(';');
            string BaseDir = Path.GetFullPath(dirs[0]);
            string UninstallDir =   Path.GetFullPath(dirs[1]);
            string GlobalDir =      UninstallDir + @"\Global";
            string UserDir =        UninstallDir + @"\" + SID;
            mkdir(GlobalDir, UserDir);
            List<string> regs = new List<string>();
            Dictionary<string, Pair<string, string> > ugens = new Dictionary<string, Pair<string, string>>();
            for (int i = 2; i < dirs.Length; i++)
            {
                string reg = BaseDir + @"\" + dirs[i];
                regs.Add(reg);
                ugens.Add(reg, new Pair<string, string>(GlobalDir + @"\" + dirs[i], UserDir + @"\" + dirs[i]));
            }

            //Start the Main Program
            foreach (string reg in regs)
            {
                StreamWriter writer_global = null;
                StreamWriter writer_user = null;
                try
                {
                    //Prepare the Global and User Reg Files
                    Pair<string, string> p = ugens[reg];
                    List<string> filelines = GetAllFileLines(reg);

                    //Gen Uninstall Data
                    if (UNINSTALL_GLOBAL || UNINSTALL_USER)
                    {
                        RegistryKey key_tree = null;
                        RegistryKey key_sub = null;
                        string str_tree = null;
                        string str_sub = null;
                        StreamWriter writer_current = null;
                        //TODO: Ensure it doesn't Gen Uninstall Info if the file already exists
                        //TODO: DE-ESC Value Names and properly parse them
                        if (UNINSTALL_GLOBAL)
                        {
                            writer_global = new StreamWriter(p.First);
                            writer_global.WriteLine("Windows Registry Editor Version 5.00");
                        }
                        if (UNINSTALL_USER)
                        {
                            writer_user = new StreamWriter(p.Second);
                            writer_user.WriteLine("Windows Registry Editor Version 5.00");
                        }
                        foreach (string line in filelines)
                        {
                            string tl = line.Trim();
                            if (tl.StartsWith("["))
                            {
                                Close(key_sub);
                                str_tree = tl.StartsWith("[-") ? tl.Substring(2, tl.IndexOf(@"\") - 2).ToUpper() : tl.Substring(1, tl.IndexOf(@"\") - 1).ToUpper();
                                str_sub = tl.Substring(tl.IndexOf(@"\") + 1, tl.Length - tl.IndexOf(@"\") - 2);
                                bool IsUSR = str_tree.Equals("HKEY_CURRENT_USER");
                                if (IsUSR)
                                {
                                    str_tree = str_tree.Replace(@"HKEY_CURRENT_USER", @"HKEY_USERS");
                                    str_sub = SID + @"\" + str_sub;
                                }
                                try
                                {
                                    key_tree = GetRegTree(str_tree);
                                    key_sub = key_tree.OpenSubKey(str_sub, false);
                                    if(UNINSTALL_USER && IsUSR)
                                    {
                                        writer_current = writer_user;
                                        //Handle Missing Keys
                                        if (key_sub == null)
                                            writer_user.WriteLine("\r\n[-" + str_tree + @"\" + str_sub + "]");
                                        else
                                        {
                                            writer_user.WriteLine("\r\n[" +  str_tree + @"\" + str_sub + "]");
                                        }
                                    }
                                    else if (UNINSTALL_GLOBAL && !IsUSR)
                                    {
                                        writer_current = writer_global;
                                        //Handle Missing Keys
                                        if (key_sub == null)
                                            writer_global.WriteLine("\r\n[-" + str_tree + @"\" + str_sub + "]");
                                        else
                                        {
                                            writer_global.WriteLine("\r\n[" + str_tree + @"\" + str_sub + "]");
                                        }
                                    }
                                    else
                                    {
                                        writer_current = null;
                                    }
                                }
                                catch (SecurityException)
                                {
                                    key_sub = null;
                                    Console.Error.WriteLine("Access Denied Reg:" + str_tree + @"\" + str_sub);
                                }
                                catch (Exception e)
                                {
                                    key_sub = null;
                                    Console.Error.WriteLine("Error Reg:" + str_tree + @"\" + str_sub);
                                    Console.Error.WriteLine(e);
                                }
                            }
                            if (tl.StartsWith(";") || tl.Equals("") || key_sub == null || writer_current == null)
                                continue;
                            //Console.WriteLine(line);
                            int index = tl.IndexOf('=');
                            if (index < 0)
                                continue;
                            string strval = tl.Substring(0, index);
                            if (strval.StartsWith("\""))
                                strval = strval.Substring(1, strval.Length - 2);
                            try
                            {
                                var val = key_sub.GetValue(strval);
                                if(val == null)
                                {
                                    writer_current.WriteLine("\"" + strval + "\"=-");
                                }
                                else
                                {
                                    RegistryValueKind type = key_sub.GetValueKind(strval);
                                    if(type == RegistryValueKind.DWord)
                                    {
                                        uint v = (uint) (int) val;
                                        writer_current.WriteLine("\"" + strval + "\"=dword:" + v.ToString("x8"));
                                    }
                                    else if(type == RegistryValueKind.String)
                                    {
                                        writer_current.WriteLine("\"" + strval + "\"=\"" + ESC((string) val) + "\"");
                                    }
                                    else if(type == RegistryValueKind.QWord)
                                    {
                                        ulong qval = (ulong)(long)val;
                                        byte[] bytes = BitConverter.GetBytes(qval);
                                        string hexString = BitConverter.ToString(bytes).Replace("-", ",").ToLower();
                                        writer_current.WriteLine("\"" + strval + "\"=hex(b):" + hexString);
                                    }
                                    else if(type == RegistryValueKind.ExpandString)
                                    {
                                        byte[] bytes = Encoding.Unicode.GetBytes(val.ToString());
                                        string hexString = BitConverter.ToString(bytes).Replace("-", ",").ToLower();
                                        hexString += ",00,00";//Null Terminator UTF-16 two bytes to represent '\0'
                                        write_binary(writer_current, ("\"" + strval + "\"=hex(2):"), hexString);
                                    }
                                    else if(type == RegistryValueKind.None)
                                    {
                                        if (val is System.Byte[])
                                        {
                                            byte[] bytes = (byte[]) val;
                                            string hexString = BitConverter.ToString(bytes).Replace("-", ",").ToLower();
                                            write_binary(writer_current, ("\"" + strval + "\"=hex(0):"), hexString);
                                        }
                                        else
                                        {
                                            Console.Error.WriteLine("Unsupported Reg Type NONE CLASS:" + val.GetType());
                                        }
                                    }
                                    else if(type == RegistryValueKind.Binary)
                                    {
                                        byte[] bytes = (byte[])val;
                                        string hexString = BitConverter.ToString(bytes).Replace("-", ",").ToLower();
                                        write_binary(writer_current, ("\"" + strval + "\"=hex:"), hexString);
                                    }
                                    else if(type == RegistryValueKind.MultiString)
                                    {
                                        string[] arr = (string[]) val;
                                        write_binary(writer_current, ("\"" + strval + "\"=hex(7):"), MultiSzToHexString(arr));
                                    }
                                    else
                                    {
                                        Console.Error.WriteLine("Unsupported Type:" + type);
                                    }
                                }
                               
                            }
                            catch(Exception e)
                            {
                                Console.Error.WriteLine("Error Reading Reg Value:" + str_tree + @"\" + str_sub + @"\" + strval);
                                Console.Error.WriteLine(e);
                            }
                        }
                    }
                }
                catch(Exception e)
                {
                    Console.Error.WriteLine("Error While Processing Reg File:" + reg);
                    Console.Error.WriteLine(e);
                }
                finally
                {
                    Close(writer_global);
                    Close(writer_user);
                    writer_global = null;
                    writer_user = null;
                }
            }
        }

        private static void write_binary(StreamWriter w, string v, string hexString)
        {
            string[] hex = hexString.Split(',');
            w.Write(v);
            int count = v.Length;
            int index = 0;
            bool flag = true;
            foreach(string s in hex)
            {
                bool more = (index + 1) >= hex.Length;
                w.Write(s + (more ? "" : ","));
                count += 3;
                if (count >= (flag ? 77 : 74))
                {
                    w.WriteLine(@"\");
                    w.Write("  ");
                    count = 0;
                    flag = false;
                }
                index++;
            }
            w.WriteLine();
        }

        static string MultiSzToHexString(string[] values)
        {
            using (MemoryStream ms = new MemoryStream())
            {
                using (BinaryWriter bw = new BinaryWriter(ms))
                {
                    foreach (string value in values)
                    {
                        bw.Write(System.Text.Encoding.Unicode.GetBytes(value));
                        bw.Write((byte)0);
                        bw.Write((byte)0);
                    }
                    // NULL Terminator
                    bw.Write((byte)0);
                    bw.Write((byte)0);
                }
                return BitConverter.ToString(ms.ToArray()).Replace("-", ",").ToLower();
            }
        }
        public static string ESC(string val)
        {
            return val.Replace(@"\", @"\\").Replace(@"""", @"\""");
        }

        public static List<string> GetAllFileLines(string reg)
        {
            List<string> l = new List<string>();
            using (StreamReader sr = new StreamReader(reg))
            {
                string line;
                while ((line = sr.ReadLine()) != null)
                {
                    l.Add(line);
                }
            }
            return l;
        }

        public static void Close(RegistryKey key)
        {
            try
            {
                if(key != null)
                    key.Close();
            }
            catch (Exception e)
            {
                Console.Error.WriteLine(e.Message);
            }
        }

        public static void Close(StreamWriter writer)
        {
            try
            {
                if(writer != null)
                    writer.Close();
            }
            catch (Exception e)
            {
                Console.Error.WriteLine(e);
            }
        }

        private static void mkdir(params string[] dirs)
        {
            foreach (string dir in dirs)
            {
                if(!Directory.Exists(dir))
                    Directory.CreateDirectory(dir);
            }
        }

        static void Help()
        {
            Console.WriteLine(@"RegImport.exe <Options> <SID or NULL> <BaseDir>");
            Console.WriteLine("Options T/F {Import Global, Import User, Gen Uninstall Global, Gen Uninstall User}");
            Console.WriteLine(@"Example Current User: RegImport.exe ""TTTT"" """" ""C:\GameModeLib\Dir;FileName.reg;SubDir/FileName2.reg""");
            Console.WriteLine(@"Example Other User: RegImport.exe ""TTFF"" ""S-1-5-21-368394509-689051271-14200874-1011"" ""C:\GameModeLib\Dir;FileName.reg;SubDir/FileName2.reg""");
            Console.WriteLine(@"NOTE: NTUSER.DAT has to be loaded to HKU\<SID> Before Calling RegImport.exe");
            Environment.Exit(0);
        }

        static RegistryKey GetRegTree(string k)
        {
            string u = k.ToUpper();
            if (u.StartsWith("HKLM") || u.StartsWith("HKEY_LOCAL_MACHINE"))
            {
                return RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, Environment.Is64BitOperatingSystem ? RegistryView.Registry64 : RegistryView.Default);
            }
            else if (u.StartsWith("HKCU") || u.StartsWith("HKEY_CURRENT_USER"))
            {
                return RegistryKey.OpenBaseKey(RegistryHive.CurrentUser, Environment.Is64BitOperatingSystem ? RegistryView.Registry64 : RegistryView.Default);
            }
            else if (u.StartsWith("HKCR") || u.StartsWith("HKEY_CLASSES_ROOT"))
            {
                return RegistryKey.OpenBaseKey(RegistryHive.ClassesRoot, Environment.Is64BitOperatingSystem ? RegistryView.Registry64 : RegistryView.Default);
            }
            else if (u.StartsWith("HKU") || u.StartsWith("HKEY_USERS"))
            {
                return RegistryKey.OpenBaseKey(RegistryHive.Users, Environment.Is64BitOperatingSystem ? RegistryView.Registry64 : RegistryView.Default);
            }
            else if (u.StartsWith("HKCC") || u.StartsWith("HKEY_CURRENT_CONFIG"))
            {
                return RegistryKey.OpenBaseKey(RegistryHive.CurrentConfig, Environment.Is64BitOperatingSystem ? RegistryView.Registry64 : RegistryView.Default); ;
            }
            return null;
        }
    }
}

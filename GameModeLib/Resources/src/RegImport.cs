using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.DirectoryServices.AccountManagement;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
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

    class Hive
    {
        [DllImport("advapi32.dll", SetLastError = true)]
        static extern int RegLoadKey(IntPtr hKey, string lpSubKey, string lpFile);

        [DllImport("advapi32.dll", SetLastError = true)]
        static extern int RegUnLoadKey(IntPtr hKey, string lpSubKey);

        [DllImport("ntdll.dll", SetLastError = true)]
        static extern IntPtr RtlAdjustPrivilege(int Privilege, bool bEnablePrivilege, bool IsThreadPrivilege, out bool PreviousValue);

        [DllImport("advapi32.dll")]
        static extern bool LookupPrivilegeValue(string lpSystemName, string lpName, ref UInt64 lpLuid);

        [DllImport("advapi32.dll")]
        static extern bool LookupPrivilegeValue(IntPtr lpSystemName, string lpName, ref UInt64 lpLuid);

        private string file_hive;
        private string subkey;
        private RegistryHive root;

        public Hive(string file_hive, string subkey, RegistryHive root)
        {
            this.file_hive = file_hive;
            this.subkey = subkey;
            this.root = root;
        }

        public void Load()
        {
            if (!File.Exists(this.file_hive))
            {
                throw new FileNotFoundException("Missing Hive:" + this.file_hive);
            }
            RegistryKey key_tree = RegistryKey.OpenBaseKey(root, Environment.Is64BitOperatingSystem ? RegistryView.Registry64 : RegistryView.Default);
            IntPtr TreeHandle = key_tree.Handle.DangerousGetHandle();
            RegLoadKey(TreeHandle, this.subkey, this.file_hive);
            key_tree.Close();
        }
        public void UnLoad()
        {
            RegistryKey key_tree = RegistryKey.OpenBaseKey(root, Environment.Is64BitOperatingSystem ? RegistryView.Registry64 : RegistryView.Default);
            IntPtr TreeHandle = key_tree.Handle.DangerousGetHandle();
            RegUnLoadKey(TreeHandle, this.subkey);
            key_tree.Close();
        }
        public static void GetHivePrivileges()
        {
            ulong luid = 0;
            bool throwaway;
            LookupPrivilegeValue(IntPtr.Zero, "SeRestorePrivilege", ref luid);
            RtlAdjustPrivilege((int)luid, true, false, out throwaway);
            LookupPrivilegeValue(IntPtr.Zero, "SeBackupPrivilege", ref luid);
            RtlAdjustPrivilege((int)luid, true, false, out throwaway);
        }
    }

    public class RegFile
    {
        public string File { get; set; }
        public string RelPath { get; set; }
        public List<RegObj> Global { get; set; }
        public List<RegObj> User { get; set; }

        public RegFile(string file, string relpath)
        {
            this.File = file;
            this.RelPath = relpath;
        }

        public void Parse()
        {
            this.Global = new List<RegObj>();
            this.User = new List<RegObj>();

            int index_line = -1;
            RegKey LastKey = null;
            List<string> filelines = Program.GetAllFileLines(this.File);
            foreach (string line in filelines)
            {
                index_line++;
                try
                {
                    string tl = line.Trim();
                    if (tl.Equals("") || tl.StartsWith(";"))
                        continue;

                    //Parse Keys
                    if (tl.StartsWith("["))
                    {
                        bool delkey = tl.StartsWith("[-");
                        string str_key = Program.SubStringIndex(tl, (delkey ? 2 : 1), tl.Length - 2);
                        LastKey = new RegKey(str_key, delkey);
                        bool IsUSR = LastKey.IsUser;
                        if (IsUSR)
                        {
                            User.Add(LastKey);
                        }
                        else
                        {
                            Global.Add(LastKey);
                        }
                    }
                    //Parse Values
                    else if (LastKey != null && !LastKey.Delete)
                    {
                        if (tl.IndexOf("\"") < 0)
                        {
                            continue;
                        }
                        string start = tl.Substring(tl.IndexOf("\""));
                        bool s = false;
                        char prev = ' ';
                        int index_end = -1;
                        for (int i = 0; i < start.Length; i++)
                        {
                            var c = start[i];
                            //Escape double backslash
                            if (prev.Equals('\\') && c.Equals('\\'))
                                prev = ' ';
                            if (c.Equals('"') && !prev.Equals('\\'))
                            {
                                if (s)
                                {
                                    index_end = i - 1;
                                    break;
                                }
                                s = true;
                            }
                            prev = c;
                        }
                        string str_name = Program.DeESC(Program.SubStringIndex(start, 1, index_end));
                        start = start.Substring(index_end + 2);
                        string str_dat = start.Substring(start.IndexOf("=") + 1).Trim();
                        bool delval = str_dat.Equals("-");
                        RegValue v = null;
                        //REG_DELETE
                        if (delval)
                        {
                            v = new RegValue(str_name, "-", true, RegistryValueKind.Unknown);
                        }
                        //REG_DWORD
                        else if (str_dat.StartsWith("dword:"))
                        {
                            uint d = Convert.ToUInt32(str_dat.Substring(6), 16);
                            v = new RegValue(str_name, (int)d, RegistryValueKind.DWord);
                        }
                        //REG_SZ
                        else if (str_dat.StartsWith("\""))
                        {
                            v = new RegValue(str_name, Program.DeESC(Program.SubStringIndex(str_dat, 1, str_dat.Length - 2)), RegistryValueKind.String);
                        }
                        //TODO: Make sure DWORD and QWORD Works with ARM64
                        //QWORD
                        else if (str_dat.StartsWith("hex(b):"))
                        {
                            //convert the multi line hex string to a single parsible bytes
                            string hexString = Program.GetBinaryHex(str_dat.Substring(7).Replace(",", ""), index_line, filelines);
                            // Convert hexadecimal string to byte array
                            byte[] byteArray = new byte[hexString.Length / 2];
                            for (int i = 0; i < byteArray.Length; i++)
                            {
                                byteArray[i] = Convert.ToByte(hexString.Substring(i * 2, 2), 16);
                            }
                            ulong qword = BitConverter.ToUInt64(byteArray, 0);
                            v = new RegValue(str_name, (long)qword, RegistryValueKind.QWord);
                        }
                        //BINARY & TYPE NONE
                        else if (str_dat.StartsWith("hex:") || str_dat.StartsWith("hex(0):"))
                        {
                            bool isnone = str_dat.StartsWith("hex(0):");
                            string str_data = Program.GetBinaryHex(str_dat.Substring((isnone ? 7 : 4)).Replace(",", ""), index_line, filelines);
                            byte[] byteArray = new byte[str_data.Length / 2];
                            for (int i = 0; i < byteArray.Length; i++)
                            {
                                byteArray[i] = Convert.ToByte(str_data.Substring(i * 2, 2), 16);
                            }
                            v = new RegValue(str_name, byteArray, (isnone ? RegistryValueKind.None : RegistryValueKind.Binary));
                        }
                        //REG_EXPAND_SZ Expandable String
                        else if (str_dat.StartsWith("hex(2):"))
                        {
                            string str_data = Program.GetBinaryHex(str_dat.Substring(7).Replace(",", ""), index_line, filelines);
                            byte[] byteArray = new byte[str_data.Length / 2];
                            for (int i = 0; i < byteArray.Length; i++)
                            {
                                byteArray[i] = Convert.ToByte(str_data.Substring(i * 2, 2), 16);
                            }
                            string expand_sz = System.Text.Encoding.Unicode.GetString(byteArray, 0, byteArray.Length - 2);//Remove Null Terminator as C# adds it
                            v = new RegValue(str_name, expand_sz, RegistryValueKind.ExpandString);
                        }
                        //REG_MULTI_SZ String Array
                        else if (str_dat.StartsWith("hex(7):"))
                        {
                            string str_data = Program.GetBinaryHex(str_dat.Substring(7).Replace(",", ""), index_line, filelines);
                            List<int> indexes = new List<int>();
                            for (int i = 0; (i + 3) < str_data.Length; i += 4)
                            {
                                string h = str_data.Substring(i, 4);
                                if (h.Equals("0000") && (i + 4) < str_data.Length)
                                {
                                    indexes.Add(i);//NULL terminator seperates the strings
                                }
                            }
                            string[] multi_sz = new string[indexes.Count];
                            int counter = 0;
                            int counter_multi = 0;
                            foreach (int k in indexes)
                            {
                                var vv = Program.SubStringIndex(str_data, counter, k - 1);
                                counter = k + 4;
                                byte[] byteArray = new byte[vv.Length / 2];
                                for (int j = 0; j < byteArray.Length; j++)
                                {
                                    byteArray[j] = Convert.ToByte(vv.Substring(j * 2, 2), 16);
                                }
                                string sz = System.Text.Encoding.Unicode.GetString(byteArray);
                                multi_sz[counter_multi++] = sz;
                            }
                            v = new RegValue(str_name, multi_sz, RegistryValueKind.MultiString);
                        }

                        if (LastKey.IsUser)
                        {
                            User.Add(v);
                        }
                        else
                        {
                            Global.Add(v);
                        }
                    }
                }
                catch (Exception e)
                {
                    Console.Error.WriteLine("Error:" + e);
                }
            }
        }
    }

    public class RegKey : RegObj
    {
        public RegistryKey Hive;
        public string SubKey { get; set; }
        public bool IsUser;
        public RegKey(string key, bool delete)
        {
            int i = key.IndexOf(@"\");
            this.Hive = Program.GetRegTree(Program.SubStringIndex(key, 0, i));
            this.SubKey = key.Substring(i + 1);
            this.Name = this.Hive.Name + @"\" + this.SubKey;
            this.Delete = delete;
            this.IsUser = this.Hive.Name.Equals("HKEY_CURRENT_USER");
        }
        public RegKey(string hive, string subkey, bool delete)
        {
            this.Hive = Program.GetRegTree(hive);
            this.SubKey = subkey;
            this.Name = hive + @"\" + subkey;
            this.Delete = delete;
            this.IsUser = this.Hive.Name.Equals("HKEY_CURRENT_USER");
        }

        public RegKey GetRegKey(string sid)
        {
            return new RegKey("HKEY_USERS", sid + @"\" + this.SubKey, this.Delete);
        }
    }

    public class RegValue : RegObj
    {
        public object Data { get; set; }
        public RegistryValueKind RegType { get; set; }

        public RegValue(string value, object data, bool delete, RegistryValueKind kind)
        {
            this.Name = value;
            this.Data = data;
            this.Delete = delete;
            this.RegType = this.Delete ? RegistryValueKind.Unknown : kind;
        }

        public RegValue(string value, object data, RegistryValueKind kind) : this(value, data, false, kind)
        {

        }
    }

    public class RegObj
    {
        public bool Delete { get; set; }
        public string Name { get; set; }
    }

    class Program
    {
        public static bool IMPORT_GLOBAL, IMPORT_USER, UNINSTALL_GLOBAL, UNINSTALL_USER, UNINSTALL_OVERWRITE;
        public static string BaseDir;
        public static string UninstallDir;
        public static string[] dirs;
        static void Main(string[] args)
        {
            long milliseconds = DateTimeOffset.Now.ToUnixTimeMilliseconds();
            //HELP
            if (args.Length < 3 || args[0].Equals("/?") || args[0].ToLower().Equals("/help"))
            {
                Help();
            }
            //Fix Arguments Path Seperators
            for (int i = 0; i < args.Length; i++)
                args[i] = args[i].Replace(@"/", @"\");

            //Parse Flags
            string set = args[0].ToUpper();
            for (int i = set.Length; i < 5; i++)
            {
                set += "F";
            }
            IMPORT_GLOBAL = set[0] == 'T';
            IMPORT_USER = set[1] == 'T';
            UNINSTALL_GLOBAL = set[2] == 'T';
            UNINSTALL_USER = set[3] == 'T';
            UNINSTALL_OVERWRITE = set[4] == 'T';

            //Parse the Reg File Into Objects
            dirs = args[2].Split(';');
            BaseDir = Path.GetFullPath(dirs[0]);
            UninstallDir = Path.GetFullPath(dirs[1]);
            List<string> reg_files = new List<string>();
            List<RegFile> regs = new List<RegFile>();
            for (int i = 2; i < dirs.Length; i++)
            {
                string d = dirs[i];
                string rel = null;
                int index_rel = d.IndexOf(@"|");
                if (index_rel >= 0)
                {
                    
                    rel = SubStringIndex(d, index_rel + 1, d.Length - 1);
                    d = SubStringIndex(d, 0, index_rel - 1);
                }
                bool full = d.Substring(1, 1).Equals(":");
                string r = full ? d : (BaseDir + @"\" + d);
                RegFile reg = new RegFile(r, rel != null ? rel : (full ? Path.GetFileName(d) : d));
                reg.Parse();
                regs.Add(reg);

                //Apply Global Settings
                RegGenUninstall(reg, null);
                RegImport(reg, null);
            }

            //Get ARG_SID
            string ARG_SID = args[1].Trim().ToUpper();

            //Install Current User
            if (ARG_SID.Equals("") || ARG_SID.Equals("NULL"))
            {
                ARG_SID = GetCurrentSID();
                foreach (RegFile r in regs)
                {
                    RegGenUninstall(r, ARG_SID);
                    RegImport(r, ARG_SID);
                }
            }
            //Install For A Different User(s) or * for All Users
            else
            {
                bool allsids = ARG_SID.Equals("*");
                List<string> sids = new List<string>(ARG_SID.Split(';'));
                string HomeDrive = Environment.ExpandEnvironmentVariables("%HOMEDRIVE%").Substring(0, 1).ToUpper();
                string Users = HomeDrive + @":\Users\";

                //Loop through All SIDS / Users Specified
                Hive.GetHivePrivileges();//Get Proper Privileges
                using (PrincipalContext context = new PrincipalContext(ContextType.Machine))
                {
                    using (PrincipalSearcher searcher = new PrincipalSearcher(new UserPrincipal(context)))
                    {
                        foreach (Principal result in searcher.FindAll())
                        {
                            if (result is UserPrincipal user)
                            {
                                string sid = user.Sid.ToString();
                                string file_hive = Users + user.SamAccountName + @"\NTUSER.DAT";
                                if (File.Exists(file_hive) && (allsids || sids.Contains(sid.ToUpper()) || sids.Contains(user.SamAccountName.ToUpper())))
                                {
                                    Hive h = new Hive(file_hive, sid, RegistryHive.Users);
                                    try
                                    {
                                        h.Load();
                                        Console.WriteLine($"Reg Import User: {user.SamAccountName}, SID: {sid}");
                                    }
                                    catch (Exception)
                                    {
                                        Console.Error.WriteLine("Failed to Load NTUSER.DAT:" + user.SamAccountName);
                                        h = null;
                                    }
                                    finally
                                    {
                                        try
                                        {
                                            foreach (RegFile r in regs)
                                            {
                                                RegGenUninstall(r, sid);
                                                RegImport(r, sid);
                                            }
                                        }
                                        catch (Exception f)
                                        {
                                            Console.Error.WriteLine(f);
                                        }
                                        if (h != null)
                                        {
                                            try
                                            {
                                                h.UnLoad();
                                            }
                                            catch (Exception)
                                            {
                                                Console.Error.WriteLine("Failed to Unload NTUSER.DAT:" + user.SamAccountName);
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            long done = DateTimeOffset.Now.ToUnixTimeMilliseconds();
            Console.WriteLine("Done MS:" + (done - milliseconds));
        }

        public static string GetCurrentSID()
        {
            return WindowsIdentity.GetCurrent().User.Value;
        }

        public static void RegGenUninstall(RegFile reg, string SID)
        {
            bool USR = (SID != null);
            if (!UNINSTALL_GLOBAL && !USR || !UNINSTALL_USER && USR)
                return;

            //Fetch the StreamWriter to write the REG file
            string UserReg = UninstallDir + @"\Users\" + SID + @"\" + reg.RelPath;
            StreamWriter writer_current = null;
            if (UNINSTALL_GLOBAL && !USR)
            {
                string GlobalReg = UninstallDir + @"\Global\" + reg.RelPath;
                if (!UNINSTALL_OVERWRITE && File.Exists(GlobalReg))
                    return;
                mkdir(Path.GetDirectoryName(GlobalReg));
                writer_current = new StreamWriter(GlobalReg);
            }
            else if (UNINSTALL_USER && USR && (UNINSTALL_OVERWRITE || !File.Exists(UserReg)))
            {
                mkdir(Path.GetDirectoryName(UserReg));
                writer_current = new StreamWriter(UserReg);
            }
            else
            {
                return;
            }
            writer_current.WriteLine("Windows Registry Editor Version 5.00");
            RegistryKey LastKey = null;

            //Gen Uninstall Data
            foreach (RegObj o in (USR ? reg.User : reg.Global))
            {
                if (o is RegKey k)
                {
                    try
                    {
                        Close(LastKey);
                        try
                        {
                            writer_current.Flush();
                        }
                        catch (Exception e)
                        {
                            Console.Error.WriteLine(e);
                        }
                        k = USR ? k.GetRegKey(SID) : k; //Redirect Current User Keys to SID User Keys
                        LastKey = k.Hive.OpenSubKey(k.SubKey, false);
                        if (k.Delete && LastKey != null)
                        {
                            ExportKey(LastKey, writer_current);
                            LastKey = null;
                        }
                        else if (LastKey == null)
                        {
                            writer_current.WriteLine("\r\n[-" + k.Name + "]");
                        }
                        else
                        {
                            writer_current.WriteLine("\r\n[" + k.Name + "]");
                        }
                    }
                    catch (SecurityException)
                    {
                        LastKey = null;
                        Console.Error.WriteLine("Access Denied Reg Gen Uninstall Key:" + k.Name);
                    }
                    catch (Exception e)
                    {
                        LastKey = null;
                        Console.Error.Write("Error Reg Gen Uninstall Key:" + k.Name + " ");
                        Console.Error.WriteLine(e);
                    }
                }
                else if (o is RegValue v)
                {
                    if (LastKey != null)
                    {
                        WriteValue(writer_current, LastKey, v.Name);
                    }
                }
            }
            Close(writer_current);
            Close(LastKey);
        }

        public static void RegImport(RegFile reg, string SID)
        {
            bool USR = (SID != null);
            if (!IMPORT_GLOBAL && !USR || !IMPORT_USER && USR)
                return;

            RegistryKey LastKey = null;
            foreach (RegObj o in (USR ? reg.User : reg.Global))
            {
                if (o is RegKey k)
                {
                    Close(LastKey);
                    k = USR ? k.GetRegKey(SID) : k; //Redirect Current User Keys to SID User Keys
                    RegistryKey root = k.Hive;
                    if (o.Delete)
                    {
                        LastKey = null;
                        try
                        {
                            root.DeleteSubKeyTree(k.SubKey, false);
                        }
                        catch (Exception e)
                        {
                            Console.Error.Write("Error While Deleting Reg Key:" + k.Name + " ");
                            Console.Error.WriteLine(e);
                        }
                    }
                    else
                    {
                        try
                        {
                            LastKey = k.Hive.OpenSubKey(k.SubKey, true);
                            if (LastKey == null)
                                LastKey = root.CreateSubKey(k.SubKey);
                        }
                        catch (SecurityException)
                        {
                            LastKey = null;
                            Console.Error.WriteLine("Access Denied Reg Import Key:" + k.Name);
                        }
                        catch (Exception e)
                        {
                            LastKey = null;
                            Console.Error.Write("Error Reg Import Key:" + k.Name + " ");
                            Console.Error.WriteLine(e);
                        }
                    }
                }
                else if (o is RegValue v)
                {
                    if (LastKey != null)
                    {
                        try
                        {
                            if (v.Delete)
                            {
                                LastKey.DeleteValue(v.Name, false);
                            }
                            else
                            {
                                LastKey.SetValue(v.Name, v.Data, v.RegType);
                            }
                        }
                        catch (UnauthorizedAccessException)
                        {
                            Console.Error.WriteLine("Access Denied Reg Import Value:" + LastKey.Name + @"\" + v.Name);
                        }
                        catch (SecurityException)
                        {
                            Console.Error.WriteLine("Access Denied Reg Import Value:" + LastKey.Name + @"\" + v.Name);
                        }
                        catch (Exception e)
                        {
                            Console.Error.Write("Error Reg Import Value:" + LastKey.Name + @"\" + v.Name + " ");
                            Console.Error.WriteLine(e);
                        }
                    }
                }
            }
            Close(LastKey);
        }

        public static string GetBinaryHex(string hexString, int index_line, List<string> filelines)
        {
            string v = "NULL";
            int index_get = index_line;
            while (hexString.EndsWith("\\"))
            {
                hexString = hexString.Replace("\\", "");
                do
                {
                    index_get++;
                    if (index_get < filelines.Count)
                    {
                        v = filelines[index_get].Replace(",", "").Replace(" ", "");
                        if (v.StartsWith("\"") || v.StartsWith("["))
                        {
                            v = "";
                            break;
                        }
                    }
                    else
                    {
                        v = "";
                        break;
                    }
                } while (v.StartsWith(";") || v.Equals(""));
                hexString += v;
            }
            return hexString;
        }

        public static void ExportKey(RegistryKey key_sub, StreamWriter writer_current)
        {
            try
            {
                writer_current.WriteLine("\r\n[" + key_sub.Name + "]");

                //Write the values
                foreach (string v in key_sub.GetValueNames())
                {
                    WriteValue(writer_current, key_sub, v);
                }

                foreach (string v in key_sub.GetSubKeyNames())
                {
                    RegistryKey sub = key_sub.OpenSubKey(v, false);
                    ExportKey(sub, writer_current);
                    Close(sub);
                }
            }
            catch (Exception e)
            {
                Console.Error.Write(key_sub.Name + " ");
                Console.Error.Write(e + "\r\n");
            }
        }
        /// <summary>
        /// Writes a Registry Value from memory to the disk
        /// </summary>
        /// <param name="w">The writer</param>
        /// <param name="v">A De-Escaped registry value</param>
        public static void WriteValue(StreamWriter writer_current, RegistryKey k, string vname)
        {
            try
            {
                var val = k.GetValue(vname);
                string valESC = ESC(vname);
                if (val == null)
                {
                    writer_current.WriteLine("\"" + valESC + "\"=-");
                    return;
                }
                RegistryValueKind type = k.GetValueKind(vname);
                if (type == RegistryValueKind.DWord)
                {
                    uint v = (uint)(int)val;
                    writer_current.WriteLine("\"" + valESC + "\"=dword:" + v.ToString("x8"));
                }
                else if (type == RegistryValueKind.String)
                {
                    writer_current.WriteLine("\"" + valESC + "\"=\"" + ESC((string)val) + "\"");
                }
                else if (type == RegistryValueKind.QWord)
                {
                    ulong qval = (ulong)(long)val;
                    byte[] bytes = BitConverter.GetBytes(qval);
                    string hexString = BitConverter.ToString(bytes).Replace("-", ",").ToLower();
                    write_binary(writer_current, ("\"" + valESC + "\"=hex(b):"), hexString);
                }
                else if (type == RegistryValueKind.ExpandString)
                {
                    byte[] bytes = Encoding.Unicode.GetBytes(val.ToString());
                    string hexString = BitConverter.ToString(bytes).Replace("-", ",").ToLower();
                    hexString += ",00,00";//Null Terminator UTF-16 two bytes to represent '\0'
                    write_binary(writer_current, ("\"" + valESC + "\"=hex(2):"), hexString);
                }
                else if (type == RegistryValueKind.None)
                {
                    if (val is System.Byte[])
                    {
                        byte[] bytes = (byte[])val;
                        string hexString = BitConverter.ToString(bytes).Replace("-", ",").ToLower();
                        write_binary(writer_current, ("\"" + valESC + "\"=hex(0):"), hexString);
                    }
                    else
                    {
                        Console.Error.WriteLine("Unsupported Reg Type NONE CLASS:" + val.GetType());
                    }
                }
                else if (type == RegistryValueKind.Binary)
                {
                    byte[] bytes = (byte[])val;
                    string hexString = BitConverter.ToString(bytes).Replace("-", ",").ToLower();
                    write_binary(writer_current, ("\"" + valESC + "\"=hex:"), hexString);
                }
                else if (type == RegistryValueKind.MultiString)
                {
                    string[] arr = (string[])val;
                    write_binary(writer_current, ("\"" + valESC + "\"=hex(7):"), MultiSzToHexString(arr));
                }
                else
                {
                    Console.Error.WriteLine("Unsupported Type:" + type);
                }
            }
            catch (Exception e)
            {
                Console.Error.WriteLine("Error Reading Reg Value:" + k.Name + @"\" + vname);
                Console.Error.WriteLine(e);
            }
        }

        public static string DeESC(string v)
        {
            return v.Replace(@"\\", @"\").Replace(@"\""", "\"");
        }

        public static string SubStringIndex(string input, int startIndex, int endIndex)
        {
            int length = endIndex - startIndex + 1;
            return input.Substring(startIndex, length);
        }

        private static void write_binary(StreamWriter w, string v, string hexString)
        {
            string[] hex = hexString.Split(',');
            w.Write(v);
            int count = v.Length;
            int index = 0;
            bool flag = true;
            foreach (string s in hex)
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
                    // NULL Terminator of the REG_MULTI_SZ Array
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
                if (key != null)
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
                if (writer != null)
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
                if (!Directory.Exists(dir))
                    Directory.CreateDirectory(dir);
            }
        }

        static void Help()
        {
            Console.WriteLine("_________________________________________________________________________________");
            Console.WriteLine(@"RegImport.exe <Options> <SID;SID2, *, or NULL """"> <BaseDir;UninstallDir;File.REG>");
            Console.WriteLine("Options T/F {Import Global, Import User, Gen Uninstall Global, Gen Uninstall User, OverWrite Previous Install Data}");
            Console.WriteLine("_________________________________________________________________________________");
            Console.WriteLine("Examples:");
            Console.WriteLine("_________________________________________________________________________________");
            Console.WriteLine(@"Current User: RegImport.exe ""TTTT"" """" ""C:\GameModeLib\Dir;FileName.reg;SubDir/FileName2.reg""");
            Console.WriteLine(@"Example Other User: RegImport.exe ""TTFF"" ""S-1-5-21-368394509-689051271-14200874-1011;S-1-5-21-368394509-689051271-14200874-1099"" ""C:\GameModeLib\Resources;C:\GameModeLib\Uninstall;FileName.reg;SubDir/FileName2.reg""");
            Console.WriteLine(@"Example OverWrite Previous Uninstall Gen For All Users: RegImport.exe ""TTTTT"" ""*"" ""C:\GameModeLib\Dir;FileName.reg;SubDir/FileName2.reg""");
            Console.WriteLine(@"NOTE: NTUSER.DAT has to be loaded to HKU\<SID> Before Calling RegImport.exe");
            Console.WriteLine("_________________________________________________________________________________");
            Environment.Exit(0);
        }

        public static RegistryKey HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER, HKEY_CLASSES_ROOT, HKEY_USERS, HKEY_CURRENT_CONFIG;

        public static RegistryKey GetRegTree(string k)
        {
            string u = k.ToUpper();
            if (u.StartsWith("HKLM") || u.StartsWith("HKEY_LOCAL_MACHINE"))
            {
                if (HKEY_LOCAL_MACHINE == null)
                    HKEY_LOCAL_MACHINE = RegistryKey.OpenBaseKey(RegistryHive.LocalMachine, Environment.Is64BitOperatingSystem ? RegistryView.Registry64 : RegistryView.Default);
                return HKEY_LOCAL_MACHINE;
            }
            else if (u.StartsWith("HKCU") || u.StartsWith("HKEY_CURRENT_USER"))
            {
                if (HKEY_CURRENT_USER == null)
                    HKEY_CURRENT_USER = RegistryKey.OpenBaseKey(RegistryHive.CurrentUser, Environment.Is64BitOperatingSystem ? RegistryView.Registry64 : RegistryView.Default);
                return HKEY_CURRENT_USER;
            }
            else if (u.StartsWith("HKCR") || u.StartsWith("HKEY_CLASSES_ROOT"))
            {
                if (HKEY_CLASSES_ROOT == null)
                    HKEY_CLASSES_ROOT = RegistryKey.OpenBaseKey(RegistryHive.ClassesRoot, Environment.Is64BitOperatingSystem ? RegistryView.Registry64 : RegistryView.Default);
                return HKEY_CLASSES_ROOT;
            }
            else if (u.StartsWith("HKU") || u.StartsWith("HKEY_USERS"))
            {
                if (HKEY_USERS == null)
                    HKEY_USERS = RegistryKey.OpenBaseKey(RegistryHive.Users, Environment.Is64BitOperatingSystem ? RegistryView.Registry64 : RegistryView.Default);
                return HKEY_USERS;
            }
            else if (u.StartsWith("HKCC") || u.StartsWith("HKEY_CURRENT_CONFIG"))
            {
                if (HKEY_CURRENT_CONFIG == null)
                    HKEY_CURRENT_CONFIG = RegistryKey.OpenBaseKey(RegistryHive.CurrentConfig, Environment.Is64BitOperatingSystem ? RegistryView.Registry64 : RegistryView.Default);
                return HKEY_CURRENT_CONFIG;
            }
            return null;
        }
    }
}

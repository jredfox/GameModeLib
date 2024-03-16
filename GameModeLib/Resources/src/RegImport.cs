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

    public class RegWriter : StreamWriter
    {
        public bool HasWritten = false;
        public string File = null;

        public RegWriter(string path) : base(path)
        {
            this.File = path;
        }

        public override void WriteLine(string v)
        {
            HasWritten = true;
            base.WriteLine(v);
        }

        public override void Write(string v)
        {
            HasWritten = true;
            base.Write(v);
        }

        /// <summary>
        /// Writes a Line Without Triggering HasWritten
        /// </summary>
        /// <param name="v"></param>
        public void WriteLineParent(string v)
        {
            base.WriteLine(v);
        }

        /// <summary>
        /// Writes without Triggering HasWritten
        /// </summary>
        /// <param name="v"></param>
        public void WriteParent(string v)
        {
            base.Write(v);
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

        public string file_hive;
        public string rootname;
        public string subkey;
        public RegistryHive root;

        public Hive(string file_hive, string subkey, RegistryHive root)
        {
            this.file_hive = file_hive;
            this.subkey = subkey;
            this.root = root;
            this.rootname = Program.GetRegTree(this.root);
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
        public bool IsMeta { get; set; }
        public List<RegObj> Global { get; set; }
        public List<RegObj> User { get; set; }

        public RegFile(string file, string relpath)
        {
            this.File = this.GetRegPath(file, null);
            this.RelPath = this.GetRegPath(relpath, null);
            if (this.RelPath.StartsWith(@"Global\"))
                this.RelPath = this.RelPath.Substring(7);
            else if (this.RelPath.StartsWith(@"Users\<SID>\"))
            {
                this.RelPath = this.RelPath.Substring(this.RelPath.IndexOf('\\', this.RelPath.IndexOf('\\') + 1) + 1);
            }
            this.IsMeta = this.File.Contains("<SID>");
        }

        public RegFile GetRegFile(string sid)
        {
            if (!this.IsMeta)
                return this;
            RegFile reg = new RegFile(this.GetRegPath(this.File, sid), this.GetRegPath(this.RelPath, sid));
            reg.Parse();
            return reg;
        }

        public string GetRegPath(string path, string SID)
        {
            if (!path.Contains("<"))
                return path;
            foreach (var v in Program.fields)
            {
                path = path.Replace(v.Key, v.Value);
            }
            if (SID != null)
                path = path.Replace("<SID>", SID);
            return path;
        }

        public void Parse()
        {
            //If the Reg File is Meta Data Object then it's not a real Reg File don't parse
            if (this.IsMeta)
                return;
            this.Global = new List<RegObj>();
            this.User = new List<RegObj>();
            //Don't Parse Non Existing Files
            if (!System.IO.File.Exists(this.File))
                return;

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
        public bool IsCurrentUser;
        public RegKey(string key, bool delete)
        {
            int i = key.IndexOf(@"\");
            //Safe Gaurd for Empty SubKeys
            if(i == -1)
            {
                key += @"\";
                i = key.IndexOf(@"\");
            }
            this.Hive = Program.GetRegTree(Program.SubStringIndex(key, 0, i));
            this.SubKey = key.Substring(i + 1);
            //Redirect the Default Hive
            if (Program.CDH && this.Hive.Name.Equals("HKEY_USERS") && (this.SubKey.StartsWith(@"DEFAULT\", StringComparison.OrdinalIgnoreCase) || this.SubKey.Equals(@"DEFAULT", StringComparison.OrdinalIgnoreCase)))
            {
                this.Hive = Program.GetRegTree(Program.DefHive.rootname);
                this.SubKey = Program.DefHive.subkey + (this.SubKey.Length > 8 ? (@"\" + this.SubKey.Substring(8)) : "");
            }
            this.Name = this.Hive.Name + (this.SubKey.Length != 0 ? (@"\" + this.SubKey) : "");
            this.Delete = delete;
            this.IsCurrentUser = this.Hive.Name.Equals("HKEY_CURRENT_USER");
            this.IsUser = this.IsCurrentUser || this.Hive.Name.Equals("HKEY_USERS");
        }
        public RegKey(string hive, string subkey, bool delete)
        {
            this.Hive = Program.GetRegTree(hive);
            //Redirect the Default Hive
            if (Program.CDH && this.Hive.Name.Equals("HKEY_USERS") && (subkey.StartsWith(@"DEFAULT\", StringComparison.OrdinalIgnoreCase) || subkey.Equals(@"DEFAULT", StringComparison.OrdinalIgnoreCase)))
            {
                this.Hive = Program.GetRegTree(Program.DefHive.rootname);
                subkey = Program.DefHive.subkey + (subkey.Length > 8 ? (@"\" + subkey.Substring(8)) : "");
            }
            this.SubKey = subkey;
            this.Name = this.Hive.Name + (this.SubKey.Length != 0 ? (@"\" + this.SubKey) : "");
            this.Delete = delete;
            this.IsCurrentUser = this.Hive.Name.Equals("HKEY_CURRENT_USER");
            this.IsUser = this.IsCurrentUser || this.Hive.Name.Equals("HKEY_USERS");
        }

        public RegKey GetRegKey(string sid)
        {
            return this.IsCurrentUser ? new RegKey("HKEY_USERS", sid + @"\" + this.SubKey, this.Delete) : this;
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
        public static bool IMPORT_GLOBAL, IMPORT_USER, UNINSTALL_GLOBAL, UNINSTALL_USER, UNINSTALL_OVERWRITE, UNINSTALL_DEL;
        public static string BaseDir;
        public static string UninstallDir;
        public static bool CDH = false;//CUSTOM DEFAULT HIVE
        public static Hive DefHive = null;
        public static string[] dirs;
        public static Dictionary<string, string> fields = new Dictionary<string, string>();

        static void Main(string[] args)
        {
            long milliseconds = DateTimeOffset.Now.ToUnixTimeMilliseconds();
            //Fix Arguments Path Seperators
            for (int i = 0; i < args.Length; i++)
                args[i] = args[i].Replace(@"/", @"\");

            //HELP
            if (args.Length < 3 || args[0].Equals("/?") || args[0].ToLower().Equals("/help"))
            {
                Help();
            }

            //Get Proper Privileges
            Hive.GetHivePrivileges();
            //Load the Default User Template Hive
            DefHive = LoadDefaultHive();

            //Parse Flags
            string set = args[0].ToUpper();
            for (int i = set.Length; i < 6; i++)
            {
                set += "F";
            }
            IMPORT_GLOBAL = set[0] == 'T';
            IMPORT_USER = set[1] == 'T';
            UNINSTALL_GLOBAL = set[2] == 'T';
            UNINSTALL_USER = set[3] == 'T';
            UNINSTALL_OVERWRITE = set[4] == 'T';
            UNINSTALL_DEL = set[5] == 'T';

            //Get Command Line Variables
            if (args.Length > 3)
            {
                Dictionary<string, string> tmp = new Dictionary<string, string>();
                foreach (string arr in args[3].Split(';'))
                {
                    string[] parts = arr.Split('=');
                    fields.Add("<" + parts[0].Trim('<', '>').Trim() + ">", parts[1]);
                }

                //Expand the global variables with 256 tries
                foreach (var k in fields)
                {
                    var key = k.Key;
                    var v = k.Value.Replace("<SID>", ">SID>");
                    for (int i = 0; i < 256; i++)
                    {
                        foreach (var kk in fields)
                            v = v.Replace(kk.Key, kk.Value);
                        if (!v.Contains("<"))
                            break;
                    }
                    tmp.Add(key, v.Replace(">SID>", "<SID>"));
                }
                fields.Clear();
                fields = tmp;
            }

            //Get ARG_SID
            string ARG_SID = args[1].Trim().ToUpper();
            string SID_CURRENT = GetCurrentSID();
            bool allsids = false;
            bool USER_DEFAULT = false;
            bool USER_DOTDEFAULT = false;
            bool CurrentUser = false;
            bool OtherUsers = false;
            bool HasRunDefault = false;
            List<string> sids = new List<string>(ARG_SID.Split(';'));
            //Transform Empty SID or KeyWords Into the Current SID
            for (int i = 0; i < sids.Count; i++)
            {
                var v = sids[i];
                if (v.Equals("") || v.Equals("NULL") || v.Equals("CURRENT_USER") || v.Equals(SID_CURRENT))
                {
                    sids[i] = SID_CURRENT;
                    CurrentUser = true;
                }
                else if (v.Equals("DEFAULT"))
                    USER_DEFAULT = true;
                else if (v.Equals(".DEFAULT"))
                    USER_DOTDEFAULT = true;
                else if (v.Equals("*"))
                    allsids = true;
                else
                    OtherUsers = true;
            }

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

                //Gen Uninstall Data
                if (UNINSTALL_GLOBAL && !reg.IsMeta)
                {
                    RegGenUninstall(reg, null);
                }
            }

            //Import all Global Data
            if (IMPORT_GLOBAL)
            {
                foreach (var reg in regs)
                {
                    if (!reg.IsMeta)
                        RegImport(reg, null);
                }
            }

            //Install Current User
            if (!OtherUsers)
            {
                ARG_SID = SID_CURRENT;
                //Gen Uninstall Data
                if (UNINSTALL_USER)
                {
                    foreach (RegFile re in regs)
                    {
                        RegFile r = re.GetRegFile(ARG_SID);
                        if(CurrentUser)
                            RegGenUninstall(r, ARG_SID);
                    }
                }
                //Import Data
                if (IMPORT_USER)
                {
                    foreach (RegFile re in regs)
                    {
                        RegFile r = re.GetRegFile(ARG_SID);
                        if (CurrentUser)
                            RegImport(r, ARG_SID);
                    }
                }
            }
            //Install For A Different User(s) or * for All Users
            else if (UNINSTALL_USER || IMPORT_USER)
            {
                string Users = GetUsersDir() + @"\";

                //Loop through All SIDS / Users Specified
                using (PrincipalContext context = new PrincipalContext(ContextType.Machine))
                {
                    using (PrincipalSearcher searcher = new PrincipalSearcher(new UserPrincipal(context)))
                    {
                        foreach (Principal result in searcher.FindAll())
                        {
                            if (result is UserPrincipal user)
                            {
                                string usrname = user.SamAccountName.ToUpper();
                                string file_hive = Users + user.SamAccountName + @"\NTUSER.DAT";
                                string sid = user.Sid.ToString().ToUpper();
                                //Handle Default Account SID
                                if(usrname.Equals("DEFAULTACCOUNT") || usrname.Equals("DEFAULT"))
                                {
                                    sid = "DEFAULT";
                                    HasRunDefault = true;
                                }
                                bool exists = File.Exists(file_hive);
                                if (exists && allsids || sids.Contains(sid) || sids.Contains(usrname))
                                {
                                    Hive h = new Hive(file_hive, sid, RegistryHive.Users);
                                    try
                                    {
                                        if (exists)
                                            h.Load();
                                        else
                                            h = null;
                                    }
                                    catch (Exception)
                                    {
                                        Console.Error.WriteLine("Failed to Load NTUSER.DAT:" + user.SamAccountName);
                                        h = null;
                                    }
                                    finally
                                    {
                                        if (HasUser(sid))
                                        {
                                            Console.WriteLine($"Reg Import User: {user.SamAccountName}, SID: {sid}");
                                            try
                                            {
                                                //Gen Uninstall Data Per User
                                                if (UNINSTALL_USER)
                                                {
                                                    foreach (RegFile re in regs)
                                                    {
                                                        RegFile r = re.GetRegFile(sid);
                                                        RegGenUninstall(r, sid);
                                                    }
                                                }

                                                //Import Data Per User
                                                if (IMPORT_USER)
                                                {
                                                    foreach (RegFile re in regs)
                                                    {
                                                        RegFile r = re.GetRegFile(sid);
                                                        RegImport(r, sid);
                                                    }
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
            }

            //Gen Uninstall Data
            if (UNINSTALL_USER && (USER_DEFAULT || USER_DOTDEFAULT))
            {
                foreach (RegFile re in regs)
                {
                    RegFile r = re.GetRegFile(ARG_SID);
                    if (USER_DEFAULT && !HasRunDefault)
                        RegGenUninstall(re.GetRegFile("DEFAULT"), "DEFAULT");
                    if (USER_DOTDEFAULT)
                        RegGenUninstall(re.GetRegFile(".DEFAULT"), ".DEFAULT");
                }
            }
            //Import Data
            if (IMPORT_USER && (USER_DEFAULT || USER_DOTDEFAULT))
            {
                foreach (RegFile re in regs)
                {
                    RegFile r = re.GetRegFile(ARG_SID);
                    if (USER_DEFAULT && !HasRunDefault)
                        RegImport(re.GetRegFile("DEFAULT"), "DEFAULT");
                    if (USER_DOTDEFAULT)
                        RegImport(re.GetRegFile(".DEFAULT"), ".DEFAULT");
                }
            }

            //Unload Default Hive Quietly
            try
            {
                DefHive.UnLoad();
            }
            catch(Exception)
            {

            }

            long done = DateTimeOffset.Now.ToUnixTimeMilliseconds();
            Console.WriteLine("Import Settings Done in MS:" + (done - milliseconds));
        }

        /// <summary>
        /// Loads the Default Hive and Returns The Registry Key Name
        /// </summary>
        /// <returns></returns>
        public static Hive LoadDefaultHive()
        {
            string DefHive = GetDefaultUserDat();
            //Backup The Default NTUSER.DAT if it's not been backed up before
            if (!File.Exists(DefHive + @".bak"))
            {
                try
                {
                    File.Copy(DefHive, DefHive + @".bak");
                }
                catch (Exception)
                {

                }
            }
            //See if the Default NTUSER.DAT is Already Opened and if so Return it
            try
            {
                string DosPath = GetDevicePath(DefHive);
                RegistryKey k = GetRegTree("HKLM").OpenSubKey(@"SYSTEM\CurrentControlSet\Control\hivelist", false);
                foreach (var v in k.GetValueNames())
                {
                    var o = k.GetValue(v);
                    if(o is string)
                    {
                        string s = o.ToString().Trim().Trim('\0');
                        if (s.Equals(DosPath, StringComparison.OrdinalIgnoreCase))
                        {
                            CDH = !v.Equals(@"\REGISTRY\USER\Default", StringComparison.OrdinalIgnoreCase);
                            if (v.StartsWith(@"\REGISTRY\USER\", StringComparison.OrdinalIgnoreCase))
                            {
                                return new Hive(DefHive, v.Substring(15).Trim('\0'), RegistryHive.Users);
                            }
                            else if (v.StartsWith(@"\REGISTRY\MACHINE\", StringComparison.OrdinalIgnoreCase))
                            {
                                return new Hive(DefHive, v.Substring(18).Trim('\0'), RegistryHive.LocalMachine);
                            }
                        }
                    }
                }
            }
            catch(Exception e)
            {
                Console.Error.WriteLine(e);
            }
            Hive d = new Hive(DefHive, "Default", RegistryHive.Users);
            try
            {
                d.Load();
            }
            catch(Exception)
            {
                Console.Error.WriteLine("Failed to Load Default NTUSER.DAT");
            }
            return d;
        }

        private static string GetDefaultUserDat()
        {
            try
            {
                RegistryKey k = GetRegTree("HKLM").OpenSubKey(@"SOFTWARE\Microsoft\Windows NT\CurrentVersion\ProfileList", false);
                string u = Environment.ExpandEnvironmentVariables((string)k.GetValue("Default"));
                if (u.EndsWith(@"\"))
                    u = u.Substring(0, u.Length - 1);
                Close(k);
                return Path.Combine(u ,"NTUSER.DAT");
            }
            catch(Exception e)
            {
                Console.Error.WriteLine(e);
            }
            return GetHomeDrive() + @":\Users\Default\NTUSER.DAT";
        }

        private static string GetUsersDir()
        {
            try
            {
                RegistryKey k = GetRegTree("HKLM").OpenSubKey(@"SOFTWARE\Microsoft\Windows NT\CurrentVersion\ProfileList", false);
                string u = Environment.ExpandEnvironmentVariables((string)k.GetValue("ProfilesDirectory"));
                if (u.EndsWith(@"\"))
                    u = u.Substring(0, u.Length - 1);
                Close(k);
                return u;
            }
            catch (Exception e)
            {
                Console.Error.WriteLine(e);
            }
            
            return GetHomeDrive() + @":\Users";
        }

        private static string GetHomeDrive()
        {
            return Environment.ExpandEnvironmentVariables("%HOMEDRIVE%").Substring(0, 1).ToUpper();
        }

        public static bool HasUser(string sid)
        {
            try
            {
                RegistryKey k = GetRegTree("HKU").OpenSubKey(sid, false);
                Close(k);
                return k != null;
            }
            catch (Exception)
            {

            }
            return false;
        }

        public static string GetCurrentSID()
        {
            return WindowsIdentity.GetCurrent().User.Value.ToUpper();
        }

        public static RegWriter GetRegWriter(string RelPath, string SID, bool USR)
        {
            if (UNINSTALL_GLOBAL && !USR)
            {
                string GlobalReg = UninstallDir + @"\Global\" + RelPath;
                if (!UNINSTALL_OVERWRITE && File.Exists(GlobalReg))
                    return null;
                mkdir(Path.GetDirectoryName(GlobalReg));
                RegWriter w = new RegWriter(GlobalReg);
                w.WriteLineParent("Windows Registry Editor Version 5.00");
                return w;
            }
            else if (UNINSTALL_USER && USR)
            {
                string UserReg = UninstallDir + @"\Users\" + SID + @"\" + RelPath;
                if (!UNINSTALL_OVERWRITE && File.Exists(UserReg))
                    return null;
                mkdir(Path.GetDirectoryName(UserReg));
                RegWriter w = new RegWriter(UserReg);
                w.WriteLineParent("Windows Registry Editor Version 5.00");
                return w;
            }
            return null;
        }

        public static Dictionary<string, RegWriter> writer_cache = new Dictionary<string, RegWriter>();

        public static void RegGenUninstall(RegFile reg, string SID)
        {
            bool USR = (SID != null);
            if (!UNINSTALL_GLOBAL && !USR || !UNINSTALL_USER && USR)
                return;

            RegWriter writer_org = GetRegWriter(reg.RelPath, SID, USR);
            if (writer_org == null)
                return;
            //Clear the Writer Cache of other users
            if (writer_cache.Count != 0)
                writer_cache.Clear();

            RegWriter writer_current = writer_org;

            //Gen Uninstall Data
            RegistryKey LastKey = null;
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
                        //Re-Direct Other Users to a Different Reg File
                        if (USR && !k.SubKey.StartsWith(SID))
                        {
                            string OtherSID = k.SubKey.Split('\\')[0];
                            if (!writer_cache.ContainsKey(OtherSID))
                            {
                                RegWriter w = GetRegWriter(Path.GetFileNameWithoutExtension(reg.RelPath) + "_gen.reg", OtherSID, true);
                                writer_cache.Add(OtherSID, w);
                                writer_current = w;
                            }
                            else
                            {
                                writer_current = writer_cache[OtherSID];
                            }
                            //Handle Previously Generated Reg Files
                            if (writer_current == null)
                            {
                                LastKey = null;
                                writer_current = writer_org;
                                continue;
                            }
                        }
                        else
                        {
                            writer_current = writer_org;
                        }
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
            //Close Other User RegWriters
            foreach (var c in writer_cache)
            {
                if (c.Value == null)
                    continue;
                Close(c.Value);
                //Delete Blank Reg Files
                if (!c.Value.HasWritten)
                {
                    try
                    {
                        if (File.Exists(c.Value.File))
                        {
                            File.Delete(c.Value.File);
                        }
                    }
                    catch (Exception e)
                    {
                        Console.Error.Write("Error Deleting Blank REG File: ");
                        Console.Error.WriteLine(e);
                    }
                }
            }
            Close(writer_org);
            Close(LastKey);

            //Delete Blank Reg Files
            if (!writer_org.HasWritten)
            {
                try
                {
                    if (File.Exists(writer_org.File))
                    {
                        File.Delete(writer_org.File);
                    }
                }
                catch (Exception e)
                {
                    Console.Error.Write("Error Deleting Blank REG File: ");
                    Console.Error.WriteLine(e);
                }
            }
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

        public static string GetRegTree(RegistryHive k)
        {
            switch (k)
            {
                case RegistryHive.LocalMachine:
                    return "HKEY_LOCAL_MACHINE";
                case RegistryHive.CurrentUser:
                    return "HKEY_CURRENT_USER";
                case RegistryHive.Users:
                    return "HKEY_USERS";
                case RegistryHive.ClassesRoot:
                    return "HKEY_CLASSES_ROOT";
                case RegistryHive.CurrentConfig:
                    return "HKEY_CURRENT_CONFIG";
                default:
                    return null;
            }
        }

        [DllImport("kernel32.dll", CharSet = CharSet.Auto, SetLastError = true)]
        static extern uint QueryDosDevice(string lpDeviceName, IntPtr lpTargetPath, uint ucchMax);

        public static string GetDevicePath(string path)
        {
            IntPtr lpTargetPath = IntPtr.Zero;

            try
            {
                string driveLetter = path.Substring(0, 2);
                lpTargetPath = Marshal.AllocHGlobal(260);
                // Query DOS device to get the device path
                uint result = QueryDosDevice(driveLetter, lpTargetPath, 260);
                if (result != 0)
                {
                    return Marshal.PtrToStringAuto(lpTargetPath) + path.Substring(2);
                }
                else
                {
                    Console.Error.WriteLine("Failed to retrieve device path");
                }
            }
            finally
            {
                if (lpTargetPath != IntPtr.Zero)
                    Marshal.FreeHGlobal(lpTargetPath);
            }
            return null;
        }
    }
}

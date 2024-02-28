using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.DirectoryServices.AccountManagement;
using System.DirectoryServices;
using System.IO;
using System.Security.Principal;
using System.Runtime.InteropServices;
using Microsoft.Win32;

namespace SIDPRINTER
{

    class Program
    {
        static void Main(string[] args)
        {
            //PARSE SIDS
            string ARG_SID = args[0].Trim().ToUpper();
            if (ARG_SID.Equals("") || ARG_SID.Equals("NULL"))
                ARG_SID = GetCurrentSID();
            List<string> sids = new List<string>(ARG_SID.Split(';'));
            bool allsids = ARG_SID.Equals("*");
            string HomeDrive = Environment.ExpandEnvironmentVariables("%HOMEDRIVE%").Substring(0, 1).ToUpper();
            string Users = HomeDrive + @":\Users\";

            //Get Proper Privileges
            Hive.GetHivePrivileges();

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
                            if(File.Exists(file_hive) && (allsids || sids.Contains(sid.ToUpper())))
                            {
                                Console.WriteLine($"Loading Hive User: {user.SamAccountName}, SID: {sid}");
                                try
                                {
                                    Hive h = new Hive(file_hive, sid, RegistryHive.Users);
                                    h.Load();
                                    //CALL CUSTOM CODE HERE
                                    h.UnLoad();
                                }
                                catch(Exception e)
                                {
                                    Console.Error.WriteLine("Failed to Load NTUSER.DAT:" + user.SamAccountName + " " + e.Message);
                                }
                            }
                        }
                    }
                }
            }
        }

        public static string GetCurrentSID()
        {
            WindowsIdentity windowsIdentity = WindowsIdentity.GetCurrent();
            return windowsIdentity.User.ToString();
        }
    }

    class Hive
    {
        [DllImport("advapi32.dll", SetLastError = true)]
        static extern int RegLoadKey(IntPtr hKey, string lpSubKey, string lpFile);

        [DllImport("advapi32.dll", SetLastError = true)]
        static extern int RegSaveKey(IntPtr hKey, string lpFile, uint securityAttrPtr = 0);

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
            if(!File.Exists(this.file_hive))
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

}

using System;
using System.Runtime.InteropServices;
using System.Security.AccessControl;
using System.Security.Principal; // Add this using directive for SecurityIdentifier, WellKnownSidType, NTAccount, IdentityReference
using Microsoft.Win32;

class Program
{
    static void Main(string[] args)
    {
        if(args.Length < 1)
        {
            Console.Error.WriteLine("GrantReg.exe <RegistryPath>");
            return;
        }
        string registryKeyPath = args[0].Trim().Replace('/', '\\');

        try
        {
            AddPrivilege("SeRestorePrivilege");
            AddPrivilege("SeBackupPrivilege");
            AddPrivilege("SeTakeOwnershipPrivilege");
            AddPrivilege("SeSecurityPrivilege");
            RegistryKey TREE = GetRegTree(registryKeyPath);
            if(TREE == null)
            {
                Console.Error.WriteLine(@"Registry Tree is Invalid. Examples Include HKLM\Path or HKCU\Path");
                return;
            }
            registryKeyPath = registryKeyPath.Substring(registryKeyPath.IndexOf('\\') + 1);
            Console.WriteLine(registryKeyPath);
            TakeOwnRecurse(registryKeyPath, TREE);
        }
        catch (Exception ex)
        {
            Console.WriteLine("Error: " + ex.Message);
        }
    }

    static RegistryKey GetRegTree(string k)
    {
        string u = k.ToUpper();
        if(u.StartsWith("HKLM") || u.StartsWith("HKEY_LOCAL_MACHINE"))
        {
            return Registry.LocalMachine;
        }
        else if(u.StartsWith("HKCU") || u.StartsWith("HKEY_CURRENT_USER"))
        {
            return Registry.CurrentUser;
        }
        else if(u.StartsWith("HKCR") || u.StartsWith("HKEY_CLASSES_ROOT"))
        {
            return Registry.ClassesRoot;
        }
        else if (u.StartsWith("HKU") || u.StartsWith("HKEY_USERS"))
        {
            return Registry.Users;
        }
        else if (u.StartsWith("HKCC") || u.StartsWith("HKEY_CURRENT_CONFIG"))
        {
            return Registry.CurrentConfig;
        }
        return null;
    }

    static void AddPrivilege(string privKey)
    {
        if(!TokenManipulator.AddPrivilege(privKey))
        {
            Console.Error.WriteLine("Error Setting Privilege " + privKey);
        }
    }

    static void TakeOwnRecurse(string regPath, RegistryKey TREE)
    {
        TakeOwn(regPath, TREE);
        RegistryKey key = null;
        try
        {
            key = TREE.OpenSubKey(regPath);
            foreach (string valueName in key.GetValueNames())
            {
                string subPath = regPath + "\\" + valueName;
                //TakeOwn(subPath, TREE);
                //Console.WriteLine(subPath);
            }
            string[] subkeynames = key.GetSubKeyNames();
            key.Close();
            foreach (string valueName in subkeynames)
            {
                string subPath = regPath + "\\" + valueName;
               // Console.WriteLine(subPath);
                TakeOwnRecurse(subPath, TREE);
            }
        }
        catch
        {
            Console.WriteLine("WTF:");
        }
        finally
        {
            key.Close();
        }
    }

    static void TakeOwn(string registryKeyPath, RegistryKey TREE)
    {
        RegistryKey key = null;
        RegistryKey key2 = null;
        try
        {
            SecurityIdentifier administratorsSid = new SecurityIdentifier(WellKnownSidType.BuiltinAdministratorsSid, null);
            IdentityReference administratorsIdentity = administratorsSid.Translate(typeof(NTAccount));

            //TakeOwn
            key = TREE.OpenSubKey(registryKeyPath, RegistryKeyPermissionCheck.ReadWriteSubTree, RegistryRights.TakeOwnership);
            RegistrySecurity keySecurity = key.GetAccessControl();
            keySecurity.SetOwner(administratorsIdentity);
            key.SetAccessControl(keySecurity);

            //Grant Administrators Access
            key2 = TREE.OpenSubKey(registryKeyPath, RegistryKeyPermissionCheck.ReadWriteSubTree, RegistryRights.ChangePermissions);
            RegistrySecurity keySecurity2 = key2.GetAccessControl();
            RegistryAccessRule rule = new RegistryAccessRule(administratorsIdentity, RegistryRights.FullControl, InheritanceFlags.ContainerInherit | InheritanceFlags.ObjectInherit, PropagationFlags.None, AccessControlType.Allow);
            keySecurity2.AddAccessRule(rule);
            key2.SetAccessControl(keySecurity2);

            Console.WriteLine("RegGrant:" + registryKeyPath);
        }
        catch (Exception e)
        {
            Console.WriteLine("Error: " + e.StackTrace);
        }
        finally
        {
            closeKey(key);
            closeKey(key2);
        }
    }

    public static void closeKey(RegistryKey key)
    {
        try
        {
            key.Close();
        }
        catch(Exception e)
        {
            Console.Error.WriteLine(e.Message);
        }
    }
}

public class TokenManipulator
{
    [DllImport("advapi32.dll", ExactSpelling = true, SetLastError = true)]
    internal static extern bool AdjustTokenPrivileges(IntPtr htok, bool disall,
    ref TokPriv1Luid newst, int len, IntPtr prev, IntPtr relen);


    [DllImport("kernel32.dll", ExactSpelling = true)]
    internal static extern IntPtr GetCurrentProcess();


    [DllImport("advapi32.dll", ExactSpelling = true, SetLastError = true)]
    internal static extern bool OpenProcessToken(IntPtr h, int acc, ref IntPtr
    phtok);


    [DllImport("advapi32.dll", SetLastError = true)]
    internal static extern bool LookupPrivilegeValue(string host, string name,
    ref long pluid);


    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    internal struct TokPriv1Luid
    {
        public int Count;
        public long Luid;
        public int Attr;
    }

    internal const int SE_PRIVILEGE_DISABLED = 0x00000000;
    internal const int SE_PRIVILEGE_ENABLED = 0x00000002;
    internal const int TOKEN_QUERY = 0x00000008;
    internal const int TOKEN_ADJUST_PRIVILEGES = 0x00000020;

    public static bool AddPrivilege(string privilege)
    {
        try
        {
            bool retVal;
            TokPriv1Luid tp;
            IntPtr hproc = GetCurrentProcess();
            IntPtr htok = IntPtr.Zero;
            retVal = OpenProcessToken(hproc, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, ref htok);
            tp.Count = 1;
            tp.Luid = 0;
            tp.Attr = SE_PRIVILEGE_ENABLED;
            retVal = LookupPrivilegeValue(null, privilege, ref tp.Luid);
            retVal = AdjustTokenPrivileges(htok, false, ref tp, 0, IntPtr.Zero, IntPtr.Zero);
            return retVal;
        }
        catch (Exception ex)
        {
            throw ex;
        }

    }
    public static bool RemovePrivilege(string privilege)
    {
        try
        {
            bool retVal;
            TokPriv1Luid tp;
            IntPtr hproc = GetCurrentProcess();
            IntPtr htok = IntPtr.Zero;
            retVal = OpenProcessToken(hproc, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, ref htok);
            tp.Count = 1;
            tp.Luid = 0;
            tp.Attr = SE_PRIVILEGE_DISABLED;
            retVal = LookupPrivilegeValue(null, privilege, ref tp.Luid);
            retVal = AdjustTokenPrivileges(htok, false, ref tp, 0, IntPtr.Zero, IntPtr.Zero);
            return retVal;
        }
        catch (Exception ex)
        {
            throw ex;
        }
    }
}

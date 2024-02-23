using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace GenReg
{
    class GenReg
    {
        static string basedir;
        static Dictionary<string, string> Global;
        static string SID = null;
        static void Main(string[] args)
        {
            basedir = Path.GetFullPath(args[0]);
            string[] defs = args[1].Split(';');
            Global = new Dictionary<string, string>();
            foreach (string def in defs)
            {
                string[] arr = def.Split('=');
                string t = GetTree(arr[0]);
                if (t.Equals("HKEY_CURRENT_USER"))
                    SID = arr[1];
                Global.Add(t, basedir + @"\" + arr[1]);
            }
            string[] files = args[2].Split(';');
            foreach (string f in files)
            {
                try
                {
                    string file = Path.GetFullPath(f);
                    string tree = null;
                    Dictionary<string, StreamWriter> gens = new Dictionary<string, StreamWriter>();
                    using (StreamReader sr = new StreamReader(file))
                    {
                        string line;
                        bool isTreeLine = false;
                        // Read and display lines from the file until the end of the file is reached
                        while ((line = sr.ReadLine()) != null)
                        {
                            string tl = line.Trim();
                            if(tl.StartsWith(";"))
                            {
                                continue;
                            }
                            if(tl.StartsWith("["))
                            {
                                tree = tl.StartsWith("[-") ? tl.Substring(2, tl.IndexOf(@"\") - 2) : tl.Substring(1, tl.IndexOf(@"\") - 1);
                                isTreeLine = true;
                            }
                            else
                            {
                                isTreeLine = false;
                            }
                            if (tree == null)
                                continue;
                            string oreg = GetTreeDir(tree) + @"\" + Path.GetFileName(file);
                            if(!gens.ContainsKey(oreg))
                            {
                                var dir = Directory.GetParent(oreg);
                                if(!dir.Exists)
                                    Directory.CreateDirectory(dir.FullName);
                                gens.Add(oreg, new StreamWriter(oreg));
                                gens[oreg].WriteLine("Windows Registry Editor Version 5.00\r\n");
                            }
                            StreamWriter reg = gens[oreg];
                            if (isTreeLine && SID != null)
                                line = line.Replace(@"HKEY_CURRENT_USER\", @"HKEY_USERS\" + SID + @"\");
                            reg.WriteLine(line);
                        }
                    }
                    foreach (var t in gens)
                    {
                        try
                        {
                            t.Value.Close();
                        }
                        catch (Exception e)
                        {
                            Console.Error.WriteLine(e);
                        }
                    }
                }
                catch(Exception e)
                {
                    Console.Error.WriteLine(e);
                }
            }
        }

        static string GetTreeDir(string tree)
        {
            return Global.ContainsKey(tree.ToUpper()) ? Global[tree] : basedir;
        }

        static string GetTree(string t)
        {
            t = t.ToUpper();
            switch (t)
            {
                case "HKLM":
                    return "HKEY_LOCAL_MACHINE";
                case "HKCU":
                    return "HKEY_CURRENT_USER";
                case "HKCR":
                    return "HKEY_CLASSES_ROOT";
                case "HKU":
                    return "HKEY_USERS";
                case "HKCC":
                    return "HKEY_CURRENT_CONFIG";
                default:
                    return t;
            }
        }
    }
}

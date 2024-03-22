using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace FindSplit
{
    class Program
    {
        static void Main(string[] args)
        {
            string str = args[0];
            string delim = args[1];
            delim = delim.Equals("") ? " " : delim;
            string tofind = args[2];
            bool ignorecase = args.Length > 3 ? args[3].StartsWith("T", StringComparison.OrdinalIgnoreCase) : false;
            bool FoundStr = false;
            foreach (var v in str.Split(new string[] { delim }, StringSplitOptions.None))
            {
                if (!ignorecase && v.Equals(tofind) || ignorecase && v.Equals(tofind, StringComparison.OrdinalIgnoreCase))
                {
                    FoundStr = true;
                    break;
                }
            }
            Environment.Exit(FoundStr ? 0 : -1);
        }
    }
}

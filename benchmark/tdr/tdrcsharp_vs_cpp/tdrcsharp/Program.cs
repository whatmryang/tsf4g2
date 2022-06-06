using System;
using System.Collections.Generic;
using System.Text;
using tsf4g_tdr_csharp;
using performance;
using System.Globalization;
namespace tdrcsharp
{
    class Program
    {
        static void initPkg(ref Mixture pkg)
        {
            pkg.iCommand  = 100;
            pkg.dwMagic   = 0xffee0001;
            pkg.ullSrc    = 0xff0000000001;
            pkg.ullDst    = 0xff0000000002;
            pkg.iHasAttach = 1;

            pkg.stAttach.fProfit = 0.0123f;
            pkg.stAttach.dMoney = 1234567890123.123;
            pkg.stAttach.dwHasDesc = 1;

            string szDesc_def = "this was a rough performance test";
            byte[] szDesc_tmp = Encoding.ASCII.GetBytes(szDesc_def);
            for (int i = 0; i < szDesc_tmp.GetLength(0); i++)
            {
                pkg.stAttach.szDesc[i] = (sbyte)szDesc_tmp[i];
            }

            pkg.stAttach.dwCount = 100;
            for (int i = 0; i < (int)pkg.stAttach.dwCount; i++)
            {
                pkg.stAttach.astPoints[i].x = (uint)i;
                pkg.stAttach.astPoints[i].y = (uint)i;
            }


        }

        static void Main(string[] args)
        {
            TdrError.ErrorType ret = TdrError.ErrorType.TDR_NO_ERROR;
            if (args.GetLength(0) < 1)
            {
                Console.WriteLine("Usage:  tdrcsharp.exe " + " LOOP_COUNT");
                return;
            }
            int iLoopCount = Convert.ToInt32(args[0]);

            Mixture pkg = new Mixture();
            initPkg(ref pkg);

            byte[] buffer = new byte[4096];
            int usesize = 0;

            long lBegin;
            long lEnd;
            double lDelta;
            long lBegin2;
            long lEnd2;
            double lDelta2;
            int i = 0;
            double dw = 10000000.0;

            lBegin = DateTime.Now.ToFileTime();

            for (i = 0; i < iLoopCount; i++)
            {
                ret = pkg.pack(ref buffer, buffer.GetLength(0), ref usesize, 0);
                if (ret != TdrError.ErrorType.TDR_NO_ERROR)
                {
                    Console.WriteLine("Failed to pack pkg into buffer. loop: " + Convert.ToString(i));
                    return;
                }

            }

            lEnd = DateTime.Now.ToFileTime();
            lDelta = (double)(lEnd - lBegin) / dw;
            Console.WriteLine("pack, time : " + Convert.ToString(lDelta));

            Mixture pkg2 = new Mixture();
            int usesize2 = 0;
            lBegin2 = DateTime.Now.ToFileTime();
            for (i = 0; i < iLoopCount; i++)
            {
                ret = pkg2.unpack(ref buffer, usesize, ref usesize2, 0);
                if (ret != TdrError.ErrorType.TDR_NO_ERROR)
                {
                    Console.WriteLine("Failed to unpack pkg into buffer. loop: " + Convert.ToString(i));
                    return;

                }
            }

            lEnd2 = DateTime.Now.ToFileTime();
            lDelta2 = (double)(lEnd2 - lBegin2) / dw;
            Console.WriteLine("unpack, time : " + Convert.ToString(lDelta2));

            while (true)
            {
                Console.WriteLine("press any key to continue ......");
                Console.Read();
                break;
            }
            
       


            //sbyte[] src  = new sbyte[10];
            //for (int i = 0; i < 10;i++ )
            //{
            //    src[i] = (sbyte)i;
            //}
            //byte[] dest = new byte[100];

            //src.CopyTo(dest, 10);
        }
    }
}

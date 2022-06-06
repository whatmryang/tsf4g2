using System;
using System.Collections.Generic;
using System.Text;
using tsf4g_tdr_csharp;
using net;
namespace TdrCSharpTest
{
    class Program
    {
        static void InitPkg(ref Pkg stPkg)
        {

            stPkg.stHead.dwMsgid = 100;
            stPkg.stHead.nCmd = 0;
            stPkg.stHead.nVersion = 9;
            stPkg.stHead.nMagic = 0x3344;
            stPkg.dwArray = 1;
            stPkg.wCount = 2;
            stPkg.stBody.stLogin = new CmdLogin();



            TdrTypeUtil.str2TdrIP(out stPkg.ip[0], "172.25.40.107");
            TdrTypeUtil.str2TdrIP(out stPkg.ip[1], "172.25.40.108");

            TdrTypeUtil.str2TdrTime(out stPkg.time[0], "11:06:30");
            TdrTypeUtil.str2TdrTime(out stPkg.time[1], "15:26:10");

            TdrTypeUtil.str2TdrDate(out stPkg.date[0], "2011-04-01");
            TdrTypeUtil.str2TdrDate(out stPkg.date[1], "2011-03-31");

            TdrTypeUtil.str2TdrDateTime(out stPkg.datetime[0], "2011-04-01 15:21:30");
            TdrTypeUtil.str2TdrDateTime(out stPkg.datetime[1], "2008-08-08 20:08:08");

            string szName_def = "michael";
            byte[] szName_tmp = Encoding.ASCII.GetBytes(szName_def);
            for (int i = 0; i < szName_tmp.GetLength(0); i++)
            {
                stPkg.stBody.stLogin.szName[i] = (sbyte)szName_def[i];
            }

            string szPass_def = "is9002ISO";
            byte[] szPass_tmp = Encoding.ASCII.GetBytes(szPass_def);
            for (int i = 0; i < szPass_tmp.GetLength(0); i++)
            {
                stPkg.stBody.stLogin.szPass[i] = (sbyte)szPass_tmp[i];
            }

            string szZone_def = "chengdu";
            byte[] szZone_tmp = Encoding.ASCII.GetBytes(szZone_def);
            for (int i = 0; i < szZone_tmp.GetLength(0); i++)
            {
                stPkg.stBody.stLogin.szZone[i] = (sbyte)szZone_tmp[i];
            }



        }
        static void Main(string[] args)
        {
            TdrError.ErrorType ret = TdrError.ErrorType.TDR_NO_ERROR;

            byte[] buffer = new byte[10240];
  

            TdrWriteBuf packBuf = new TdrWriteBuf(ref buffer, buffer.GetLength(0));
            Pkg pkg = new Pkg();

            InitPkg(ref pkg);

            /*打包 */
            ret = pkg.pack(ref packBuf, 0);
            if (ret != TdrError.ErrorType.TDR_NO_ERROR)
            {
                Console.WriteLine("pack error: "  + TdrError.getErrorString(ret));
                return;
            }

            /*如果在元数据中定义，定义了sizeinfo属性，获得数据包的总长度*/
            uint pkgSize = 0;
            ret = pkg.getSizeInfo(buffer, 10, ref pkgSize);
            if (ret != TdrError.ErrorType.TDR_NO_ERROR)
            {
                Console.WriteLine("get pkg size error: " + TdrError.getErrorString(ret));
                return;
            }

            /* 解包 */
            TdrReadBuf unpackBuf = new TdrReadBuf(ref buffer, buffer.GetLength(0));
            ret = pkg.unpack(ref unpackBuf, 0);
            if (ret != TdrError.ErrorType.TDR_NO_ERROR)
            {
                Console.WriteLine("uppack error: "  + TdrError.getErrorString(ret));
                return;
            }

            /*可视化*/

            TdrVisualBuf visualBuf = new TdrVisualBuf();

            ret = pkg.visualize(ref visualBuf, 0, '\n');
            if (ret != TdrError.ErrorType.TDR_NO_ERROR)
            {
                Console.WriteLine("visualize error: " + TdrError.getErrorString(ret));
                return;
            }

            Console.Write(visualBuf.getVisualBuf());

  

            while (true)
            {
                Console.WriteLine("press any key to over .......");
                int ch  = Console.Read();
                break;
            }
        }
    }
}

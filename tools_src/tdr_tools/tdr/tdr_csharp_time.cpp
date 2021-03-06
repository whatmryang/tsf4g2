#include "tdr_csharp.h"

const char*
TTransCSharp::timeCode[] = {
    "public class TdrTime",
    "{",
    "    public short nHour;",
    "    public byte  bMin ;",
    "    public byte  bSec ;",
    "",
    "    public TdrTime(){}",
    "",
    "    public TdrTime(uint time)",
    "    {",
    "        nHour = (short)(time & 0xFFFF)      ;",
    "        bMin  = (byte) ((time >> 16) & 0xFF);",
    "        bSec  = (byte) ((time >> 24) & 0xFF);",
    "    }",
    "",
    "    public TdrError.ErrorType parse(uint time)",
    "    {",
    "        TdrError.ErrorType ret = TdrError.ErrorType.TDR_NO_ERROR;",
    "",
    "        nHour  = (short)(time & 0xFFFF)      ;",
    "        bMin   = (byte) ((time >> 16) & 0xFF);",
    "        bSec   = (byte) ((time >> 24) & 0xFF);",
    "",
    "        if(!isValid())",
    "        {",
    "            ret = TdrError.ErrorType.TDR_ERR_INVALID_TDRTIME_VALUE;",
    "        }",
    "",
    "        return ret;",
    "    }",
    "",
    "    public  bool isValid()",
    "    {",
    "        string str = string.Format(\"{0:d2}:{1:d2}:{2:d2}\", nHour, bMin, bSec);",
    "        DateTime dt;",
    "",
    "        if (!DateTime.TryParse(str, out dt))",
    "        {",
    "#if (DEBUG)",
    "            StackTrace st = new StackTrace(true);",
    "            for (int i = 0; i < st.FrameCount; i++)",
    "            {",
    "                if (null != st.GetFrame(i).GetFileName())",
    "                {",
    "                    Console.WriteLine(st.GetFrame(i).ToString());",
    "                }",
    "            }", 
    "#endif",
    "            return false;",
    "        }",
    "",
    "        return true;",
    "    }",
    "",
    "    public void toTime(out uint time)",
    "    {",
    "        time = (uint)(((ushort)nHour | ((uint)bMin << 16) | ((uint)bSec << 24)));",
    "    }",
    "}",
    "",
    "public class TdrDate",
    "{",
    "    public short nYear;",
    "    public byte  bMon ;",
    "    public byte  bDay ;",
    "",
    "    public TdrDate(){}",
    "",
    "    public TdrDate(uint date)",
    "    {",
    "        nYear = (short)(date & 0xFFFF)     ;",
    "        bMon  = (byte)((date >> 16) & 0xFF);",
    "        bDay  = (byte)((date >> 24) & 0xFF);",
    "    }",
    "",
    "    public TdrError.ErrorType parse(uint date)",
    "    {",
    "        TdrError.ErrorType ret = TdrError.ErrorType.TDR_NO_ERROR;",
    "",
    "        nYear = (short)(date & 0xFFFF)     ;",
    "        bMon  = (byte)((date >> 16) & 0xFF);",
    "        bDay  = (byte)((date >> 24) & 0xFF);",
    "",
    "        if (!isValid())",
    "        {",
    "            ret = TdrError.ErrorType.TDR_ERR_INVALID_TDRTIME_VALUE;",
    "        }",
    "",
    "        return ret;",
    "    }",
    "",
    "    public  bool isValid()",
    "    {",
    "        string str = string.Format(\"{0:d4}-{1:d2}-{2:d2}\", nYear, bMon, bDay);",
    "        DateTime dt;",
    "",
    "        if (!DateTime.TryParse(str, out dt))",
    "        {",
    "#if (DEBUG)",
    "            StackTrace st = new StackTrace(true);",
    "            for (int i = 0; i < st.FrameCount; i++)",
    "            {",
    "                if (null != st.GetFrame(i).GetFileName())",
    "                {",
    "                    Console.WriteLine(st.GetFrame(i).ToString());",
    "                }",
    "            }", 
    "#endif",
    "            return false;",
    "        }",
    "",
    "        return true;",
    "    }",
    "",
    "    public void toDate(out uint date)",
    "    {",
    "        date = (uint)(((ushort)nYear | ((uint)bMon << 16) | ((uint)bDay << 24)));",
    "    }",
    "",
    "}",
    "",
    "public class TdrDateTime",
    "{",
    "    public TdrDate tdrDate;",
    "    public TdrTime tdrTime;",
    "",
    "    public TdrDateTime()",
    "    {",
    "        tdrDate = new TdrDate();",
    "        tdrTime = new TdrTime();",
    "    }",
    "",
    "    public TdrDateTime(ulong datetime)",
    "    {",
    "        tdrDate = new TdrDate((uint)(datetime & 0xFFFFFFFF));",
    "        tdrTime = new TdrTime((uint)((datetime >> 32) & 0xFFFFFFFF));",
    "    }",
    "",
    "    public TdrError.ErrorType parse(ulong datetime)",
    "    {",
    "        TdrError.ErrorType ret = TdrError.ErrorType.TDR_NO_ERROR;",
    "",
    "        uint date = (uint)(datetime & 0xFFFFFFFF);",
    "        uint time = (uint)((datetime>>32) & 0xFFFFFFFF);",
    "",
    "        ret = tdrDate.parse(date);",
    "        if (ret == TdrError.ErrorType.TDR_NO_ERROR)",
    "        {",
    "            ret = tdrTime.parse(time);",
    "        }",
    "",
    "        return ret;",
    "    }",
    "",
    "    public void toDateTime(out ulong datetime)",
    "    {",
    "        uint date = 0;",
    "        uint time = 0;",
    "",
    "        tdrDate.toDate(out date);",
    "        tdrTime.toTime(out time);",
    "",
    "        datetime = ((ulong)date | (ulong)time << 32);",
    "    }",
    "",
    "    public bool isValid()",
    "    {",
    "        return tdrDate.isValid() && tdrTime.isValid();",
    "    }",
    "",
    "}",
    "",
};



void
TTransCSharp::makeTdrTime()
{
    DEFINE_CSHARP_CONTAINER();
    APPEND_CSHARP_CONTAINER("");
    APPEND_CSHARP_CONTAINER("using System;");
    APPEND_CSHARP_CONTAINER("using System.Diagnostics;");
    APPEND_CSHARP_CONTAINER("");
    APPEND_CSHARP_CONTAINER("namespace tsf4g_tdr_csharp");
    APPEND_CSHARP_CONTAINER("{");
    {
        APPEND_CSHARP_CONTAINER("");
        loadCodeRes(0,  Lines, timeCode, 
            sizeof(TTransCSharp::timeCode)/sizeof(TTransCSharp::timeCode[0]));
    }

    APPEND_CSHARP_CONTAINER("}");
    OUTPUT_CSHARP_CONTAINER_NOMETA("TdrTime.cs");
}



#coding=gbk

import os
import basetemp

heads = '''
<table align=center style="border-collapse:collapse;width:894">
<tr style="background-image:url('$(resouce)/res/headbg.jpg');width:894;height:130">
<td>
	<div style="position:relative; left:520px; top:-40px; color:white">
		<a style="color:white;text-decoration:none;font-size:14;font-family:Tahoma;" href="http://ied.oa.com/TSF4G/Publish/Publish.aspx">发布系统</a> |
<a style="color:white;text-decoration:none;font-size:14;font-family:Tahoma;" href="http://tapd.oa.com/v3/frame/prong/stories/add">提提需求</a> |
<a style="color:white;text-decoration:none;font-size:14;font-family:Tahoma;" href="http://tapd.oa.com/v2/bugtrace/1002191/bugs/add">提交Bug</a> |
<a style="color:white;text-decoration:none;font-size:14;font-family:Tahoma;" href="http://tapd.oa.com/v3/frame/comments">随便聊聊</a> |
<a style="color:white;text-decoration:none;font-size:14;font-family:Tahoma;" href="..\/contactus.htm">联系我们</a>

	</div>

</div>

<div style="position:relative; left:15px; top:42px; color:white">您的位置 : $(location)
</tr>
</table>
<br>
<table align=center border=1 style="border-collapse:collapse;width:894;border-style:solid; border-width:1px">
<tr><td style =" padding: 10px; vertical-align:top;">
'''.decode('gbk')

trails = '''
</table>
<br>
<div align='center' style='color:#aaaaaa;font-size:14;font-family:Tahoma'>互娱研发中心架构平台组<br>Copyright&copy; 2006-2009 TSF4G All Rights Reserved</div>
'''.decode('gbk')

def Word2Html(srcfile, destfile, locations=[]):
    from win32com import client as wc
    #创建保存目录
    realpath = os.path.realpath((os.path.dirname(destfile)))
    if not os.path.exists(realpath):
        os.makedirs(realpath)
    word = wc.Dispatch('Word.Application')
    doc = word.Documents.Open(srcfile)
    doc.SaveAs(destfile, 8)
    doc.Close()
    word.Quit()

    text = open(destfile).read().decode('gbk')
    realheads = heads
    pos = text.find('<body')
    if pos == -1 : return
    pos = text.find('>', pos) +1
    text = text[:pos] + realheads + text[pos:]
    pos = text.find('</body')-1
    text = text[:pos] + trails + text[pos:]
    realsrcfile = basetemp.getPathDelta(destfile)
    text = text.replace('$(resouce)',realsrcfile)

    locstr = []
    #print locations
    for l in locations:
        realhref =  basetemp.getrealurl(l[1], destfile)
        str = ur'<a href="%s" style="color:white;text-decoration:none;font-size:14;font-family:Tahoma">%s</a>' % (realhref, l[0])
        locstr.append(str)
    navigation = u' - '.join(locstr)
    text = text.replace('$(location)', navigation)

    open(destfile, 'w').write(text.encode('gbk'))

if __name__ == '__main__':
    Word2Html(r'E:\project\tconnd文档\TSF4G_TCONND_Develop.doc', r'E:\python\unifiedenroll\tconnd\TSF4G_TCONND_Develop.htm', [[r'主页','index.htm'],[r'统一登录组件',r'unifiedenroll.htm'],[r'网络接入组件(tconnd)', r'unifiedenroll\tconnd.htm'], [r'在线文档', r'unifiedenroll\tconnd\tconnd_online_doc.htm']])


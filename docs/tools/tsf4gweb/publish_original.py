#coding=gbk

import os
import wx
import re
import sys
import yaml
import build
from webgen.doc2html import *
from webgen.tree import *

re_uri = re.compile(r'[\w_\-]+\.htm\Z')

TREESTYLE = wx.TR_LINES_AT_ROOT|wx.TR_HAS_BUTTONS

class MainFrame(wx.Frame):
    
    def __init__(self):
        wx.Frame.__init__(self, None, -1, 'TSF4G 网页管理系统', None, (800, 600))
        self.Bind(wx.EVT_CLOSE, self.onClose)
        
        bodyPanel = wx.Panel(self)
        
        bodybox = wx.BoxSizer(wx.VERTICAL)
        
        # 顶部控件
        topPanel = wx.Panel(bodyPanel)
        topbox = wx.BoxSizer(wx.HORIZONTAL)
        topPanel.SetSizer(topbox)
        text1 = wx.StaticText(topPanel, label=r'需要发布的文件：', style=wx.ALIGN_CENTER)
        self.srcFileEdt = wx.TextCtrl(topPanel)
        button1 = wx.Button(topPanel, label=r'选择文件')
        button1.Bind(wx.EVT_BUTTON, self.onSelectFile)
        topbox.Add(text1, 0, wx.TOP|wx.LEFT, 7)
        topbox.Add(self.srcFileEdt, 1, wx.EXPAND|wx.ALL, 3)
        topbox.Add(button1, 0, wx.ALL, 3)
        bodybox.Add(topPanel, 0, wx.EXPAND|wx.TOP, 3)
        
        # 中间控件
        middlePanel = wx.Panel(bodyPanel)
        controlPanel = wx.Panel(middlePanel, size=(400, 100))
        self.tree = wx.TreeCtrl(middlePanel, style=TREESTYLE)
        middleBox = wx.BoxSizer(wx.HORIZONTAL)
        middleBox.Add(self.tree, 1, wx.EXPAND|wx.ALL)
        middleBox.Add(controlPanel, 0, wx.RIGHT)
        middlePanel.SetSizer(middleBox)
        bodybox.Add(middlePanel, 1, wx.EXPAND|wx.ALL, 3)
        
        # 中间的控制版面
        wx.StaticText(controlPanel, -1, '显示标题：', (10, 10))
        self.itemDesc = wx.TextCtrl(controlPanel, pos=(70, 8), size=(300, 20))
        self.itemDesc.Bind(wx.EVT_TEXT, self.onItemDescChanged)

        self.itemUriCaption = wx.StaticText(controlPanel, -1, '页面文件：', (10, 40))
        self.itemUri = wx.TextCtrl(controlPanel, pos=(70, 38), size=(300, 20))
        self.itemUri.Bind(wx.EVT_TEXT, self.onUriChanged)
        
        wx.Button(controlPanel, -1, '增加结点', (10, 100)).Bind(wx.EVT_BUTTON, self.onInsert)
        wx.Button(controlPanel, -1, '删除结点', (90, 100)).Bind(wx.EVT_BUTTON, self.onDelete)
        wx.Button(controlPanel, -1, '保存修改', (170, 100)).Bind(wx.EVT_BUTTON, self.onSave)

        wx.Button(controlPanel, -1, '向上移动', (10, 130)).Bind(wx.EVT_BUTTON, self.onMoveUp)
        wx.Button(controlPanel, -1, '向下移动', (90, 130)).Bind(wx.EVT_BUTTON, self.onMoveDown)
        wx.Button(controlPanel, -1, '发布节点', (170, 130)).Bind(wx.EVT_BUTTON, self.onPublishDocument)
        
        # 底部控件
        self.memo = wx.ListCtrl(bodyPanel, size=(100, 100))
        bodybox.Add(self.memo, 0, wx.EXPAND|wx.ALL, 3)

        # 总界面
        bodyPanel.SetSizer(bodybox)
        
        # 初始化树控件
        root = self.tree.AddRoot('根目录')
        data = wx.TreeItemData()
        data.SetData({'uri':'index.htm'})
        self.tree.SetItemData(root, data)
        dirfile = open(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'directory.yaml'))
        info = yaml.load(dirfile.read().decode('gbk'))
        self.insertTree(self.tree.GetRootItem(), info['root'])
        BackupTree(self.tree)
        
        self.tree.Expand(self.tree.GetRootItem())
        self.tree.Bind(wx.EVT_TREE_SEL_CHANGED, self.treeSelChanged)
        self.tree.Bind(wx.EVT_TREE_SEL_CHANGING, self.treeSelChanging)
    
    def onSelectFile(self, event):
        filename = wx.FileSelector('请选择一个需要发布的文档')
        if filename != '':
            self.srcFileEdt.SetValue(filename)
        
    def onItemDescChanged(self, event):
        node = self.getCurrentSelection()
        if node:
            self.tree.SetItemText(node, self.itemDesc.GetValue())
    
    def onUriChanged(self, event):
        node = self.getCurrentSelection()
        if node:
            data = self.tree.GetItemData(node).GetData()
            data['uri'] = self.itemUri.GetValue()
        
    def copyAllChildren(self, src, dest):
        if self.tree.ItemHasChildren(src):
            child, cookie = self.tree.GetFirstChild(src)
            while child.IsOk():
                txt = self.tree.GetItemText(child)
                data = wx.TreeItemData()
                data.SetData(self.tree.GetItemData(child).GetData())
                newitem = self.tree.AppendItem(dest, txt, data=data)
                self.copyAllChildren(child, newitem)
                child, cookie = self.tree.GetNextChild(src, cookie)
    
    def onDelete(self, event):
        node = self.tree.GetSelection()
        isroot = (node == self.tree.GetRootItem())
        if (not node.IsOk()) or isroot:
            wx.MessageBox('请选择一个合法的节点')
            return
        if wx.MessageBox('确定要删除"'+self.tree.GetItemText(node)+'"吗？', style=wx.YES_NO) == wx.YES:
            self.tree.Delete(node)
    
    def onMoveUp(self, event):
        node = self.tree.GetSelection()
        if not node.IsOk : return
        if node == self.tree.GetRootItem() : return
        pre = self.tree.GetPrevSibling(node)
        if pre.IsOk():
            parent = self.tree.GetItemParent(pre)
            pre2 = self.tree.GetPrevSibling(pre)
            txt = self.tree.GetItemText(node)
            data = wx.TreeItemData()
            data.SetData(self.tree.GetItemData(node).GetData())
            if pre2.IsOk():
                newitem = self.tree.InsertItem(parent, pre2, txt, data=data)
            else:
                newitem = self.tree.InsertItemBefore(parent, 0, txt, data=data)
            self.copyAllChildren(node, newitem)
            self.tree.Delete(node)
            self.tree.SelectItem(newitem)
    
    def onMoveDown(self, event):
        node = self.tree.GetSelection()
        if not node.IsOk() : return
        if node == self.tree.GetRootItem() : return
        nextnode = self.tree.GetNextSibling(node)
        if nextnode.IsOk():
            parent = self.tree.GetItemParent(node)
            txt = self.tree.GetItemText(node)
            data = wx.TreeItemData()
            data.SetData(self.tree.GetItemData(node).GetData())
            newitem = self.tree.InsertItem(parent, nextnode, txt, data=data)
            self.copyAllChildren(node, newitem)
            self.tree.Delete(node)
            self.tree.SelectItem(newitem)
    
    def getAbsoPath(self, relaPath):
        root = os.path.dirname(os.path.realpath(__file__))
        return os.path.join(root, relaPath)

    def getCurrentSelection(self):
        node = self.tree.GetSelection()
        if not node.IsOk() : return None
        if node == self.tree.GetRootItem() : return None
        return node
    
    def getDirPath(self, node):
        result = []
        rootitem = self.tree.GetRootItem()
        while node.IsOk() and node != rootitem:
            name = self.tree.GetItemText(node).decode('gbk')
            htm = self.tree.GetItemData(node).GetData()['uri'].decode('gbk')
            result.append([name, htm])
            node = self.tree.GetItemParent(node)
        result.append([u'主页', u'index.htm'])
        result.reverse()
        return result
        
    def onPublishDocument(self, event):
        self.saveDirectory()
        uri = self.srcFileEdt.GetValue().lower()
        if os.path.splitext(uri)[1] != '.doc':
            wx.MessageBox('现只支持Word(.doc)文档的转换')
            return
        if not os.path.isfile(uri):
            wx.MessageBox('找不到指定的文件')
            return
        node = self.getCurrentSelection()
        if node:
            if self.tree.ItemHasChildren(node):
                wx.MessageBox('只能针对叶子节点进行发布')
            else:
                outpath = open(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'outputpath.txt')).read()
                dest = self.getAbsoPath(os.path.join(outpath, self.itemUri.GetValue()))
                Word2Html(uri, dest, self.getDirPath(node))
                wx.MessageBox('发布完毕')
        else:
            wx.MessageBox('请先选择一个节点再进行发布')
    
    def setNodeData(self, desc, uri):
        self.itemDesc.SetValue(desc)
        self.itemUri.SetValue(uri)
    
    def treeSelChanging(self, event):
        if not self.updateNodeData():
            event.Veto()
            
    def treeSelChanged(self, event):
        selnode = self.tree.GetSelection()
        if not selnode.IsOk() : return
        attributes = self.tree.GetItemData(selnode).GetData()
        if attributes:
            self.setNodeData(self.tree.GetItemText(selnode), attributes['uri'])
        else:
            self.setNodeData('', '')

    def insertTree(self, parentNode, infos):
        for i in infos:
            data = wx.TreeItemData()
            data.SetData({'uri':i['uri']})
            newNode = self.tree.AppendItem(parentNode, i['desc'], data=data)
            if i.has_key('children'):
                self.insertTree(newNode, i['children'])

    def updateNodeData(self):
        selnode = self.tree.GetSelection()
        if not selnode.IsOk() : return True
        if selnode == self.tree.GetRootItem() : return True
        txt = self.itemUri.GetValue()
        if True:#re_uri.match(txt):
            selnode = self.tree.GetSelection()
            attributes = self.tree.GetItemData(selnode).GetData()
            attributes['uri'] = self.itemUri.GetValue()
            self.tree.SetItemText(selnode, self.itemDesc.GetValue())
            return True
        else:
            wx.MessageBox('请输入正确的.htm文件名', '提示')
            return False

    def saveDirectory(self):
        if self.updateNodeData():
            yaml = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'directory.yaml')
            #print yaml
            os.remove(yaml)
            outfile = open(yaml, 'w')
            print >> outfile, 'root:'
            root = self.tree.GetRootItem()
            self.outputNode(root, outfile, '    ')
            outfile.close()
            modifieds = FindModifiers(self.tree)
            print modifieds
            BackupTree(self.tree)
            #build.buildHtmls(modifieds)
            build.buildHtmls()
            wx.MessageBox('保存成功')
        
    def onSave(self, event):
        self.saveDirectory()
    
    def onInsert(self, event):
        node = self.tree.GetSelection()
        if node.IsOk():
            data = wx.TreeItemData()
            data.SetData({'uri':'newpage.htm'})
            newnode = self.tree.AppendItem(node, '新建网页', data=data)
            self.tree.EnsureVisible(newnode)
            self.tree.SelectItem(newnode)
            self.itemDesc.SetFocus()
            self.itemDesc.SetSelection(0, -1)
        
    def outputNode(self, node, outfile, preblanks):
        child, cookie = self.tree.GetFirstChild(node)
        while(child.IsOk()):
            print >> outfile, preblanks + '-'
            self.outputNodeDetail(child, outfile, preblanks + '    ')
            self.outputNode(child, outfile, preblanks+'    ')
            child, cookie = self.tree.GetNextChild(node, cookie)            

    def outputNodeDetail(self, node, outfile, preblanks):
        print >> outfile, preblanks + 'desc: ' + self.tree.GetItemText(node)
        print >> outfile, preblanks + 'uri: ' + self.tree.GetItemData(node).GetData()['uri']
        child, cookie = self.tree.GetFirstChild(node)
        if child.IsOk():
            print >> outfile, preblanks + 'children:'

    def onClose(self, event):
        modifieds = FindModifiers(self.tree)
        if len(modifieds) > 0:
            if wx.MessageBox('树目录结构有修改，是否保存？', style=wx.YES_NO) == wx.YES:
                self.saveDirectory()
        self.Destroy()
    
def CheckOut():
    chkfile = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'checkout')
    if os.path.isfile(chkfile):
        user = open(chkfile).read().strip()
        wx.MessageBox('网站正在被 "%s" 编辑，请稍后再尝试' % user)
    else:
        return True

app = wx.PySimpleApp()
if CheckOut():
    chkfilepath = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'checkout')
    try:
        chkfile = open(chkfilepath, 'w')
        import socket
        chkfile.write(socket.gethostname())
        chkfile.close();
        frame = MainFrame()
        if len(sys.argv) > 1:
            frame.srcFileEdt.SetValue(sys.argv[1])
        frame.Show()
        app.MainLoop()
    finally:
        os.remove(chkfilepath)


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
        wx.Frame.__init__(self, None, -1, 'TSF4G ��ҳ����ϵͳ', None, (800, 600))
        self.Bind(wx.EVT_CLOSE, self.onClose)
        
        bodyPanel = wx.Panel(self)
        
        bodybox = wx.BoxSizer(wx.VERTICAL)
        
        # �����ؼ�
        topPanel = wx.Panel(bodyPanel)
        topbox = wx.BoxSizer(wx.HORIZONTAL)
        topPanel.SetSizer(topbox)
        text1 = wx.StaticText(topPanel, label=r'��Ҫ�������ļ���', style=wx.ALIGN_CENTER)
        self.srcFileEdt = wx.TextCtrl(topPanel)
        button1 = wx.Button(topPanel, label=r'ѡ���ļ�')
        button1.Bind(wx.EVT_BUTTON, self.onSelectFile)
        topbox.Add(text1, 0, wx.TOP|wx.LEFT, 7)
        topbox.Add(self.srcFileEdt, 1, wx.EXPAND|wx.ALL, 3)
        topbox.Add(button1, 0, wx.ALL, 3)
        bodybox.Add(topPanel, 0, wx.EXPAND|wx.TOP, 3)
        
        # �м�ؼ�
        middlePanel = wx.Panel(bodyPanel)
        controlPanel = wx.Panel(middlePanel, size=(400, 100))
        self.tree = wx.TreeCtrl(middlePanel, style=TREESTYLE)
        middleBox = wx.BoxSizer(wx.HORIZONTAL)
        middleBox.Add(self.tree, 1, wx.EXPAND|wx.ALL)
        middleBox.Add(controlPanel, 0, wx.RIGHT)
        middlePanel.SetSizer(middleBox)
        bodybox.Add(middlePanel, 1, wx.EXPAND|wx.ALL, 3)
        
        # �м�Ŀ��ư���
        wx.StaticText(controlPanel, -1, '��ʾ���⣺', (10, 10))
        self.itemDesc = wx.TextCtrl(controlPanel, pos=(70, 8), size=(300, 20))
        self.itemDesc.Bind(wx.EVT_TEXT, self.onItemDescChanged)

        self.itemUriCaption = wx.StaticText(controlPanel, -1, 'ҳ���ļ���', (10, 40))
        self.itemUri = wx.TextCtrl(controlPanel, pos=(70, 38), size=(300, 20))
        self.itemUri.Bind(wx.EVT_TEXT, self.onUriChanged)
        
        wx.Button(controlPanel, -1, '���ӽ��', (10, 100)).Bind(wx.EVT_BUTTON, self.onInsert)
        wx.Button(controlPanel, -1, 'ɾ�����', (90, 100)).Bind(wx.EVT_BUTTON, self.onDelete)
        wx.Button(controlPanel, -1, '�����޸�', (170, 100)).Bind(wx.EVT_BUTTON, self.onSave)

        wx.Button(controlPanel, -1, '�����ƶ�', (10, 130)).Bind(wx.EVT_BUTTON, self.onMoveUp)
        wx.Button(controlPanel, -1, '�����ƶ�', (90, 130)).Bind(wx.EVT_BUTTON, self.onMoveDown)
        wx.Button(controlPanel, -1, '�����ڵ�', (170, 130)).Bind(wx.EVT_BUTTON, self.onPublishDocument)
        
        # �ײ��ؼ�
        self.memo = wx.ListCtrl(bodyPanel, size=(100, 100))
        bodybox.Add(self.memo, 0, wx.EXPAND|wx.ALL, 3)

        # �ܽ���
        bodyPanel.SetSizer(bodybox)
        
        # ��ʼ�����ؼ�
        root = self.tree.AddRoot('��Ŀ¼')
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
        filename = wx.FileSelector('��ѡ��һ����Ҫ�������ĵ�')
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
            wx.MessageBox('��ѡ��һ���Ϸ��Ľڵ�')
            return
        if wx.MessageBox('ȷ��Ҫɾ��"'+self.tree.GetItemText(node)+'"��', style=wx.YES_NO) == wx.YES:
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
        result.append([u'��ҳ', u'index.htm'])
        result.reverse()
        return result
        
    def onPublishDocument(self, event):
        self.saveDirectory()
        uri = self.srcFileEdt.GetValue().lower()
        if os.path.splitext(uri)[1] != '.doc':
            wx.MessageBox('��ֻ֧��Word(.doc)�ĵ���ת��')
            return
        if not os.path.isfile(uri):
            wx.MessageBox('�Ҳ���ָ�����ļ�')
            return
        node = self.getCurrentSelection()
        if node:
            if self.tree.ItemHasChildren(node):
                wx.MessageBox('ֻ�����Ҷ�ӽڵ���з���')
            else:
                outpath = open(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'outputpath.txt')).read()
                dest = self.getAbsoPath(os.path.join(outpath, self.itemUri.GetValue()))
                Word2Html(uri, dest, self.getDirPath(node))
                wx.MessageBox('�������')
        else:
            wx.MessageBox('����ѡ��һ���ڵ��ٽ��з���')
    
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
            wx.MessageBox('��������ȷ��.htm�ļ���', '��ʾ')
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
            wx.MessageBox('����ɹ�')
        
    def onSave(self, event):
        self.saveDirectory()
    
    def onInsert(self, event):
        node = self.tree.GetSelection()
        if node.IsOk():
            data = wx.TreeItemData()
            data.SetData({'uri':'newpage.htm'})
            newnode = self.tree.AppendItem(node, '�½���ҳ', data=data)
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
            if wx.MessageBox('��Ŀ¼�ṹ���޸ģ��Ƿ񱣴棿', style=wx.YES_NO) == wx.YES:
                self.saveDirectory()
        self.Destroy()
    
def CheckOut():
    chkfile = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'checkout')
    if os.path.isfile(chkfile):
        user = open(chkfile).read().strip()
        wx.MessageBox('��վ���ڱ� "%s" �༭�����Ժ��ٳ���' % user)
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


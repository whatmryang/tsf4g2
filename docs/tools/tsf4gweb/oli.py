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
        self.icon = wx.Icon('mark.bmp', wx.BITMAP_TYPE_ICO)
        self.SetIcon(self.icon)
        
        
        # �����ؼ�
        topPanel = wx.Panel(bodyPanel)
        topbox = wx.BoxSizer(wx.HORIZONTAL)
        topPanel.SetSizer(topbox)
        text1 = wx.StaticText(topPanel, label=r'TSF4G����Ŀ¼��', style=wx.ALIGN_CENTER)
        self.srcDir=""
        self.srcFileEdt = wx.TextCtrl(topPanel)
        button1 = wx.Button(topPanel, label=r'ѡ��Ŀ¼')
        button1.Bind(wx.EVT_BUTTON, self.onSelectDir)
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

        self.itemSrcCaption = wx.StaticText(controlPanel, -1, 'Դ�ļ���', (10, 70))
        self.itemSrc = wx.TextCtrl(controlPanel, pos=(70, 68), size=(300, 20))
        self.itemSrc.Bind(wx.EVT_TEXT, self.onUriChanged)
        self.itemSrc.Disable()
       
        self.selectDoc = wx.Button(controlPanel, -1, 'ѡ��doc�ļ�', (280, 100))
        self.selectDoc.Bind(wx.EVT_BUTTON, self.onSelectFile)
        
        self.addNode = wx.Button(controlPanel, -1, '���ӽ��', (10, 150))
        self.addNode.Bind(wx.EVT_BUTTON, self.onInsert)
        self.removeNode = wx.Button(controlPanel, -1, 'ɾ�����', (90, 150))
        self.removeNode.Bind(wx.EVT_BUTTON, self.onDelete)
        self.save = wx.Button(controlPanel, -1, '�����޸�', (170, 150))
        self.save.Bind(wx.EVT_BUTTON, self.onSave)

        self.moveUp = wx.Button(controlPanel, -1, '�����ƶ�', (10, 180))
        self.moveUp.Bind(wx.EVT_BUTTON, self.onMoveUp)
        self.moveDown = wx.Button(controlPanel, -1, '�����ƶ�', (90, 180))
        self.moveDown.Bind(wx.EVT_BUTTON, self.onMoveDown)
        self.publishNode = wx.Button(controlPanel, -1, '�����ڵ�', (170, 180))
        self.publishNode.Bind(wx.EVT_BUTTON, self.onPublishDocument)

        wx.Button(controlPanel, -1, 'ˢ��Ŀ¼��', (10, 210)).Bind(wx.EVT_BUTTON, self.onRefreshTree)
        wx.Button(controlPanel, -1, '��������', (170, 210)).Bind(wx.EVT_BUTTON, self.onPublishAll)
        
        # �ײ��ؼ�
        self.memo = wx.TextCtrl(bodyPanel, size=(100, 100), style=wx.TE_MULTILINE)
        bodybox.Add(self.memo, 0, wx.EXPAND|wx.ALL, 3)

        # �ܽ���
        bodyPanel.SetSizer(bodybox)
        
        # ��ʼ�����ؼ�
        root = self.tree.AddRoot('��Ŀ¼')
        data = wx.TreeItemData()
        data.SetData({'uri':'index.htm','src':'_no_needs_'})
        self.tree.SetItemData(root, data)
        dirfile = open(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'directory.yaml'))
        info = yaml.load(dirfile.read().decode('gbk'))
        self.insertTree(self.tree.GetRootItem(), info['root'])
        self.oldColour = self.tree.GetItemBackgroundColour(root)
        BackupTree(self.tree)
        
        self.tree.Expand(self.tree.GetRootItem())
        self.tree.Bind(wx.EVT_TREE_SEL_CHANGED, self.treeSelChanged)
        self.tree.Bind(wx.EVT_TREE_SEL_CHANGING, self.treeSelChanging)

        self.srcDir = open(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'inputpath.txt')).read().rstrip()
        self.srcFileEdt.SetValue(self.srcDir)
        self.srcFileEdt.Disable()
        self.disable()

    def log(self, str):
        self.memo.AppendText(str)
        self.memo.AppendText('\n')

    def disable(self):
        self.itemDesc.Disable()
        self.itemUri.Disable()
        self.selectDoc.Disable()
        self.addNode.Disable()
        self.removeNode.Disable()
        self.save.Disable()
        self.moveUp.Disable()
        self.moveDown.Disable()
        self.publishNode.Disable()

    def enable(self):
        self.itemDesc.Enable()
        self.itemUri.Enable()
        self.selectDoc.Enable()
        self.addNode.Enable()
        self.removeNode.Enable()
        #self.save.Enable()
        self.moveUp.Enable()
        self.moveDown.Enable()
        #self.publishNode.Enable()
    
    def onSelectFile(self, event):
        #temp = self.srcDir
        #print self.srcDir
        filename = wx.FileSelector('��ѡ��һ����Ҫ�������ĵ�', self.srcDir)
        
        if filename == '':
            if self.itemSrc.GetValue() == '_no_needs_':
                return
            if wx.MessageBox('�Ƿ����õ�ǰ�ڵ�Ϊ���ٷ�����', style=wx.YES_NO) == wx.NO:
                return
            #self.itemUri.SetValue('newpage.htm')
            self.itemSrc.SetValue('_no_needs_')
            self.publishNode.Disable()
            return

        if not filename.endswith('.doc'):
            self.log('Ŀǰ��֧��word(.doc)�ļ�')
            wx.MessageBox('Ŀǰ��֧��word(.doc)�ļ�')
            return

        if self.srcDir == '':
            self.srcDir = os.path.dirname(os.path.realpath(__file__))
            return

        if not filename.startswith(self.srcDir):
            self.log('����ѡ��'+self.srcDir+'Ŀ¼�µ�һ���ļ�')
            wx.MessageBox('����ѡ��'+self.srcDir+'Ŀ¼�µ�һ���ļ�')
            return

        filename = filename.replace(self.srcDir, "", 1)
        filename = filename[1:]
        self.publishNode.Enable()
        self.itemSrc.SetValue(filename)

    def  onSelectDir(self, event):
        dirname = wx.DirSelector('��ѡ��TSF4G����Ŀ¼')
        if dirname != '':
            self.srcFileEdt.SetValue(dirname)
            if self.srcDir != 'dirname':
                self.save.Enable()
                self.srcDir = dirname
                self.log('�ı�TSF4G����Ŀ¼Ϊ: ' + self.srcDir)
        
    def onItemDescChanged(self, event):
        node = self.getCurrentSelection()
        if node:
            self.tree.SetItemText(node, self.itemDesc.GetValue())
    
    def onUriChanged(self, event):
        node = self.getCurrentSelection()
        if node:
            data = self.tree.GetItemData(node).GetData()
            data['uri'] = self.itemUri.GetValue()
            data['src'] = self.itemSrc.GetValue()
        
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
            self.log('��ѡ��һ���Ϸ��Ľڵ�')
            wx.MessageBox('��ѡ��һ���Ϸ��Ľڵ�')
            return
            parentNode = self.tree.GetItemParent(node)
            self.tree.Delete(node)
            if not self.tree.ItemHasChildren(parentNode):
                self.itemUri.SetValue('newpage.htm')
                self.tree.GetItemData(parentNode).GetData()['uri'] = 'newpage.htm'
    
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

    def onRefreshTree(self, event):
        root = self.tree.GetRootItem();
        if root == None:
            return
        else:
            self._onRefreshTree(root)

    def _onRefreshTree(self, curnode):
        if self.tree.ItemHasChildren(curnode):
            child, cookie = self.tree.GetFirstChild(curnode)
            while child.IsOk():
                iRet = self._onRefreshTree(child)
                child, cookie = self.tree.GetNextChild(curnode, cookie)
        self.tree.SetItemTextColour(curnode, self.oldColour)
        
    def onPublishAll(self, event):
        self.log('��ʼ����')
        modifieds = FindModifiers(self.tree)
        if len(modifieds) > 0:
                if self.saveDirectory() < 0:
                    return
        self.onRefreshTree(None)
        root = self.tree.GetRootItem();
        if not os.path.isdir(self.srcDir):
            self.log('TSF4G����Ŀ¼:' + self.srcDir + '������! ��ѡ����ȷ��Ŀ¼.')
            wx.MessageBox('TSF4G����Ŀ¼:' + self.srcDir + '������! ��ѡ����ȷ��Ŀ¼.')
            return
        if root == None:
            return
        else:
            iRet = self._onPublishAll(root)
            if iRet <0:
                return
        self.log('"��������"�ɹ�')
        wx.MessageBox('"��������"�ɹ�')

    def _onPublishAll(self, curnode):
        # set colour
        self.tree.SelectItem(curnode)
        self.treeSelChanged(None)
        self.tree.SetItemTextColour(curnode, wx.Colour(0, 0, 255))

        if self.tree.ItemHasChildren(curnode):
            child, cookie = self.tree.GetFirstChild(curnode)
            while child.IsOk():
                iRet = self._onPublishAll(child)
                if iRet < 0:
                    return iRet
                child, cookie = self.tree.GetNextChild(curnode, cookie)
        else:
            ralativeDocPath = self.tree.GetItemData(curnode).GetData()['src']
            if ralativeDocPath == '_no_needs_':
                return 0
            uri = (self.srcDir + '\\' + ralativeDocPath).lower()
            if os.path.splitext(uri)[1] != '.doc':
                self.tree.SetItemTextColour(curnode, wx.Colour(255, 0, 0))
                self.log('ʧ��: ��֧�ֵ��ĵ���ʽ'+ uri + '(����ֻ֧��.doc)')
                wx.MessageBox('ʧ��: ��֧�ֵ��ĵ���ʽ'+ uri + '(����ֻ֧��.doc)')
                return -1
            if not os.path.isfile(uri):
                self.tree.SetItemTextColour(curnode, wx.Colour(255, 0, 0))
                self.log('ʧ�ܣ��Ҳ���ָ�����ļ�: ' + uri)
                wx.MessageBox('ʧ�ܣ��Ҳ���ָ�����ļ�: ' + uri)
                return -1
            outpath = open(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'outputpath.txt')).read()
            ralativeDestPath = self.tree.GetItemData(curnode).GetData()['uri']
            dest = self.getAbsoPath(os.path.join(outpath, ralativeDestPath))
            self.log('���ڷ���: ' + uri + ' -> ' + dest + '...')
            Word2Html(uri, dest, self.getDirPath(curnode))

        return 0


    def onPublishDocument(self, event):
        self.onRefreshTree(None)
        node = self.getCurrentSelection()
        if node:
            if self.tree.ItemHasChildren(node):
                self.log('ֻ�����Ҷ�ӽڵ���з���')
                wx.MessageBox('ֻ�����Ҷ�ӽڵ���з���')
                return
        else:
            self.log('����ѡ��һ���ڵ��ٽ��з���')
            wx.MessageBox('����ѡ��һ���ڵ��ٽ��з���')
            return

        if self.saveDirectory() < 0:
            return
        uri = (self.srcDir + '\\' + self.itemSrc.GetValue()).lower()
        if os.path.splitext(uri)[1] != '.doc':
            self.tree.SetItemTextColour(node, wx.Colour(255, 0, 0))
            self.log('ʧ��: ��֧�ֵ��ĵ���ʽ'+ uri + '(����ֻ֧��.doc)')
            wx.MessageBox('ʧ��: ��֧�ֵ��ĵ���ʽ'+ uri + '(����ֻ֧��.doc)')
            return
        if not os.path.isfile(uri):
            self.tree.SetItemTextColour(node, wx.Colour(255, 0, 0))
            self.log('ʧ�ܣ��Ҳ���ָ�����ļ�: ' + uri)
            wx.MessageBox('ʧ�ܣ��Ҳ���ָ�����ļ�: ' + uri)
            return
        if self.tree.GetItemData(node).GetData()['uri'] == 'newpage.htm':
            self.log('ʧ��: ��Ϊ����Ŀ���ҳ���ļ���·������Ϊ"newpage.htm"')
            wx.MessageBox('ʧ��: ��Ϊ����Ŀ���ҳ���ļ���·������Ϊ"newpage.htm"')
            return
        outpath = open(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'outputpath.txt')).read()
        dest = self.getAbsoPath(os.path.join(outpath, self.itemUri.GetValue()))
        self.log('���ڷ���:')
        self.log(uri + ' -> ' + dest + '...')
        Word2Html(uri, dest, self.getDirPath(node))
        self.log('"�����ڵ�"�ɹ�')
        
    def setNodeData(self, desc, uri, src):
        self.itemDesc.SetValue(desc)
        self.itemUri.SetValue(uri)
        self.itemSrc.SetValue(src)
    
    def treeSelChanging(self, event):
        if not self.updateNodeData():
            event.Veto()
            
    def treeSelChanged(self, event):
        self.enable()
        self.save.Enable()
        selnode = self.tree.GetSelection()
        if not selnode.IsOk() : return
        attributes = self.tree.GetItemData(selnode).GetData()
        if not self.tree.ItemHasChildren(selnode):
            self.selectDoc.Enable()
            self.setNodeData(self.tree.GetItemText(selnode), attributes['uri'], attributes['src'])
            if attributes['src'] != '_no_needs_':
                self.publishNode.Enable()
            else:
                self.publishNode.Disable()
        else:
            self.selectDoc.Disable()
            self.publishNode.Disable()
            self.setNodeData(self.tree.GetItemText(selnode), attributes['uri'], '_no_needs_')

    def insertTree(self, parentNode, infos):
        for i in infos:
            data = wx.TreeItemData()
            data.SetData({'uri':i['uri'], 'src':i['src']})
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
            attributes['src'] = self.itemSrc.GetValue()
            self.tree.SetItemText(selnode, self.itemDesc.GetValue())
            return True
        else:
            self.log('��������ȷ��.htm�ļ���', '��ʾ')
            return False

    def preSaveDirectory(self):
        root = self.tree.GetRootItem();
        if root == None:
            return 0
        else:
            return self._preSaveDirectory(root)

    def _preSaveDirectory(self, curnode):
        if self.tree.ItemHasChildren(curnode):
            if self.tree.GetItemData(curnode).GetData()['uri'] == 'newpage.htm':
                self.tree.SelectItem(curnode)
                self.treeSelChanged(None)
                self.tree.SetItemTextColour(curnode, wx.Colour(255, 0, 0))
                self.itemUri.SetFocus()
                self.itemUri.SetSelection(0, -1)
                self.log('ʧ�ܣ���Ҷ�ӽڵ�ҳ���ļ�·������Ϊ"newpage.htm"')
                wx.MessageBox('ʧ�ܣ���Ҷ�ӽڵ�ҳ���ļ�·������Ϊ"newpage.htm"')
                return -1
            child, cookie = self.tree.GetFirstChild(curnode)
            while child.IsOk():
                iRet = self._preSaveDirectory(child)
                if iRet < 0:
                    return iRet;
                child, cookie = self.tree.GetNextChild(curnode, cookie)
        else:
            itemdata = self.tree.GetItemData(curnode).GetData()
            if itemdata['uri'] == 'newpage.htm' and itemdata['src'] != '_no_needs_':
                self.tree.SelectItem(curnode)
                self.treeSelChanged(None)
                self.tree.SetItemTextColour(curnode, wx.Colour(255, 0, 0))
                self.itemUri.SetFocus()
                self.itemUri.SetSelection(0, -1)
                wx.MessageBox('ʧ��: ��Ϊ����Ŀ���ҳ���ļ���·������Ϊ"newpage.htm"')
                self.log('ʧ��: ��Ϊ����Ŀ���ҳ���ļ���·������Ϊ"newpage.htm"')
                return -1
        return 0;

    def saveDirectory(self):
        iRet = self.preSaveDirectory()
        if iRet < 0:
            return iRet
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
            #print modifieds
            BackupTree(self.tree)
            #build.buildHtmls(modifieds)
            build.buildHtmls()
        return 0;
        
    def onSave(self, event):
        input = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'inputpath.txt')
        inputfile = open(input, 'w')
        if inputfile == None:
            self.log('unable to open file: ' + input)
            wx.MessageBox('unable to open file: ' + input)
            return
        #print self.srcFileEdt.GetValue()
        print >> inputfile, self.srcFileEdt.GetValue()
        selnode = self.tree.GetSelection()
        if selnode != None:
            if self.saveDirectory() < 0:
                return
        self.log('����ɹ�')
        wx.MessageBox('����ɹ�')
        self.save.Disable()
    
    def onInsert(self, event):
        node = self.tree.GetSelection()
        if node.IsOk():
            data = wx.TreeItemData()
            data.SetData({'uri':'newpage.htm','src':'_no_needs_'})
            newnode = self.tree.AppendItem(node, '�½���ҳ', data=data)
            self.tree.EnsureVisible(newnode)
            self.tree.SelectItem(newnode)
            self.itemDesc.SetFocus()
            self.itemDesc.SetSelection(0, -1)
            #self.publishNode.Enable()
        
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
        print >> outfile, preblanks + 'src: ' + self.tree.GetItemData(node).GetData()['src']
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
        self.log('��վ���ڱ� "%s" �༭�����Ժ��ٳ���' % user)
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



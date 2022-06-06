#coding=gbk

import os
import wx
import re
import sys
import yaml
import build
from webgen.doc2html import *
from webgen.tree import *

from optparse import OptionParser

re_uri = re.compile(r'[\w_\-]+\.htm\Z')

TREESTYLE = wx.TR_LINES_AT_ROOT|wx.TR_HAS_BUTTONS

class MainFrame(wx.Frame):

    def __init__(self):
        self._outpath=os.path.abspath('\\\\10.1.252.49\\TSF4GOnLine')
        wx.Frame.__init__(self, None, -1, 'TSF4G 网页管理系统', None, (800, 600))
        self.Bind(wx.EVT_CLOSE, self.onClose)

        bodyPanel = wx.Panel(self)

        bodybox = wx.BoxSizer(wx.VERTICAL)
        self.icon = wx.Icon('mark.bmp', wx.BITMAP_TYPE_ICO)
        self.SetIcon(self.icon)


        # 顶部控件
        topPanel = wx.Panel(bodyPanel)
        topbox = wx.BoxSizer(wx.HORIZONTAL)
        topPanel.SetSizer(topbox)
        text1 = wx.StaticText(topPanel, label=r'TSF4G所在目录：', style=wx.ALIGN_CENTER)
        self.srcDir=""
        self.srcFileEdt = wx.TextCtrl(topPanel)
        #button1 = wx.Button(topPanel, label=r'选择目录')
        #button1.Bind(wx.EVT_BUTTON, self.onSelectDir)
        topbox.Add(text1, 0, wx.TOP|wx.LEFT, 7)
        topbox.Add(self.srcFileEdt, 1, wx.EXPAND|wx.ALL, 3)
        #topbox.Add(button1, 0, wx.ALL, 3)
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

        self.itemSrcCaption = wx.StaticText(controlPanel, -1, '源文件：', (10, 100))
        self.itemSrc = wx.TextCtrl(controlPanel, pos=(70, 98), size=(300, 20))
        self.itemSrc.Bind(wx.EVT_TEXT, self.onUriChanged)
        self.itemSrc.Disable()

        self.selectDoc = wx.Button(controlPanel, -1, '选择doc文件', (280, 130))
        self.selectDoc.Bind(wx.EVT_BUTTON, self.onSelectFile)

        self.selectHtml = wx.Button(controlPanel, -1, '选择html文件', (280, 70))
        self.selectHtml.Bind(wx.EVT_BUTTON, self.onSelectHtmlFile)

        #模块名编辑选项
        #用来编辑当前模块的模块名
        self.itemModuleNameCaption = wx.StaticText(controlPanel, -1, '所属模块：', (10, 160))
        self.itemModuleName = wx.TextCtrl(controlPanel, pos=(70,158), size=(300, 20))
        self.itemModuleName.Bind(wx.EVT_TEXT, self.onUriChanged)

        self.addNode = wx.Button(controlPanel, -1, '增加结点', (10, 180))
        self.addNode.Bind(wx.EVT_BUTTON, self.onInsert)
        self.removeNode = wx.Button(controlPanel, -1, '删除结点', (90, 180))
        self.removeNode.Bind(wx.EVT_BUTTON, self.onDelete)
        self.save = wx.Button(controlPanel, -1, '保存修改', (170, 180))
        self.save.Bind(wx.EVT_BUTTON, self.onSave)

        self.btPublishModule = wx.Button(controlPanel, -1, '发布模块', (250, 180))
        self.btPublishModule.Bind(wx.EVT_BUTTON, self.onPublishModule)

        self.moveUp = wx.Button(controlPanel, -1, '向上移动', (10, 210))
        self.moveUp.Bind(wx.EVT_BUTTON, self.onMoveUp)
        self.moveDown = wx.Button(controlPanel, -1, '向下移动', (90, 210))
        self.moveDown.Bind(wx.EVT_BUTTON, self.onMoveDown)
        self.publishNode = wx.Button(controlPanel, -1, '发布节点', (170, 210))
        self.publishNode.Bind(wx.EVT_BUTTON, self.onPublishDocument)




        wx.Button(controlPanel, -1, '刷新目录树', (10, 240)).Bind(wx.EVT_BUTTON, self.onRefreshTree)
        self.publishAllNodes = wx.Button(controlPanel, -1, '发布所有', (170, 240))
        self.publishAllNodes.Bind(wx.EVT_BUTTON, self.onPublishAll)
        self.publishAllNodes.Disable()

        # 底部控件
        self.memo = wx.TextCtrl(bodyPanel, size=(100, 100), style=wx.TE_MULTILINE)
        bodybox.Add(self.memo, 0, wx.EXPAND|wx.ALL, 3)

        # 总界面
        bodyPanel.SetSizer(bodybox)

        # 初始化树控件
        root = self.tree.AddRoot('根目录')
        data = wx.TreeItemData()
        data.SetData({'uri':'index.htm','src':'_no_needs_','moduleName':'None'})
        self.tree.SetItemData(root, data)
        dirfile = open(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'directory.yaml'))
        info = yaml.load(dirfile.read().decode('gbk'))
        self.insertTree(self.tree.GetRootItem(), info['root'])
        self.oldColour = self.tree.GetItemBackgroundColour(root)
        BackupTree(self.tree)

        self.tree.Expand(self.tree.GetRootItem())
        self.tree.Bind(wx.EVT_TREE_SEL_CHANGED, self.treeSelChanged)
        self.tree.Bind(wx.EVT_TREE_SEL_CHANGING, self.treeSelChanging)

        self.srcDir =os.path.abspath('../../../') #open(os.path.join(os.path.dirname(os.path.realpath(__file__)), 'inputpath.txt')).read().rstrip()
        self.srcFileEdt.SetValue(self.srcDir)
        self.srcFileEdt.Disable()
        self.disable()

        self.destPath = os.path.abspath('../../index_html')


        parser = OptionParser()
        parser.add_option("-p","--publish",dest="publish",default='0',help="whether to publish or not default 0",metavar="Int")

        parser.add_option("-m","--module",dest="module",default='base',help="which module to publish",metavar="string")

        (res,args) = parser.parse_args()
        print res
        self.options = res

        if res.publish == '1':
            print "Publish all"
            root = self.tree.GetRootItem();
            self._publishModules(root,res.module)

    def log(self, str, ret=True):
        self.memo.AppendText(str)
        if ret:
            self.memo.AppendText('\n')

    def disable(self):
        self.itemDesc.Disable()
        self.itemUri.Disable()
        self.itemModuleName.Disable()
        self.selectDoc.Disable()
        self.selectHtml.Disable()
        self.addNode.Disable()
        self.removeNode.Disable()
        self.save.Disable()
        self.moveUp.Disable()
        self.moveDown.Disable()
        self.publishNode.Disable()

    def enable(self):
        self.itemDesc.Enable()
        self.itemUri.Enable()
        self.itemModuleName.Enable()
        self.selectDoc.Enable()
        self.selectHtml.Enable()
        self.addNode.Enable()
        self.removeNode.Enable()
        #self.save.Enable()
        self.moveUp.Enable()
        self.moveDown.Enable()
        #self.publishNode.Enable()

    def onSelectFile(self, event):
        #temp = self.srcDir
        #print self.srcDir
        filename = wx.FileSelector('请选择一个需要发布的文档', self.srcDir)

        if filename == '':
            if self.itemSrc.GetValue() == '_no_needs_':
                return
            if wx.MessageBox('是否设置当前节点为不再发布？', style=wx.YES_NO) == wx.NO:
                return
            #self.itemUri.SetValue('newpage.htm')
            self.itemSrc.SetValue('_no_needs_')
            self.publishNode.Disable()
            self.save.Enable()
            return

        if not filename.endswith('.doc'):
            self.log('目前仅支持word(.doc)文件')
            wx.MessageBox('目前仅支持word(.doc)文件')
            return

        if self.srcDir == '':
            self.srcDir = os.path.dirname(os.path.realpath(__file__))
            return

        if not filename.startswith(self.srcDir):
            self.log('必须选择'+self.srcDir+'目录下的一个文件')
            wx.MessageBox('必须选择'+self.srcDir+'目录下的一个文件')
            return

        filename = filename.replace(self.srcDir, "", 1)
        filename = filename[1:]
        self.publishNode.Enable()
        self.save.Enable()
        self.itemSrc.SetValue(filename)

    def onSelectHtmlFile(self, event):
        #temp = self.srcDir
        #print self.srcDir
        filename = wx.FileSelector('请选择发布文档的位置', self.destPath)

        if filename == '':
            if self.itemUri.GetValue() == 'newpage.htm':
                return
            if wx.MessageBox('是否设置当前节点为不再发布？', style=wx.YES_NO) == wx.NO:
                return
            #self.itemUri.SetValue('newpage.htm')
            self.itemUri.SetValue('_no_needs_')
            self.publishNode.Disable()
            self.save.Enable()
            return


        if not filename.endswith('.html') and not filename.endswith('.htm'):
            self.log('目前仅支持html(.htm)文件')
            wx.MessageBox('目前仅支持html(.htm)文件')
            return

        if self.destPath == '':
            self.destPath = os.path.dirname(os.path.realpath(__file__))
            return

        if not filename.startswith(self.destPath):
            self.log('必须选择'+self.destPath+'目录下的一个文件')
            wx.MessageBox('必须选择'+self.destPath+'目录下的一个文件')
            return

        filename = filename.replace(self.destPath, "", 1)
        filename = filename[1:]
        self.publishNode.Enable()
        self.save.Enable()
        self.itemUri.SetValue(filename)

    def  onSelectDir(self, event):
        dirname = wx.DirSelector('请选择TSF4G所在目录')
        if dirname != '':
            self.srcFileEdt.SetValue(dirname)
            if self.srcDir != 'dirname':
                self.save.Enable()
                self.srcDir = dirname
                self.log('改变TSF4G所在目录为: ' + self.srcDir)

    def onItemDescChanged(self, event):
        node = self.getCurrentSelection()
        if node:
            self.tree.SetItemText(node, self.itemDesc.GetValue())

    def onUriChanged(self, event):
        node = self.getCurrentSelection()

        if(self.itemModuleName.GetValue() == ""):
            self.itemModuleName.SetValue("None")

        if node:
            data = self.tree.GetItemData(node).GetData()
            data['uri'] = self.itemUri.GetValue()
            data['src'] = self.itemSrc.GetValue()
            data['moduleName']=self.itemModuleName.GetValue()

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
            self.log('请选择一个合法的节点')
            wx.MessageBox('请选择一个合法的节点')
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
        result.append([u'主页', u'index.htm'])
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

    def onPublishModule(self, event):
        print "publishModule"
        moduleName = self.itemModuleName.GetValue()
        if 'None' == moduleName or ''==moduleName:
            moduleName = 'base'
        root = self.tree.GetRootItem();
        self._publishModules(root,moduleName)

    def onPublishAll(self, event):
        self.log('开始发布')
        modifieds = FindModifiers(self.tree)
        if len(modifieds) > 0:
            if self.saveDirectory() < 0:
                return
        self.onRefreshTree(None)
        root = self.tree.GetRootItem();
        if not os.path.isdir(self.srcDir):
            self.log('TSF4G所在目录:' + self.srcDir + '不存在! 请选择正确的目录.')
            wx.MessageBox('TSF4G所在目录:' + self.srcDir + '不存在! 请选择正确的目录.')
            return
        if root == None:
            return
        else:
            iRet = self._onPublishAll(root)
            if iRet <0:
                return
        self.log('"发布所有"成功')
        wx.MessageBox('"发布所有"成功')

    def _publishModules(self,curnode,ModuleName):
        # set colour
        self.tree.SelectItem(curnode)
        self.treeSelChanged(None)
        self.tree.SetItemTextColour(curnode, wx.Colour(128, 128, 0))

        if self.tree.ItemHasChildren(curnode):
            child, cookie = self.tree.GetFirstChild(curnode)
            while child.IsOk():
                iRet = self._publishModules(child,ModuleName)
                if iRet < 0:
                    return iRet
                child, cookie = self.tree.GetNextChild(curnode, cookie)
        else:
            ralativeDocPath = self.tree.GetItemData(curnode).GetData()['src']
            curModName = self.tree.GetItemData(curnode).GetData()['moduleName']
            #print "ModuleName %s" %curModName
            if curModName != ModuleName:
                #print "Ignor %s" % curModName
                return 0
            if ralativeDocPath == '_no_needs_':
                return 0
            uri = (self.srcDir + '\\' + ralativeDocPath).lower()
            if os.path.splitext(uri)[1] != '.doc':
                self.tree.SetItemTextColour(curnode, wx.Colour(255, 0, 0))
                self.log('失败: 不支持的文档格式'+ uri + '(现在只支持.doc)')
                wx.MessageBox('失败: 不支持的文档格式'+ uri + '(现在只支持.doc)')
                return -1
            if not os.path.isfile(uri):
                self.tree.SetItemTextColour(curnode, wx.Colour(255, 0, 0))
                self.log('失败：找不到指定的文件: ' + uri)
                wx.MessageBox('失败：找不到指定的文件: ' + uri)
                return -1
            outpath = self._outpath
            ralativeDestPath = self.tree.GetItemData(curnode).GetData()['uri']
            dest = self.getAbsoPath(os.path.join(outpath, ralativeDestPath))
            self.log('正在发布: ' + uri + ' -> ' + dest + '...', False)
            Word2Html(uri, dest, self.getDirPath(curnode))
            self.log('   成功!')

        return 0

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
                self.log('失败: 不支持的文档格式'+ uri + '(现在只支持.doc)')
                wx.MessageBox('失败: 不支持的文档格式'+ uri + '(现在只支持.doc)')
                return -1
            if not os.path.isfile(uri):
                self.tree.SetItemTextColour(curnode, wx.Colour(255, 0, 0))
                self.log('失败：找不到指定的文件: ' + uri)
                wx.MessageBox('失败：找不到指定的文件: ' + uri)
                return -1
            outpath = self._outpath
            ralativeDestPath = self.tree.GetItemData(curnode).GetData()['uri']
            dest = self.getAbsoPath(os.path.join(outpath, ralativeDestPath))
            self.log('正在发布: ' + uri + ' -> ' + dest + '...', False)
            Word2Html(uri, dest, self.getDirPath(curnode))
            self.log('   成功!')

        return 0


    def onPublishDocument(self, event):
        self.onRefreshTree(None)
        node = self.getCurrentSelection()
        if node:
            if self.tree.ItemHasChildren(node):
                self.log('只能针对叶子节点进行发布')
                wx.MessageBox('只能针对叶子节点进行发布')
                return
        else:
            self.log('请先选择一个节点再进行发布')
            wx.MessageBox('请先选择一个节点再进行发布')
            return

        if self.saveDirectory() < 0:
            return
        uri = (self.srcDir + '\\' + self.itemSrc.GetValue()).lower()
        if os.path.splitext(uri)[1] != '.doc':
            self.tree.SetItemTextColour(node, wx.Colour(255, 0, 0))
            self.log('失败: 不支持的文档格式'+ uri + '(现在只支持.doc)')
            wx.MessageBox('失败: 不支持的文档格式'+ uri + '(现在只支持.doc)')
            return
        if not os.path.isfile(uri):
            self.tree.SetItemTextColour(node, wx.Colour(255, 0, 0))
            self.log('失败：找不到指定的文件: ' + uri)
            wx.MessageBox('失败：找不到指定的文件: ' + uri)
            return
        if self.tree.GetItemData(node).GetData()['uri'] == 'newpage.htm':
            self.log('失败: 作为发布目标的页面文件的路径不能为"newpage.htm"')
            wx.MessageBox('失败: 作为发布目标的页面文件的路径不能为"newpage.htm"')
            return


        outpath = self._outpath
        dest = self.getAbsoPath(os.path.join(outpath, self.itemUri.GetValue()))
        self.log('正在发布: ' + uri + ' -> ' + dest + '...', False)
        Word2Html(uri, dest, self.getDirPath(node))
        self.log('   成功!')

    def setNodeData(self, desc, uri, src,moduleName):
        self.itemDesc.SetValue(desc)
        self.itemUri.SetValue(uri)
        self.itemSrc.SetValue(src)
        self.itemModuleName.SetValue(moduleName)

    def treeSelChanging(self, event):
        if not self.updateNodeData():
            event.Veto()

    def treeSelChanged(self, event):
        self.enable()
        self.save.Enable()
        selnode = self.tree.GetSelection()
        if not selnode.IsOk() : return
        attributes = self.tree.GetItemData(selnode).GetData()
        if attributes.has_key('moduleName'):
            moduleName = attributes['moduleName']
        else:
            moduleName = 'None'

        if not self.tree.ItemHasChildren(selnode):
            self.selectDoc.Enable()
            self.selectHtml.Enable()
            self.setNodeData(self.tree.GetItemText(selnode), attributes['uri'], attributes['src'],moduleName)
            if attributes['src'] != '_no_needs_':
                self.publishNode.Enable()
            else:
                self.publishNode.Disable()
        else:
            self.selectDoc.Disable()
            self.selectHtml.Disable()
            self.publishNode.Disable()
            self.setNodeData(self.tree.GetItemText(selnode), attributes['uri'], '_no_needs_',moduleName)

    def insertTree(self, parentNode, infos):
        for i in infos:
            data = wx.TreeItemData()
            data.SetData({'uri':i['uri'], 'src':i['src'],'moduleName':i['moduleName']})
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
            self.log('请输入正确的.htm文件名', '提示')
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
                self.log('失败：非叶子节点页面文件路径不能为"newpage.htm"')
                wx.MessageBox('失败：非叶子节点页面文件路径不能为"newpage.htm"')
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
                wx.MessageBox('失败: 作为发布目标的页面文件的路径不能为"newpage.htm"')
                self.log('失败: 作为发布目标的页面文件的路径不能为"newpage.htm"')
                return -1
        return 0;

    def saveDirectory(self):
        iRet = self.preSaveDirectory()
        selNode = self.tree.GetSelection()
        if iRet < 0:
            return iRet
        if self.updateNodeData():
            try:
                yaml = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'directory.yaml')
                temp = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'temp.yaml')
                print yaml
                outfile = open(temp, 'w')
                print >> outfile, 'root:'
                root = self.tree.GetRootItem()
                self.outputNode(root, outfile, '    ')
                outfile.close()
                os.remove(yaml)
                os.rename(temp, yaml)
                modifieds = FindModifiers(self.tree)
                #print modifieds
                BackupTree(self.tree)
                #build.buildHtmls(modifieds)
                build.buildHtmls()
            except UnicodeEncodeError:
                wx.MessageBox('因编码问题, 保存修改失败, 请重试')
                return -1
        self.tree.CollapseAll()
        self.tree.SelectItem(selNode)
        return 0;

    def onSave(self, event):
        input = os.path.abspath('../../../')#os.path.join(os.path.dirname(os.path.realpath(__file__)), 'inputpath.txt')
        #inputfile = open(input, 'w')
        #if inputfile == None:
        #    self.log('unable to open file: ' + input)
        #    wx.MessageBox('unable to open file: ' + input)
        #    return
        #print self.srcFileEdt.GetValue()
        #print >> inputfile, self.srcFileEdt.GetValue()
        selnode = self.tree.GetSelection()
        if selnode != None:
            if self.saveDirectory() < 0:
                return
        self.log('保存成功')
        wx.MessageBox('保存成功')
        #self.save.Disable()

    def onInsert(self, event):
        node = self.tree.GetSelection()
        if node.IsOk():
            data = wx.TreeItemData()
            data.SetData({'uri':'newpage.htm','src':'_no_needs_'})
            newnode = self.tree.AppendItem(node, '新建网页', data=data)
            self.tree.EnsureVisible(newnode)
            self.tree.SelectItem(newnode)
            self.itemDesc.SetFocus()
            self.itemDesc.SetSelection(0, -1)
            #self.publishNode.Enable()

    def outputNode(self, node, outfile, preblanks):
        self.tree.SelectItem(node)
        child, cookie = self.tree.GetFirstChild(node)
        while(child.IsOk()):
            self.tree.SelectItem(child)
            print >> outfile, preblanks + '-'
            self.outputNodeDetail(child, outfile, preblanks + '    ')
            self.outputNode(child, outfile, preblanks+'    ')
            child, cookie = self.tree.GetNextChild(node, cookie)

    def outputNodeDetail(self, node, outfile, preblanks):
        print "Out puting modified node"
        print >> outfile, preblanks + 'desc: ' + self.tree.GetItemText(node)
        print >> outfile, preblanks + 'uri: ' + self.tree.GetItemData(node).GetData()['uri']
        print >> outfile, preblanks + 'src: ' + self.tree.GetItemData(node).GetData()['src']
        print >> outfile, preblanks + 'moduleName: '+self.tree.GetItemData(node).GetData()['moduleName']
        child, cookie = self.tree.GetFirstChild(node)
        if child.IsOk():
            print >> outfile, preblanks + 'children:'

    def onClose(self, event):
        modifieds =list()#= FindModifiers(self.tree)
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
    #chkfilepath = os.path.join(os.path.dirname(os.path.realpath(__file__)), 'checkout')
    try:
        #chkfile = open(chkfilepath, 'w')
        #import socket
        #chkfile.write(socket.gethostname())
        #chkfile.close();
        frame = MainFrame()
        if len(sys.argv) > 1:
            frame.srcFileEdt.SetValue(sys.argv[1])
        frame.Show()
        app.MainLoop()
    finally:
        #os.remove(chkfilepath)
        print "Done"



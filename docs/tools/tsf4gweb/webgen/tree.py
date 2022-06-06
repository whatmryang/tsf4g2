#coding=gbk

#-----------------------------------------------------------------------

def _GetNameAndUri(tree, node):
    name = tree.GetItemText(node)
    uri = tree.GetItemData(node).GetData()['uri']
    return [name, uri]

def _BackupNode(tree, node):
    attributes = _GetNameAndUri(tree, node)
    children = []
    child, cookie = tree.GetFirstChild(node)
    while child.IsOk():
        children.append(_GetNameAndUri(tree, child))
        _BackupNode(tree, child)
        child, cookie = tree.GetNextChild(node, cookie)
    attributes.append(children)
    tree.GetItemData(node).GetData()['backup'] = attributes
            
def BackupTree(tree):
    '���ݵ�ǰ���ݣ��Ա��ڼ���Ƿ����˸ı�'
    root = tree.GetRootItem()
    _BackupNode(tree, root)

#-----------------------------------------------------------------------

def _PickModifiedNode(tree, node, collection, parentmodified):
    attributes = _GetNameAndUri(tree, node)
    orgattributes = tree.GetItemData(node).GetData().setdefault('backup', None)
    thismodified = False
    if not orgattributes:
        thismodified = True
    elif attributes[0] != orgattributes[0] or attributes[1] != orgattributes[1]:
        thismodified = True
    uri = attributes[1]
    children = []
    child, cookie = tree.GetFirstChild(node)
    while child.IsOk():
        children.append(_GetNameAndUri(tree, child))
        _PickModifiedNode(tree, child, collection, thismodified or parentmodified)
        child, cookie = tree.GetNextChild(node, cookie)
    attributes.append(children)
    if attributes != orgattributes or parentmodified:
        collection.append(uri)
    
def FindModifiers(tree):
    '�ҳ�������Ҫ�����޸ĵĽڵ�'
    root = tree.GetRootItem()
    modifieds = []
    _PickModifiedNode(tree, root, modifieds, False)
    return modifieds

#-----------------------------------------------------------------------

/**
*
* @file     tdr_XMLTags_i_h.h  
* @brief    ��ȡ��ͬ�汾��Ԫ��������XML��ǩ����
* 
* @author jackyai  
* @version 1.0
* @date 2007-04-05 
*
*
* Copyright (c)  2007, ��Ѷ�Ƽ����޹�˾���������з���
* All rights reserved.
*
*/

#ifndef TDR_XMLTAGS_I_H
#define TDR_XMLTAGS_I_H

#include "tdr/tdr_XMLtags.h"
#include "tdr/tdr_define.h"

/**
*����DR XML����汾��ȡ ��Ԫ�صı�ǩ����
*/
#define TDR_GET_METALIB_ROOT_TAG(rootTag, ver)  \
    { \
        if (TDR_XML_TAGSET_VERSION_0 == ver) \
        {\
            rootTag = TDR_TAG_TYPELIB; \
        } else\
        {\
            rootTag = TDR_TAG_METALIB;\
        }\
    }

/**
*����DR XML����汾��ȡ ��Ԫ�صı�ǩ����
*/
#define TDR_GET_MACRO_VALUE_TAG(Tag, ver)  \
    { \
        if (TDR_XML_TAGSET_VERSION_0 == ver) \
        {\
            Tag = TDR_TAG_MACRO_ID; \
        } else\
        {\
            Tag = TDR_TAG_MACRO_VALUE;\
        }\
    }

/**
*����DR XML����汾��ȡ entryԪ��sizeinfo�ı�ǩ����
*/
#define TDR_GET_SIZEINFO_TAG(Tag, ver)  \
    { \
        if (TDR_XML_TAGSET_VERSION_0 == ver) \
        {\
            Tag = TDR_TAG_TARGET; \
        } else\
        {\
            Tag = TDR_TAG_SIZEINFO;\
        }\
    }

#define TDR_GET_ENTRY_TAG_NAME(Tag, ver) \
    { \
    if (TDR_XML_TAGSET_VERSION_0 == ver) \
        {\
        Tag = TDR_TAG_ITEM; \
        } else\
        {\
        Tag = TDR_TAG_ENTRY;\
        }\
    }

#endif /*TDR_XMLTAGS_I_H*/


#ifndef _OG_INI_H
#define _OG_INI_H

#include <stdio.h>
#include <stddef.h>
#include <string.h>

#if !defined(_WIN32) && !defined(_WIN64)
#include <netinet/in.h>
#endif

#include "oi_str.h"
#include "oi_cfg.h"

#define MAX_CFG_ID_LEN	32
#define MAX_CFG_SECTION_MEMBER	100

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	char sID[MAX_CFG_ID_LEN];
	int iType;
	int iOffset;
	char sInitVal[256];
	int iArrayMax;
	char cSeparator;
} TypeDesc;

typedef struct {
	char sID[MAX_CFG_ID_LEN];
	int iType;
	int iOffset;
	TypeDesc stMember[MAX_CFG_SECTION_MEMBER];
	int iArrayNumOffset;
	int iArrayMemberSize;
	int iArrayMax;
} SectionDesc;

int ReadCfg(char *sFile, void *pstCfg, TypeDesc *pstTypeDesc);
int ReadIni(char *sFile, void *pstIni, SectionDesc *pstSecDesc);
int ReadSection(FILE *f, void *pstIni, SectionDesc *pstDesc, char *sSecID, char *sNext);
int InitSection(void *pstIni, SectionDesc *pstDesc);
int InitMember(void *pstSec, TypeDesc *pstDesc);
#ifdef __cplusplus
}
#endif

#endif

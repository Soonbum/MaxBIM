#ifndef	__EXPORT__
#define __EXPORT__

#include "MaxBIM.hpp"

// 기둥의 위치, 크기 정보
struct ColumnInfo
{
	// [가로][세로][층]
	short	pos		[100][100][100];	// 기둥의 가로열, 세로열, 층열 위치
	double	horLen	[100][100][100];	// 해당 위치 기둥의 가로 길이
	double	verLen	[100][100][100];	// 해당 위치 기둥의 세로 길이
	double	height	[100][100][100];	// 해당 위치 기둥의 높이

	short	nHor;	// 가로 방향 기둥 개수
	short	nVer;	// 세로 방향 기둥 개수
	short	nHei;	// 존재하는 층 수
};

int compare (const void* first, const void* second);	// 오름차순으로 정렬할 때 사용하는 비교함수
GSErrCode	exportElementInfo (void);					// 부재(기둥,보,슬래브)들의 정보를 추출하고 정리해서 엑셀 파일로 내보내기

#endif
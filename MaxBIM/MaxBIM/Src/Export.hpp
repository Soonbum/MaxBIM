#ifndef	__EXPORT__
#define __EXPORT__

#include "MaxBIM.hpp"


// 개별 기둥의 정보
struct ColumnInfo
{
	short	floorInd;	// 층 인덱스
	double	posX;		// 중심점 X좌표
	double	posY;		// 중심점 Y좌표
	double	horLen;		// 가로 길이
	double	verLen;		// 세로 길이
	double	height;		// 높이

	short	iHor;		// 가로 순번
	short	iVer;		// 세로 순번
};

// 기둥의 위치 정보
struct ColumnPos
{
	ColumnInfo	columns [5000];		// 기둥 정보
	short	nColumns;				// 기둥 개수

	double	node_hor [100][100];	// 층별 가로 방향 기둥 좌표 (X) : dim1 - 층, dim2 - 노드 X좌표
	short	nNodes_hor [100];		// 층별 가로 방향 기둥 개수
	double	node_ver [100][100];	// 층별 세로 방향 기둥 좌표 (Y) : dim1 - 층, dim2 - 노드 Y좌표
	short	nNodes_ver [100];		// 층별 세로 방향 기둥 개수

	short	nStories;				// 층 수
	short	firstStory;				// 최하층
	short	lastStory;				// 최상층
};

void initArray (double arr [], short arrSize);			// 배열 초기화 함수
int compare (const void* first, const void* second);	// 오름차순으로 정렬할 때 사용하는 비교함수
GSErrCode	exportElementInfo (void);					// 부재(기둥,보,슬래브)들의 정보를 추출하고 정리해서 엑셀 파일로 내보내기

#endif
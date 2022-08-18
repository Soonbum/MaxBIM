/**
 * @file Contains the includes and definitions necessary for the Add-On to
 *       function.
 */

#if !defined (__MAXBIM_HPP__)
#define __MAXBIM_HPP__

#ifdef _WIN32
	#pragma warning (push, 3)
	#include	<Win32Interface.hpp>
	#pragma warning (pop)

	#ifndef WINDOWS
		#define WINDOWS
	#endif
#endif

#ifdef macintosh
	#include <CoreServices/CoreServices.h>
#endif

#ifndef ACExtension
	#define	ACExtension
#endif

#ifdef WINDOWS
	#pragma warning (disable: 4068)
	#pragma warning (disable: 4239)
	#pragma warning (disable: 4127)
	#pragma warning (disable: 4701)
#endif

#include "ACAPinc.h"
#include "Location.hpp"
#include "DGModule.hpp"
#include "UC.h"
#include "DG.h"
#include "APICommon.h"
#include "sqlite3.h"
#include <string.h>
#include <vector>
#include <exception>

#define TRUE	1
#define FALSE	0


// 팔레트 창 제어를 위한 함수
static void	EnablePaletteControls (short dialogID, bool enable)
{
	if (dialogID != 0 && DGIsDialogOpen (dialogID)) {
		if (enable == true)
			DGEnableItem (dialogID, DG_ALL_ITEMS);
		else
			DGDisableItem (dialogID, DG_ALL_ITEMS);
	}

	return;
}

// 벽 관련 정보
struct InfoWall
{
	API_Guid	guid;			// 벽의 GUID
	double	wallThk;			// 벽 두께
	short	floorInd;			// 층 인덱스
	double	bottomOffset;		// 벽 하단 오프셋

	double	begX;				// 시작점 X
	double	begY;				// 시작점 Y
	double	endX;				// 끝점 X
	double	endY;				// 끝점 Y
};

// 슬래브 관련 정보
struct InfoSlab
{
	API_Guid	guid;			// 슬래브의 GUID
	short	floorInd;			// 층 인덱스
	double	offsetFromTop;		// 슬래브 윗면과 레퍼런스 레벨과의 수직 거리
	double	thickness;			// 슬래브 두께
	double	level;				// 레퍼런스 레벨의 고도
};

// 보 관련 정보
struct InfoBeam
{
	API_Guid	guid;	// 보의 GUID

	short	floorInd;	// 층 인덱스
	double	height;		// 보 높이
	double	width;		// 보 너비
	double	offset;		// 보 중심으로부터 보의 레퍼런스 라인의 오프셋입니다.
	double	level;		// 바닥 레벨에 대한 보의 위쪽면 높이입니다.
	double	slantAngle;	// 기울어진 각도

	API_Coord	begC;	// 보 시작 좌표
	API_Coord	endC;	// 보 끝 좌표
};

// 기둥 관련 정보
struct InfoColumn
{
	API_Guid	guid;		// 기둥의 GUID
	short	floorInd;		// 층 인덱스

	bool	bRectangle;		// 직사각형 형태가 맞는가?
	short	coreAnchor;		// 코어의 앵커 포인트
	double	coreWidth;		// 기둥의 X 길이
	double	coreDepth;		// 기둥의 Y 길이
	double	venThick;		// 기둥 베니어 두께
	double	height;			// 기둥의 높이
	double	bottomOffset;	// 바닥 레벨에 대한 기둥 베이스 레벨
	double	topOffset;		// 만약 기둥이 윗층과 연결되어 있는 경우 윗층으로부터의 오프셋
	double	angle;			// 기둥 축을 중심으로 한 회전 각도 (단위: Radian)
	API_Coord	origoPos;	// 기둥의 위치
};

// 모프 관련 정보
struct InfoMorph
{
	API_Guid	guid;		// 모프의 GUID
	short		floorInd;	// 층 인덱스

	double		level;		// 모프의 고도
	double		height;		// 모프의 높이

	double	leftBottomX;	// 좌하단 좌표 X
	double	leftBottomY;	// 좌하단 좌표 Y
	double	leftBottomZ;	// 좌하단 좌표 Z

	double	rightTopX;		// 우상단 좌표 X
	double	rightTopY;		// 우상단 좌표 Y
	double	rightTopZ;		// 우상단 좌표 Z

	double	horLen;			// 가로 길이
	double	verLen;			// 세로 길이
	double	ang;			// 회전 각도 (단위: Degree, 회전축: Z축)
};

// 메시 관련 정보
struct InfoMesh
{
	API_Guid	guid;		// 메시의 GUID
	short		floorInd;	// 층 인덱스

	double		level;		// 메시 베이스 평면의 고도
	double		skirtLevel;	// 메시 베이스 평면으로부터 메시 밑면의 거리
};

#endif //__MAXBIM_HPP__

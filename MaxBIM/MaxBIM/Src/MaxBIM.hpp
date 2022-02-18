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
#include <string.h>
#include <vector>
#include <exception>

#define TRUE	1
#define FALSE	0


// 벽 관련 정보
struct InfoWall
{
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

#endif //__MAXBIM_HPP__

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


// �ȷ�Ʈ â ��� ���� �Լ�
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

// �� ���� ����
struct InfoWall
{
	API_Guid	guid;			// ���� GUID
	double	wallThk;			// �� �β�
	short	floorInd;			// �� �ε���
	double	bottomOffset;		// �� �ϴ� ������

	double	begX;				// ������ X
	double	begY;				// ������ Y
	double	endX;				// ���� X
	double	endY;				// ���� Y
};

// ������ ���� ����
struct InfoSlab
{
	API_Guid	guid;			// �������� GUID
	short	floorInd;			// �� �ε���
	double	offsetFromTop;		// ������ ����� ���۷��� �������� ���� �Ÿ�
	double	thickness;			// ������ �β�
	double	level;				// ���۷��� ������ ��
};

// �� ���� ����
struct InfoBeam
{
	API_Guid	guid;	// ���� GUID

	short	floorInd;	// �� �ε���
	double	height;		// �� ����
	double	width;		// �� �ʺ�
	double	offset;		// �� �߽����κ��� ���� ���۷��� ������ �������Դϴ�.
	double	level;		// �ٴ� ������ ���� ���� ���ʸ� �����Դϴ�.
	double	slantAngle;	// ������ ����

	API_Coord	begC;	// �� ���� ��ǥ
	API_Coord	endC;	// �� �� ��ǥ
};

// ��� ���� ����
struct InfoColumn
{
	API_Guid	guid;		// ����� GUID
	short	floorInd;		// �� �ε���

	bool	bRectangle;		// ���簢�� ���°� �´°�?
	short	coreAnchor;		// �ھ��� ��Ŀ ����Ʈ
	double	coreWidth;		// ����� X ����
	double	coreDepth;		// ����� Y ����
	double	venThick;		// ��� ���Ͼ� �β�
	double	height;			// ����� ����
	double	bottomOffset;	// �ٴ� ������ ���� ��� ���̽� ����
	double	topOffset;		// ���� ����� ������ ����Ǿ� �ִ� ��� �������κ����� ������
	double	angle;			// ��� ���� �߽����� �� ȸ�� ���� (����: Radian)
	API_Coord	origoPos;	// ����� ��ġ
};

// ���� ���� ����
struct InfoMorph
{
	API_Guid	guid;		// ������ GUID
	short		floorInd;	// �� �ε���

	double		level;		// ������ ��
	double		height;		// ������ ����

	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	rightTopX;		// ���� ��ǥ X
	double	rightTopY;		// ���� ��ǥ Y
	double	rightTopZ;		// ���� ��ǥ Z

	double	horLen;			// ���� ����
	double	verLen;			// ���� ����
	double	ang;			// ȸ�� ���� (����: Degree, ȸ����: Z��)
};

// �޽� ���� ����
struct InfoMesh
{
	API_Guid	guid;		// �޽��� GUID
	short		floorInd;	// �� �ε���

	double		level;		// �޽� ���̽� ����� ��
	double		skirtLevel;	// �޽� ���̽� ������κ��� �޽� �ظ��� �Ÿ�
};

#endif //__MAXBIM_HPP__

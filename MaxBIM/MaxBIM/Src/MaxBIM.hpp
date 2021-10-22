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


// �� ���� ����
struct InfoWall
{
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

// ������ ����
struct Euroform
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	bool	eu_stan_onoff;	// �԰��� On/Off
	double	eu_wid;			// �ʺ� (�԰�) : *600, 500, 450, 400, 300, 200
	double	eu_hei;			// ���� (�԰�) : *1200, 900, 600
	double	eu_wid2;		// �ʺ� (��԰�) : 50 ~ 900
	double	eu_hei2;		// ���� (��԰�) : 50 ~ 1500
	bool	u_ins_wall;		// ��ġ���� : �������(true), ��������(false)
	double	ang_x;			// ȸ��X : ��(90), õ��(0), �ٴ�(180)
	/*
	double	ang_y;			// ȸ��Y
	*/
	double	width;			// �ʺ� (�԰���/��԰��� ���� ����)
	double	height;			// ���� (�԰���/��԰��� ���� ����)
};

// �ٷ������̼� ����
struct FillerSpacer
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	double	f_thk;			// �β� : 10 ~ 50 (*20)
	double	f_leng;			// ���� : 150 ~ 2400
	double	f_ang;			// ���� : 90
	double	f_rota;			// ȸ�� : 0
};

// ���ڳ��ǳ� ����
struct IncornerPanel
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	double	wid_s;			// ����(����) : 80 ~ 500 (*100)
	double	leng_s;			// ����(�Ķ�) : 80 ~ 500 (*100)
	double	hei_s;			// ���� : 50 ~ 1500
	/*
	std::string		dir_s;			// ��ġ���� : *�����, ������, ������
	*/
};

// �ƿ��ڳ��ǳ� ����
struct OutcornerPanel
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	double	wid_s;			// ����(����) : 80 ~ 500 (*100)
	double	leng_s;			// ����(�Ķ�) : 80 ~ 500 (*100)
	double	hei_s;			// ���� : 50 ~ 1500
	/*
	std::string		dir_s;			// ��ġ���� : *�����, ������, ������
	*/
};

// ���� ����
struct Plywood
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	/*
	std::string		p_stan;			// �԰� : *3x6 [910x1820], 4x8 [1220x2440], ��԰�, ������
	std::string		w_dir;			// ��ġ���� : *�������, ��������, �ٴڱ��, �ٴڵ���
	std::string		p_thk;			// �β� : 2.7T, 4.8T, 8.5T, *11.5T, 14.5T
	*/
	double	p_wid;			// ����
	double	p_leng;			// ����
	bool	w_dir_wall;		// ��ġ���� : �������(true), ��������(false)
	short	w_dir;			// (Ȯ��) ��ġ���� : �������(1), ��������(2), �ٴڱ��(3), �ٴڵ���(4)
	/*
	double			p_ang;			// ���� : 0
	bool			sogak;			// ����Ʋ *On/Off
	std::string		prof;			// �������� : *�Ұ�, �߰�, �밢
	*/
	bool	bInverseSogak;	// ������������ ����
};

// ���� ����
struct Wood
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	/*
	std::string		w_ins;			// ��ġ���� : *�������, �ٴڴ�����, �ٴڵ���
	*/
	double	w_w;			// �β�
	double	w_h;			// �ʺ�
	double	w_leng;			// ����
	double	w_ang;			// ����
};

// �ƿ��ڳʾޱ� ����
struct OutcornerAngle
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	double	a_leng;			// ����
	double	a_ang;			// ����
};

// ������ ����
struct MagicBar
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	double	ZZYZX;				// ����

	double	angX;				// ȸ�� X
	double	angY;				// ȸ�� Y

	bool	bPlywood;			// ���� On/Off
	double	plywoodWidth;		// ���� �ʺ�
	double	plywoodOverhangH;	// ���� ������
	double	plywoodUnderhangH;	// ���� �����
};

// �������ڳ� ����
struct MagicIncorner
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	double	ZZYZX;				// ����

	short	type;				// Ÿ�� (50, 100)

	double	angX;				// ȸ�� X
	double	angY;				// ȸ�� Y

	bool	bPlywood;			// ���� On/Off
	double	plywoodWidth;		// ���� �ʺ�
	double	plywoodOverhangH;	// ���� ������
	double	plywoodUnderhangH;	// ���� �����
};

// ������ ���̺��� ����
struct SlabTableform
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	bool	direction;		// ��ġ���� : ���ι���(true), ���ι���(false)
	double	horLen;			// ���� ����
	double	verLen;			// ���� ����

	bool	bLprofileOnNorth;	// ���� ���⿡ L���� ����
	bool	bLprofileOnSouth;	// ���� ���⿡ L���� ����
	bool	bLprofileOnWest;	// ���� ���⿡ L���� ����
	bool	bLprofileOnEast;	// ���� ���⿡ L���� ����

	char	type [20];		// Ÿ��
};

// �� ���̺��� ����
struct WallTableform
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	double	width;			// �ʺ� (1800~2300, 50 ����)
	double	height;			// ���� (1500~6000, 300 ����)
};

// ��� ������ ����
struct SquarePipe
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	double	length;			// ������ ����
	double	pipeAng;		// ���� (����: 0, ����: 90)
};

// �ɺ�Ʈ ��Ʈ ����
struct PinBoltSet
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	bool	bPinBoltRot90;	// �ɺ�Ʈ 90�� ȸ��
	double	boltLen;		// ��Ʈ ����
	double	angX;			// X�� ȸ��
	double	angY;			// Y�� ȸ��
};

// ��ü Ÿ�� ����
struct WallTie
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	double	boltLen;		// ��Ʈ ����
	double	pipeBeg;		// ������ ������
	double	pipeEnd;		// ������ ����
	double	clampBeg;		// ���Ӽ� ������
	double	clampEnd;		// ���Ӽ� ����
};

// ���� Ŭ���� ����
struct CrossClamp
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	double	angX;			// ��ü ȸ�� (X)
	double	angY;			// ��ü ȸ�� (Y)
};

// ����ǽ� ����
struct HeadpieceOfPushPullProps
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)
};

// ����ö�� ����
struct MetalFittings
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	double	angX;			// ��ü ȸ�� (X)
	double	angY;			// ��ü ȸ�� (Y)
};

// KS�������� ����
struct KSProfile
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	double	angX;			// ��ü ȸ�� (X)
	double	angY;			// ��ü ȸ�� (Y)

	char	type [10];		// �з� (���, ��)
	char	shape [30];		// ���� (C����, H����...)
	short	iAnchor;		// ��Ŀ ����Ʈ (1~9)
	double	len;			// ����
	char	nom [50];		// �԰�
	long	mat;			// ����
};

//  ����ö�� (�簢�ͼ�Ȱ��) ����
struct MetalFittingsWithRectWasher
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	double	angX;			// ��ü ȸ�� (X)
	double	angY;			// ��ü ȸ�� (Y)

	double	bolt_len;		// ��Ʈ ����
	double	bolt_dia;		// ��Ʈ ����
	bool	bWasher1;		// �ͼ�1 On/Off
	double	washer_pos1;	// �ͼ�1 ��ġ
	bool	bWasher2;		// �ͼ�2 On/Off
	double	washer_pos2;	// �ͼ�2 ��ġ
	double	washer_size;	// �ͼ� ũ��
	char	nutType [15];	// ��Ʈ Ÿ��
};

// ������ ��ũ
struct EuroformHook
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	double	angX;			// ��ü ȸ�� (X)
	double	angY;			// ��ü ȸ�� (Y)

	short	iHookType;		// ��ũ Ÿ��
	short	iHookShape;		// ��ũ ���
};

// �������� ���
struct RectPipeHanger
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	double	angX;			// ��ü ȸ�� (X)
	double	angY;			// ��ü ȸ�� (Y)
};

// ���� 19
struct Cylinder
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	// ���� ���� "����-����" ����
	double	angleFromPlane;	// XY ��鿡���� ����
	double	radius;			// ���� ������
	double	length;			// ���� ����
};

// PERI���ٸ� ������
struct PERI_VPost
{
	double	leftBottomX;		// ���ϴ� ��ǥ X
	double	leftBottomY;		// ���ϴ� ��ǥ Y
	double	leftBottomZ;		// ���ϴ� ��ǥ Z

	double	ang;				// ȸ�� ���� (����: Radian, ȸ����: Z��)

	char	stType [16];		// �԰�
	bool	bCrosshead;			// ũ�ν���� On/Off
	char	posCrosshead [16];	// ũ�ν���� ��ġ (���, �ϴ�)
	char	crossheadType [16];	// ũ�ν���� Ÿ�� (PERI, ��� ����ǰ)
	double	angCrosshead;		// ũ�ν���� ȸ������
	double	len_current;		// ���� ����
	double	pos_lever;			// ���� ��ġ
	double	angY;				// ȸ�� Y

	bool	text2_onoff;		// 2D �ؽ�Ʈ On/Off
	bool	text_onoff;			// 3D �ؽ�Ʈ On/Off
	bool	bShowCoords;		// ��ǥ�� ǥ�� On/Off
};

// PERI���ٸ� ������
struct PERI_HPost
{
	double	leftBottomX;		// ���ϴ� ��ǥ X
	double	leftBottomY;		// ���ϴ� ��ǥ Y
	double	leftBottomZ;		// ���ϴ� ��ǥ Z

	double	ang;				// ȸ�� ���� (����: Radian, ȸ����: Z��)

	char	stType [16];		// �԰�
	double	angX;				// ȸ�� X
	double	angY;				// ȸ�� Y
	double	customLength;		// Ŀ���� ����
};

// ����� (�����)
struct BlueTimberRail
{
	double	leftBottomX;		// ���ϴ� ��ǥ X
	double	leftBottomY;		// ���ϴ� ��ǥ Y
	double	leftBottomZ;		// ���ϴ� ��ǥ Z

	double	ang;				// ȸ�� ���� (����: Radian, ȸ����: Z��)

	char	railType [16];		// �԰�
	double	angX;				// ȸ�� X
	double	angY;				// ȸ�� Y
};

// ���Ŭ���� (���ǿ�)
struct BlueClamp
{
	double	leftBottomX;		// ���ϴ� ��ǥ X
	double	leftBottomY;		// ���ϴ� ��ǥ Y
	double	leftBottomZ;		// ���ϴ� ��ǥ Z

	double	ang;				// ȸ�� ���� (����: Radian, ȸ����: Z��)

	char	type [64];			// Ÿ��
	double	angX;				// ȸ�� X
	double	angY;				// ȸ�� Y
	double	openingWidth;		// ���� �ʺ�
};

// ����Ʈ(���� ���ٸ�)
struct SuppPost
{
	double	leftBottomX;		// ���ϴ� ��ǥ X
	double	leftBottomY;		// ���ϴ� ��ǥ Y
	double	leftBottomZ;		// ���ϴ� ��ǥ Z

	double	ang;				// ȸ�� ���� (����: Radian, ȸ����: Z��)

	char	s_stan [20];		// �԰�
	double	s_leng;				// ����
	double	s_ang;				// ����
};

// GT24 �Ŵ�
struct GT24Girder
{
	double	leftBottomX;		// ���ϴ� ��ǥ X
	double	leftBottomY;		// ���ϴ� ��ǥ Y
	double	leftBottomZ;		// ���ϴ� ��ǥ Z

	double	ang;				// ȸ�� ���� (����: Radian, ȸ����: Z��)

	double	angX;				// ��ü ȸ�� (X)
	double	angY;				// ��ü ȸ�� (Y)

	char	type [8];			// �԰�
	double	length;				// ���� ����
};

// ��� �� �����
struct BlueBeamBracket
{
	double	leftBottomX;		// ���ϴ� ��ǥ X
	double	leftBottomY;		// ���ϴ� ��ǥ Y
	double	leftBottomZ;		// ���ϴ� ��ǥ Z

	double	ang;				// ȸ�� ���� (����: Radian, ȸ����: Z��)

	char	type [8];			// Ÿ��
	double	verticalHeight;		// ����
};

// �� �ۿ���
struct Yoke
{
	double	leftBottomX;		// ���ϴ� ��ǥ X
	double	leftBottomY;		// ���ϴ� ��ǥ Y
	double	leftBottomZ;		// ���ϴ� ��ǥ Z

	double	ang;				// ȸ�� ���� (����: Radian, ȸ����: Z��)

	double	beamLength;			// �� ����
	double	verticalGap;		// ������ ����
	double	innerVerticalLen;	// ���� ������ ����
	bool	bScale;				// ���� �� ���� ǥ��
	bool	bLrod;				// ���� ȯ��
	bool	bLstrut;			// ���� ��ħ��
	bool	bRrod;				// ���� ȯ��
	bool	bRstrut;			// ���� ��ħ��

	double	LsupVoffset;		// ���� ����Ʈ ������
	double	LsupVdist;			// ���� ����Ʈ �����Ÿ�
	double	LnutVdist;			// ���� ��Ʈ �����Ÿ�
	double	RsupVoffset;		// ���� ����Ʈ ������
	double	RsupVdist;			// ���� ����Ʈ �����Ÿ�
	double	RnutVdist;			// ���� ��Ʈ �����Ÿ�
	double	LbarHdist;			// ���� �������� ����Ÿ�
	double	LbarVdist;			// ���� �������� �����Ÿ�
	double	RbarHdist;			// ���� �������� ����Ÿ�
	double	RbarVdist;			// ���� �������� �����Ÿ�
};

#endif //__MAXBIM_HPP__

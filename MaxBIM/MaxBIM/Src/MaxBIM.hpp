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
#endif

#include "ACAPinc.h"
#include "Location.hpp"
#include "DGModule.hpp"
#include "UC.h"
#include "DG.h"
#include "APICommon.h"

#ifdef WINDOWS
	#pragma warning (default: 4068)
#endif

#pragma warning (disable: 4239)
#pragma warning (disable: 4127)

#define TRUE	1
#define FALSE	0

#endif //__MAXBIM_HPP__


// ���� ����
enum	idxItems_1;
enum	idxItems_2;
enum	idxItems_3;
enum	libPartObjType;
struct	InfoWall;
struct	InfoMorph;
struct	InterfereBeam;
struct	Euroform;
struct	FillerSpacer;
struct	IncornerPanel;
struct	Plywood;
struct	Wood;
struct	Cell;
struct	PlacingZone;

// ��ƿ��Ƽ �Լ�
double	DegreeToRad (double degree);																		// degree ������ radian ������ ��ȯ
double	RadToDegree (double rad);																			// radian ������ degree ������ ��ȯ
double	GetDistance (const double begX, const double begY, const double endX, const double endY);			// 2�������� 2�� ���� �Ÿ��� �˷���
long	compareDoubles (const double a, const double b);													// � ���� �� ū�� ����
long	compareRanges (double aMin, double aMax, double bMin, double bMax);									// a�� b�� �� �� ������ ���踦 �˷���
void	exchangeDoubles (double* a, double* b);																// a�� b ���� ��ȯ��
long	findDirection (const double begX, const double begY, const double endX, const double endY);			// ���������� �������� ���ϴ� ������ ������ Ȯ��
API_Coord	IntersectionPoint1 (const API_Coord* p1, const API_Coord* p2, const API_Coord* p3, const API_Coord* p4);	// (p1, p2)�� ���� ������ (p3, p4)�� ���� ������ �������� ���ϴ� �Լ�
API_Coord	IntersectionPoint2 (double m1, double b1, double m2, double b2);											// y = m1*x + b1, y = m2*x + b2 �� ������ �������� ���ϴ� �Լ�
API_Coord	IntersectionPoint3 (double a1, double b1, double c1, double a2, double b2, double c2);						// a1*x + b1*y + c1 = 0, a2*x + b2*y + c2 = 0 �� ������ �������� ���ϴ� �Լ�

// ������ �� ��ġ �Լ�
void	initCellsForWall (PlacingZone* placingZone);														// Cell �迭�� �ʱ�ȭ��
void	firstPlacingSettingsForWall (PlacingZone* placingZone);												// 1�� ��ġ: ���ڳ�, ������
void	copyPlacingZoneSymmetricForWall (PlacingZone* src_zone, PlacingZone* dst_zone, InfoWall* infoWall);	// ���� ���� ���� ������ ��Ī�ϴ� �ݴ��ʿ��� ������
void	alignPlacingZoneForWall (PlacingZone* target_zone);													// Cell ������ ����ʿ� ���� ����ȭ�� ��ġ�� ��������
GSErrCode	placeEuroformOnWall (void);		// 1�� �޴�: ������/���ڳ� ���� ��ġ�ϴ� ���� ��ƾ
GSErrCode	fillRestAreasForWall (void);																	// ���� ä������ �Ϸ�� �� ������ ���� ä���
short DGCALLBACK wallPlacerHandlerPrimary (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// 1�� ��ġ�� ���� ���Ǹ� ��û�ϴ� 1�� ���̾�α�
short DGCALLBACK wallPlacerHandlerSecondary (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);	// 1�� ��ġ �� ������ ��û�ϴ� 2�� ���̾�α�
short DGCALLBACK wallPlacerHandlerThird (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// 2�� ���̾�α׿��� �� ���� ��ü Ÿ���� �����ϱ� ���� 3�� ���̾�α�

// ���� �Լ�
void	copyCellsToAnotherLine (PlacingZone* target_zone, short src_row, short dst_row);					// src���� Cell ��ü ������ dst������ ����
std::string	format_string(const std::string fmt, ...);														// std::string ���� ���� formatted string�� �Է� ����
GSErrCode	placeCoordinateLabel (double xPos, double yPos, double zPos, bool bComment, std::string comment, short layerInd, short floorInd);		// ��ǥ ���� ��ġ��
API_Guid	placeLibPart (Cell objInfo);																	// �ش� �� ������ ������� ���̺귯�� ��ġ
double		getCellPositionLeftBottomX (PlacingZone *src_zone, short arr1, short idx);						// [arr1]�� - �ش� ���� ���ϴ� ��ǥX ��ġ�� ����
void		setCellPositionLeftBottomZ (PlacingZone *src_zone, short arr1, double new_hei);					// [arr1]�� - ��ü ���� ���ϴ� ��ǥZ ��ġ�� ����


// ���̾�α� �׸� �ε���
enum	idxItems_1 {
	LABEL_INCORNER				= 3,
	CHECKBOX_SET_LEFT_INCORNER	= 4,
	EDITCONTROL_LEFT_INCORNER	= 5,
	CHECKBOX_SET_RIGHT_INCORNER	= 6,
	EDITCONTROL_RIGHT_INCORNER	= 7,
	SEPARATOR_1					= 8,

	LABEL_PLACING_EUROFORM		= 9,
	LABEL_EUROFORM_WIDTH		= 10,
	POPUP_EUROFORM_WIDTH		= 11,
	LABEL_EUROFORM_HEIGHT		= 12,
	POPUP_EUROFORM_HEIGHT		= 13,
	LABEL_EUROFORM_ORIENTATION	= 14,
	POPUP_EUROFORM_ORIENTATION	= 15,
	SEPARATOR_2					= 16,

	ICON_LAYER					= 17,
	USERCONTROL_LAYER			= 18
};

enum	idxItems_2 {
	LABEL_REMAIN_HORIZONTAL_LENGTH			= 3,
	EDITCONTROL_REMAIN_HORIZONTAL_LENGTH	= 4,
	GROUPBOX_GRID_EUROFORM_FILLERSPACER		= 5,
	PUSHBUTTON_CONFIRM_REMAIN_LENGTH		= 6,

	// ���Ŀ��� �׸��� ��ư�� ��ġ��
	GRIDBUTTON_IDX_START					= 7
};

enum	idxItem_3 {
	//////////////////////////////////////// ������ �� ����
	// ��ü Ÿ��
	LABEL_OBJ_TYPE								= 3,
	POPUP_OBJ_TYPE,

	// ���ڳ�, �ٷ������̼�, ������ ��� (�β��� ���� ����)
	LABEL_WIDTH,
	EDITCONTROL_WIDTH,
	LABEL_HEIGHT,
	EDITCONTROL_HEIGHT,
	LABEL_THK,
	EDITCONTROL_THK,
	LABEL_ORIENTATION,
	RADIO_ORIENTATION_1_PLYWOOD,
	RADIO_ORIENTATION_2_PLYWOOD,

	// �������� ���
	CHECKBOX_SET_STANDARD,
	LABEL_EUROFORM_WIDTH_OPTIONS,
	POPUP_EUROFORM_WIDTH_OPTIONS,
	EDITCONTROL_EUROFORM_WIDTH_OPTIONS,
	LABEL_EUROFORM_HEIGHT_OPTIONS,
	POPUP_EUROFORM_HEIGHT_OPTIONS,
	EDITCONTROL_EUROFORM_HEIGHT_OPTIONS,
	LABEL_EUROFORM_ORIENTATION_OPTIONS,
	RADIO_ORIENTATION_1_EUROFORM,
	RADIO_ORIENTATION_2_EUROFORM,

	//////////////////////////////////////// ���� �� ����
	// ��ü Ÿ��
	LABEL_OBJ_TYPE_PREV,
	POPUP_OBJ_TYPE_PREV,

	// ���ڳ�, �ٷ������̼�, ������ ��� (�β��� ���� ����)
	LABEL_WIDTH_PREV,
	EDITCONTROL_WIDTH_PREV,
	LABEL_HEIGHT_PREV,
	EDITCONTROL_HEIGHT_PREV,
	LABEL_THK_PREV,
	EDITCONTROL_THK_PREV,
	LABEL_ORIENTATION_PREV,
	RADIO_ORIENTATION_1_PLYWOOD_PREV,
	RADIO_ORIENTATION_2_PLYWOOD_PREV,

	// �������� ���
	CHECKBOX_SET_STANDARD_PREV,
	LABEL_EUROFORM_WIDTH_OPTIONS_PREV,
	POPUP_EUROFORM_WIDTH_OPTIONS_PREV,
	EDITCONTROL_EUROFORM_WIDTH_OPTIONS_PREV,
	LABEL_EUROFORM_HEIGHT_OPTIONS_PREV,
	POPUP_EUROFORM_HEIGHT_OPTIONS_PREV,
	EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_PREV,
	LABEL_EUROFORM_ORIENTATION_OPTIONS_PREV,
	RADIO_ORIENTATION_1_EUROFORM_PREV,
	RADIO_ORIENTATION_2_EUROFORM_PREV,

	//////////////////////////////////////// ���� �� ����
	// ��ü Ÿ��
	LABEL_OBJ_TYPE_NEXT,
	POPUP_OBJ_TYPE_NEXT,

	// ���ڳ�, �ٷ������̼�, ������ ��� (�β��� ���� ����)
	LABEL_WIDTH_NEXT,
	EDITCONTROL_WIDTH_NEXT,
	LABEL_HEIGHT_NEXT,
	EDITCONTROL_HEIGHT_NEXT,
	LABEL_THK_NEXT,
	EDITCONTROL_THK_NEXT,
	LABEL_ORIENTATION_NEXT,
	RADIO_ORIENTATION_1_PLYWOOD_NEXT,
	RADIO_ORIENTATION_2_PLYWOOD_NEXT,

	// �������� ���
	CHECKBOX_SET_STANDARD_NEXT,
	LABEL_EUROFORM_WIDTH_OPTIONS_NEXT,
	POPUP_EUROFORM_WIDTH_OPTIONS_NEXT,
	EDITCONTROL_EUROFORM_WIDTH_OPTIONS_NEXT,
	LABEL_EUROFORM_HEIGHT_OPTIONS_NEXT,
	POPUP_EUROFORM_HEIGHT_OPTIONS_NEXT,
	EDITCONTROL_EUROFORM_HEIGHT_OPTIONS_NEXT,
	LABEL_EUROFORM_ORIENTATION_OPTIONS_NEXT,
	RADIO_ORIENTATION_1_EUROFORM_NEXT,
	RADIO_ORIENTATION_2_EUROFORM_NEXT
};

// ��ü ��ȣ
enum	libPartObjType {
	NONE,			// ����
	INCORNER,		// ���ڳ��ǳ�v1.0
	EUROFORM,		// ������v2.0
	FILLERSPACER,	// �ٷ������̼�v1.0
	PLYWOOD,		// ����v1.0
	WOOD			// ����v1.0
};


// �� ���� ����
struct InfoWall
{
	double	wallThk;			// �� �β�
	short	floorInd;			// �� �ε���

	double	begX;				// ������ X
	double	begY;				// ������ Y
	double	endX;				// ���� X
	double	endY;				// ���� Y
};

// ���� ���� ����
struct InfoMorph
{
	API_Guid	guid;		// ������ GUID

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

// ���� �� ����
struct InterfereBeam
{
	double	leftBottomX;
	double	leftBottomY;
	double	leftBottomZ;

	double	horLen;
	double	verLen;
};

// ������ ����
struct Euroform
{
	bool			eu_stan_onoff;	// �԰��� On/Off
	double			eu_wid;			// �ʺ� (�԰�) : *600, 500, 450, 400, 300, 200
	double			eu_hei;			// ���� (�԰�) : *1200, 900, 600
	double			eu_wid2;		// �ʺ� (��԰�) : 50 ~ 900
	double			eu_hei2;		// ���� (��԰�) : 50 ~ 1500
	bool			u_ins_wall;		// ��ġ���� : �������(true), ��������(false)
	/*
	std::string		u_ins;			// ��ġ���� : *�������, ��������
	double			ang_x;			// ȸ��X : ��(90), õ��(0), �ٴ�(180)
	double			ang_y;			// ȸ��Y
	*/
};

// �ٷ������̼� ����
struct FillerSpacer
{
	double			f_thk;			// �β� : 10 ~ 50 (*20)
	double			f_leng;			// ���� : 150 ~ 2400
	/*
	double			f_ang;			// ���� : 90
	double			f_rota;			// ȸ�� : 0
	*/
};

// ���ڳ��ǳ� ����
struct IncornerPanel
{
	double			wid_s;			// ����(����) : 80 ~ 500 (*100)
	double			leng_s;			// ����(�Ķ�) : 80 ~ 500 (*100)
	double			hei_s;			// ���� : 50 ~ 1500
	/*
	double			dir_s;			// ��ġ���� : *�����, ������, ������
	*/
};

// ���� ����
struct Plywood
{
	/*
	std::string		p_stan;			// �԰� : *3x6 [910x1820], 4x8 [1220x2440], ��԰�, ������
	std::string		w_dir;			// ��ġ���� : *�������, ��������, �ٴڱ��, �ٴڵ���
	std::string		p_thk;			// �β� : 2.7T, 4.8T, 8.5T, *11.5T, 14.5T
	*/
	double			p_wid;			// ����
	double			p_leng;			// ����
	bool			w_dir_wall;		// ��ġ���� : �������(true), ��������(false)
	/*
	double			p_ang;			// ���� : 0
	bool			sogak;			// ����Ʋ *On/Off
	std::string		prof;			// �������� : *�Ұ�, �߰�, �밢
	*/
};

// ���� ����
struct Wood
{
	/*
	std::string		w_ins;			// ��ġ���� : *�������, �ٴڴ�����, �ٴڵ���
	*/
	double			w_w;			// �β�
	double			w_h;			// �ʺ�
	double			w_leng;			// ����
	double			w_ang;			// ����
};

// �׸��� �� �� ����
struct Cell
{
	short		objType;	// enum libPartObjType ����

	API_Guid	guid;		// ��ü�� GUID

	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	horLen;			// ���� ����
	double	verLen;			// ���� ����
	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	union {
		Euroform		form;
		FillerSpacer	fillersp;
		IncornerPanel	incorner;
		Plywood			plywood;
		Wood			wood;
	} libPart;
};

// ���� ���� ���� (���� �߿��� ����)
struct PlacingZone
{
	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z

	double	horLen;			// ���� ����
	double	verLen;			// ���� ����
	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	// ������ (0�� �̻�)
	short	nInterfereBeams;	// ������ ����
	InterfereBeam	beams [30];

	// ������ ���� (1. �⺻ ä���)
	double	remain_hor;				// ���� ���� ���� ����
	double	remain_hor_updated;		// ���� ���� ���� ���� (������Ʈ ��)
	double	remain_ver;				// ���� ���� ���� ����
	double	remain_ver_wo_beams;	// �������� ������ ���� �ʴ� ���� ����

	bool	bLIncorner;				// ���ڳ� ���� ��ġ
	double	lenLIncorner;			// ���ڳ� ���� �ʺ�
	bool	bRIncorner;				// ���ڳ� ������ ��ġ
	double	lenRIncorner;			// ���ڳ� ������ �ʺ�

	std::string		eu_wid;			// ������ �ʺ�
	std::string		eu_hei;			// ������ ����
	std::string		eu_ori;			// ������ ����
	double	eu_wid_numeric;			// ������ �ʺ� (�Ǽ���)
	double	eu_hei_numeric;			// ������ ���� (�Ǽ���)
	short	eu_count_hor;			// ���� ���� ������ ����
	short	eu_count_ver;			// ���� ���� ������ ����

	// ������ ���� (2. ��ġ�� ��ü ������ �׸���� ����)
	// ���ڳ�[0] | ����[Ȧ��] | ��[¦��] | ... | ���ڳ�[n-1]
	Cell	cells [50][100];		// ������ �ε���: [eu_count_ver-1][nCells-1]
	short	nCells;
};

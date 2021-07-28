#ifndef	__BEAM_TABLEFORM_PLACER__
#define __BEAM_TABLEFORM_PLACER__

#include "MaxBIM.hpp"

namespace beamTableformPlacerDG {
	// ��ü Ÿ��
	enum	libPartObjType {
		NONE,			// ����
		EUROFORM,		// ������v2.0
		PLYWOOD,		// ����v1.0
		WOOD,			// ����v1.0
		OUTCORNER_ANGLE,// �ƿ��ڳʾޱ�v1.0
		FILLERSPACER	// �ٷ������̼�v1.0
	};

	// �����Ǵ� �� ��ġ
	enum	attachedSide {
		BOTTOM_SIDE,
		LEFT_SIDE,
		RIGHT_SIDE
	};
}

// ���� ���� ����
struct InfoMorphForBeamTableform
{
	API_Guid	guid;		// ������ GUID
	short		floorInd;	// �� �ε���
	double		level;		// ������ ��
};

// �׸��� �� �� ����
struct CellForBeamTableform
{
	short		objType;	// enum libPartObjType ����

	API_Guid	guid;		// ��ü�� GUID

	double	leftBottomX;	// ���ϴ� ��ǥ X
	double	leftBottomY;	// ���ϴ� ��ǥ Y
	double	leftBottomZ;	// ���ϴ� ��ǥ Z
	double	ang;			// ȸ�� ���� (����: Radian, ȸ����: Z��)

	double	dirLen;			// �� ��ġ���� ����
	double	perLen;			// �� �������� ����
	short	attached_side;	// �Ϻ�(BOTTOM_SIDE), ������(LEFT_SIDE), ������(RIGHT_SIDE)

	union {
		Euroform		form;
		Plywood			plywood;
		FillerSpacer	fillersp;
		Wood			wood;
		OutcornerAngle	outangle;
	} libPart;
};

// �� ���� ����
class BeamTableformPlacingZone
{
public:
	// ���� ���� ��ǥ�� ����, �� ��ǥ�� �������̶�� ������

	// �� ���� ����
	double	level;				// �� ���� ��
	double	ang;				// ȸ�� ���� (����: Radian, ȸ����: Z��)
	double	areaHeight;			// ���� ����

	API_Coord3D		begC;		// ��ġ ���� ������
	API_Coord3D		endC;		// ��ġ ���� ����
	double	beamLength;			// ���� �� ��ü ����

	// ���� �� ���� ����
	bool	bInterfereBeam;				// ���� �� ����
	double	posInterfereBeamFromLeft;	// ���� �� ��ġ (�� ���� ��ǥ-����-�κ���)
	double	interfereBeamWidth;			// ���� �� �ʺ�
	double	interfereBeamHeight;		// ���� �� ����

	// ���� ���� ������ ���� ����
	double	centerLengthAtSide;			// ����

	// �� �糡 ����
	double	marginBeginAtSide;				// ���� ���� �κ� ����
	double	marginEndAtSide;				// ���� �� �κ� ����
	double	marginBeginAtBottom;			// �Ϻ� ���� �κ� ����
	double	marginEndAtBottom;				// �Ϻ� �� �κ� ����

	// �� �糡 ���� ä���� ����
	bool	bFillMarginBeginAtSide;			// ���� ���� �κ� ���� ä��
	bool	bFillMarginEndAtSide;			// ���� �� �κ� ���� ä��
	bool	bFillMarginBeginAtBottom;		// �Ϻ� ���� �κ� ���� ä��
	bool	bFillMarginEndAtBottom;			// �Ϻ� �� �κ� ���� ä��

	// �� ���� (����)
	CellForBeamTableform	cellsFromBeginAtLSide [4][50];	// ���� �κк��� �ٿ����� �� (����)
	CellForBeamTableform	cellsFromBeginAtRSide [4][50];	// ���� �κк��� �ٿ����� �� (������)
	CellForBeamTableform	cellsFromEndAtLSide [4][50];	// �� �κк��� �ٿ����� �� (����)
	CellForBeamTableform	cellsFromEndAtRSide [4][50];	// �� �κк��� �ٿ����� �� (������)
	CellForBeamTableform	cellCenterAtLSide [4];			// ��� �κп� ���̴� �� (����)
	CellForBeamTableform	cellCenterAtRSide [4];			// ��� �κп� ���̴� �� (������)
	short			nCellsFromBeginAtSide;			// ������ ���� �κ� �� ����
	short			nCellsFromEndAtSide;			// ������ �� �κ� �� ����

	// �� ���� (�Ϻ�)
	CellForBeamTableform	cellsFromBeginAtBottom [3][50];	// ���� �κк��� �ٿ����� ��
	CellForBeamTableform	cellsFromEndAtBottom [3][50];	// �� �κк��� �ٿ����� ��
	CellForBeamTableform	cellCenterAtBottom [3];			// ��� �κп� ���̴� ��
	short			nCellsFromBeginAtBottom;		// �Ϻ��� ���� �κ� �� ����
	short			nCellsFromEndAtBottom;			// �Ϻ��� �� �κ� �� ����

	double			gapSide;			// ������ ���� (����)
	double			gapBottom;			// ������ ���� (�Ϻ�)

public:
	void		initCells (BeamTableformPlacingZone* placingZone);						// Cell �迭�� �ʱ�ȭ��
	void		firstPlacingSettings (BeamTableformPlacingZone* placingZone);			// 1�� ��ġ ����
	void		addNewColFromBeginAtSide (BeamTableformPlacingZone* target_zone);		// ���� ���� �κ� - ���ο� ���� �߰��� (�� �ϳ��� �ø��� �߰��� ���� ������ �� ���� ����)
	void		delLastColFromBeginAtSide (BeamTableformPlacingZone* target_zone);		// ���� ���� �κ� - ������ ���� ������
	void		addNewColFromEndAtSide (BeamTableformPlacingZone* target_zone);			// ���� �� �κ� - ���ο� ���� �߰��� (�� �ϳ��� �ø��� �߰��� ���� ������ �� ���� ����)
	void		delLastColFromEndAtSide (BeamTableformPlacingZone* target_zone);		// ���� �� �κ� - ������ ���� ������
	void		addNewColFromBeginAtBottom (BeamTableformPlacingZone* target_zone);		// �Ϻ� ���� �κ� - ���ο� ���� �߰��� (�� �ϳ��� �ø��� �߰��� ���� ������ �� ���� ����)
	void		delLastColFromBeginAtBottom (BeamTableformPlacingZone* target_zone);	// �Ϻ� ���� �κ� - ������ ���� ������
	void		addNewColFromEndAtBottom (BeamTableformPlacingZone* target_zone);		// �Ϻ� �� �κ� - ���ο� ���� �߰��� (�� �ϳ��� �ø��� �߰��� ���� ������ �� ���� ����)
	void		delLastColFromEndAtBottom (BeamTableformPlacingZone* target_zone);		// �Ϻ� �� �κ� - ������ ���� ������
	void		alignPlacingZone (BeamTableformPlacingZone* placingZone);				// Cell ������ ����ʿ� ���� ����ȭ�� ��ġ�� ��������
	API_Guid	placeLibPart (CellForBeamTableform objInfo);							// �ش� �� ������ ������� ���̺귯�� ��ġ
	GSErrCode	fillRestAreas (BeamTableformPlacingZone* placingZone);					// ������/�ٷ�/���縦 ä�� �� ������ ���� ä��� (������ ����/���� �� �ƿ��ڳʾޱ�)
};

// ���̺����� �� ��ġ �Լ�
GSErrCode	placeTableformOnBeam (void);		// ���� ���̺����� ��ġ�ϴ� ���� ��ƾ

#endif

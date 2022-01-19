#ifndef	__EXPORT__
#define __EXPORT__

#include "MaxBIM.hpp"

using namespace std;

namespace exportDG {
	// ���̾�α� �׸� �ε���
	enum	idxItems_1_exportDG {
		LABEL_DIST_BTW_COLUMN		= 3,
		EDITCONTROL_DIST_BTW_COLUMN
	};

	enum	layerTypeEnum {
		UNDEFINED,	// ���ǵ��� ����
		WALL,	// �� -> ���� �յ޸� ĸ���� ��
		SLAB,	// ������ -> ���� �Ʒ��� ĸ���� ��
		COLU,	// ��� -> ������ ĸ���� ��
		BEAM,	// �� -> ����, �Ϻ� ĸ���� ��
		WLBM	// ���纸 -> ����, �Ϻ� ĸ���� ��
	};

	enum	filterSelectionDG {
		BUTTON_ALL_SEL = 3,
		BUTTON_ALL_UNSEL,
		CHECKBOX_INCLUDE_UNKNOWN_OBJECT
	};

	enum	BEAMTABLEFORM_DIRECTION {
		HORIZONTAL_DIRECTION,
		VERTICAL_DIRECTION
	};

	enum	BEAMTABLEFORM_OBJECT_TYPE {
		EUROFORM,
		PLYWOOD,
		TIMBER,
		FILLERSP
	};

	enum	ATTACH_POSITION {
		LEFT_SIDE,
		RIGHT_SIDE,
		BOTTOM_SIDE
	};
}

// ���� ����� ����
struct ColumnInfo
{
	short	floorInd;	// �� �ε���
	double	posX;		// �߽��� X��ǥ
	double	posY;		// �߽��� Y��ǥ
	double	horLen;		// ���� ����
	double	verLen;		// ���� ����
	double	height;		// ����

	short	iHor;		// ���� ����
	short	iVer;		// ���� ����
};

// ����� ��ġ ����
struct ColumnPos
{
	ColumnInfo	columns [5000];		// ��� ����
	short	nColumns;				// ��� ����

	// �ֿ� ����
	double	node_hor [100][100];	// ���� ���� ���� ��� ��ǥ (X) : dim1 - ��, dim2 - ��� X��ǥ
	short	nNodes_hor [100];		// ���� ���� ���� ��� ����
	double	node_ver [100][100];	// ���� ���� ���� ��� ��ǥ (Y) : dim1 - ��, dim2 - ��� Y��ǥ
	short	nNodes_ver [100];		// ���� ���� ���� ��� ����

	short	nStories;				// �� ���� �ǹ���, = storyInfo.lastStory - storyInfo.firstStory + (storyInfo.skipNullFloor) * 1 (0���� �����ߴٸ� +1)
	short	firstStory;				// ������ �� �ε��� (��: ���� 4���� ���, -4)
	short	lastStory;				// �ֻ��� �� �ε��� (��: ���� 35���� ���, 34)
};

// ������ ������� ��� ����
class SummaryOfObjectInfo
{
public:
	SummaryOfObjectInfo ();		// ������
	void clear ();				// ���ڵ� ���� �����
	int	quantityPlus1 (vector<string> record);	// ��ü�� ���ڵ� ���� 1 ���� (������ ����, ������ �ű� �߰�)

public:
	// objectInfo.csv ���� ����
	vector<string>				keyName;	// ��ü�� ������ �� �ִ� ��(���ڿ�)�� ��� �ִ� ���� �̸� (��: u_comp)
	vector<string>				keyDesc;	// ��ü�� �̸� (��: ������)
	vector<int>					nInfo;		// ǥ���� ���� �ʵ� ����
	vector<vector<string>>		varName;	// ������ �̸� (��: eu_stan_onoff)		-- ���ο� �� �ٸ� vector<string>�� ���ԵǸ�, �װ��� ���̴� �ش� ��ü�� nInfo�� ����
	vector<vector<string>>		varDesc;	// ������ �̸��� ���� ���� (��: �԰���)	-- ���ο� �� �ٸ� vector<string>�� ���ԵǸ�, �װ��� ���̴� �ش� ��ü�� nInfo�� ����

	// ��ü�� ���� ������ ���պ� ����
	vector<vector<string>>		records;	// ��ü�� �̸�, ���� ������ ����, ���տ� �ش��ϴ� ��ü ������ ��� ���� (�ʵ� 1���� ����: ���ڳ��ǳ� | 100 | 100 | 1200 | 3)

	// ��Ÿ
	int nKnownObjects;						// ������ ��ü�� ����
	int nUnknownObjects;					// �������� ���� ��ü�� ����

	// ��ü�� ���� (Beam Ÿ��)
	vector<int>		beamLength;				// �� ����
	vector<int>		beamCount;				// �ش� �� ���̿� ���� ����
	int				nCountsBeam;			// �� ������ ����
};

// ���̴� ���̾� ���� ��ü�� ��Ī, ���� ����, ���̱� ����
struct VisibleObjectInfo
{
	// Object Ÿ��
	short	nKinds;				// ��ü ���� ����
	char	varName [100][64];	// 1��: ������
	char	objName [100][128];	// 2��: ��ü��
	bool	bExist [100];		// ���� ����
	bool	bShow [100];		// ǥ�� ����
	short	itmIdx [100];		// ���̾�α� �� �ε���

	// �˷����� ���� Object Ÿ���� ��ü
	bool	bShow_Unknown;
	long	nUnknownObjects;

	// ������ Ÿ��
	bool	bExist_Walls;
	bool	bShow_Walls;
	short	itmIdx_Walls;

	bool	bExist_Columns;
	bool	bShow_Columns;
	short	itmIdx_Columns;

	bool	bExist_Beams;
	bool	bShow_Beams;
	short	itmIdx_Beams;

	bool	bExist_Slabs;
	bool	bShow_Slabs;
	short	itmIdx_Slabs;

	bool	bExist_Roofs;
	bool	bShow_Roofs;
	short	itmIdx_Roofs;

	bool	bExist_Meshes;
	bool	bShow_Meshes;
	short	itmIdx_Meshes;

	bool	bExist_Morphs;
	bool	bShow_Morphs;
	short	itmIdx_Morphs;

	bool	bExist_Shells;
	bool	bShow_Shells;
	short	itmIdx_Shells;

	// �����ϴ� �׸� ��ü ����
	short	nItems;
};

// �� ���̺��� �� ����鿡 ���� ����
struct objectInBeamTableform
{
	short	objType;			// EUROFORM, PLYWOOD, TIMBER, FILLERSP
	short	attachPosition;		// LEFT_SIDE, RIGHT_SIDE, BOTTOM_SIDE
	API_Coord3D	origin;			// ���� ��ǥ
	double	width;				// ��ü �ʺ�
	double	length;				// ��ü ����
};

// �� ���̺��� ����ǥ �ۼ��� ���� ����ü�� ���Ǵ� �� ����
struct BeamTableformCell
{
	double	euroform_1_leftHeight;		// ����/���� ���� (1�� ������)
	double	euroform_1_rightHeight;		// ������/�Ʒ��� ���� (1�� ������)
	double	euroform_1_bottomWidth;		// �Ϻ� �ʺ� (1�� ������)

	double	euroform_2_leftHeight;		// ����/���� ���� (2�� ������)
	double	euroform_2_rightHeight;		// ������/�Ʒ��� ���� (2�� ������)
	double	euroform_2_bottomWidth;		// �Ϻ� �ʺ� (2�� ������)

	double	fillersp_leftHeight;		// ����/���� ���� (�ٷ������̼�)
	double	fillersp_rightHeight;		// ������/�Ʒ��� ���� (�ٷ������̼�)
	double	fillersp_bottomWidth;		// �Ϻ� �ʺ� (�ٷ������̼�)

	double	timber_leftHeight;			// ����/���� ���� (���� �Ǵ� ����)
	double	timber_rightHeight;			// ������/�Ʒ��� ���� (���� �Ǵ� ����)

	double	plywoodOnly_leftHeight;		// ����/���� ���� (��ü ����)
	double	plywoodOnly_rightHeight;	// ������/�Ʒ��� ���� (��ü ����)
	double	plywoodOnly_bottomWidth;	// �Ϻ� �ʺ� (��ü ����)
};

// �� ���̺��� ����ǥ �ۼ��� ���� Ŭ����
class BeamTableformInfo
{
public:
	short	iBeamDirection;		// �� ���� (HORIZONTAL_DIRECTION, VERTICAL_DIRECTION)

	double	leftHeight;			// ����/���� ����
	double	rightHeight;		// ������/�Ʒ��� ����
	double	bottomWidth;		// �Ϻ� �ʺ�

	short	nCells;						// �� ����
	BeamTableformCell	cells [30];		// �� ����

public:
	BeamTableformInfo ();		// �� ���̺��� ����ǥ �ۼ��� ���� Ŭ���� ������ (�ʱ�ȭ)
	void init ();				// �ʱ�ȭ
};

void		initArray (double arr [], short arrSize);											// �迭 �ʱ�ȭ �Լ�
int			compare (const void* first, const void* second);									// ������������ ������ �� ����ϴ� ���Լ� (����Ʈ)

ColumnInfo	findColumn (ColumnPos* columnPos, short iHor, short iVer, short floorInd);			// �����ֿ�, �����ֿ�, �� ������ �̿��Ͽ� ��� ã��
GSErrCode	exportGridElementInfo (void);														// ����(���,��,������)���� ������ �����ϰ� �����ؼ� ���� ���Ϸ� ��������
short		DGCALLBACK inputThresholdHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [���̾�α�] ��� �� �ּ� ���� �Ÿ��� ����ڿ��� �Է� ���� (�⺻��: 2000 mm)

GSErrCode	exportSelectedElementInfo (void);													// ������ ���� ���� �������� (Single ���)
GSErrCode	exportElementInfoOnVisibleLayers (void);											// ������ ���� ���� �������� (Multi ���)

GSErrCode	filterSelection (void);																// ���纰 ���� �� �����ֱ�
short		DGCALLBACK filterSelectionHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [���̾�α�] ���̾�α׿��� ���̴� ���̾� �� �ִ� ��ü���� ������ �����ְ�, üũ�� ������ ��ü�鸸 ���� �� ������

GSErrCode	exportBeamTableformInformation (void);												// �� ���̺��� ���� ���� ��������

#endif
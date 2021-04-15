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
struct SummaryOfSelectedObjects
{
	// ������
	int uformWidth [100];				// ������ �ʺ�
	int uformHeight [100];				// ������ ����
	int uformCount [100];				// �ش� ������ �ʺ�/���� ���տ� ���� ����
	int sizeOfUformKinds;				// ������ ������ ����

	// ��ƿ��
	int steelformWidth [100];			// ��ƿ�� �ʺ�
	int steelformHeight [100];			// ��ƿ�� ����
	int steelformCount [100];			// �ش� ��ƿ�� �ʺ�/���� ���տ� ���� ����
	int	sizeOfSteelformKinds;			// ��ƿ�� ������ ����

	// ���ڳ��ǳ�
	int incornerPanelHor [100];			// ���ڳ��ǳ� ����
	int incornerPanelVer [100];			// ���ڳ��ǳ� ����
	int incornerPanelHei [100];			// ���ڳ��ǳ� ����
	int incornerPanelCount [100];		// �ش� ���ڳ��ǳ� ����/����/���� ���տ� ���� ����
	int	sizeOfIncornerPanelKinds;		// ���ڳ��ǳ� ������ ����

	// �ƿ��ڳ��ǳ�
	int outcornerPanelHor [100];		// �ƿ��ڳ��ǳ� ����
	int outcornerPanelVer [100];		// �ƿ��ڳ��ǳ� ����
	int outcornerPanelHei [100];		// �ƿ��ڳ��ǳ� ����
	int outcornerPanelCount [100];		// �ش� �ƿ��ڳ��ǳ� ����/����/���� ���տ� ���� ����
	int	sizeOfOutcornerPanelKinds;		// �ƿ��ڳ��ǳ� ������ ����

	// �ƿ��ڳʾޱ�
	int outcornerAngleLength [100];		// �ƿ��ڳʾޱ� ����
	int outcornerAngleCount [100];		// �ش� �ƿ��ڳʾޱ� ���̿� ���� ����
	int sizeOfOutcornerAngleKinds;		// �ƿ��ڳʾޱ� ������ ����

	// ����
	int woodThk [100];					// ���� �β�
	int woodWidth [100];				// ���� �ʺ�
	int woodLength [100];				// ���� ����
	int woodCount [100];				// �ش� ���� �β�/�ʺ�/���� ���տ� ���� ����
	int sizeOfWoodKinds;				// ���� ������ ����

	// �ٷ������̼�
	int fsThk [100];					// �ٷ������̼� �β�
	int fsLength [100];					// �ٷ������̼� ����
	int fsCount [100];					// �ش� �ٷ������̼� �β�/���� ���տ� ���� ����
	int sizeOfFsKinds;					// �ٷ������̼� ������ ����

	// �簢������ (���������, ���簢������)
	int sqrPipeHor [100];				// �簢������ �ܸ� ����
	int sqrPipeVer [100];				// �簢������ �ܸ� ����
	int sqrPipeLength [100];			// �簢������ ����
	int sqrPipeCount [100];				// �ش� �簢������ ����/����/���� ���տ� ���� ����
	int sizeOfSqrPipeKinds;				// �簢������ ������ ����

	// ����
	int plywoodHor [100];				// ����
	int plywoodVer [100];				// ����
	double plywoodThk [100];			// �β�
	int plywoodCount [100];				// �ش� ���� ����/����/�β� ���տ� ���� ����
	int sizeOfPlywoodKinds;				// ���� ������ ����

	// RS Push-Pull Props ����ǽ� (�ξ�� ����)
	int nHeadpiece;

	// RS Push-Pull Props
	bool bPropsUpperSupp [100];			// ���(����) ������ ����
	char PropsNomUpperSupp [100][30];	// ���(����) ������ �԰�
	bool bPropsLowerSupp [100];			// �Ϻ�(����) ������ ����
	char PropsNomLowerSupp [100][30];	// �Ϻ�(����) ������ �԰�
	bool bPropsBase [100];				// ���̽� �÷���Ʈ ����
	int propsCount [100];				// �ش� Props ������ ����/�԰�, ���̽� �÷���Ʈ ���� ���տ� ���� ����
	int sizeOfPropsKinds;				// Props ������ ����

	// ��ü Ÿ��
	int nTie;

	// ����Ŭ����
	int nClamp;

	// �ɺ�Ʈ��Ʈ
	int pinboltLen [100];				// �ɺ�Ʈ ����
	int pinboltCount [100];				// �ش� ���̿� ���� ����
	int sizeOfPinboltKinds;				// �ɺ�Ʈ ������ ����

	// ����ö�� (�簢�ͼ�Ȱ��)
	int nJoin;

	// �� �ۿ���
	int beamYokeLength [10];			// �� �ۿ��� ����
	int beamYokeCount [10];				// �ش� �� �ۿ��� ���̿� ���� ����
	int sizeOfBeamYokeKinds;			// �� �ۿ��� ������ ����

	// �Ϲ� ���
	// ��
	int beamLength [100];				// �� ����
	int beamCount [100];				// �ش� �� ���̿� ���� ����
	int sizeOfBeamKinds;				// �� ������ ����

	// �� �� ���� ��ü
	int nUnknownObjects;				// �������� ���� ��ü�� ����
};

void		initArray (double arr [], short arrSize);											// �迭 �ʱ�ȭ �Լ�
int			compare (const void* first, const void* second);									// ������������ ������ �� ����ϴ� ���Լ� (����Ʈ)
ColumnInfo	findColumn (ColumnPos* columnPos, short iHor, short iVer, short floorInd);			// �����ֿ�, �����ֿ�, �� ������ �̿��Ͽ� ��� ã��
GSErrCode	exportGridElementInfo (void);														// ����(���,��,������)���� ������ �����ϰ� �����ؼ� ���� ���Ϸ� ��������
short		DGCALLBACK inputThresholdHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [���̾�α�] ��� �� �ּ� ���� �Ÿ��� ����ڿ��� �Է� ���� (�⺻��: 2000 mm)
GSErrCode	exportSelectedElementInfo (void);													// ������ ���� ���� ��������

#endif
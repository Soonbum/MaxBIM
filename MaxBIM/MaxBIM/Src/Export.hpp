#ifndef	__EXPORT__
#define __EXPORT__

#include "MaxBIM.hpp"
#include <vector>
#include <cmath>

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
	vector<int>	uformWidth;				// ������ �ʺ�
	vector<int>	uformHeight;			// ������ ����
	vector<int>	uformCount;				// �ش� ������ �ʺ�/���� ���տ� ���� ����
	int	sizeOfUformVectors;				// ������ ������ ����

	// ��ƿ��
	vector<int>	steelformWidth;			// ��ƿ�� �ʺ�
	vector<int>	steelformHeight;		// ��ƿ�� ����
	vector<int>	steelformCount;			// �ش� ��ƿ�� �ʺ�/���� ���տ� ���� ����
	int	sizeOfSteelformVectors;			// ��ƿ�� ������ ����

	// ���ڳ��ǳ�
	vector<int>	incornerPanelHor;		// ���ڳ��ǳ� ����
	vector<int> incornerPanelVer;		// ���ڳ��ǳ� ����
	vector<int> incornerPanelHei;		// ���ڳ��ǳ� ����
	vector<int>	incornerPanelCount;		// �ش� ���ڳ��ǳ� ����/����/���� ���տ� ���� ����
	int	sizeOfIncornerPanelVectors;		// ���ڳ��ǳ� ������ ����

	// �ƿ��ڳ��ǳ�

	// �ƿ��ڳʾޱ�

	// ����

	// �ٷ������̼�

	// �簢������

	// ����

	// RS Push-Pull Props ����ǽ� (�ξ�� ����)

	// RS Push-Pull Props

	// ��ü Ÿ��

	// ����Ŭ����

	// �ɺ�Ʈ��Ʈ
};

void		initArray (double arr [], short arrSize);											// �迭 �ʱ�ȭ �Լ�
int			compare (const void* first, const void* second);									// ������������ ������ �� ����ϴ� ���Լ� (����Ʈ)
ColumnInfo	findColumn (ColumnPos* columnPos, short iHor, short iVer, short floorInd);			// �����ֿ�, �����ֿ�, �� ������ �̿��Ͽ� ��� ã��
GSErrCode	exportGridElementInfo (void);														// ����(���,��,������)���� ������ �����ϰ� �����ؼ� ���� ���Ϸ� ��������
short		DGCALLBACK inputThresholdHandler (short message, short dialogID, short item, DGUserData userData, DGMessageData msgData);		// [���̾�α�] ��� �� �ּ� ���� �Ÿ��� ����ڿ��� �Է� ���� (�⺻��: 2000 mm)
GSErrCode	exportSelectedElementInfo (void);													// ������ ���� ���� ��������

#endif
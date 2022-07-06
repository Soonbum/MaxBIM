#include "UtilityFunctions.hpp"


////////////////////////////////////////////////// ���� ��ȯ
// degree ������ radian ������ ��ȯ
double	DegreeToRad (double degree)
{
	return degree * M_PI / 180;
}

// radian ������ degree ������ ��ȯ
double	RadToDegree (double rad)
{
	return rad * 180 / M_PI;
}

////////////////////////////////////////////////// �Ÿ� ����
// 2�������� 2�� ���� �Ÿ��� �˷���
double	GetDistance (const double begX, const double begY, const double endX, const double endY)
{
	return sqrt ( (begX - endX) * (begX - endX) + (begY - endY) * (begY - endY) );
}

// 2�������� 2�� ���� �Ÿ��� �˷���
double	GetDistance (const API_Coord begPoint, API_Coord endPoint)
{
	return sqrt ( (begPoint.x - endPoint.x) * (begPoint.x - endPoint.x) + (begPoint.y - endPoint.y) * (begPoint.y - endPoint.y) );
}

// 3�������� 2�� ���� �Ÿ��� �˷���
double	GetDistance (const double begX, const double begY, const double begZ, const double endX, const double endY, const double endZ)
{
	return sqrt ( (begX - endX) * (begX - endX) + (begY - endY) * (begY - endY) + (begZ - endZ) * (begZ - endZ) );
}

// 3�������� 2�� ���� �Ÿ��� �˷���
double	GetDistance (const API_Coord3D begPoint, API_Coord3D endPoint)
{
	return sqrt ( (begPoint.x - endPoint.x) * (begPoint.x - endPoint.x) + (begPoint.y - endPoint.y) * (begPoint.y - endPoint.y) + (begPoint.z - endPoint.z) * (begPoint.z - endPoint.z) );
}

// ���� AB�� �� P���� �Ÿ��� ���ϴ� �Լ�
double	distOfPointBetweenLine (API_Coord p, API_Coord a, API_Coord b)
{
	double	area;
	double	dist_ab;

	area = abs ( (a.x - p.x) * (b.y - p.y) - (a.y - p.y) * (b.x - p.x) );
	dist_ab = sqrt ( (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) );

	return	area / dist_ab;
}

// ���� AB�� �� P���� �Ÿ��� ���ϴ� �Լ�
double	distOfPointBetweenLine (API_Coord3D p, API_Coord a, API_Coord b)
{
	double	area;
	double	dist_ab;

	area = abs ( (a.x - p.x) * (b.y - p.y) - (a.y - p.y) * (b.x - p.x) );
	dist_ab = sqrt ( (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) );

	return	area / dist_ab;
}

////////////////////////////////////////////////// ���ϱ�
// � ���� �� ū�� ���� : ����(-100), A<B(-1), A==B(0), A>B(+1)
long	compareDoubles (const double a, const double b)
{
	// ������ 0
	if ( (abs (a - b) < EPS) && (abs (b - a) < EPS) )
		return 0;

	// a > b�̸� 1
	if ( ((a - b) > 0) && abs (a - b) > EPS )
		return 1;

	// a < b�̸� -1
	if ( ((a - b) < 0) && abs (a - b) > EPS )
		return -1;

	return -100;
}

// a�� b�� �� �� ������ ���踦 �˷��� (0~13 �� �ϳ�)
long	compareRanges (double aMin, double aMax, double bMin, double bMax)
{
	/*
	 *			 Min  Max
	 *  A   :	[10]-[20]
	 *  		 Min  Max
	 *  B
	 *	0	:	����
	 *	1	:	[ 0]-[ 5]
	 *	2	:	[ 5]-[10]
	 *	3	:	[ 5]-[15]
	 *	4	:	[10]-[15]
	 *	5	:	[12]-[18]	IN
	 *	6	:	[15]-[20]
	 *	7	:	[15]-[25]
	 *	8	:	[20]-[25]
	 *	9	:	[25]-[30]
	 *	10	:	[ 5]-[20]
	 *	11	:	[10]-[25]
	 *	12	:	[10]-[20]
	 *	13	:	[ 5]-[25]
	 */
	
	long	result = 0;
	long	ab, aB, Ab, AB;
	bool	invertedA = false, invertedB = false;

	// a, b�� min, max ������ �ݴ��̸� ��������
	if ( aMin > aMax ) {
		exchangeDoubles (&aMin, &aMax);
		invertedA = true;
	}
	if ( bMin > bMax ) {
		exchangeDoubles (&bMin, &bMax);
		invertedB = true;
	}

	ab = compareDoubles (aMin, bMin);
	aB = compareDoubles (aMin, bMax);
	Ab = compareDoubles (aMax, bMin);
	AB = compareDoubles (aMax, bMax);

	if ( (ab  > 0) && (aB  > 0) && (Ab  > 0) && (AB  > 0) )		return 1;
	if ( (ab  > 0) && (aB == 0) && (Ab  > 0) && (AB  > 0) )		return 2;
	if ( (ab  > 0) && (aB  < 0) && (Ab  > 0) && (AB  > 0) )		return 3;
	if ( (ab == 0) && (aB  < 0) && (Ab  > 0) && (AB  > 0) )		return 4;
	if ( (ab  < 0) && (aB  < 0) && (Ab  > 0) && (AB  > 0) )		return 5;
	if ( (ab  < 0) && (aB  < 0) && (Ab  > 0) && (AB == 0) )		return 6;
	if ( (ab  < 0) && (aB  < 0) && (Ab  > 0) && (AB  < 0) )		return 7;
	if ( (ab  < 0) && (aB  < 0) && (Ab == 0) && (AB  < 0) )		return 8;
	if ( (ab  < 0) && (aB  < 0) && (Ab  < 0) && (AB  < 0) )		return 9;
	if ( (ab  > 0) && (aB  < 0) && (Ab  > 0) && (AB == 0) )		return 10;
	if ( (ab == 0) && (aB  < 0) && (Ab  > 0) && (AB  < 0) )		return 11;
	if ( (ab == 0) && (aB  < 0) && (Ab  > 0) && (AB == 0) )		return 12;
	if ( (ab  > 0) && (aB  < 0) && (Ab  > 0) && (AB  < 0) )		return 13;

	// �������� ���� ���� ���·� ����
	if (invertedA == true)	exchangeDoubles (&aMin, &aMax);
	if (invertedB == true)	exchangeDoubles (&bMin, &bMax);

	return	result;
}

// ���ڿ� ��
int		my_strcmp (const char *str1, const char *str2)
{
	for (; *str1 && (*str1 == *str2); ++str1, ++str2); 
		return *str1 - *str2;

	//while (*str1 != '\0' || *str2 != '\0') {

	//	*str1++;
	//	*str2++;

	//	if (*str1 == *str2)
	//		continue;
	//	else if (*str1 > *str2)
	//		return 1;
	//	else if (*str1 < *str2)
	//		return -1;
	//}

	//return 0;
}

////////////////////////////////////////////////// ��ȯ�ϱ�
// a�� b ���� ��ȯ��
void	exchangeDoubles (double* a, double* b)
{
	double buffer;

	buffer = *a;
	*a = *b;
	*b = buffer;

	return;
}

////////////////////////////////////////////////// ���� (�Ϲ�)
// ���������� �������� ���ϴ� ������ ������ Ȯ��
long	findDirection (const double begX, const double begY, const double endX, const double endY)
{
	double deltaX = endX - begX;
	double deltaX_abs = abs (deltaX);
	double deltaY = endY - begY;
	double deltaY_abs = abs (deltaY);

	/*
	if ( deltaX == 0 && deltaY == 0 )	return 0;
	if ( deltaX >  0 && deltaY == 0 )	return 1;
	if ( deltaX >  0 && deltaY >  0 )	return 2;
	if ( deltaX == 0 && deltaY >  0 )	return 3;
	if ( deltaX <  0 && deltaY >  0 )	return 4;
	if ( deltaX <  0 && deltaY == 0 )	return 5;
	if ( deltaX <  0 && deltaY <  0 )	return 6;
	if ( deltaX == 0 && deltaY <  0 )	return 7;
	if ( deltaX >  0 && deltaY <  0 )	return 8;
	*/

	if ((deltaX_abs	< EPS)	&&	(deltaY_abs	< EPS))		return 0;	// Point
	if ((deltaX		> EPS)	&&	(deltaY_abs	< EPS))		return 1;	// East-ward
	if ((deltaX		> EPS)	&&	(deltaY		> EPS))		return 2;	// Northeast-ward
	if ((deltaX_abs	< EPS)	&&	(deltaY		> EPS))		return 3;	// North-ward
	if ((deltaX		< -EPS)	&&	(deltaY		> EPS))		return 4;	// Northwest-ward
	if ((deltaX		< -EPS)	&&	(deltaY_abs	< EPS))		return 5;	// West-ward
	if ((deltaX		< -EPS)	&&	(deltaY		< -EPS))	return 6;	// Southwest-ward
	if ((deltaX_abs	< EPS)	&&	(deltaY		< -EPS))	return 7;	// South-ward
	if ((deltaX		> EPS)	&&	(deltaY		< -EPS))	return 8;	// Southeast-ward

	return -1;
}

// (p1, p2)�� ���� ������ (p3, p4)�� ���� ������ �������� ���ϴ� �Լ�
// Function to get intersection point with line connecting points (p1, p2) and another line (p3, p4).
API_Coord	IntersectionPoint1 (const API_Coord* p1, const API_Coord* p2, const API_Coord* p3, const API_Coord* p4)
{
	API_Coord ret;
	ret.x = ((p1->x*p2->y - p1->y*p2->x)*(p3->x - p4->x) - (p1->x - p2->x)*(p3->x*p4->y - p3->y*p4->x))/( (p1->x - p2->x)*(p3->y - p4->y) - (p1->y - p2->y)*(p3->x - p4->x) );
	ret.y = ((p1->x*p2->y - p1->y*p2->x)*(p3->y - p4->y) - (p1->y - p2->y)*(p3->x*p4->y - p3->y*p4->x)) / ( (p1->x - p2->x)*(p3->y - p4->y) - (p1->y - p2->y)*(p3->x - p4->x) );
	return	ret;
}

// y = m1*x + b1, y = m2*x + b2 �� ������ �������� ���ϴ� �Լ�
// Function to get intersection point of two lines y = m1*x + b1, y = m2*x + b2
API_Coord	IntersectionPoint2 (double m1, double b1, double m2, double b2)
{
	API_Coord ret;
	ret.x = (b2 - b1) / (m1 - m2);
	ret.y = m1*(b2 - b1)/(m1 - m2) + b1;    
	return	ret;
}

// a1*x + b1*y + c1 = 0, a2*x + b2*y + c2 = 0 �� ������ �������� ���ϴ� �Լ�
// Function to get intersection point of two lines a1*x + b1*y + c1 = 0, a2*x + b2*y + c2 = 0
API_Coord	IntersectionPoint3 (double a1, double b1, double c1, double a2, double b2, double c2)
{
	API_Coord ret;
	ret.x = (b1*c2 - b2*c1)/(a1*b2 - a2*b1);
	ret.y = -a1/b1*(b1*c2-b2*c1)/(a1*b2-a2*b1)-c1/b1;
	return	ret;
}

// aPoint�� bPoint�� ���� ������ Ȯ����
bool	isSamePoint (API_Coord3D aPoint, API_Coord3D bPoint)
{
	if ( (abs (aPoint.x - bPoint.x) < EPS) && (abs (aPoint.y - bPoint.y) < EPS) && (abs (aPoint.z - bPoint.z) < EPS) &&
		(abs (bPoint.x - aPoint.x) < EPS) && (abs (bPoint.y - aPoint.y) < EPS) && (abs (bPoint.z - aPoint.z) < EPS) ) {
		return true;
	} else
		return false;
}

// curPoint�� ����� ���� p1, p2 �� � ���Դϱ�?
short	moreCloserPoint (API_Coord3D curPoint, API_Coord3D p1, API_Coord3D p2)
{
	double dist1, dist2;

	dist1 = GetDistance (curPoint, p1);
	dist2 = GetDistance (curPoint, p2);

	// curPoint�� p1�� �� ������ 1 ����
	if ((dist2 - dist1) > EPS)	return 1;
	
	// curPoint�� p2�� �� ������ 2 ����
	if ((dist1 - dist2) > EPS)	return 2;

	// �� �ܿ��� 0 ����
	return 0;
}

////////////////////////////////////////////////// ���� (����)
// srcPoint ���� target ���� �ȿ� ��� �ִ°�?
bool	inRange (double srcPoint, double targetMin, double targetMax)
{
	if ((srcPoint > targetMin - EPS) && (srcPoint < targetMax + EPS))
		return true;
	else
		return false;
}

// src ������ target ������ ��ġ�� ���̸� ������
double	overlapRange (double srcMin, double srcMax, double targetMin, double targetMax)
{
	// srcMin�� srcMax �� �ϳ��� target ���� �ȿ� ���� ���
	if (inRange (srcMin, targetMin, targetMax) || inRange (srcMax, targetMin, targetMax)) {
		
		// srcMin�� srcMax�� ��� targetMin ~ targetMax �ȿ� �� �ִ� ���
		if (inRange (srcMin, targetMin, targetMax) && inRange (srcMax, targetMin, targetMax))
			return abs (srcMax - srcMin);
		
		// srcMin�� targetMin ~ targetMax �ȿ� �� �ִ� ���
		if (inRange (srcMin, targetMin, targetMax) && !inRange (srcMax, targetMin, targetMax))
			return abs (targetMax - srcMin);
		
		// srcMax�� targetMin ~ targetMax �ȿ� �� �ִ� ���
		if (!inRange (srcMin, targetMin, targetMax) && inRange (srcMax, targetMin, targetMax))
			return abs (srcMax - targetMin);
	}
	// srcMin�� targetMin���� �۰� srcMax�� targetMax���� ū ���
	if ((srcMin < targetMin + EPS) && (srcMax > targetMax - EPS)) {
		return abs (targetMax - targetMin);
	}
	// targetMin�� srcMin���� �۰� targetMax�� srcMax���� ū ���
	if ((targetMin < srcMin + EPS) && (targetMax > srcMax - EPS)) {
		return abs (srcMax - srcMin);
	}

	return 0.0;
}

////////////////////////////////////////////////// ���� (������ ����)
// aPoint�� pointList�� ������ �Ǿ����� Ȯ����
bool	isAlreadyStored (API_Coord3D aPoint, API_Coord3D pointList [], short startInd, short endInd)
{
	short	xx;

	for (xx = startInd ; xx <= endInd ; ++xx) {
		// ��� ��ǥ ���� ��ġ�� ���, �̹� ���Ե� ��ǥ ���̶�� ������
		if ( (abs (aPoint.x - pointList [xx].x) < EPS) && (abs (aPoint.y - pointList [xx].y) < EPS) && (abs (aPoint.z - pointList [xx].z) < EPS) &&
			(abs (pointList [xx].x - aPoint.x) < EPS) && (abs (pointList [xx].y - aPoint.y) < EPS) && (abs (pointList [xx].z - aPoint.z) < EPS) ) {
			return true;
		}
	}

	return false;
}

// nextPoint�� curPoint�� ���� ���Դϱ�?
bool	isNextPoint (API_Coord3D prevPoint, API_Coord3D curPoint, API_Coord3D nextPoint)
{
	bool	cond1 = false;
	bool	cond2_1 = false;
	bool	cond2_2 = false;

	// curPoint�� nextPoint�� ���� Z���� ���°�?
	if ( (abs (curPoint.z - nextPoint.z) < EPS) && (abs (nextPoint.z - curPoint.z) < EPS) )
		cond1 = true;

	// ���� ���� ���� ���� Y�� �� ���� ���, ���� ���� ���� ���� X�� �� �־�� �ϰ�, ���� ���� ���� �� ������ X���� ���̰� �־�� ��
	if ((abs (curPoint.x - prevPoint.x) < EPS) && (abs (prevPoint.x - curPoint.x) < EPS) &&
		(abs (curPoint.y - nextPoint.y) < EPS) && (abs (nextPoint.y - curPoint.y) < EPS) &&
		((abs (curPoint.x - nextPoint.x) > EPS) || (abs (nextPoint.x - curPoint.x) > EPS)))
		cond2_1 = true;

	// ���� ���� ���� ���� X�� �� ���� ���, ���� ���� ���� ���� Y�� �� �־�� �ϰ�, ���� ���� ���� �� ������ Y���� ���̰� �־�� ��
	if ((abs (curPoint.y - prevPoint.y) < EPS) && (abs (prevPoint.y - curPoint.y) < EPS) &&
		(abs (curPoint.x - nextPoint.x) < EPS) && (abs (nextPoint.x - curPoint.x) < EPS) &&
		((abs (curPoint.y - nextPoint.y) > EPS) || (abs (nextPoint.y - curPoint.y) > EPS)))
		cond2_2 = true;

	// ���� Z���̸鼭 ���� �� ���� ������ �Ÿ��� ���� ���
	if (cond1 && (cond2_1 || cond2_2))
		return true;
	else
		return false;
}

// ȸ���� ������� �ʾ��� ���� ��ġ (��ġ�Ǿ�� �� ���� ��ġ�� ����), ������ Degree
API_Coord3D		getUnrotatedPoint (API_Coord3D rotatedPoint, API_Coord3D axisPoint, double ang)
{
	API_Coord3D		unrotatedPoint;

	unrotatedPoint.x = axisPoint.x + ((rotatedPoint.x - axisPoint.x)*cos(DegreeToRad (ang)) - (rotatedPoint.y - axisPoint.y)*sin(DegreeToRad (ang)));
	unrotatedPoint.y = axisPoint.y + ((rotatedPoint.x - axisPoint.x)*sin(DegreeToRad (ang)) + (rotatedPoint.y - axisPoint.y)*cos(DegreeToRad (ang)));
	unrotatedPoint.z = rotatedPoint.z;

	return unrotatedPoint;
}

// ȸ���� ������� �ʾ��� ���� ��ġ (��ġ�Ǿ�� �� ���� ��ġ�� ����), ������ Degree
API_Coord	getUnrotatedPoint (API_Coord rotatedPoint, API_Coord axisPoint, double ang)
{
	API_Coord		unrotatedPoint;

	unrotatedPoint.x = axisPoint.x + ((rotatedPoint.x - axisPoint.x)*cos(DegreeToRad (ang))) - ((rotatedPoint.y - axisPoint.y)*sin(DegreeToRad (ang)));
	unrotatedPoint.y = axisPoint.y + ((rotatedPoint.x - axisPoint.x)*sin(DegreeToRad (ang))) + ((rotatedPoint.y - axisPoint.y)*cos(DegreeToRad (ang)));

	return unrotatedPoint;
}

////////////////////////////////////////////////// ���
// �ݿø�
double	round (double x, int digit)
{
	return	floor((x) * pow (float (10), digit) + 0.5f) / pow (float (10), digit);
}

////////////////////////////////////////////////// ���ڿ�
// std::string ���� ���� formatted string�� �Է� ����
std::string	format_string (const std::string fmt, ...)
{
	int			size = ((int)fmt.size ()) * 2;
	va_list		ap;
	std::string	buffer;

	while (1) {
		buffer.resize(size);
		va_start (ap, fmt);
		int n = vsnprintf ((char*)buffer.data (), size, fmt.c_str (), ap);
		va_end (ap);

		if (n > -1 && n < size) {
			buffer.resize (n);
			return buffer;
		}

		if (n > -1)
			size = n + 1;
		else
			size *= 2;
	}

	return buffer;
}

// ���ڿ� s�� ���ڷ� �� ���ڿ����� �˷��� (���ڴ� 1, ���ڿ��� 0)
short	isStringDouble (char *str)
{
	size_t size = strlen (str);
	
	if (size == 0)
		return 0;		// 0����Ʈ ���ڿ��� ���ڰ� �ƴ�
	
	for (size_t i = 0 ; i < size ; i++) {
		if (str [i] == '.' || str [i] == '-' || str [i] == '+')
			continue;		// .-+ ���ڴ� ������
		if (str [i] < '0' || str [i] > '9')
			return 0;		// ���ĺ� ���� ������ ���� �ƴ�
	}
	
	return 1;	// �׹��� ���� ����
}

// ���ڿ� s�� ���ڷ� �� ���ڿ����� �˷��� (���ڴ� 1, ���ڿ��� 0)
short	isStringDouble (const char *str)
{
	size_t size = strlen (str);
	
	if (size == 0)
		return 0;		// 0����Ʈ ���ڿ��� ���ڰ� �ƴ�
	
	for (size_t i = 0 ; i < size ; i++) {
		if (str [i] == '.' || str [i] == '-' || str [i] == '+')
			continue;		// .-+ ���ڴ� ������
		if (str [i] < '0' || str [i] > '9')
			return 0;		// ���ĺ� ���� ������ ���� �ƴ�
	}
	
	return 1;	// �׹��� ���� ����
}

// ���ڿ� str���� Ư�� ���� ch�� ������ (���ŵǸ� true, �״���̸� false)
bool	removeCharInStr (char *str, const char ch)
{
	int count = 0;
	int curIndex;

	int len = (int)strlen (str);

	curIndex = 0;
	while (curIndex < len) {
		if (str [curIndex] == ch)
			count ++;

		// ������ ���ڸ� ã���� �ϳ��� ������ ���
		if (count > 0)
			str [curIndex] = str [curIndex + 1];

		curIndex ++;

		// ���� ������ ���ڰ� 1������ ������ ������ ó������ �ٽ� ��ȸ�� ��
		if ((curIndex == len-1) && (count > 1))
			curIndex = 0;
	}

	if (count > 0)
		return true;
	else
		return false;
}

// ���ҽ� resID�� (1-���) index��° ���ڿ��� ������
char*	getResourceStr (short resID, short index)
{
	static char str [512];

	ACAPI_Resource_GetLocStr (str, resID, index, ACAPI_GetOwnResModule ());

	return str;
}

// char�� ���ڿ��� wchar_t�� ���ڿ��� ��ȯ
wchar_t*	charToWchar (const char *str)
{
	static wchar_t retStr [512];

	int strSize = MultiByteToWideChar (CP_ACP, 0, str, -1, NULL, NULL);

	MultiByteToWideChar (CP_ACP, 0, str, strlen(str)+1, retStr, strSize);

	return retStr;
}

// wchar_t�� ���ڿ��� char�� ���ڿ��� ��ȯ
char*		wcharToChar (const wchar_t *wstr)
{
	static char retStr [512];

	int strSize = WideCharToMultiByte (CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);

	WideCharToMultiByte (CP_ACP, 0, wstr, -1, retStr, strSize, 0, 0);

	return retStr;
}

////////////////////////////////////////////////// ��ü ��ġ
// ��ǥ ���� ��ġ��
GSErrCode	placeCoordinateLabel (double xPos, double yPos, double zPos, bool bComment, std::string comment, short layerInd, short floorInd)
{
	GSErrCode	err = NoError;
	API_Element			element;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	const	GS::uchar_t* gsmName = NULL;
	double	aParam;
	double	bParam;
	Int32	addParNum;

	// ���̺귯�� �̸� ����
	gsmName = L("��v1.0.gsm");

	// ��ü �ε�
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return err;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	BNZeroMemory (&element, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));

	element.header.typeID = API_ObjectID;
	element.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&element, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// ���̺귯���� �Ķ���� �� �Է�
	element.object.libInd = libPart.index;
	element.object.reflected = false;
	element.object.pos.x = xPos;
	element.object.pos.y = yPos;
	element.object.level = zPos;
	element.object.xRatio = aParam;
	element.object.yRatio = bParam;
	element.header.layer = layerInd;
	element.header.floorInd = floorInd;
	if (bComment == true)
		setParameterByName (&memo, "bComment", "1");
	else
		setParameterByName (&memo, "bComment", "0");
	
	char txtComment [256];
	strcpy (txtComment, comment.c_str ());
	setParameterByName (&memo, "txtComment", txtComment);

	// ��ü ��ġ
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	err;
}

// Ŭ����: ���� ��ü ��ġ - ������
EasyObjectPlacement::EasyObjectPlacement ()
{
}

// Ŭ����: ���� ��ü ��ġ - ������
EasyObjectPlacement::EasyObjectPlacement (const GS::uchar_t* gsmName, short layerInd, short floorInd, double posX = 0.0, double posY = 0.0, double posZ = 0.0, double radAng = 0.0)
{
	this->gsmName = gsmName;
	this->layerInd = layerInd;
	this->floorInd = floorInd;

	this->posX = posX;
	this->posY = posY;
	this->posZ = posZ;
	this->radAng = radAng;
}

// Ŭ����: ���� ��ü ��ġ - �ʱ�ȭ �Լ�
void		EasyObjectPlacement::init (const GS::uchar_t* gsmName, short layerInd, short floorInd, double posX, double posY, double posZ, double radAng)
{
	this->gsmName = gsmName;
	this->layerInd = layerInd;
	this->floorInd = floorInd;
	this->posX = posX;
	this->posY = posY;
	this->posZ = posZ;
	this->radAng = radAng;
}

// Ŭ����: ���� ��ü ��ġ - GSM ���ϸ� ����
void		EasyObjectPlacement::setGsmName (const GS::uchar_t* gsmName)
{
	this->gsmName = gsmName;
}

// Ŭ����: ���� ��ü ��ġ - ���̾� �ε��� ���� (1���� ����)
void		EasyObjectPlacement::setLayerInd (short layerInd)
{
	this->layerInd = layerInd;
}

// Ŭ����: ���� ��ü ��ġ - �� �ε��� ���� (����, 0, ��� ����)
void		EasyObjectPlacement::setFloorInd (short floorInd)
{
	this->floorInd = floorInd;
}

// Ŭ����: ���� ��ü ��ġ - �Ķ���� ����
void		EasyObjectPlacement::setParameters (short nParams, const char* paramNameList [], API_AddParID* paramTypeList, const char* paramValList [])
{
	// �Ķ���� ���� ����
	this->nParams = nParams;

	for (short xx = 0 ; xx < nParams ; ++xx) {
		this->paramName [xx] = paramNameList [xx];
		this->paramType [xx] = paramTypeList [xx];
		this->paramVal [xx] = paramValList [xx];
	}
}

// Ŭ����: ���� ��ü ��ġ - ��ü ��ġ
API_Guid	EasyObjectPlacement::placeObject (double posX, double posY, double posZ, double radAng)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double				aParam;
	double				bParam;
	Int32				addParNum;

	char				paramName [128];
	char				paramValStr [128];
	double				paramValDouble;
	
	// ��ü �ε�
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, this->gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	this->posX = posX;
	this->posY = posY;
	this->posZ = posZ;
	this->radAng = radAng;

	// ���̺귯���� �Ķ���� �� �Է�
	elem.object.libInd = libPart.index;
	elem.object.reflected = false;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.pos.x = posX;
	elem.object.pos.y = posY;
	elem.object.level = posZ;
	elem.object.angle = radAng;
	elem.header.floorInd = this->floorInd;	// �� �ε���
	elem.header.layer = this->layerInd;		// ���̾� �ε���

	for (short xx = 0 ; xx < this->nParams ; ++xx) {

		if ((this->paramType [xx] != APIParT_Separator) || (this->paramType [xx] != APIParT_Title) || (this->paramType [xx] != API_ZombieParT)) {
			if (this->paramType [xx] == APIParT_CString) {
				sprintf (paramName, "%s", this->paramName [xx]);
				sprintf (paramValStr, "%s", this->paramVal [xx]);
				setParameterByName (&memo, paramName, paramValStr);
			} else {
				sprintf (paramName, "%s", this->paramName [xx]);
				paramValDouble = atof (this->paramVal [xx]);
				setParameterByName (&memo, paramName, paramValDouble);
				if (my_strcmp (paramName, "A") == 0)	elem.object.xRatio = paramValDouble;
				if (my_strcmp (paramName, "B") == 0)	elem.object.yRatio = paramValDouble;
			}
		}
	}

	// ��ü ��ġ
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// Ŭ����: ���� ��ü ��ġ - ��ü ��ġ
API_Guid	EasyObjectPlacement::placeObject (const GS::uchar_t* gsmName, short nParams, const char* paramNameList [], API_AddParID* paramTypeList, const char* paramValList [], short layerInd, short floorInd, double posX, double posY, double posZ, double radAng)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double				aParam;
	double				bParam;
	Int32				addParNum;

	char				paramName [128];
	char				paramValStr [128];
	double				paramValDouble;
	
	// �Ķ���� ���� ����
	this->nParams = nParams;

	// ��ü �ε�
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	this->gsmName = gsmName;
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	this->posX = posX;
	this->posY = posY;
	this->posZ = posZ;
	this->radAng = radAng;

	// ���̺귯���� �Ķ���� �� �Է�
	elem.object.libInd = libPart.index;
	elem.object.reflected = false;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.pos.x = posX;
	elem.object.pos.y = posY;
	elem.object.level = posZ;
	elem.object.angle = radAng;
	this->floorInd = elem.header.floorInd = floorInd;	// �� �ε���
	this->layerInd = elem.header.layer = layerInd;		// ���̾� �ε���

	for (short xx = 0 ; xx < nParams ; ++xx) {

		this->paramName [xx] = paramNameList [xx];
		this->paramType [xx] = paramTypeList [xx];
		this->paramVal [xx] = paramValList [xx];

		if ((paramTypeList [xx] != APIParT_Separator) || (paramTypeList [xx] != APIParT_Title) || (paramTypeList [xx] != API_ZombieParT)) {
			if (paramTypeList [xx] == APIParT_CString) {
				sprintf (paramName, "%s", paramNameList [xx]);
				sprintf (paramValStr, "%s", paramValList [xx]);
				setParameterByName (&memo, paramName, paramValStr);
			} else {
				sprintf (paramName, "%s", paramNameList [xx]);
				paramValDouble = atof (paramValList [xx]);
				setParameterByName (&memo, paramName, paramValDouble);
				if (my_strcmp (paramName, "A") == 0)	elem.object.xRatio = paramValDouble;
				if (my_strcmp (paramName, "B") == 0)	elem.object.yRatio = paramValDouble;
			}
		}
	}

	// ��ü ��ġ
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// Ŭ����: ���� ��ü ��ġ - ��ü ��ġ (�Ķ������ ���� �� �Ķ���� �̸�/Ÿ��/���� �Է�)
API_Guid	EasyObjectPlacement::placeObject (short nParams, ...)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double				aParam;
	double				bParam;
	Int32				addParNum;

	char				paramName [128];
	char				paramValStr [128];
	double				paramValDouble;
	
	// �Ķ���� ���� ����
	this->nParams = nParams;

	// ��ü �ε�
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, this->gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// ���̺귯���� �Ķ���� �� �Է�
	elem.object.libInd = libPart.index;
	elem.object.reflected = false;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.pos.x = this->posX;
	elem.object.pos.y = this->posY;
	elem.object.level = this->posZ;
	elem.object.angle = this->radAng;
	elem.header.floorInd = this->floorInd;	// �� �ε���
	elem.header.layer = this->layerInd;		// ���̾� �ε���

	va_list ap;

	va_start (ap, nParams);

	for (short xx = 0 ; xx < this->nParams ; ++xx) {

		this->paramName [xx] = va_arg (ap, char*);
		this->paramType [xx] = va_arg (ap, API_AddParID);
		this->paramVal [xx] = va_arg (ap, char*);
		
		if ((this->paramType [xx] != APIParT_Separator) || (this->paramType [xx] != APIParT_Title) || (this->paramType [xx] != API_ZombieParT)) {
			if (this->paramType [xx] == APIParT_CString) {
				sprintf (paramName, "%s", this->paramName [xx]);
				sprintf (paramValStr, "%s", this->paramVal [xx]);
				setParameterByName (&memo, paramName, paramValStr);
			} else {
				sprintf (paramName, "%s", this->paramName [xx]);
				paramValDouble = atof (this->paramVal [xx]);
				setParameterByName (&memo, paramName, paramValDouble);
				if (my_strcmp (paramName, "A") == 0)	elem.object.xRatio = paramValDouble;
				if (my_strcmp (paramName, "B") == 0)	elem.object.yRatio = paramValDouble;
			}
		}
	}

	va_end (ap);

	// ��ü ��ġ
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// Ŭ����: ���� ��ü ��ġ - �ʱ�ȭ�� �ϴ� ���ÿ� ��ü ��ġ (���� �Ķ����: �Ķ������ ���� �� �Ķ���� �̸�/Ÿ��/���� �Է�)
API_Guid	EasyObjectPlacement::placeObject (const GS::uchar_t* gsmName, short layerInd, short floorInd, double posX, double posY, double posZ, double radAng, short nParams, ...)
{
	this->init (gsmName, layerInd, floorInd, posX, posY, posZ, radAng);

	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double				aParam;
	double				bParam;
	Int32				addParNum;

	char				paramName [128];
	char				paramValStr [128];
	double				paramValDouble;
	
	// �Ķ���� ���� ����
	this->nParams = nParams;

	// ��ü �ε�
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, this->gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// ���̺귯���� �Ķ���� �� �Է�
	elem.object.libInd = libPart.index;
	elem.object.reflected = false;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.pos.x = this->posX;
	elem.object.pos.y = this->posY;
	elem.object.level = this->posZ;
	elem.object.angle = this->radAng;
	elem.header.floorInd = this->floorInd;	// �� �ε���
	elem.header.layer = this->layerInd;		// ���̾� �ε���

	va_list ap;

	va_start (ap, nParams);

	for (short xx = 0 ; xx < this->nParams ; ++xx) {

		this->paramName [xx] = va_arg (ap, char*);
		this->paramType [xx] = va_arg (ap, API_AddParID);
		this->paramVal [xx] = va_arg (ap, char*);
		
		if ((this->paramType [xx] != APIParT_Separator) || (this->paramType [xx] != APIParT_Title) || (this->paramType [xx] != API_ZombieParT)) {
			if (this->paramType [xx] == APIParT_CString) {
				sprintf (paramName, "%s", this->paramName [xx]);
				sprintf (paramValStr, "%s", this->paramVal [xx]);
				setParameterByName (&memo, paramName, paramValStr);
			} else {
				sprintf (paramName, "%s", this->paramName [xx]);
				paramValDouble = atof (this->paramVal [xx]);
				setParameterByName (&memo, paramName, paramValDouble);
				if (my_strcmp (paramName, "A") == 0)	elem.object.xRatio = paramValDouble;
				if (my_strcmp (paramName, "B") == 0)	elem.object.yRatio = paramValDouble;
			}
		}
	}

	va_end (ap);

	// ��ü ��ġ
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// Ŭ����: ���� ��ü ��ġ - ��ü ��ġ
API_Guid	EasyObjectPlacement::placeObjectMirrored (double posX, double posY, double posZ, double radAng)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double				aParam;
	double				bParam;
	Int32				addParNum;

	char				paramName [128];
	char				paramValStr [128];
	double				paramValDouble;
	
	// ��ü �ε�
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, this->gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	this->posX = posX;
	this->posY = posY;
	this->posZ = posZ;
	this->radAng = radAng;

	// ���̺귯���� �Ķ���� �� �Է�
	elem.object.libInd = libPart.index;
	elem.object.reflected = true;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.pos.x = posX;
	elem.object.pos.y = posY;
	elem.object.level = posZ;
	elem.object.angle = radAng;
	elem.header.floorInd = this->floorInd;	// �� �ε���
	elem.header.layer = this->layerInd;		// ���̾� �ε���

	for (short xx = 0 ; xx < this->nParams ; ++xx) {

		if ((this->paramType [xx] != APIParT_Separator) || (this->paramType [xx] != APIParT_Title) || (this->paramType [xx] != API_ZombieParT)) {
			if (this->paramType [xx] == APIParT_CString) {
				sprintf (paramName, "%s", this->paramName [xx]);
				sprintf (paramValStr, "%s", this->paramVal [xx]);
				setParameterByName (&memo, paramName, paramValStr);
			} else {
				sprintf (paramName, "%s", this->paramName [xx]);
				paramValDouble = atof (this->paramVal [xx]);
				setParameterByName (&memo, paramName, paramValDouble);
				if (my_strcmp (paramName, "A") == 0)	elem.object.xRatio = paramValDouble;
				if (my_strcmp (paramName, "B") == 0)	elem.object.yRatio = paramValDouble;
			}
		}
	}

	// ��ü ��ġ
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// Ŭ����: ���� ��ü ��ġ - ��ü ��ġ
API_Guid	EasyObjectPlacement::placeObjectMirrored (const GS::uchar_t* gsmName, short nParams, const char* paramNameList [], API_AddParID* paramTypeList, const char* paramValList [], short layerInd, short floorInd, double posX, double posY, double posZ, double radAng)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double				aParam;
	double				bParam;
	Int32				addParNum;

	char				paramName [128];
	char				paramValStr [128];
	double				paramValDouble;
	
	// �Ķ���� ���� ����
	this->nParams = nParams;

	// ��ü �ε�
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	this->gsmName = gsmName;
	GS::ucscpy (libPart.file_UName, gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	this->posX = posX;
	this->posY = posY;
	this->posZ = posZ;
	this->radAng = radAng;

	// ���̺귯���� �Ķ���� �� �Է�
	elem.object.libInd = libPart.index;
	elem.object.reflected = true;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.pos.x = posX;
	elem.object.pos.y = posY;
	elem.object.level = posZ;
	elem.object.angle = radAng;
	this->floorInd = elem.header.floorInd = floorInd;	// �� �ε���
	this->layerInd = elem.header.layer = layerInd;		// ���̾� �ε���

	for (short xx = 0 ; xx < nParams ; ++xx) {

		this->paramName [xx] = paramNameList [xx];
		this->paramType [xx] = paramTypeList [xx];
		this->paramVal [xx] = paramValList [xx];

		if ((paramTypeList [xx] != APIParT_Separator) || (paramTypeList [xx] != APIParT_Title) || (paramTypeList [xx] != API_ZombieParT)) {
			if (paramTypeList [xx] == APIParT_CString) {
				sprintf (paramName, "%s", paramNameList [xx]);
				sprintf (paramValStr, "%s", paramValList [xx]);
				setParameterByName (&memo, paramName, paramValStr);
			} else {
				sprintf (paramName, "%s", paramNameList [xx]);
				paramValDouble = atof (paramValList [xx]);
				setParameterByName (&memo, paramName, paramValDouble);
				if (my_strcmp (paramName, "A") == 0)	elem.object.xRatio = paramValDouble;
				if (my_strcmp (paramName, "B") == 0)	elem.object.yRatio = paramValDouble;
			}
		}
	}

	// ��ü ��ġ
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// Ŭ����: ���� ��ü ��ġ - ��ü ��ġ (�Ķ������ ���� �� �Ķ���� �̸�/Ÿ��/���� �Է�)
API_Guid	EasyObjectPlacement::placeObjectMirrored (short nParams, ...)
{
	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double				aParam;
	double				bParam;
	Int32				addParNum;

	char				paramName [128];
	char				paramValStr [128];
	double				paramValDouble;
	
	// �Ķ���� ���� ����
	this->nParams = nParams;

	// ��ü �ε�
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, this->gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// ���̺귯���� �Ķ���� �� �Է�
	elem.object.libInd = libPart.index;
	elem.object.reflected = true;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.pos.x = this->posX;
	elem.object.pos.y = this->posY;
	elem.object.level = this->posZ;
	elem.object.angle = this->radAng;
	elem.header.floorInd = this->floorInd;	// �� �ε���
	elem.header.layer = this->layerInd;		// ���̾� �ε���

	va_list ap;

	va_start (ap, nParams);

	for (short xx = 0 ; xx < this->nParams ; ++xx) {

		this->paramName [xx] = va_arg (ap, char*);
		this->paramType [xx] = va_arg (ap, API_AddParID);
		this->paramVal [xx] = va_arg (ap, char*);
		
		if ((this->paramType [xx] != APIParT_Separator) || (this->paramType [xx] != APIParT_Title) || (this->paramType [xx] != API_ZombieParT)) {
			if (this->paramType [xx] == APIParT_CString) {
				sprintf (paramName, "%s", this->paramName [xx]);
				sprintf (paramValStr, "%s", this->paramVal [xx]);
				setParameterByName (&memo, paramName, paramValStr);
			} else {
				sprintf (paramName, "%s", this->paramName [xx]);
				paramValDouble = atof (this->paramVal [xx]);
				setParameterByName (&memo, paramName, paramValDouble);
				if (my_strcmp (paramName, "A") == 0)	elem.object.xRatio = paramValDouble;
				if (my_strcmp (paramName, "B") == 0)	elem.object.yRatio = paramValDouble;
			}
		}
	}

	va_end (ap);

	// ��ü ��ġ
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// Ŭ����: ���� ��ü ��ġ - �ʱ�ȭ�� �ϴ� ���ÿ� ��ü ��ġ (���� �Ķ����: �Ķ������ ���� �� �Ķ���� �̸�/Ÿ��/���� �Է�)
API_Guid	EasyObjectPlacement::placeObjectMirrored (const GS::uchar_t* gsmName, short layerInd, short floorInd, double posX, double posY, double posZ, double radAng, short nParams, ...)
{
	this->init (gsmName, layerInd, floorInd, posX, posY, posZ, radAng);

	GSErrCode	err = NoError;
	API_Element			elem;
	API_ElementMemo		memo;
	API_LibPart			libPart;

	double				aParam;
	double				bParam;
	Int32				addParNum;

	char				paramName [128];
	char				paramValStr [128];
	double				paramValDouble;
	
	// �Ķ���� ���� ����
	this->nParams = nParams;

	// ��ü �ε�
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy (libPart.file_UName, this->gsmName);
	err = ACAPI_LibPart_Search (&libPart, false);
	if (err != NoError)
		return elem.header.guid;
	if (libPart.location != NULL)
		delete libPart.location;

	ACAPI_LibPart_Get (&libPart);

	elem.header.typeID = API_ObjectID;
	elem.header.guid = GSGuid2APIGuid (GS::Guid (libPart.ownUnID));

	ACAPI_Element_GetDefaults (&elem, &memo);
	ACAPI_LibPart_GetParams (libPart.index, &aParam, &bParam, &addParNum, &memo.params);

	// ���̺귯���� �Ķ���� �� �Է�
	elem.object.libInd = libPart.index;
	elem.object.reflected = true;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.pos.x = this->posX;
	elem.object.pos.y = this->posY;
	elem.object.level = this->posZ;
	elem.object.angle = this->radAng;
	elem.header.floorInd = this->floorInd;	// �� �ε���
	elem.header.layer = this->layerInd;		// ���̾� �ε���

	va_list ap;

	va_start (ap, nParams);

	for (short xx = 0 ; xx < this->nParams ; ++xx) {

		this->paramName [xx] = va_arg (ap, char*);
		this->paramType [xx] = va_arg (ap, API_AddParID);
		this->paramVal [xx] = va_arg (ap, char*);
		
		if ((this->paramType [xx] != APIParT_Separator) || (this->paramType [xx] != APIParT_Title) || (this->paramType [xx] != API_ZombieParT)) {
			if (this->paramType [xx] == APIParT_CString) {
				sprintf (paramName, "%s", this->paramName [xx]);
				sprintf (paramValStr, "%s", this->paramVal [xx]);
				setParameterByName (&memo, paramName, paramValStr);
			} else {
				sprintf (paramName, "%s", this->paramName [xx]);
				paramValDouble = atof (this->paramVal [xx]);
				setParameterByName (&memo, paramName, paramValDouble);
				if (my_strcmp (paramName, "A") == 0)	elem.object.xRatio = paramValDouble;
				if (my_strcmp (paramName, "B") == 0)	elem.object.yRatio = paramValDouble;
			}
		}
	}

	va_end (ap);

	// ��ü ��ġ
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

////////////////////////////////////////////////// ���̺귯�� ���� ���� (Getter/Setter)
// pName �Ķ������ ���� value�� ������ (�Ǽ���) - �����ϸ� true, �����ϸ� false
bool	setParameterByName (API_ElementMemo* memo, char* pName, double value)
{
	const char*	retStr = NULL;
	int totalParams = BMGetHandleSize ((GSConstHandle)memo->params) / sizeof (API_AddParType);		// �Ű����� �� = �ڵ� ũ�� / ���� �ڵ� ũ��

	for (int xx = 0 ; xx < totalParams ; ++xx) {
		retStr = GS::UniString (memo->params [0][xx].name).ToCStr ().Get ();
		if (retStr != NULL) {
			if (GS::ucscmp (GS::UniString (memo->params [0][xx].name).ToUStr ().Get (), GS::UniString (pName).ToUStr ().Get ()) == 0) {
				memo->params [0][xx].value.real = value;
				return	true;
			}
		} else {
			return false;
		}
	}

	return	false;
}

// pName �Ķ������ ���� value�� ������ (���ڿ�) - �����ϸ� true, �����ϸ� false
bool	setParameterByName (API_ElementMemo* memo, char* pName, char* value)
{
	const char*	retStr = NULL;
	int totalParams = BMGetHandleSize ((GSConstHandle)memo->params) / sizeof(API_AddParType);		// �Ű����� �� = �ڵ� ũ�� / ���� �ڵ� ũ��

	for (int xx = 0 ; xx < totalParams ; ++xx) {
		retStr = GS::UniString (memo->params [0][xx].name).ToCStr ().Get ();
		if (retStr != NULL) {
			if (GS::ucscmp (GS::UniString (memo->params [0][xx].name).ToUStr ().Get (), GS::UniString (pName).ToUStr ().Get ()) == 0) {
				GS::ucscpy (memo->params [0][xx].value.uStr, GS::UniString (value).ToUStr ().Get ());
				return	true;
			}
		} else {
			return false;
		}
	}

	return	false;
}

// pName �Ķ������ ���� ������ (�Ǽ���)
double	getParameterValueByName (API_ElementMemo* memo, char* pName)
{
	const char*	retStr = NULL;
	int totalParams = BMGetHandleSize ((GSConstHandle)memo->params) / sizeof(API_AddParType);		// �Ű����� �� = �ڵ� ũ�� / ���� �ڵ� ũ��

	for (int xx = 0 ; xx < totalParams ; ++xx) {
		retStr = GS::UniString (memo->params [0][xx].name).ToCStr ().Get ();
		if (GS::ucscmp (GS::UniString (memo->params [0][xx].name).ToUStr ().Get (), GS::UniString (pName).ToUStr ().Get ()) == 0) {
			return memo->params [0][xx].value.real;
		}
	}

	return 0.0;
}

// pName �Ķ������ ���� ������ (���ڿ�)
const char*	getParameterStringByName (API_ElementMemo* memo, char* pName)
{
	const char*	retStr = NULL;
	int totalParams = BMGetHandleSize ((GSConstHandle)memo->params) / sizeof(API_AddParType);		// �Ű����� �� = �ڵ� ũ�� / ���� �ڵ� ũ��

	for (int xx = 0 ; xx < totalParams ; ++xx) {
		retStr = GS::UniString (memo->params [0][xx].name).ToCStr ().Get ();
		if (GS::ucscmp (GS::UniString (memo->params [0][xx].name).ToUStr ().Get (), GS::UniString (pName).ToUStr ().Get ()) == 0) {
			retStr = GS::UniString (memo->params [0][xx].value.uStr).ToCStr ().Get ();
			return retStr;
		}
	}

	retStr = "";
	return	retStr;
}

// pName �Ķ������ Ÿ���� ������ - API_AddParID
API_AddParID	getParameterTypeByName (API_ElementMemo* memo, char* pName)
{
	const char* retStr = NULL;
	int totalParams = BMGetHandleSize ((GSConstHandle)memo->params) / sizeof(API_AddParType);		// �Ű����� �� = �ڵ� ũ�� / ���� �ڵ� ũ��

	for (int xx = 0 ; xx < totalParams ; ++xx) {
		retStr = GS::UniString (memo->params [0][xx].name).ToCStr ().Get ();
		if (GS::ucscmp (GS::UniString (memo->params [0][xx].name).ToUStr ().Get (), GS::UniString (pName).ToUStr ().Get ()) == 0) {
			return memo->params [0][xx].typeID;
		}
	}

	return API_ZombieParT;
}

////////////////////////////////////////////////// ���� (�̵�)
// X, Y, Z�� ������ �����ϰ�, �ش� �������� �Ÿ��� �̵��� ��ǥ�� ������ (���� ���, ����: radian)
void		moveIn3D (char direction, double ang, double offset, API_Coord3D* curPos)
{
	if (direction == 'x' || direction == 'X') {
		curPos->x = curPos->x + (offset * cos(ang));
		curPos->y = curPos->y + (offset * sin(ang));
	}

	if (direction == 'y' || direction == 'Y') {
		curPos->x = curPos->x - (offset * sin(ang));
		curPos->y = curPos->y + (offset * cos(ang));
	}

	if (direction == 'z' || direction == 'Z') {
		curPos->z = curPos->z + offset;
	}
}

// X, Y, Z�� ������ �����ϰ�, �ش� �������� �Ÿ��� �̵��� ��ǥ�� ������ (���� ���, ����: radian)
void		moveIn3D (char direction, double ang, double offset, double* curX, double* curY, double* curZ)
{
	if (direction == 'x' || direction == 'X') {
		*curX = *curX + (offset * cos(ang));
		*curY = *curY + (offset * sin(ang));
	}

	if (direction == 'y' || direction == 'Y') {
		*curX = *curX - (offset * sin(ang));
		*curY = *curY + (offset * cos(ang));
	}

	if (direction == 'z' || direction == 'Z') {
		*curZ = *curZ + offset;
	}
}

// X, Y�� ������ �����ϰ�, �ش� �������� �Ÿ��� �̵��� ��ǥ�� ������ (���� ���, ����: radian)
void		moveIn2D (char direction, double ang, double offset, API_Coord* curPos)
{
	if (direction == 'x' || direction == 'X') {
		curPos->x = curPos->x + (offset * cos(ang));
		curPos->y = curPos->y + (offset * sin(ang));
	}

	if (direction == 'y' || direction == 'Y') {
		curPos->x = curPos->x - (offset * sin(ang));
		curPos->y = curPos->y + (offset * cos(ang));
	}
}

// X, Y�� ������ �����ϰ�, �ش� �������� �Ÿ��� �̵��� ��ǥ�� ������ (���� ���, ����: radian)
void		moveIn2D (char direction, double ang, double offset, double* curX, double* curY)
{
	if (direction == 'x' || direction == 'X') {
		*curX = *curX + (offset * cos(ang));
		*curY = *curY + (offset * sin(ang));
	}

	if (direction == 'y' || direction == 'Y') {
		*curX = *curX - (offset * sin(ang));
		*curY = *curY + (offset * cos(ang));
	}
}

// X, Y�� ������ �����ϰ�, �ش� �������� �Ÿ��� �̵��� ��ǥ�� ������ (��� �󿡼��� ȸ������ plainAng, ����� ��簢�� slopeAng, ����: radian)
void		moveIn3DSlope (char direction, double plainAng, double slopeAng, double offset, API_Coord3D* curPos)
{
	double projOffset = offset * cos (slopeAng);

	if (direction == 'x' || direction == 'X') {
		curPos->x = curPos->x + (projOffset * cos(plainAng));
		curPos->y = curPos->y + (projOffset * sin(plainAng));
	}

	if (direction == 'y' || direction == 'Y') {
		curPos->x = curPos->x - (projOffset * sin(plainAng));
		curPos->y = curPos->y + (projOffset * cos(plainAng));
	}

	curPos->z = curPos->z + offset * sin (slopeAng);
}

// X, Y�� ������ �����ϰ�, �ش� �������� �Ÿ��� �̵��� ��ǥ�� ������ (��� �󿡼��� ȸ������ plainAng, ����� ��簢�� slopeAng, ����: radian)
void		moveIn3DSlope (char direction, double plainAng, double slopeAng, double offset, double* curX, double* curY, double* curZ)
{
	double projOffset = offset * cos (slopeAng);

	if (direction == 'x' || direction == 'X') {
		*curX = *curX + (projOffset * cos(plainAng));
		*curY = *curY + (projOffset * sin(plainAng));
	}

	if (direction == 'y' || direction == 'Y') {
		*curX = *curX - (projOffset * sin(plainAng));
		*curY = *curY + (projOffset * cos(plainAng));
	}

	*curZ = *curZ + offset * sin (slopeAng);
}

////////////////////////////////////////////////// ���̾�
// ���̾� �̸����� ���̾� �ε��� ã��
short findLayerIndex (const char* layerName)
{
	short	nLayers;
	short	xx;
	GSErrCode	err;

	API_Attribute	attrib;

	// ������Ʈ �� ���̾� ������ �˾Ƴ�
	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.header.typeID = API_LayerID;
	err = ACAPI_Attribute_GetNum (API_LayerID, &nLayers);

	// �Է��� ���̾� �̸��� ��ġ�ϴ� ���̾��� �ε��� ��ȣ ����
	for (xx = 1; xx <= nLayers ; ++xx) {
		attrib.header.index = xx;
		err = ACAPI_Attribute_Get (&attrib);

		if (err == NoError) {
			if (my_strcmp (attrib.layer.head.name, layerName) == 0) {
				return	attrib.layer.head.index;
			}
		}
	}

	return 0;
}

// ��ü�� ���̾� �̸��� 01-S�� �����ϴ� ��� ���λ縦 05-T�� �ٲٰ�, ������ + ���̻� ���ڿ��� ���� ���̾� �̸��� ������ �� ���̾� �ε����� �̸��� ������
short	makeTemporaryLayer (API_Guid structurualObject, const char* suffix, char* returnedLayerName)
{
	GSErrCode	err = NoError;
	API_Element			elem;

	API_Attribute		attrib;
	API_AttributeDef	defs;

	char*				layerName;
	char				createdLayerName [128];

	BNZeroMemory (&elem, sizeof (API_Element));
	elem.header.guid = structurualObject;

	if (ACAPI_Element_Get (&elem) == NoError) {
		// ���� ��ü�� ���̾� �̸��� ����
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.header.typeID = API_LayerID;
		attrib.header.index = elem.header.layer;
		
		if (ACAPI_Attribute_Get (&attrib) == NoError) {
			// �� ���� ���̾� �̸��� 01-S�� �����ϴ°�?
			layerName = strstr (attrib.layer.head.name, "01-S");

			if (layerName == NULL)
				return 0;

			strncpy (layerName, "05-T", 4);		// ���λ縦 01-S���� 05-T�� ����
			if (suffix != NULL) {
				strcat (layerName, "-");			// ������ �ڿ� ����
				strcat (layerName, suffix);			// ���̻� �ڿ� ����
			}
			strcpy (createdLayerName, layerName);
			createdLayerName [strlen (createdLayerName)] = '\0';

			// ����� ���̾� �̸��� ������
			BNZeroMemory (&attrib, sizeof (API_Attribute));
			BNZeroMemory (&defs, sizeof (API_AttributeDef));
			
			attrib.header.typeID = API_LayerID;
			attrib.layer.conClassId = 1;
			CHCopyC (createdLayerName, attrib.header.name);
			err = ACAPI_Attribute_Create (&attrib, &defs);
			ACAPI_DisposeAttrDefsHdls (&defs);

			// ������ ���̾��� �ε����� ������
			if (err == NoError) {
				// ���̾� �̸� ����
				if (returnedLayerName != NULL)
					strcpy (returnedLayerName, createdLayerName);

				return	attrib.layer.head.index;

			} else {
				// �̹� �����Ǿ� ���� �� �����Ƿ� ���̾� �̸��� ã�ƺ� ��
				BNZeroMemory (&attrib, sizeof (API_Attribute));
				attrib.header.typeID = API_LayerID;
				CHCopyC (createdLayerName, attrib.header.name);
				err = ACAPI_Attribute_Get (&attrib);

				// ���̾� �̸� ����
				if (returnedLayerName != NULL)
					strcpy (returnedLayerName, createdLayerName);

				return	attrib.layer.head.index;
			}
		}
	}

	return 0;
}

////////////////////////////////////////////////// ���� ����
// ������ ��ҵ� �߿��� ��� ID�� elemType�� ��ü���� GUID�� ������, ������ ������ ������
GSErrCode	getGuidsOfSelection (GS::Array<API_Guid>* guidList, API_ElemTypeID elemType, long *nElem)
{
	short			xx;
	long			nSel;
	GSErrCode		err;

	API_SelectionInfo	selectionInfo;
	API_Element			tElem;
	API_Neig			**selNeigs;

	err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, true);
	BMKillHandle ((GSHandle *) &selectionInfo.marquee.coords);

	if (selectionInfo.typeID != API_SelEmpty) {
		nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
		for (xx = 0 ; xx < nSel && err == NoError ; ++xx) {
			tElem.header.typeID = Neig_To_ElemID ((*selNeigs)[xx].neigID);

			tElem.header.guid = (*selNeigs)[xx].guid;
			if (ACAPI_Element_Get (&tElem) != NoError)	// ������ �� �ִ� ����ΰ�?
				continue;

			if (tElem.header.typeID == elemType)
				guidList->Push (tElem.header.guid);
		}
	}
	BMKillHandle ((GSHandle *) &selNeigs);

	*nElem = guidList->GetSize ();

	return	err;

	// AC 25
	//short			xx;
	//long			nSel;
	//GSErrCode		err;

	//API_SelectionInfo	selectionInfo;
	//API_Element			tElem;
	//GS::Array<API_Neig> selNeigs;

	//err = ACAPI_Selection_Get(&selectionInfo, &selNeigs, true);
	//BMKillHandle((GSHandle*)&selectionInfo.marquee.coords);

	//if (selectionInfo.typeID != API_SelEmpty) {
	//	nSel = selNeigs.GetSize();
	//	for (xx = 0; xx < nSel && err == NoError; ++xx) {
	//		tElem.header.typeID = Neig_To_ElemID(selNeigs[xx].neigID);

	//		tElem.header.guid = selNeigs[xx].guid;
	//		if (ACAPI_Element_Get(&tElem) != NoError)	// ������ �� �ִ� ����ΰ�?
	//			continue;

	//		if (tElem.header.typeID == elemType)
	//			guidList->Push(tElem.header.guid);
	//	}
	//}

	//*nElem = guidList->GetSize();

	//return	err;
}

// �ش� ���� �۾� �� ���� ������
double		getWorkLevel (short floorInd)
{
	short			xx;
	API_StoryInfo	storyInfo;
	double			workLevel = 0.0;

	BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
	ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
	for (xx = 0 ; xx <= (storyInfo.lastStory - storyInfo.firstStory) ; ++xx) {
		if (storyInfo.data [0][xx].index == floorInd) {
			workLevel = storyInfo.data [0][xx].level;
			break;
		}
	}
	BMKillHandle ((GSHandle *) &storyInfo.data);

	return	workLevel;
}

// ���̾� ������ ������
short		getLayerCount ()
{
	API_Attribute	attrib;
	short			nLayers = 0;

	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.layer.head.typeID = API_LayerID;
	ACAPI_Attribute_GetNum (API_LayerID, &nLayers);

	return	nLayers;
}

// ������ ���������� �����ϰ� ������ ������ ������
long		getVerticesOfMorph (API_Guid guid, GS::Array<API_Coord3D>* coords)
{
	GSErrCode		err;
	API_Element		elem;
	API_ElemInfo3D	info3D;

	API_Component3D	component;
	API_Tranmat		tm;
	Int32			nVert, nEdge, nPgon;
	Int32			elemIdx, bodyIdx;
	API_Coord3D		trCoord;

	BNZeroMemory (&elem, sizeof (API_Element));
	elem.header.guid = guid;
	err = ACAPI_Element_Get (&elem);
	err = ACAPI_Element_Get3DInfo (elem.header, &info3D);

	// ������ �ƴϸ� ����
	if (elem.header.typeID != API_MorphID)
		return	0;

	// ������ 3D �ٵ� ������
	BNZeroMemory (&component, sizeof (API_Component3D));
	component.header.typeID = API_BodyID;
	component.header.index = info3D.fbody;
	err = ACAPI_3D_GetComponent (&component);

	nVert = component.body.nVert;
	nEdge = component.body.nEdge;
	nPgon = component.body.nPgon;
	tm = component.body.tranmat;
	elemIdx = component.body.head.elemIndex - 1;
	bodyIdx = component.body.head.bodyIndex - 1;

	// ���� ��ǥ�� ���� ������� ������
	for (short xx = 1 ; xx <= nVert ; ++xx) {
		component.header.typeID	= API_VertID;
		component.header.index	= xx;
		err = ACAPI_3D_GetComponent (&component);
		if (err == NoError) {
			trCoord.x = tm.tmx[0]*component.vert.x + tm.tmx[1]*component.vert.y + tm.tmx[2]*component.vert.z + tm.tmx[3];
			trCoord.y = tm.tmx[4]*component.vert.x + tm.tmx[5]*component.vert.y + tm.tmx[6]*component.vert.z + tm.tmx[7];
			trCoord.z = tm.tmx[8]*component.vert.x + tm.tmx[9]*component.vert.y + tm.tmx[10]*component.vert.z + tm.tmx[11];
			// ���� ���� ����
			if ( ((abs (trCoord.x - 0) < EPS) && (abs (trCoord.y - 0) < EPS) && (abs (trCoord.z - 0) < EPS)) ||
					((abs (trCoord.x - 1) < EPS) && (abs (trCoord.y - 0) < EPS) && (abs (trCoord.z - 0) < EPS)) ||
					((abs (trCoord.x - 0) < EPS) && (abs (trCoord.y - 1) < EPS) && (abs (trCoord.z - 0) < EPS)) ||
					((abs (trCoord.x - 0) < EPS) && (abs (trCoord.y - 0) < EPS) && (abs (trCoord.z - 1) < EPS)) )
					continue;
			coords->Push (trCoord);
		}
	}

	return	coords->GetSize ();
}

////////////////////////////////////////////////// ��� ����
// ����Ʈ�� �ִ� ��ҵ��� ��� ������
void	deleteElements (GS::Array<API_Element> elemList)
{
	short	xx;
	long	nElems = elemList.GetSize ();

	API_Elem_Head* headList = new API_Elem_Head [nElems];
	for (xx = 0 ; xx < nElems ; ++xx)
		headList [xx] = elemList [xx].header;

	ACAPI_Element_Delete (&headList, nElems);

	delete headList;

	// AC 25
	//short	xx;
	//long	nElems = elemList.GetSize();

	//GS::Array<API_Guid>	guidList;
	//for (xx = 0; xx < nElems; ++xx)
	//	guidList.Push(elemList[xx].header.guid);

	//ACAPI_Element_Delete(guidList);
}

// ����Ʈ�� �ִ� ��ҵ��� ��� ������
void	deleteElements (GS::Array<API_Guid> elemList)
{
	short	xx;
	API_Element		elem;
	long	nElems = elemList.GetSize ();

	API_Elem_Head* headList = new API_Elem_Head [nElems];
	for (xx = 0 ; xx < nElems ; ++xx) {
		BNZeroMemory (&elem, sizeof (API_Element));
		elem.header.guid = elemList [xx];
		ACAPI_Element_Get (&elem);

		headList [xx] = elem.header;
	}

	ACAPI_Element_Delete (&headList, nElems);

	delete headList;

	// AC 25
	// ACAPI_Element_Delete(elemList);
}
// ����Ʈ�� �ִ� ��ҵ��� �׷�ȭ
void	groupElements (GS::Array<API_Guid> elemList)
{
	if (!elemList.IsEmpty ()) {
		GSSize nElems = elemList.GetSize ();
		API_Elem_Head** elemHead = (API_Elem_Head **) BMAllocateHandle (nElems * sizeof (API_Elem_Head), ALLOCATE_CLEAR, 0);
		if (elemHead != NULL) {
			for (GSIndex i = 0; i < nElems; i++)
				(*elemHead)[i].guid = elemList[i];

			ACAPI_Element_Tool (elemHead, nElems, APITool_Group, NULL);

			BMKillHandle ((GSHandle *) &elemHead);
		}
	}

	// AC 25
	//ACAPI_Element_Tool(elemList, APITool_Group, NULL);
}

// ����Ʈ�� �ִ� ��ҵ��� ������
void	selectElements (GS::Array<API_Guid> elemList)
{
	short		xx;

	short		selCount;
	API_Neig**	selNeig;

	selCount = (short)elemList.GetSize ();
	selNeig = (API_Neig**)(BMAllocateHandle (selCount * sizeof (API_Neig), ALLOCATE_CLEAR, 0));
	for (xx = 0 ; xx < selCount ; ++xx)
		(*selNeig)[xx].guid = elemList [xx];

	ACAPI_Element_Select (selNeig, selCount, true);
	BMKillHandle (reinterpret_cast<GSHandle*> (&selNeig));

	return;

	// AC 25
	//short		xx;
	//short		selCount;
	//API_Neig	item;
	//GS::Array<API_Neig> selNeig;

	//for (xx = 0; xx < elemList.GetSize(); ++xx) {
	//	item.guid = elemList[xx];
	//	selNeig.Push(item);
	//}

	//ACAPI_Element_Select(selNeig, true);
}

// �׷�ȭ �Ͻ����� Ȱ��ȭ/��Ȱ��ȭ
void		suspendGroups (bool on)
{
	bool	suspGrp;

	ACAPI_Environment (APIEnv_IsSuspendGroupOnID, &suspGrp);	// �׷�ȭ �Ͻ����� ���� ���� ������

	if ((suspGrp == false) && (on == true))		ACAPI_Element_Tool (NULL, NULL, APITool_SuspendGroups, NULL);
	if ((suspGrp == true) && (on == false))		ACAPI_Element_Tool (NULL, NULL, APITool_SuspendGroups, NULL);
}
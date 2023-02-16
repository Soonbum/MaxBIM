#include "UtilityFunctions.hpp"
#include "Layers.hpp"


////////////////////////////////////////////////// 각도 변환
// degree 각도를 radian 각도로 변환
double	DegreeToRad (double degree)
{
	return degree * M_PI / 180;
}

// radian 각도를 degree 각도로 변환
double	RadToDegree (double rad)
{
	return rad * 180 / M_PI;
}

////////////////////////////////////////////////// 거리 측정
// 2차원에서 2점 간의 거리를 알려줌
double	GetDistance (const double begX, const double begY, const double endX, const double endY)
{
	return sqrt ( (begX - endX) * (begX - endX) + (begY - endY) * (begY - endY) );
}

// 2차원에서 2점 간의 거리를 알려줌
double	GetDistance (const API_Coord begPoint, API_Coord endPoint)
{
	return sqrt ( (begPoint.x - endPoint.x) * (begPoint.x - endPoint.x) + (begPoint.y - endPoint.y) * (begPoint.y - endPoint.y) );
}

// 3차원에서 2점 간의 거리를 알려줌
double	GetDistance (const double begX, const double begY, const double begZ, const double endX, const double endY, const double endZ)
{
	return sqrt ( (begX - endX) * (begX - endX) + (begY - endY) * (begY - endY) + (begZ - endZ) * (begZ - endZ) );
}

// 3차원에서 2점 간의 거리를 알려줌
double	GetDistance (const API_Coord3D begPoint, API_Coord3D endPoint)
{
	return sqrt ( (begPoint.x - endPoint.x) * (begPoint.x - endPoint.x) + (begPoint.y - endPoint.y) * (begPoint.y - endPoint.y) + (begPoint.z - endPoint.z) * (begPoint.z - endPoint.z) );
}

// 선분 AB와 점 P와의 거리를 구하는 함수
double	distOfPointBetweenLine (API_Coord p, API_Coord a, API_Coord b)
{
	double	area;
	double	dist_ab;

	area = abs ( (a.x - p.x) * (b.y - p.y) - (a.y - p.y) * (b.x - p.x) );
	dist_ab = sqrt ( (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) );

	return	area / dist_ab;
}

// 선분 AB와 점 P와의 거리를 구하는 함수
double	distOfPointBetweenLine (API_Coord3D p, API_Coord a, API_Coord b)
{
	double	area;
	double	dist_ab;

	area = abs ( (a.x - p.x) * (b.y - p.y) - (a.y - p.y) * (b.x - p.x) );
	dist_ab = sqrt ( (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y) );

	return	area / dist_ab;
}

////////////////////////////////////////////////// 비교하기
// 어떤 수가 더 큰지 비교함 : 오류(-100), A<B(-1), A==B(0), A>B(+1)
long	compareDoubles (const double a, const double b)
{
	// 같으면 0
	if ( (abs (a - b) < EPS) && (abs (b - a) < EPS) )
		return 0;

	// a > b이면 1
	if ( ((a - b) > 0) && abs (a - b) > EPS )
		return 1;

	// a < b이면 -1
	if ( ((a - b) < 0) && abs (a - b) > EPS )
		return -1;

	return -100;
}

// a와 b의 각 값 범위의 관계를 알려줌 (0~13 중 하나)
long	compareRanges (double aMin, double aMax, double bMin, double bMax)
{
	/*
	 *			 Min  Max
	 *  A   :	[10]-[20]
	 *  		 Min  Max
	 *  B
	 *	0	:	오류
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

	// a, b의 min, max 범위가 반대이면 뒤집어줌
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

	// 뒤집어준 값은 원래 상태로 복구
	if (invertedA == true)	exchangeDoubles (&aMin, &aMax);
	if (invertedB == true)	exchangeDoubles (&bMin, &bMax);

	return	result;
}

// 문자열 비교
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

////////////////////////////////////////////////// 교환하기
// a와 b 값을 교환함
void	exchangeDoubles (double* a, double* b)
{
	double buffer;

	buffer = *a;
	*a = *b;
	*b = buffer;

	return;
}

////////////////////////////////////////////////// 기하 (일반)
// 시작점에서 끝점으로 향하는 벡터의 방향을 확인
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

// (p1, p2)를 이은 직선과 (p3, p4)를 이은 직선의 교차점을 구하는 함수
// Function to get intersection point with line connecting points (p1, p2) and another line (p3, p4).
API_Coord	IntersectionPoint1 (const API_Coord* p1, const API_Coord* p2, const API_Coord* p3, const API_Coord* p4)
{
	API_Coord ret;
	ret.x = ((p1->x*p2->y - p1->y*p2->x)*(p3->x - p4->x) - (p1->x - p2->x)*(p3->x*p4->y - p3->y*p4->x))/( (p1->x - p2->x)*(p3->y - p4->y) - (p1->y - p2->y)*(p3->x - p4->x) );
	ret.y = ((p1->x*p2->y - p1->y*p2->x)*(p3->y - p4->y) - (p1->y - p2->y)*(p3->x*p4->y - p3->y*p4->x)) / ( (p1->x - p2->x)*(p3->y - p4->y) - (p1->y - p2->y)*(p3->x - p4->x) );
	return	ret;
}

// y = m1*x + b1, y = m2*x + b2 두 직선의 교차점을 구하는 함수
// Function to get intersection point of two lines y = m1*x + b1, y = m2*x + b2
API_Coord	IntersectionPoint2 (double m1, double b1, double m2, double b2)
{
	API_Coord ret;
	ret.x = (b2 - b1) / (m1 - m2);
	ret.y = m1*(b2 - b1)/(m1 - m2) + b1;    
	return	ret;
}

// a1*x + b1*y + c1 = 0, a2*x + b2*y + c2 = 0 두 직선의 교차점을 구하는 함수
// Function to get intersection point of two lines a1*x + b1*y + c1 = 0, a2*x + b2*y + c2 = 0
API_Coord	IntersectionPoint3 (double a1, double b1, double c1, double a2, double b2, double c2)
{
	API_Coord ret;
	ret.x = (b1*c2 - b2*c1)/(a1*b2 - a2*b1);
	ret.y = -a1/b1*(b1*c2-b2*c1)/(a1*b2-a2*b1)-c1/b1;
	return	ret;
}

// aPoint가 bPoint와 같은 점인지 확인함
bool	isSamePoint (API_Coord3D aPoint, API_Coord3D bPoint)
{
	if ( (abs (aPoint.x - bPoint.x) < EPS) && (abs (aPoint.y - bPoint.y) < EPS) && (abs (aPoint.z - bPoint.z) < EPS) &&
		(abs (bPoint.x - aPoint.x) < EPS) && (abs (bPoint.y - aPoint.y) < EPS) && (abs (bPoint.z - aPoint.z) < EPS) ) {
		return true;
	} else
		return false;
}

// curPoint에 가까운 점이 p1, p2 중 어떤 점입니까?
short	moreCloserPoint (API_Coord3D curPoint, API_Coord3D p1, API_Coord3D p2)
{
	double dist1, dist2;

	dist1 = GetDistance (curPoint, p1);
	dist2 = GetDistance (curPoint, p2);

	// curPoint와 p1가 더 가까우면 1 리턴
	if ((dist2 - dist1) > EPS)	return 1;
	
	// curPoint와 p2가 더 가까우면 2 리턴
	if ((dist1 - dist2) > EPS)	return 2;

	// 그 외에는 0 리턴
	return 0;
}

////////////////////////////////////////////////// 기하 (범위)
// srcPoint 값이 target 범위 안에 들어 있는가?
bool	inRange (double srcPoint, double targetMin, double targetMax)
{
	if ((srcPoint > targetMin - EPS) && (srcPoint < targetMax + EPS))
		return true;
	else
		return false;
}

// src 범위와 target 범위가 겹치는 길이를 리턴함
double	overlapRange (double srcMin, double srcMax, double targetMin, double targetMax)
{
	// srcMin과 srcMax 중 하나라도 target 범위 안에 들어가는 경우
	if (inRange (srcMin, targetMin, targetMax) || inRange (srcMax, targetMin, targetMax)) {
		
		// srcMin과 srcMax가 모두 targetMin ~ targetMax 안에 들어가 있는 경우
		if (inRange (srcMin, targetMin, targetMax) && inRange (srcMax, targetMin, targetMax))
			return abs (srcMax - srcMin);
		
		// srcMin만 targetMin ~ targetMax 안에 들어가 있는 경우
		if (inRange (srcMin, targetMin, targetMax) && !inRange (srcMax, targetMin, targetMax))
			return abs (targetMax - srcMin);
		
		// srcMax만 targetMin ~ targetMax 안에 들어가 있는 경우
		if (!inRange (srcMin, targetMin, targetMax) && inRange (srcMax, targetMin, targetMax))
			return abs (srcMax - targetMin);
	}
	// srcMin이 targetMin보다 작고 srcMax가 targetMax보다 큰 경우
	if ((srcMin < targetMin + EPS) && (srcMax > targetMax - EPS)) {
		return abs (targetMax - targetMin);
	}
	// targetMin이 srcMin보다 작고 targetMax가 srcMax보다 큰 경우
	if ((targetMin < srcMin + EPS) && (targetMax > srcMax - EPS)) {
		return abs (srcMax - srcMin);
	}

	return 0.0;
}

////////////////////////////////////////////////// 기하 (슬래브 관련)
// aPoint가 pointList에 보관이 되었는지 확인함
bool	isAlreadyStored (API_Coord3D aPoint, API_Coord3D pointList [], short startInd, short endInd)
{
	short	xx;

	for (xx = startInd ; xx <= endInd ; ++xx) {
		// 모든 좌표 값이 일치할 경우, 이미 포함된 좌표 값이라고 인정함
		if ( (abs (aPoint.x - pointList [xx].x) < EPS) && (abs (aPoint.y - pointList [xx].y) < EPS) && (abs (aPoint.z - pointList [xx].z) < EPS) &&
			(abs (pointList [xx].x - aPoint.x) < EPS) && (abs (pointList [xx].y - aPoint.y) < EPS) && (abs (pointList [xx].z - aPoint.z) < EPS) ) {
			return true;
		}
	}

	return false;
}

// nextPoint가 curPoint의 다음 점입니까?
bool	isNextPoint (API_Coord3D prevPoint, API_Coord3D curPoint, API_Coord3D nextPoint)
{
	bool	cond1 = false;
	bool	cond2_1 = false;
	bool	cond2_2 = false;

	// curPoint와 nextPoint가 같은 Z값을 갖는가?
	if ( (abs (curPoint.z - nextPoint.z) < EPS) && (abs (nextPoint.z - curPoint.z) < EPS) )
		cond1 = true;

	// 예전 점과 현재 점이 Y축 상에 있을 경우, 현재 점과 다음 점은 X축 상에 있어야 하고, 현재 점과 다음 점 간에는 X값의 차이가 있어야 함
	if ((abs (curPoint.x - prevPoint.x) < EPS) && (abs (prevPoint.x - curPoint.x) < EPS) &&
		(abs (curPoint.y - nextPoint.y) < EPS) && (abs (nextPoint.y - curPoint.y) < EPS) &&
		((abs (curPoint.x - nextPoint.x) > EPS) || (abs (nextPoint.x - curPoint.x) > EPS)))
		cond2_1 = true;

	// 예전 점과 현재 점이 X축 상에 있을 경우, 현재 점과 다음 점은 Y축 상에 있어야 하고, 현재 점과 다음 점 간에는 Y값의 차이가 있어야 함
	if ((abs (curPoint.y - prevPoint.y) < EPS) && (abs (prevPoint.y - curPoint.y) < EPS) &&
		(abs (curPoint.x - nextPoint.x) < EPS) && (abs (nextPoint.x - curPoint.x) < EPS) &&
		((abs (curPoint.y - nextPoint.y) > EPS) || (abs (nextPoint.y - curPoint.y) > EPS)))
		cond2_2 = true;

	// 같은 Z값이면서 동일 축 상의 떨어진 거리의 점인 경우
	if (cond1 && (cond2_1 || cond2_2))
		return true;
	else
		return false;
}

// 회전이 적용되지 않았을 때의 위치 (배치되어야 할 본래 위치를 리턴), 각도는 Degree
API_Coord3D		getUnrotatedPoint (API_Coord3D rotatedPoint, API_Coord3D axisPoint, double ang)
{
	API_Coord3D		unrotatedPoint;

	unrotatedPoint.x = axisPoint.x + ((rotatedPoint.x - axisPoint.x)*cos(DegreeToRad (ang)) - (rotatedPoint.y - axisPoint.y)*sin(DegreeToRad (ang)));
	unrotatedPoint.y = axisPoint.y + ((rotatedPoint.x - axisPoint.x)*sin(DegreeToRad (ang)) + (rotatedPoint.y - axisPoint.y)*cos(DegreeToRad (ang)));
	unrotatedPoint.z = rotatedPoint.z;

	return unrotatedPoint;
}

// 회전이 적용되지 않았을 때의 위치 (배치되어야 할 본래 위치를 리턴), 각도는 Degree
API_Coord	getUnrotatedPoint (API_Coord rotatedPoint, API_Coord axisPoint, double ang)
{
	API_Coord		unrotatedPoint;

	unrotatedPoint.x = axisPoint.x + ((rotatedPoint.x - axisPoint.x)*cos(DegreeToRad (ang))) - ((rotatedPoint.y - axisPoint.y)*sin(DegreeToRad (ang)));
	unrotatedPoint.y = axisPoint.y + ((rotatedPoint.x - axisPoint.x)*sin(DegreeToRad (ang))) + ((rotatedPoint.y - axisPoint.y)*cos(DegreeToRad (ang)));

	return unrotatedPoint;
}

////////////////////////////////////////////////// 산술
// 반올림
double	round (double x, int digit)
{
	return	floor((x) * pow (float (10), digit) + 0.5f) / pow (float (10), digit);
}

////////////////////////////////////////////////// 문자열
// std::string 변수 값에 formatted string을 입력 받음
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

// 문자열 s가 숫자로 된 문자열인지 알려줌 (숫자는 1, 문자열은 0)
short	isStringDouble (char *str)
{
	size_t size = strlen (str);
	
	if (size == 0)
		return 0;		// 0바이트 문자열은 숫자가 아님
	
	for (size_t i = 0 ; i < size ; i++) {
		if (str [i] == '.' || str [i] == '-' || str [i] == '+')
			continue;		// .-+ 문자는 무시함
		if (str [i] < '0' || str [i] > '9')
			return 0;		// 알파벳 등이 있으면 숫자 아님
	}
	
	return 1;	// 그밖의 경우는 숫자
}

// 문자열 s가 숫자로 된 문자열인지 알려줌 (숫자는 1, 문자열은 0)
short	isStringDouble (const char *str)
{
	size_t size = strlen (str);
	
	if (size == 0)
		return 0;		// 0바이트 문자열은 숫자가 아님
	
	for (size_t i = 0 ; i < size ; i++) {
		if (str [i] == '.' || str [i] == '-' || str [i] == '+')
			continue;		// .-+ 문자는 무시함
		if (str [i] < '0' || str [i] > '9')
			return 0;		// 알파벳 등이 있으면 숫자 아님
	}
	
	return 1;	// 그밖의 경우는 숫자
}

// 문자열 str에서 특정 문자 ch를 제거함 (제거되면 true, 그대로이면 false)
bool	removeCharInStr (char *str, const char ch)
{
	int count = 0;
	int curIndex;

	int len = (int)strlen (str);

	curIndex = 0;
	while (curIndex < len) {
		if (str [curIndex] == ch)
			count ++;

		// 제거할 문자를 찾으면 하나씩 앞으로 당김
		if (count > 0)
			str [curIndex] = str [curIndex + 1];

		curIndex ++;

		// 만약 제거할 문자가 1개보다 많으면 루프를 처음부터 다시 순회할 것
		if ((curIndex == len-1) && (count > 1))
			curIndex = 0;
	}

	if (count > 0)
		return true;
	else
		return false;
}

// 리소스 resID의 (1-기반) index번째 문자열을 가져옴
char*	getResourceStr (short resID, short index)
{
	static char str [512];

	ACAPI_Resource_GetLocStr (str, resID, index, ACAPI_GetOwnResModule ());

	return str;
}

// char형 문자열을 wchar_t형 문자열로 변환
wchar_t*	charToWchar (const char *str)
{
	static wchar_t retStr [512];

	int strSize = MultiByteToWideChar (CP_ACP, 0, str, -1, NULL, NULL);

	MultiByteToWideChar (CP_ACP, 0, str, (int)strlen(str)+1, retStr, strSize);

	return retStr;
}

// wchar_t형 문자열을 char형 문자열로 변환
char*		wcharToChar (const wchar_t *wstr)
{
	static char retStr [512];

	int strSize = WideCharToMultiByte (CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);

	WideCharToMultiByte (CP_ACP, 0, wstr, -1, retStr, strSize, 0, 0);

	return retStr;
}


// C 문자열을 GS::UniString 문자열로 변환
GS::UniString	convertStr(const char* c_str)
{
	static GS::UniString	retStr;

	retStr = charToWchar(c_str);

	return retStr;
}

// GS::UniString 문자열을 C 문자열로 변환
char* convertStr(const GS::UniString uni_str)
{
	static char retStr[512];

	const char* convertedStr = uni_str.ToCStr().Get();

	sprintf(retStr, "%s", convertedStr);

	return retStr;
}

////////////////////////////////////////////////// 객체 배치
// 좌표 라벨을 배치함
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

	// 라이브러리 이름 선택
	gsmName = L("라벨v1.0.gsm");

	// 객체 로드
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy(libPart.file_UName, GS::UniString(gsmName).ToUStr().Get());
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

	// 라이브러리의 파라미터 값 입력
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

	// 객체 배치
	ACAPI_Element_Create (&element, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	err;
}

// 클래스: 쉬운 객체 배치 - 생성자
EasyObjectPlacement::EasyObjectPlacement ()
{
}

// 클래스: 쉬운 객체 배치 - 생성자
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

// 클래스: 쉬운 객체 배치 - 초기화 함수
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

// 클래스: 쉬운 객체 배치 - GSM 파일명 지정
void		EasyObjectPlacement::setGsmName (const GS::uchar_t* gsmName)
{
	this->gsmName = gsmName;
}

// 클래스: 쉬운 객체 배치 - 레이어 인덱스 지정 (1부터 시작)
void		EasyObjectPlacement::setLayerInd (short layerInd)
{
	this->layerInd = layerInd;
}

// 클래스: 쉬운 객체 배치 - 층 인덱스 지정 (음수, 0, 양수 가능)
void		EasyObjectPlacement::setFloorInd (short floorInd)
{
	this->floorInd = floorInd;
}

// 클래스: 쉬운 객체 배치 - 파라미터 지정
void		EasyObjectPlacement::setParameters (short nParams, const char* paramNameList [], API_AddParID* paramTypeList, const char* paramValList [])
{
	// 파라미터 개수 저장
	this->nParams = nParams;

	for (short xx = 0 ; xx < nParams ; ++xx) {
		this->paramName [xx] = paramNameList [xx];
		this->paramType [xx] = paramTypeList [xx];
		this->paramVal [xx] = paramValList [xx];
	}
}

// 클래스: 쉬운 객체 배치 - 객체 배치
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
	
	// 객체 로드
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy(libPart.file_UName, GS::UniString(this->gsmName).ToUStr().Get());
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

	// 라이브러리의 파라미터 값 입력
	elem.object.libInd = libPart.index;
	elem.object.reflected = false;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.pos.x = posX;
	elem.object.pos.y = posY;
	elem.object.level = posZ;
	elem.object.angle = radAng;
	elem.header.floorInd = this->floorInd;	// 층 인덱스
	elem.header.layer = this->layerInd;		// 레이어 인덱스

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

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// 클래스: 쉬운 객체 배치 - 객체 배치
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
	
	// 파라미터 개수 저장
	this->nParams = nParams;

	// 객체 로드
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	this->gsmName = gsmName;
	GS::ucscpy(libPart.file_UName, GS::UniString(gsmName).ToUStr().Get());
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

	// 라이브러리의 파라미터 값 입력
	elem.object.libInd = libPart.index;
	elem.object.reflected = false;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.pos.x = posX;
	elem.object.pos.y = posY;
	elem.object.level = posZ;
	elem.object.angle = radAng;
	this->floorInd = elem.header.floorInd = floorInd;	// 층 인덱스
	this->layerInd = elem.header.layer = layerInd;		// 레이어 인덱스

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

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// 클래스: 쉬운 객체 배치 - 객체 배치 (파라미터의 개수 및 파라미터 이름/타입/값만 입력)
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
	
	// 파라미터 개수 저장
	this->nParams = nParams;

	// 객체 로드
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy(libPart.file_UName, GS::UniString(this->gsmName).ToUStr().Get());
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

	// 라이브러리의 파라미터 값 입력
	elem.object.libInd = libPart.index;
	elem.object.reflected = false;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.pos.x = this->posX;
	elem.object.pos.y = this->posY;
	elem.object.level = this->posZ;
	elem.object.angle = this->radAng;
	elem.header.floorInd = this->floorInd;	// 층 인덱스
	elem.header.layer = this->layerInd;		// 레이어 인덱스

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

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// 클래스: 쉬운 객체 배치 - 초기화를 하는 동시에 객체 배치 (가변 파라미터: 파라미터의 개수 및 파라미터 이름/타입/값만 입력)
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
	
	// 파라미터 개수 저장
	this->nParams = nParams;

	// 객체 로드
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy(libPart.file_UName, GS::UniString(this->gsmName).ToUStr().Get());
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

	// 라이브러리의 파라미터 값 입력
	elem.object.libInd = libPart.index;
	elem.object.reflected = false;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.pos.x = this->posX;
	elem.object.pos.y = this->posY;
	elem.object.level = this->posZ;
	elem.object.angle = this->radAng;
	elem.header.floorInd = this->floorInd;	// 층 인덱스
	elem.header.layer = this->layerInd;		// 레이어 인덱스

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

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// 클래스: 쉬운 객체 배치 - 객체 배치
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
	
	// 객체 로드
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy(libPart.file_UName, GS::UniString(this->gsmName).ToUStr().Get());
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

	// 라이브러리의 파라미터 값 입력
	elem.object.libInd = libPart.index;
	elem.object.reflected = true;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.pos.x = posX;
	elem.object.pos.y = posY;
	elem.object.level = posZ;
	elem.object.angle = radAng;
	elem.header.floorInd = this->floorInd;	// 층 인덱스
	elem.header.layer = this->layerInd;		// 레이어 인덱스

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

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// 클래스: 쉬운 객체 배치 - 객체 배치
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
	
	// 파라미터 개수 저장
	this->nParams = nParams;

	// 객체 로드
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	this->gsmName = gsmName;
	GS::ucscpy(libPart.file_UName, GS::UniString(gsmName).ToUStr().Get());
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

	// 라이브러리의 파라미터 값 입력
	elem.object.libInd = libPart.index;
	elem.object.reflected = true;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.pos.x = posX;
	elem.object.pos.y = posY;
	elem.object.level = posZ;
	elem.object.angle = radAng;
	this->floorInd = elem.header.floorInd = floorInd;	// 층 인덱스
	this->layerInd = elem.header.layer = layerInd;		// 레이어 인덱스

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

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// 클래스: 쉬운 객체 배치 - 객체 배치 (파라미터의 개수 및 파라미터 이름/타입/값만 입력)
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
	
	// 파라미터 개수 저장
	this->nParams = nParams;

	// 객체 로드
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy(libPart.file_UName, GS::UniString(this->gsmName).ToUStr().Get());
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

	// 라이브러리의 파라미터 값 입력
	elem.object.libInd = libPart.index;
	elem.object.reflected = true;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.pos.x = this->posX;
	elem.object.pos.y = this->posY;
	elem.object.level = this->posZ;
	elem.object.angle = this->radAng;
	elem.header.floorInd = this->floorInd;	// 층 인덱스
	elem.header.layer = this->layerInd;		// 레이어 인덱스

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

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

// 클래스: 쉬운 객체 배치 - 초기화를 하는 동시에 객체 배치 (가변 파라미터: 파라미터의 개수 및 파라미터 이름/타입/값만 입력)
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
	
	// 파라미터 개수 저장
	this->nParams = nParams;

	// 객체 로드
	BNZeroMemory (&elem, sizeof (API_Element));
	BNZeroMemory (&memo, sizeof (API_ElementMemo));
	BNZeroMemory (&libPart, sizeof (libPart));
	GS::ucscpy(libPart.file_UName, GS::UniString(this->gsmName).ToUStr().Get());
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

	// 라이브러리의 파라미터 값 입력
	elem.object.libInd = libPart.index;
	elem.object.reflected = true;
	elem.object.xRatio = aParam;
	elem.object.yRatio = bParam;
	elem.object.pos.x = this->posX;
	elem.object.pos.y = this->posY;
	elem.object.level = this->posZ;
	elem.object.angle = this->radAng;
	elem.header.floorInd = this->floorInd;	// 층 인덱스
	elem.header.layer = this->layerInd;		// 레이어 인덱스

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

	// 객체 배치
	ACAPI_Element_Create (&elem, &memo);
	ACAPI_DisposeElemMemoHdls (&memo);

	return	elem.header.guid;
}

////////////////////////////////////////////////// 라이브러리 변수 접근 (Getter/Setter)
// pName 파라미터의 값을 value로 설정함 (실수형) - 성공하면 true, 실패하면 false
bool	setParameterByName (API_ElementMemo* memo, char* pName, double value)
{
	const char*	retStr = NULL;
	int totalParams = BMGetHandleSize ((GSConstHandle)memo->params) / sizeof (API_AddParType);		// 매개변수 수 = 핸들 크기 / 단일 핸들 크기

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

// pName 파라미터의 값을 value로 설정함 (문자열) - 성공하면 true, 실패하면 false
bool	setParameterByName (API_ElementMemo* memo, char* pName, char* value)
{
	const char*	retStr = NULL;
	int totalParams = BMGetHandleSize ((GSConstHandle)memo->params) / sizeof(API_AddParType);		// 매개변수 수 = 핸들 크기 / 단일 핸들 크기

	for (int xx = 0 ; xx < totalParams ; ++xx) {
		retStr = GS::UniString (memo->params [0][xx].name).ToCStr ().Get ();
		if (retStr != NULL) {
			if (GS::ucscmp (GS::UniString (memo->params [0][xx].name).ToUStr ().Get (), GS::UniString (pName).ToUStr ().Get ()) == 0) {
				GS::ucscpy(memo->params[0][xx].value.uStr, GS::UniString(convertStr(value)).ToUStr().Get());
				return	true;
			}
		} else {
			return false;
		}
	}

	return	false;
}

// pName 파라미터의 값을 가져옴 (실수형)
double	getParameterValueByName (API_ElementMemo* memo, char* pName)
{
	const char*	retStr = NULL;
	int totalParams = BMGetHandleSize ((GSConstHandle)memo->params) / sizeof(API_AddParType);		// 매개변수 수 = 핸들 크기 / 단일 핸들 크기

	for (int xx = 0 ; xx < totalParams ; ++xx) {
		retStr = GS::UniString (memo->params [0][xx].name).ToCStr ().Get ();
		if (GS::ucscmp (GS::UniString (memo->params [0][xx].name).ToUStr ().Get (), GS::UniString (pName).ToUStr ().Get ()) == 0) {
			return memo->params [0][xx].value.real;
		}
	}

	return 0.0;
}

// pName 파라미터의 값을 가져옴 (문자열)
const char*	getParameterStringByName (API_ElementMemo* memo, char* pName)
{
	const char*	retStr = NULL;
	int totalParams = BMGetHandleSize ((GSConstHandle)memo->params) / sizeof(API_AddParType);		// 매개변수 수 = 핸들 크기 / 단일 핸들 크기

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

// pName 파라미터의 타입을 가져옴 - API_AddParID
API_AddParID	getParameterTypeByName (API_ElementMemo* memo, char* pName)
{
	const char* retStr = NULL;
	int totalParams = BMGetHandleSize ((GSConstHandle)memo->params) / sizeof(API_AddParType);		// 매개변수 수 = 핸들 크기 / 단일 핸들 크기

	for (int xx = 0 ; xx < totalParams ; ++xx) {
		retStr = GS::UniString (memo->params [0][xx].name).ToCStr ().Get ();
		if (GS::ucscmp (GS::UniString (memo->params [0][xx].name).ToUStr ().Get (), GS::UniString (pName).ToUStr ().Get ()) == 0) {
			return memo->params [0][xx].typeID;
		}
	}

	return API_ZombieParT;
}

////////////////////////////////////////////////// 기하 (이동)
// X, Y, Z축 방향을 선택하고, 해당 방향으로 거리를 이동한 좌표를 리턴함 (각도 고려, 단위: radian)
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

// X, Y, Z축 방향을 선택하고, 해당 방향으로 거리를 이동한 좌표를 리턴함 (각도 고려, 단위: radian)
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

// X, Y축 방향을 선택하고, 해당 방향으로 거리를 이동한 좌표를 리턴함 (각도 고려, 단위: radian)
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

// X, Y축 방향을 선택하고, 해당 방향으로 거리를 이동한 좌표를 리턴함 (각도 고려, 단위: radian)
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

// X, Y축 방향을 선택하고, 해당 방향으로 거리를 이동한 좌표를 리턴함 (평면 상에서의 회전각도 plainAng, 평면의 경사각도 slopeAng, 단위: radian)
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

// X, Y축 방향을 선택하고, 해당 방향으로 거리를 이동한 좌표를 리턴함 (평면 상에서의 회전각도 plainAng, 평면의 경사각도 slopeAng, 단위: radian)
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

////////////////////////////////////////////////// 레이어
// 레이어 이름으로 레이어 인덱스 찾기
short findLayerIndex (const char* layerName)
{
	short	nLayers;
	short	xx;
	GSErrCode	err;

	API_Attribute	attrib;

	// 프로젝트 내 레이어 개수를 알아냄
	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.header.typeID = API_LayerID;
	err = ACAPI_Attribute_GetNum (API_LayerID, &nLayers);

	// 입력한 레이어 이름과 일치하는 레이어의 인덱스 번호 리턴
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

// 객체의 레이어 이름이 01-S로 시작하는 경우 접두사를 05-T로 바꾸고, 하이픈 + 접미사 문자열을 붙인 레이어 이름을 생성한 후 레이어 인덱스와 이름을 리턴함
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
		// 구조 객체의 레이어 이름을 구함
		BNZeroMemory (&attrib, sizeof (API_Attribute));
		attrib.header.typeID = API_LayerID;
		attrib.header.index = elem.header.layer;
		
		if (ACAPI_Attribute_Get (&attrib) == NoError) {
			// 맨 앞의 레이어 이름이 01-S로 시작하는가?
			layerName = strstr (attrib.layer.head.name, "01-S");

			if (layerName == NULL)
				return 0;

			strncpy (layerName, "05-T", 4);		// 접두사를 01-S에서 05-T로 변경
			if (suffix != NULL) {
				strcat (layerName, "-");			// 하이픈 뒤에 붙임
				strcat (layerName, suffix);			// 접미사 뒤에 붙임
			}
			strcpy (createdLayerName, layerName);
			createdLayerName [strlen (createdLayerName)] = '\0';

			// 변경된 레이어 이름을 생성함
			BNZeroMemory (&attrib, sizeof (API_Attribute));
			BNZeroMemory (&defs, sizeof (API_AttributeDef));
			
			attrib.header.typeID = API_LayerID;
			attrib.layer.conClassId = 1;
			CHCopyC (createdLayerName, attrib.header.name);
			err = ACAPI_Attribute_Create (&attrib, &defs);
			ACAPI_DisposeAttrDefsHdls (&defs);

			// 생성된 레이어의 인덱스를 리턴함
			if (err == NoError) {
				// 레이어 이름 리턴
				if (returnedLayerName != NULL)
					strcpy (returnedLayerName, createdLayerName);

				return	attrib.layer.head.index;

			} else {
				// 이미 생성되어 있을 수 있으므로 레이어 이름을 찾아볼 것
				BNZeroMemory (&attrib, sizeof (API_Attribute));
				attrib.header.typeID = API_LayerID;
				CHCopyC (createdLayerName, attrib.header.name);
				err = ACAPI_Attribute_Get (&attrib);

				// 레이어 이름 리턴
				if (returnedLayerName != NULL)
					strcpy (returnedLayerName, createdLayerName);

				return	attrib.layer.head.index;
			}
		}
	}

	return 0;
}

// 레이어 코드에 대한 설명을 리턴함
char*	getExplanationOfLayerCode (char *layerName, bool bConstructionCode, bool bDong, bool bFloor, bool bCastNum, bool bCJ, bool bOrderInCJ, bool bObjName, bool bProductSite, bool bProductNum)
{
	static char retStr [512];
	char layerNameM [256];

	LayerNameSystem	layerInfo;	// layer.csv 파일로부터 레이어 코드 정보 입력

	short	i, xx, yy;
	string	insElem;		// 토큰을 string으로 변환해서 vector로 넣음
	char	*token;			// 읽어온 문자열의 토큰

	short	nBaseFields;
	short	nExtendFields;
	bool	success;
	bool	extSuccess;
	char	tempStr [128];
	char	tok1 [32];
	char	tok2 [32];
	char	tok3 [32];
	char	tok4 [32];
	char	tok5 [32];
	char	tok6 [32];
	char	tok7 [32];
	char	tok8 [32];
	char	tok9 [32];
	char	tok10 [32];
	char	constructionCode [8];

	strcpy (retStr, "");

	strcpy (layerNameM, layerName);

	// 레이어 정보 파일 가져오기
	importLayerInfo (&layerInfo);

	// 구조체 초기화
	allocateMemory (&layerInfo);

	strcpy (tok1, "");
	strcpy (tok2, "");
	strcpy (tok3, "");
	strcpy (tok4, "");
	strcpy (tok5, "");
	strcpy (tok6, "");
	strcpy (tok7, "");
	strcpy (tok8, "");
	strcpy (tok9, "");
	i = 1;
	success = false;
	extSuccess = false;
	nBaseFields = 0;
	nExtendFields = 0;
	// 예시(기본): 05-T-0000-F01-01-01-01-WALL
	// 예시(확장): 05-T-0000-F01-01-01-01-WALL-현장제작-001
	// 레이어 이름을 "-" 문자 기준으로 쪼개기
	token = strtok (layerNameM, "-");
	while (token != NULL) {
		// 내용 및 길이 확인
		// 1차 (일련번호) - 필수 (2글자, 숫자)
		if (i == 1) {
			strcpy (tempStr, token);
			if (strlen (tempStr) == 2) {
				strcpy (tok1, tempStr);
				success = true;
				nBaseFields ++;
			} else {
				i = 100;
				success = false;
			}
		}
		// 2차 (공사 구분) - 필수 (1글자, 문자)
		else if (i == 2) {
			strcpy (tempStr, token);
			if (strlen (tempStr) == 1) {
				strcpy (tok2, tempStr);
				success = true;
				nBaseFields ++;
			} else {
				i = 100;
				success = false;
			}
		}
		// 3차 (동 구분) - 필수 (4글자)
		else if (i == 3) {
			strcpy (tempStr, token);
			if (isStringDouble (tempStr) == TRUE) {
				// 숫자인 경우
				sprintf (tok3, "%04d", atoi (tempStr));
			} else {
				// 문자열인 경우
				strcpy (tok3, tempStr);
			}
			success = true;
			nBaseFields ++;
		}
		// 4차 (층 구분) - 필수 (3글자)
		else if (i == 4) {
			strcpy (tempStr, token);
			if (strlen (tempStr) == 3) {
				strcpy (tok4, tempStr);
				success = true;
				nBaseFields ++;
			} else {
				i = 100;
				success = false;
			}
		}
		// 5차 (타설번호) - 필수 (2글자, 숫자)
		else if (i == 5) {
			strcpy (tempStr, token);
			if (isStringDouble (tempStr) == TRUE) {
				// 숫자인 경우
				sprintf (tok5, "%02d", atoi (tempStr));
			} else {
				// 문자열인 경우
				strcpy (tok5, tempStr);
			}
			success = true;
			nBaseFields ++;
		}
		// 6차 (CJ 구간) - 필수 (2글자, 숫자)
		else if (i == 6) {
			strcpy (tempStr, token);
			if (isStringDouble (tempStr) == TRUE) {
				// 숫자인 경우
				sprintf (tok6, "%02d", atoi (tempStr));
			} else {
				// 문자열인 경우
				strcpy (tok6, tempStr);
			}
			success = true;
			nBaseFields ++;
		}
		// 7차 (CJ 속 시공순서) - 필수 (2글자, 숫자)
		else if (i == 7) {
			strcpy (tempStr, token);
			if (isStringDouble (tempStr) == TRUE) {
				// 숫자인 경우
				sprintf (tok7, "%02d", atoi (tempStr));
			} else {
				// 문자열인 경우
				strcpy (tok7, tempStr);
			}
			success = true;
			nBaseFields ++;
		}
		// 8차 (부재 구분) - 필수 (3글자 이상)
		else if (i == 8) {
			strcpy (tempStr, token);
			if (strlen (tempStr) >= 3) {
				strcpy (tok8, tempStr);
				success = true;
				nBaseFields ++;
			} else {
				i = 100;
				success = false;
			}
		}
		// 9차 (제작처 구분) - 선택 (한글 4글자..)
		else if (i == 9) {
			strcpy (tempStr, token);
			if (strlen (tempStr) >= 4) {
				strcpy (tok9, tempStr);
				extSuccess = true;
				nExtendFields ++;
			} else {
				i = 100;
				extSuccess = false;
			}
		}
		// 10차 (제작 번호) - 필수 (3글자, 숫자)
		else if (i == 10) {
			strcpy (tempStr, token);
			if (isStringDouble (tempStr) == TRUE) {
				// 숫자인 경우
				sprintf (tok10, "%03d", atoi (tempStr));
			} else {
				// 문자열인 경우
				strcpy (tok10, tempStr);
			}
			extSuccess = true;
			nExtendFields ++;
		}
		++i;
		token = strtok (NULL, "-");
	}

	// 8단계 또는 10단계까지 성공적으로 완료되면 구조체에 적용
	if ((success == true) && (nBaseFields == 8)) {
		// 일련 번호와 공사 구분 문자를 먼저 합침
		sprintf (constructionCode, "%s-%s", tok1, tok2);
				
		// 1,2단계. 공사 구분 확인
		for (yy = 0 ; yy < layerInfo.code_name.size () ; ++yy) {
			if (strncmp (constructionCode, layerInfo.code_name [yy].c_str (), 4) == 0) {
				layerInfo.code_state [yy] = true;
			}
			layerInfo.bCodeAllShow = true;
		}

		// 3단계. 동 구분
		for (yy = 0 ; yy < layerInfo.dong_name.size () ; ++yy) {
			if (strncmp (tok3, layerInfo.dong_name [yy].c_str (), 4) == 0) {
				layerInfo.dong_state [yy] = true;
			}
			layerInfo.bDongAllShow = true;
		}

		// 4단계. 층 구분
		for (yy = 0 ; yy < layerInfo.floor_name.size () ; ++yy) {
			if (strncmp (tok4, layerInfo.floor_name [yy].c_str (), 3) == 0) {
				layerInfo.floor_state [yy] = true;
			}
			layerInfo.bFloorAllShow = true;
		}

		// 5단계. 타설번호
		for (yy = 0 ; yy < layerInfo.cast_name.size () ; ++yy) {
			if (my_strcmp (tok5, layerInfo.cast_name [yy].c_str ()) == 0) {
				layerInfo.cast_state [yy] = true;
			}
			layerInfo.bCastAllShow = true;
		}

		// 6단계. CJ 구간
		for (yy = 0 ; yy < layerInfo.CJ_name.size () ; ++yy) {
			if (my_strcmp (tok6, layerInfo.CJ_name [yy].c_str ()) == 0) {
				layerInfo.CJ_state [yy] = true;
			}
			layerInfo.bCJAllShow = true;
		}

		// 7단계. CJ 속 시공순서
		for (yy = 0 ; yy < layerInfo.orderInCJ_name.size () ; ++yy) {
			if (my_strcmp (tok7, layerInfo.orderInCJ_name [yy].c_str ()) == 0) {
				layerInfo.orderInCJ_state [yy] = true;
			}
			layerInfo.bOrderInCJAllShow = true;
		}

		// 8단계. 부재 구분
		for (yy = 0 ; yy < layerInfo.obj_name.size () ; ++yy) {
			if ((strncmp (constructionCode, layerInfo.obj_cat [yy].c_str (), 4) == 0) && (strncmp (tok8, layerInfo.obj_name [yy].c_str (), 5) == 0)) {
				layerInfo.obj_state [yy] = true;
			}
		}

		if ((extSuccess == true) && (nExtendFields == 2)) {
			// 9단계. 제작처 구분
			for (yy = 0 ; yy < layerInfo.productSite_name.size () ; ++yy) {
				if (my_strcmp (tok9, layerInfo.productSite_name [yy].c_str ()) == 0) {
					layerInfo.productSite_state [yy] = true;
				}
			}

			// 10단계. 제작 번호
			for (yy = 0 ; yy < layerInfo.productNum_name.size () ; ++yy) {
				if (strncmp (tok10, layerInfo.productNum_name [yy].c_str (), 3) == 0) {
					layerInfo.productNum_state [yy] = true;
				}
				layerInfo.bProductNumAllShow = true;
			}
		}
	}

	if ((success == false) || (extSuccess == false)) {
		sprintf (retStr, "");
		return retStr;
	}

	if (success == true) {
		// 공사구분
		if (bConstructionCode == true) {
			for (xx = 0 ; xx < layerInfo.code_name.size () ; ++xx) {
				sprintf (tempStr, "%s-%s", tok1, tok2);
				if (my_strcmp (layerInfo.code_name [xx].c_str (), tempStr) == 0) {
					sprintf (tempStr, "%s ", layerInfo.code_desc [xx].c_str ());
					strcat (retStr, tempStr);
					break;
				}
			}
		}

		// 동
		if (bDong == true) {
			for (xx = 0 ; xx < layerInfo.dong_name.size () ; ++xx) {
				if (my_strcmp (layerInfo.dong_name [xx].c_str (), tok3) == 0) {
					sprintf (tempStr, "[%s]동 ", layerInfo.dong_desc [xx].c_str ());
					strcat (retStr, tempStr);
					break;
				}
			}
		}

		// 층
		if (bFloor == true) {
			for (xx = 0 ; xx < layerInfo.floor_name.size () ; ++xx) {
				if (my_strcmp (layerInfo.floor_name [xx].c_str (), tok4) == 0) {
					sprintf (tempStr, "%s ", layerInfo.floor_desc [xx].c_str ());
					strcat (retStr, tempStr);
					break;
				}
			}
		}

		// 타설번호
		if (bCastNum == true) {
			for (xx = 0 ; xx < layerInfo.cast_name.size () ; ++xx) {
				if (my_strcmp (layerInfo.cast_name [xx].c_str (), tok5) == 0) {
					sprintf (tempStr, "타설번호[%s] ", layerInfo.cast_name [xx].c_str ());
					strcat (retStr, tempStr);
					break;
				}
			}
		}

		// CJ
		if (bCJ == true) {
			for (xx = 0 ; xx < layerInfo.CJ_name.size () ; ++xx) {
				if (my_strcmp (layerInfo.CJ_name [xx].c_str (), tok6) == 0) {
					sprintf (tempStr, "CJ[%s] ", layerInfo.CJ_name [xx].c_str ());
					strcat (retStr, tempStr);
					break;
				}
			}
		}

		// 시공순서
		if (bOrderInCJ == true) {
			for (xx = 0 ; xx < layerInfo.orderInCJ_name.size () ; ++xx) {
				if (my_strcmp (layerInfo.orderInCJ_name [xx].c_str (), tok7) == 0) {
					sprintf (tempStr, "시공순서[%s] ", layerInfo.orderInCJ_name [xx].c_str ());
					strcat (retStr, tempStr);
					break;
				}
			}
		}

		// 부재
		if (bObjName == true) {
			for (xx = 0 ; xx < layerInfo.obj_name.size () ; ++xx) {
				if (my_strcmp (layerInfo.obj_name [xx].c_str (), tok8) == 0) {
					sprintf (tempStr, "%s ", layerInfo.obj_desc [xx].c_str ());
					strcat (retStr, tempStr);
					break;
				}
			}
		}
	}

	if (extSuccess == true) {
		// 제작처
		if (bProductSite == true) {
			for (xx = 0 ; xx < layerInfo.productSite_name.size () ; ++xx) {
				if (my_strcmp (layerInfo.productSite_name [xx].c_str (), tok9) == 0) {
					sprintf (tempStr, "제작처[%s] ", layerInfo.productSite_name [xx].c_str ());
					strcat (retStr, tempStr);
					break;
				}
			}
		}

		// 제작번호
		if (bProductNum == true) {
			for (xx = 0 ; xx < layerInfo.productNum_name.size () ; ++xx) {
				if (my_strcmp (layerInfo.productNum_name [xx].c_str (), tok10) == 0) {
					sprintf (tempStr, "제작번호[%s] ", layerInfo.productNum_name [xx].c_str ());
					strcat (retStr, tempStr);
					break;
				}
			}
		}
	}

	// 메모리 해제
	deallocateMemory (&layerInfo);

	return retStr;
}

////////////////////////////////////////////////// 정보 수집
// 선택한 요소들 중에서 요소 ID가 elemType인 객체들의 GUID를 가져옴, 가져온 수량을 리턴함
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
			if (ACAPI_Element_Get (&tElem) != NoError)	// 가져올 수 있는 요소인가?
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
	//		if (ACAPI_Element_Get(&tElem) != NoError)	// 가져올 수 있는 요소인가?
	//			continue;

	//		if (tElem.header.typeID == elemType)
	//			guidList->Push(tElem.header.guid);
	//	}
	//}

	//*nElem = guidList->GetSize();

	//return	err;
}

// 해당 층의 작업 층 고도를 가져옴
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

// 레이어 개수를 가져옴
short		getLayerCount ()
{
	API_Attribute	attrib;
	short			nLayers = 0;

	BNZeroMemory (&attrib, sizeof (API_Attribute));
	attrib.layer.head.typeID = API_LayerID;
	ACAPI_Attribute_GetNum (API_LayerID, &nLayers);

	return	nLayers;
}

// 모프의 꼭지점들을 수집하고 꼭지점 개수를 리턴함
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

	// 모프가 아니면 종료
	if (elem.header.typeID != API_MorphID)
		return	0;

	// 모프의 3D 바디를 가져옴
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

	// 정점 좌표를 임의 순서대로 저장함
	for (short xx = 1 ; xx <= nVert ; ++xx) {
		component.header.typeID	= API_VertID;
		component.header.index	= xx;
		err = ACAPI_3D_GetComponent (&component);
		if (err == NoError) {
			trCoord.x = tm.tmx[0]*component.vert.x + tm.tmx[1]*component.vert.y + tm.tmx[2]*component.vert.z + tm.tmx[3];
			trCoord.y = tm.tmx[4]*component.vert.x + tm.tmx[5]*component.vert.y + tm.tmx[6]*component.vert.z + tm.tmx[7];
			trCoord.z = tm.tmx[8]*component.vert.x + tm.tmx[9]*component.vert.y + tm.tmx[10]*component.vert.z + tm.tmx[11];
			// 단위 벡터 생략
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

////////////////////////////////////////////////// 요소 조작
// 리스트에 있는 요소들을 모두 삭제함
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

// 리스트에 있는 요소들을 모두 삭제함
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

// 리스트에 있는 요소들을 그룹화
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

// 리스트에 있는 요소들을 선택함
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

// 그룹화 일시정지 활성화/비활성화
void		suspendGroups (bool on)
{
	bool	suspGrp;

	ACAPI_Environment (APIEnv_IsSuspendGroupOnID, &suspGrp);	// 그룹화 일시정지 상태 여부 가져옴

	if ((suspGrp == false) && (on == true))		ACAPI_Element_Tool (NULL, NULL, APITool_SuspendGroups, NULL);
	if ((suspGrp == true) && (on == false))		ACAPI_Element_Tool (NULL, NULL, APITool_SuspendGroups, NULL);

	// AC 25
	//bool	suspGrp;
	//GS::Array<API_Guid>	elemList;

	//ACAPI_Environment(APIEnv_IsSuspendGroupOnID, &suspGrp);	// 그룹화 일시정지 상태 여부 가져옴

	//if ((suspGrp == false) && (on == true))		ACAPI_Element_Tool(elemList, APITool_SuspendGroups, NULL);
	//if ((suspGrp == true) && (on == false))		ACAPI_Element_Tool(elemList, APITool_SuspendGroups, NULL);
}
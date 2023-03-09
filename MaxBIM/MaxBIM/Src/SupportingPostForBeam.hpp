#ifndef	__SUPPORTING_POST_FOR_BEAM__
#define __SUPPORTING_POST_FOR_BEAM__

#include "MaxBIM.hpp"

namespace SupportingPostForBeam {
	enum DIALOG {
		LABEL_GIRDER_LENGTH = 3,
		POPUP_GIRDER_LENGTH,
		LABEL_MRK_TYPE_1,
		POPUP_MRK_TYPE_1,
		LABEL_MRK_TYPE_2,
		POPUP_MRK_TYPE_2,
		LABEL_MRK_TYPE_3,
		POPUP_MRK_TYPE_3,
		LABEL_FLOOR,
		POPUP_FLOOR
	};

	enum GIRDER_LENGTH {
		L_1500 = 1,
		L_1800,
		L_2100,
		L_2400,
		L_2700,
		L_3000,
		L_3300,
		L_3600,
		L_3900,
		L_4200,
		L_4500
	};

	enum MRK_TYPE {
		TYPE_NONE = 1,
		TYPE_900x900,
		TYPE_900x1200,
		TYPE_900x2300,
		TYPE_1200x1200,
		TYPE_1200x2300
	};

	struct SupportingPostPlacingZone {
		API_Coord3D	oriPos;		// 원점
		double		radAng;		// Radian 각도
		short		floorInd;	// 설치층 인덱스

		GS::UniString	girder_length;				// 멍에제 길이 (호칭 치수)
		GS::UniString	girder_real_length;			// 멍에제 길이 (실제 치수)
		GS::UniString	MRK_Type_1_perpendicular;	// 보와 수직 방향
		GS::UniString	MRK_Type_1_parallel;		// 보와 평행 방향
		GS::UniString	MRK_Type_2_perpendicular;	// 보와 수직 방향
		GS::UniString	MRK_Type_2_parallel;		// 보와 평행 방향
		GS::UniString	MRK_Type_3_perpendicular;	// 보와 수직 방향
		GS::UniString	MRK_Type_3_parallel;		// 보와 평행 방향
	} supportingPostPlacingZone;

	enum LAYER_SETTINGS_DG {
		ICON_LAYER_CUSTOM = 3,
		LABEL_LAYER_SETTINGS,
		CHECKBOX_LAYER_COUPLING,

		LABEL_LAYER_PERI_SUPPORT,
		LABEL_LAYER_MRK,
		LABEL_LAYER_GT24,
		LABEL_LAYER_BEAM,
		LABEL_LAYER_JOIN,
		LABEL_LAYER_6,
		LABEL_LAYER_7,
		LABEL_LAYER_8,
		LABEL_LAYER_9,
		LABEL_LAYER_10,
		LABEL_LAYER_11,
		LABEL_LAYER_12,
		LABEL_LAYER_13,
		LABEL_LAYER_14,
		LABEL_LAYER_15,
		LABEL_LAYER_16,
		LABEL_LAYER_17,
		LABEL_LAYER_18,
		LABEL_LAYER_19,
		LABEL_LAYER_20,
		LABEL_LAYER_21,
		LABEL_LAYER_22,

		USERCONTROL_LAYER_PERI_SUPPORT,
		USERCONTROL_LAYER_MRK,
		USERCONTROL_LAYER_GT24,
		USERCONTROL_LAYER_BEAM,
		USERCONTROL_LAYER_JOIN,
		USERCONTROL_LAYER_6,
		USERCONTROL_LAYER_7,
		USERCONTROL_LAYER_8,
		USERCONTROL_LAYER_9,
		USERCONTROL_LAYER_10,
		USERCONTROL_LAYER_11,
		USERCONTROL_LAYER_12,
		USERCONTROL_LAYER_13,
		USERCONTROL_LAYER_14,
		USERCONTROL_LAYER_15,
		USERCONTROL_LAYER_16,
		USERCONTROL_LAYER_17,
		USERCONTROL_LAYER_18,
		USERCONTROL_LAYER_19,
		USERCONTROL_LAYER_20,
		USERCONTROL_LAYER_21,
		USERCONTROL_LAYER_22,

		BUTTON_AUTOSET
	};

	int	BEAM_GAP = 900;

	short	layerInd_PERI_Support;
	short	layerInd_MRK;
	short	layerInd_GT24;
	short	layerInd_Sanseunggak;
	short	layerInd_Join;

	GS::Array<API_Guid>	elemList;	// 그룹화를 위해 생성된 결과물들의 GUID를 전부 저장함

	// 부재 배치 함수: PERI 동바리
	void placePERISupports ()
	{
		EasyObjectPlacement PERI;
		PERI.init (L("PERI동바리 수직재 v0.1.gsm"), layerInd_PERI_Support, supportingPostPlacingZone.floorInd, supportingPostPlacingZone.oriPos.x, supportingPostPlacingZone.oriPos.y, supportingPostPlacingZone.oriPos.z, supportingPostPlacingZone.radAng);

		bool	bMRK1 = false;
		bool	bMRK2 = false;
		bool	bMRK3 = false;
		int		nMRKSets = 0;

		double	occupied_len = 0.0;
		double	gap;

		double	parallel_len;
		double	perpendicular_len;

		if (atoi (supportingPostPlacingZone.MRK_Type_1_parallel.ToCStr ().Get ()) != 0) {
			bMRK1 = true;
			nMRKSets++;
		}
		if (atoi (supportingPostPlacingZone.MRK_Type_2_parallel.ToCStr ().Get ()) != 0) {
			bMRK2 = true;
			nMRKSets++;
		}
		if (atoi (supportingPostPlacingZone.MRK_Type_3_parallel.ToCStr ().Get ()) != 0) {
			bMRK3 = true;
			nMRKSets++;
		}

		occupied_len += (double)(atoi (supportingPostPlacingZone.MRK_Type_1_parallel.ToCStr ().Get ()) * 10) / 1000;
		occupied_len += (double)(atoi (supportingPostPlacingZone.MRK_Type_2_parallel.ToCStr ().Get ()) * 10) / 1000;
		occupied_len += (double)(atoi (supportingPostPlacingZone.MRK_Type_3_parallel.ToCStr ().Get ()) * 10) / 1000;
		gap = (atof (supportingPostPlacingZone.girder_real_length.ToCStr ().Get ()) - occupied_len) / (nMRKSets + 1);

		if (bMRK1 == true) {
			parallel_len = (double)(atoi (supportingPostPlacingZone.MRK_Type_1_parallel.ToCStr ().Get ()) * 10) / 1000;
			perpendicular_len = (double)(atoi (supportingPostPlacingZone.MRK_Type_1_perpendicular.ToCStr ().Get ()) * 10) / 1000;

			moveIn3D ('y', PERI.radAng, perpendicular_len / 2, &PERI.posX, &PERI.posY, &PERI.posZ);

			moveIn3D ('x', PERI.radAng, gap, &PERI.posX, &PERI.posY, &PERI.posZ);
			elemList.Push (PERI.placeObject (14, "ZZYZX", APIParT_Length, "4.3", "stType", APIParT_CString, "MP 625", "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "len_min", APIParT_Length, "4.3", "len_max", APIParT_Length, "6.25", "len_current", APIParT_Length, "4.3", "pos_lever", APIParT_Length, "4.220", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)), "bShowCoords", APIParT_Boolean, "0.0", "text2_onoff", APIParT_Boolean, "0.0"));
			moveIn3D ('y', PERI.radAng, -perpendicular_len, &PERI.posX, &PERI.posY, &PERI.posZ);
			elemList.Push (PERI.placeObject (14, "ZZYZX", APIParT_Length, "4.3", "stType", APIParT_CString, "MP 625", "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "len_min", APIParT_Length, "4.3", "len_max", APIParT_Length, "6.25", "len_current", APIParT_Length, "4.3", "pos_lever", APIParT_Length, "4.220", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)), "bShowCoords", APIParT_Boolean, "0.0", "text2_onoff", APIParT_Boolean, "0.0"));
			moveIn3D ('x', PERI.radAng, parallel_len, &PERI.posX, &PERI.posY, &PERI.posZ);
			elemList.Push (PERI.placeObject (14, "ZZYZX", APIParT_Length, "4.3", "stType", APIParT_CString, "MP 625", "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "len_min", APIParT_Length, "4.3", "len_max", APIParT_Length, "6.25", "len_current", APIParT_Length, "4.3", "pos_lever", APIParT_Length, "4.220", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)), "bShowCoords", APIParT_Boolean, "0.0", "text2_onoff", APIParT_Boolean, "0.0"));
			moveIn3D ('y', PERI.radAng, perpendicular_len, &PERI.posX, &PERI.posY, &PERI.posZ);
			elemList.Push (PERI.placeObject (14, "ZZYZX", APIParT_Length, "4.3", "stType", APIParT_CString, "MP 625", "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "len_min", APIParT_Length, "4.3", "len_max", APIParT_Length, "6.25", "len_current", APIParT_Length, "4.3", "pos_lever", APIParT_Length, "4.220", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)), "bShowCoords", APIParT_Boolean, "0.0", "text2_onoff", APIParT_Boolean, "0.0"));

			moveIn3D ('y', PERI.radAng, -perpendicular_len / 2, &PERI.posX, &PERI.posY, &PERI.posZ);
		}

		if (bMRK2 == true) {
			parallel_len = (double)(atoi (supportingPostPlacingZone.MRK_Type_2_parallel.ToCStr ().Get ()) * 10) / 1000;
			perpendicular_len = (double)(atoi (supportingPostPlacingZone.MRK_Type_2_perpendicular.ToCStr ().Get ()) * 10) / 1000;

			moveIn3D ('y', PERI.radAng, perpendicular_len / 2, &PERI.posX, &PERI.posY, &PERI.posZ);

			moveIn3D ('x', PERI.radAng, gap, &PERI.posX, &PERI.posY, &PERI.posZ);
			elemList.Push (PERI.placeObject (14, "ZZYZX", APIParT_Length, "4.3", "stType", APIParT_CString, "MP 625", "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "len_min", APIParT_Length, "4.3", "len_max", APIParT_Length, "6.25", "len_current", APIParT_Length, "4.3", "pos_lever", APIParT_Length, "4.220", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)), "bShowCoords", APIParT_Boolean, "0.0", "text2_onoff", APIParT_Boolean, "0.0"));
			moveIn3D ('y', PERI.radAng, -perpendicular_len, &PERI.posX, &PERI.posY, &PERI.posZ);
			elemList.Push (PERI.placeObject (14, "ZZYZX", APIParT_Length, "4.3", "stType", APIParT_CString, "MP 625", "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "len_min", APIParT_Length, "4.3", "len_max", APIParT_Length, "6.25", "len_current", APIParT_Length, "4.3", "pos_lever", APIParT_Length, "4.220", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)), "bShowCoords", APIParT_Boolean, "0.0", "text2_onoff", APIParT_Boolean, "0.0"));
			moveIn3D ('x', PERI.radAng, parallel_len, &PERI.posX, &PERI.posY, &PERI.posZ);
			elemList.Push (PERI.placeObject (14, "ZZYZX", APIParT_Length, "4.3", "stType", APIParT_CString, "MP 625", "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "len_min", APIParT_Length, "4.3", "len_max", APIParT_Length, "6.25", "len_current", APIParT_Length, "4.3", "pos_lever", APIParT_Length, "4.220", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)), "bShowCoords", APIParT_Boolean, "0.0", "text2_onoff", APIParT_Boolean, "0.0"));
			moveIn3D ('y', PERI.radAng, perpendicular_len, &PERI.posX, &PERI.posY, &PERI.posZ);
			elemList.Push (PERI.placeObject (14, "ZZYZX", APIParT_Length, "4.3", "stType", APIParT_CString, "MP 625", "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "len_min", APIParT_Length, "4.3", "len_max", APIParT_Length, "6.25", "len_current", APIParT_Length, "4.3", "pos_lever", APIParT_Length, "4.220", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)), "bShowCoords", APIParT_Boolean, "0.0", "text2_onoff", APIParT_Boolean, "0.0"));

			moveIn3D ('y', PERI.radAng, -perpendicular_len / 2, &PERI.posX, &PERI.posY, &PERI.posZ);
		}

		if (bMRK3 == true) {
			parallel_len = (double)(atoi (supportingPostPlacingZone.MRK_Type_3_parallel.ToCStr ().Get ()) * 10) / 1000;
			perpendicular_len = (double)(atoi (supportingPostPlacingZone.MRK_Type_3_perpendicular.ToCStr ().Get ()) * 10) / 1000;

			moveIn3D ('y', PERI.radAng, perpendicular_len / 2, &PERI.posX, &PERI.posY, &PERI.posZ);

			moveIn3D ('x', PERI.radAng, gap, &PERI.posX, &PERI.posY, &PERI.posZ);
			elemList.Push (PERI.placeObject (14, "ZZYZX", APIParT_Length, "4.3", "stType", APIParT_CString, "MP 625", "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "len_min", APIParT_Length, "4.3", "len_max", APIParT_Length, "6.25", "len_current", APIParT_Length, "4.3", "pos_lever", APIParT_Length, "4.220", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)), "bShowCoords", APIParT_Boolean, "0.0", "text2_onoff", APIParT_Boolean, "0.0"));
			moveIn3D ('y', PERI.radAng, -perpendicular_len, &PERI.posX, &PERI.posY, &PERI.posZ);
			elemList.Push (PERI.placeObject (14, "ZZYZX", APIParT_Length, "4.3", "stType", APIParT_CString, "MP 625", "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "len_min", APIParT_Length, "4.3", "len_max", APIParT_Length, "6.25", "len_current", APIParT_Length, "4.3", "pos_lever", APIParT_Length, "4.220", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)), "bShowCoords", APIParT_Boolean, "0.0", "text2_onoff", APIParT_Boolean, "0.0"));
			moveIn3D ('x', PERI.radAng, parallel_len, &PERI.posX, &PERI.posY, &PERI.posZ);
			elemList.Push (PERI.placeObject (14, "ZZYZX", APIParT_Length, "4.3", "stType", APIParT_CString, "MP 625", "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "len_min", APIParT_Length, "4.3", "len_max", APIParT_Length, "6.25", "len_current", APIParT_Length, "4.3", "pos_lever", APIParT_Length, "4.220", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)), "bShowCoords", APIParT_Boolean, "0.0", "text2_onoff", APIParT_Boolean, "0.0"));
			moveIn3D ('y', PERI.radAng, perpendicular_len, &PERI.posX, &PERI.posY, &PERI.posZ);
			elemList.Push (PERI.placeObject (14, "ZZYZX", APIParT_Length, "4.3", "stType", APIParT_CString, "MP 625", "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "len_min", APIParT_Length, "4.3", "len_max", APIParT_Length, "6.25", "len_current", APIParT_Length, "4.3", "pos_lever", APIParT_Length, "4.220", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)), "bShowCoords", APIParT_Boolean, "0.0", "text2_onoff", APIParT_Boolean, "0.0"));

			moveIn3D ('y', PERI.radAng, -perpendicular_len / 2, &PERI.posX, &PERI.posY, &PERI.posZ);
		}
	}

	// 부재 배치 함수: MRK
	void placeMRKs ()
	{
		EasyObjectPlacement MRK;
		MRK.init (L("PERI동바리 수평재 v0.2.gsm"), layerInd_MRK, supportingPostPlacingZone.floorInd, supportingPostPlacingZone.oriPos.x, supportingPostPlacingZone.oriPos.y, supportingPostPlacingZone.oriPos.z, supportingPostPlacingZone.radAng);

		bool	bMRK1 = false;
		bool	bMRK2 = false;
		bool	bMRK3 = false;
		int		nMRKSets = 0;

		double	occupied_len = 0.0;
		double	gap;

		double	parallel_len;
		double	perpendicular_len;

		if (atoi (supportingPostPlacingZone.MRK_Type_1_parallel.ToCStr ().Get ()) != 0) {
			bMRK1 = true;
			nMRKSets++;
		}
		if (atoi (supportingPostPlacingZone.MRK_Type_2_parallel.ToCStr ().Get ()) != 0) {
			bMRK2 = true;
			nMRKSets++;
		}
		if (atoi (supportingPostPlacingZone.MRK_Type_3_parallel.ToCStr ().Get ()) != 0) {
			bMRK3 = true;
			nMRKSets++;
		}

		occupied_len += (double)(atoi (supportingPostPlacingZone.MRK_Type_1_parallel.ToCStr ().Get ()) * 10) / 1000;
		occupied_len += (double)(atoi (supportingPostPlacingZone.MRK_Type_2_parallel.ToCStr ().Get ()) * 10) / 1000;
		occupied_len += (double)(atoi (supportingPostPlacingZone.MRK_Type_3_parallel.ToCStr ().Get ()) * 10) / 1000;
		gap = (atof (supportingPostPlacingZone.girder_real_length.ToCStr ().Get ()) - occupied_len) / (nMRKSets + 1);

		// !!! MRK 고도 = 2000

		if (bMRK1 == true) {
			parallel_len = (double)(atoi (supportingPostPlacingZone.MRK_Type_1_parallel.ToCStr ().Get ()) * 10) / 1000;
			perpendicular_len = (double)(atoi (supportingPostPlacingZone.MRK_Type_1_perpendicular.ToCStr ().Get ()) * 10) / 1000;

			moveIn3D ('y', MRK.radAng, perpendicular_len / 2, &MRK.posX, &MRK.posY, &MRK.posZ);

			moveIn3D ('x', MRK.radAng, gap, &MRK.posX, &MRK.posY, &MRK.posZ);
			//elemList.Push (MRK.placeObject (9,
			//	"A", APIParT_Length, ? lenFrame과 동일
			//	"B", APIParT_Length, "0.100",
			//	"ZZYZX", APIParT_Length, "0.500",
			//	"stType", APIParT_CString, ? --> 120 cm ...
			//	"diaVerticalPipe", APIParT_Length, "0.100",
			//	"lenFrame", APIParT_Length, ? --> stType을 더블화 * 10 - 0.100
			//	"angX", APIParT_Angle, "0.0",
			//	"angY", APIParT_Angle, "0.0",
			//	"bOnlyCoupler", APIParT_Boolean, "0.0"));
			//elemList.Push (PERI.placeObject (14, "ZZYZX", APIParT_Length, "4.3", "stType", APIParT_CString, "MP 625", "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "len_min", APIParT_Length, "4.3", "len_max", APIParT_Length, "6.25", "len_current", APIParT_Length, "4.3", "pos_lever", APIParT_Length, "4.220", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)), "bShowCoords", APIParT_Boolean, "0.0", "text2_onoff", APIParT_Boolean, "0.0"));
			//moveIn3D ('y', PERI.radAng, -perpendicular_len, &PERI.posX, &PERI.posY, &PERI.posZ);
			//elemList.Push (PERI.placeObject (14, "ZZYZX", APIParT_Length, "4.3", "stType", APIParT_CString, "MP 625", "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "len_min", APIParT_Length, "4.3", "len_max", APIParT_Length, "6.25", "len_current", APIParT_Length, "4.3", "pos_lever", APIParT_Length, "4.220", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)), "bShowCoords", APIParT_Boolean, "0.0", "text2_onoff", APIParT_Boolean, "0.0"));
			//moveIn3D ('x', PERI.radAng, parallel_len, &PERI.posX, &PERI.posY, &PERI.posZ);
			//elemList.Push (PERI.placeObject (14, "ZZYZX", APIParT_Length, "4.3", "stType", APIParT_CString, "MP 625", "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "len_min", APIParT_Length, "4.3", "len_max", APIParT_Length, "6.25", "len_current", APIParT_Length, "4.3", "pos_lever", APIParT_Length, "4.220", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)), "bShowCoords", APIParT_Boolean, "0.0", "text2_onoff", APIParT_Boolean, "0.0"));
			//moveIn3D ('y', PERI.radAng, perpendicular_len, &PERI.posX, &PERI.posY, &PERI.posZ);
			//elemList.Push (PERI.placeObject (14, "ZZYZX", APIParT_Length, "4.3", "stType", APIParT_CString, "MP 625", "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "len_min", APIParT_Length, "4.3", "len_max", APIParT_Length, "6.25", "len_current", APIParT_Length, "4.3", "pos_lever", APIParT_Length, "4.220", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)), "bShowCoords", APIParT_Boolean, "0.0", "text2_onoff", APIParT_Boolean, "0.0"));

			//moveIn3D ('y', PERI.radAng, -perpendicular_len / 2, &PERI.posX, &PERI.posY, &PERI.posZ);
		}

		if (bMRK2 == true) {
			parallel_len = (double)(atoi (supportingPostPlacingZone.MRK_Type_2_parallel.ToCStr ().Get ()) * 10) / 1000;
			perpendicular_len = (double)(atoi (supportingPostPlacingZone.MRK_Type_2_perpendicular.ToCStr ().Get ()) * 10) / 1000;

			//moveIn3D ('y', PERI.radAng, perpendicular_len / 2, &PERI.posX, &PERI.posY, &PERI.posZ);

			//moveIn3D ('x', PERI.radAng, gap, &PERI.posX, &PERI.posY, &PERI.posZ);
			//elemList.Push (PERI.placeObject (14, "ZZYZX", APIParT_Length, "4.3", "stType", APIParT_CString, "MP 625", "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "len_min", APIParT_Length, "4.3", "len_max", APIParT_Length, "6.25", "len_current", APIParT_Length, "4.3", "pos_lever", APIParT_Length, "4.220", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)), "bShowCoords", APIParT_Boolean, "0.0", "text2_onoff", APIParT_Boolean, "0.0"));
			//moveIn3D ('y', PERI.radAng, -perpendicular_len, &PERI.posX, &PERI.posY, &PERI.posZ);
			//elemList.Push (PERI.placeObject (14, "ZZYZX", APIParT_Length, "4.3", "stType", APIParT_CString, "MP 625", "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "len_min", APIParT_Length, "4.3", "len_max", APIParT_Length, "6.25", "len_current", APIParT_Length, "4.3", "pos_lever", APIParT_Length, "4.220", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)), "bShowCoords", APIParT_Boolean, "0.0", "text2_onoff", APIParT_Boolean, "0.0"));
			//moveIn3D ('x', PERI.radAng, parallel_len, &PERI.posX, &PERI.posY, &PERI.posZ);
			//elemList.Push (PERI.placeObject (14, "ZZYZX", APIParT_Length, "4.3", "stType", APIParT_CString, "MP 625", "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "len_min", APIParT_Length, "4.3", "len_max", APIParT_Length, "6.25", "len_current", APIParT_Length, "4.3", "pos_lever", APIParT_Length, "4.220", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)), "bShowCoords", APIParT_Boolean, "0.0", "text2_onoff", APIParT_Boolean, "0.0"));
			//moveIn3D ('y', PERI.radAng, perpendicular_len, &PERI.posX, &PERI.posY, &PERI.posZ);
			//elemList.Push (PERI.placeObject (14, "ZZYZX", APIParT_Length, "4.3", "stType", APIParT_CString, "MP 625", "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "len_min", APIParT_Length, "4.3", "len_max", APIParT_Length, "6.25", "len_current", APIParT_Length, "4.3", "pos_lever", APIParT_Length, "4.220", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)), "bShowCoords", APIParT_Boolean, "0.0", "text2_onoff", APIParT_Boolean, "0.0"));

			//moveIn3D ('y', PERI.radAng, -perpendicular_len / 2, &PERI.posX, &PERI.posY, &PERI.posZ);
		}

		if (bMRK3 == true) {
			parallel_len = (double)(atoi (supportingPostPlacingZone.MRK_Type_3_parallel.ToCStr ().Get ()) * 10) / 1000;
			perpendicular_len = (double)(atoi (supportingPostPlacingZone.MRK_Type_3_perpendicular.ToCStr ().Get ()) * 10) / 1000;

			//moveIn3D ('y', PERI.radAng, perpendicular_len / 2, &PERI.posX, &PERI.posY, &PERI.posZ);

			//moveIn3D ('x', PERI.radAng, gap, &PERI.posX, &PERI.posY, &PERI.posZ);
			//elemList.Push (PERI.placeObject (14, "ZZYZX", APIParT_Length, "4.3", "stType", APIParT_CString, "MP 625", "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "len_min", APIParT_Length, "4.3", "len_max", APIParT_Length, "6.25", "len_current", APIParT_Length, "4.3", "pos_lever", APIParT_Length, "4.220", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)), "bShowCoords", APIParT_Boolean, "0.0", "text2_onoff", APIParT_Boolean, "0.0"));
			//moveIn3D ('y', PERI.radAng, -perpendicular_len, &PERI.posX, &PERI.posY, &PERI.posZ);
			//elemList.Push (PERI.placeObject (14, "ZZYZX", APIParT_Length, "4.3", "stType", APIParT_CString, "MP 625", "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "len_min", APIParT_Length, "4.3", "len_max", APIParT_Length, "6.25", "len_current", APIParT_Length, "4.3", "pos_lever", APIParT_Length, "4.220", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)), "bShowCoords", APIParT_Boolean, "0.0", "text2_onoff", APIParT_Boolean, "0.0"));
			//moveIn3D ('x', PERI.radAng, parallel_len, &PERI.posX, &PERI.posY, &PERI.posZ);
			//elemList.Push (PERI.placeObject (14, "ZZYZX", APIParT_Length, "4.3", "stType", APIParT_CString, "MP 625", "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "len_min", APIParT_Length, "4.3", "len_max", APIParT_Length, "6.25", "len_current", APIParT_Length, "4.3", "pos_lever", APIParT_Length, "4.220", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)), "bShowCoords", APIParT_Boolean, "0.0", "text2_onoff", APIParT_Boolean, "0.0"));
			//moveIn3D ('y', PERI.radAng, perpendicular_len, &PERI.posX, &PERI.posY, &PERI.posZ);
			//elemList.Push (PERI.placeObject (14, "ZZYZX", APIParT_Length, "4.3", "stType", APIParT_CString, "MP 625", "bCrosshead", APIParT_Boolean, "1.0", "posCrosshead", APIParT_CString, "하단", "crossheadType", APIParT_CString, "PERI", "angCrosshead", APIParT_Angle, format_string ("%f", DegreeToRad (90.0)), "len_min", APIParT_Length, "4.3", "len_max", APIParT_Length, "6.25", "len_current", APIParT_Length, "4.3", "pos_lever", APIParT_Length, "4.220", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (0.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (180.0)), "bShowCoords", APIParT_Boolean, "0.0", "text2_onoff", APIParT_Boolean, "0.0"));

			//moveIn3D ('y', PERI.radAng, -perpendicular_len / 2, &PERI.posX, &PERI.posY, &PERI.posZ);
		}
	}

	// 부재 배치 함수: 멍에제
	void placeGirders ()
	{
		EasyObjectPlacement girder;
		girder.init (L("GT24 거더 v1.0.gsm"), layerInd_GT24, supportingPostPlacingZone.floorInd, supportingPostPlacingZone.oriPos.x, supportingPostPlacingZone.oriPos.y, supportingPostPlacingZone.oriPos.z, supportingPostPlacingZone.radAng);

		char	type [16];
		char	length [16];
		double	perpendicular_len = (double)(atoi (supportingPostPlacingZone.MRK_Type_1_perpendicular.ToCStr ().Get ()) * 10) / 1000;

		strcpy (type, supportingPostPlacingZone.girder_length.ToCStr ().Get ());
		strcpy (length, supportingPostPlacingZone.girder_real_length.ToCStr ().Get ());

		moveIn3D ('y', girder.radAng, perpendicular_len / 2 + 0.046, &girder.posX, &girder.posY, &girder.posZ);
		elemList.Push (girder.placeObject (9, "A", APIParT_Length, length, "B", APIParT_Length, "0.080", "ZZYZX", APIParT_Length, "0.240", "type", APIParT_CString, type, "length", APIParT_Length, length, "change_rot_method", APIParT_Boolean, "1.0", "angX", APIParT_Angle, "0.0", "angY", APIParT_Angle, "0.0", "bWood", APIParT_Boolean, "0.0"));
		moveIn3D ('y', girder.radAng, -0.046 * 2, &girder.posX, &girder.posY, &girder.posZ);
		elemList.Push (girder.placeObject (9, "A", APIParT_Length, length, "B", APIParT_Length, "0.080", "ZZYZX", APIParT_Length, "0.240", "type", APIParT_CString, type, "length", APIParT_Length, length, "change_rot_method", APIParT_Boolean, "1.0", "angX", APIParT_Angle, "0.0", "angY", APIParT_Angle, "0.0", "bWood", APIParT_Boolean, "0.0"));
		moveIn3D ('y', girder.radAng, 0.046 * 2 - perpendicular_len, &girder.posX, &girder.posY, &girder.posZ);
		elemList.Push (girder.placeObject (9, "A", APIParT_Length, length, "B", APIParT_Length, "0.080", "ZZYZX", APIParT_Length, "0.240", "type", APIParT_CString, type, "length", APIParT_Length, length, "change_rot_method", APIParT_Boolean, "1.0", "angX", APIParT_Angle, "0.0", "angY", APIParT_Angle, "0.0", "bWood", APIParT_Boolean, "0.0"));
		moveIn3D ('y', girder.radAng, -0.046 * 2, &girder.posX, &girder.posY, &girder.posZ);
		elemList.Push (girder.placeObject (9, "A", APIParT_Length, length, "B", APIParT_Length, "0.080", "ZZYZX", APIParT_Length, "0.240", "type", APIParT_CString, type, "length", APIParT_Length, length, "change_rot_method", APIParT_Boolean, "1.0", "angX", APIParT_Angle, "0.0", "angY", APIParT_Angle, "0.0", "bWood", APIParT_Boolean, "0.0"));
	}

	// 부재 배치 함수: 장선
	void placeBeams ()
	{
		int girder_length = (int)floor (atof (supportingPostPlacingZone.girder_real_length.ToCStr ().Get ()) * 1000);
		int nBeams = girder_length / BEAM_GAP + 1;
		int start_offset = (girder_length - ((nBeams - 1) * BEAM_GAP) - (0.080 + 0.012) * 1000) / 2;

		EasyObjectPlacement beam;
		beam.init (L("목재v1.0.gsm"), layerInd_Sanseunggak, supportingPostPlacingZone.floorInd, supportingPostPlacingZone.oriPos.x, supportingPostPlacingZone.oriPos.y, supportingPostPlacingZone.oriPos.z, supportingPostPlacingZone.radAng);

		moveIn3D ('y', beam.radAng, 0.750, &beam.posX, &beam.posY, &beam.posZ);
		moveIn3D ('z', beam.radAng, 0.240, &beam.posX, &beam.posY, &beam.posZ);
		beam.radAng -= DegreeToRad (90.0);
		moveIn3D ('y', beam.radAng, ((double)start_offset) / 1000.0, &beam.posX, &beam.posY, &beam.posZ);

		for (int i = 0 ; i < nBeams ; i++) {
			elemList.Push (beam.placeObject (8, "A", APIParT_Length, "0.080", "B", APIParT_Length, "1.500", "w_ins", APIParT_CString, "바닥덮기", "w_w", APIParT_Length, "0.080", "w_h", APIParT_Length, "0.080", "w_leng", APIParT_Length, "1.500", "w_ang", APIParT_Angle, "0.0", "torsion_ang", APIParT_Angle, "0.0", "dirCut", APIParT_CString, "없음"));
			moveIn3D ('y', beam.radAng, (0.080 + 0.012), &beam.posX, &beam.posY, &beam.posZ);
			elemList.Push (beam.placeObject (8, "A", APIParT_Length, "0.080", "B", APIParT_Length, "1.500", "w_ins", APIParT_CString, "바닥덮기", "w_w", APIParT_Length, "0.080", "w_h", APIParT_Length, "0.080", "w_leng", APIParT_Length, "1.500", "w_ang", APIParT_Angle, "0.0", "torsion_ang", APIParT_Angle, "0.0", "dirCut", APIParT_CString, "없음"));
			moveIn3D ('y', beam.radAng, -(0.080 + 0.012), &beam.posX, &beam.posY, &beam.posZ);
			moveIn3D ('y', beam.radAng, ((double)BEAM_GAP) / 1000, &beam.posX, &beam.posY, &beam.posZ);
		}
	}

	// 부재 배치 함수: 결합철물
	void placeJoins ()
	{
		double	perpendicular_len = (double)(atoi (supportingPostPlacingZone.MRK_Type_1_perpendicular.ToCStr ().Get ()) * 10) / 1000;

		int girder_length = (int)floor (atof (supportingPostPlacingZone.girder_real_length.ToCStr ().Get ()) * 1000);
		int nBeams = girder_length / BEAM_GAP + 1;
		int start_offset = (girder_length - ((nBeams - 1) * BEAM_GAP) - (0.080 + 0.012) * 1000) / 2;

		EasyObjectPlacement join;
		join.init (L("결합철물 (사각와셔활용) v1.0.gsm"), layerInd_Join, supportingPostPlacingZone.floorInd, supportingPostPlacingZone.oriPos.x, supportingPostPlacingZone.oriPos.y, supportingPostPlacingZone.oriPos.z, supportingPostPlacingZone.radAng);

		moveIn3D ('y', join.radAng, perpendicular_len / 2, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('z', join.radAng, 0.338, &join.posX, &join.posY, &join.posZ);
		moveIn3D ('x', join.radAng, ((double)start_offset) / 1000.0 + 0.086, &join.posX, &join.posY, &join.posZ);

		for (int i = 0 ; i < nBeams ; i++) {
			elemList.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.400", "bolt_dia", APIParT_Length, "0.012", "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, "0.0", "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, "0.330", "washer_size", APIParT_Length, "0.100", "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			moveIn3D ('y', join.radAng, -perpendicular_len, &join.posX, &join.posY, &join.posZ);
			elemList.Push (join.placeObject (11, "bRotated", APIParT_Boolean, "0.0", "bolt_len", APIParT_Length, "0.400", "bolt_dia", APIParT_Length, "0.012", "bWasher1", APIParT_Boolean, "1.0", "washer_pos1", APIParT_Length, "0.0", "bWasher2", APIParT_Boolean, "1.0", "washer_pos2", APIParT_Length, "0.330", "washer_size", APIParT_Length, "0.100", "nutType", APIParT_CString, "육각너트", "angX", APIParT_Angle, format_string ("%f", DegreeToRad (270.0)), "angY", APIParT_Angle, format_string ("%f", DegreeToRad (0.0))));
			moveIn3D ('y', join.radAng, perpendicular_len, &join.posX, &join.posY, &join.posZ);

			moveIn3D ('x', join.radAng, ((double)BEAM_GAP) / 1000, &join.posX, &join.posY, &join.posZ);
		}
	}
}

using namespace SupportingPostForBeam;

// 보 전용 동바리 세트를 배치하기 위한 파라미터를 입력 받음 (멍에제 길이, MRK 규격 1~3, 설치 층)
short DGCALLBACK supportingPostPlacerHandler1 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	short	itemIndex;

	API_StoryInfo	storyInfo;
	char			floorName [256];

	switch (message) {
		case DG_MSG_INIT:
			DGSetDialogTitle (dialogID, L"보 전용 동바리 세트 배치");

			// 확인 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 95, 240, 50, 25);
			DGSetItemFont (dialogID, DG_OK, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_OK, L"확인");
			DGShowItem (dialogID, DG_OK);

			// 취소 버튼
			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 155, 240, 50, 25);
			DGSetItemFont (dialogID, DG_CANCEL, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, DG_CANCEL, L"취소");
			DGShowItem (dialogID, DG_CANCEL);

			// 멍에제(GT24) 길이
			itemIndex = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 20, 120, 23);
			DGSetItemFont (dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itemIndex, L"멍에제(GT24) 길이");
			DGShowItem (dialogID, itemIndex);

			itemIndex = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 200, 0, 160, 20-5, 110, 25);
			DGSetItemFont (dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, itemIndex);
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"1500");
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"1800");
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"2100");
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"2400");
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"2700");
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"3000");
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"3300");
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"3600");
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"3900");
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"4200");
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"4500");

			// MRK 규격 1
			itemIndex = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 60, 120, 23);
			DGSetItemFont (dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itemIndex, L"MRK 규격 1");
			DGShowItem (dialogID, itemIndex);

			itemIndex = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 200, 0, 160, 60-5, 110, 25);
			DGSetItemFont (dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, itemIndex);
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"없음");
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"900 x 900");
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"900 x 1200");
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"900 x 2300");
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"1200 x 1200");
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"1200 x 2300");

			// MRK 규격 2
			itemIndex = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 100, 120, 23);
			DGSetItemFont (dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itemIndex, L"MRK 규격 2");
			DGShowItem (dialogID, itemIndex);

			itemIndex = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 200, 0, 160, 100-5, 110, 25);
			DGSetItemFont (dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, itemIndex);
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"없음");
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"900 x 900");
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"900 x 1200");
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"900 x 2300");
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"1200 x 1200");
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"1200 x 2300");

			// MRK 규격 3
			itemIndex = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 140, 120, 23);
			DGSetItemFont (dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itemIndex, L"MRK 규격 3");
			DGShowItem (dialogID, itemIndex);

			itemIndex = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 200, 0, 160, 140-5, 110, 25);
			DGSetItemFont (dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, itemIndex);
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"없음");
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"900 x 900");
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"900 x 1200");
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"900 x 2300");
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"1200 x 1200");
			DGPopUpInsertItem (dialogID, itemIndex, DG_POPUP_BOTTOM);
			DGPopUpSetItemText (dialogID, itemIndex, DG_POPUP_BOTTOM, L"1200 x 2300");

			// 설치 층
			itemIndex = DGAppendDialogItem (dialogID, DG_ITM_STATICTEXT, DG_IS_LEFT, DG_FT_NONE, 20, 180, 120, 23);
			DGSetItemFont (dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, itemIndex, L"설치 층");
			DGShowItem (dialogID, itemIndex);

			itemIndex = DGAppendDialogItem (dialogID, DG_ITM_POPUPCONTROL, 200, 0, 160, 180-5, 110, 25);
			DGSetItemFont (dialogID, itemIndex, DG_IS_LARGE | DG_IS_PLAIN);
			DGShowItem (dialogID, itemIndex);

			BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
			ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
			for (int i = 0 ; i <= (storyInfo.lastStory - storyInfo.firstStory) ; ++i) {
				sprintf (floorName, "%d. %s", storyInfo.data [0][i].index, storyInfo.data [0][i].name);
				DGPopUpInsertItem (dialogID, POPUP_FLOOR, DG_POPUP_BOTTOM);
				DGPopUpSetItemText (dialogID, POPUP_FLOOR, DG_POPUP_BOTTOM, convertStr (floorName));
			}
			for (int i = 0 ; i <= (storyInfo.lastStory - storyInfo.firstStory) ; ++i) {
				if (storyInfo.data [0][i].index == 0) {
					DGPopUpSelectItem (dialogID, POPUP_FLOOR, i+1);
				}
			}
			BMKillHandle ((GSHandle *) &storyInfo.data);

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// GT24 길이 저장
					if (DGPopUpGetSelected (dialogID, POPUP_GIRDER_LENGTH) == L_1500) {
						supportingPostPlacingZone.girder_length = "1500";
						supportingPostPlacingZone.girder_real_length = "1.510";
					} else if (DGPopUpGetSelected (dialogID, POPUP_GIRDER_LENGTH) == L_1800) {
						supportingPostPlacingZone.girder_length = "1800";
						supportingPostPlacingZone.girder_real_length = "1.806";
					} else if (DGPopUpGetSelected (dialogID, POPUP_GIRDER_LENGTH) == L_2100) {
						supportingPostPlacingZone.girder_length = "2100";
						supportingPostPlacingZone.girder_real_length = "2.102";
					} else if (DGPopUpGetSelected (dialogID, POPUP_GIRDER_LENGTH) == L_2400) {
						supportingPostPlacingZone.girder_length = "2400";
						supportingPostPlacingZone.girder_real_length = "2.398";
					} else if (DGPopUpGetSelected (dialogID, POPUP_GIRDER_LENGTH) == L_2700) {
						supportingPostPlacingZone.girder_length = "2700";
						supportingPostPlacingZone.girder_real_length = "2.694";
					} else if (DGPopUpGetSelected (dialogID, POPUP_GIRDER_LENGTH) == L_3000) {
						supportingPostPlacingZone.girder_length = "3000";
						supportingPostPlacingZone.girder_real_length = "2.990";
					} else if (DGPopUpGetSelected (dialogID, POPUP_GIRDER_LENGTH) == L_3300) {
						supportingPostPlacingZone.girder_length = "3300";
						supportingPostPlacingZone.girder_real_length = "3.286";
					} else if (DGPopUpGetSelected (dialogID, POPUP_GIRDER_LENGTH) == L_3600) {
						supportingPostPlacingZone.girder_length = "3600";
						supportingPostPlacingZone.girder_real_length = "3.582";
					} else if (DGPopUpGetSelected (dialogID, POPUP_GIRDER_LENGTH) == L_3900) {
						supportingPostPlacingZone.girder_length = "3900";
						supportingPostPlacingZone.girder_real_length = "3.878";
					} else if (DGPopUpGetSelected (dialogID, POPUP_GIRDER_LENGTH) == L_4200) {
						supportingPostPlacingZone.girder_length = "4200";
						supportingPostPlacingZone.girder_real_length = "4.174";
					} else if (DGPopUpGetSelected (dialogID, POPUP_GIRDER_LENGTH) == L_4500) {
						supportingPostPlacingZone.girder_length = "4500";
						supportingPostPlacingZone.girder_real_length = "4.470";
					}

					// MRK 규격 1 정보 저장
					if (DGPopUpGetSelected (dialogID, POPUP_MRK_TYPE_1) == TYPE_900x900) {
						supportingPostPlacingZone.MRK_Type_1_perpendicular = L"90 cm";
						supportingPostPlacingZone.MRK_Type_1_parallel = L"90 cm";
					} else if (DGPopUpGetSelected (dialogID, POPUP_MRK_TYPE_1) == TYPE_900x1200) {
						supportingPostPlacingZone.MRK_Type_1_perpendicular = L"90 cm";
						supportingPostPlacingZone.MRK_Type_1_parallel = L"120 cm";
					} else if (DGPopUpGetSelected (dialogID, POPUP_MRK_TYPE_1) == TYPE_900x2300) {
						supportingPostPlacingZone.MRK_Type_1_perpendicular = L"90 cm";
						supportingPostPlacingZone.MRK_Type_1_parallel = L"230 cm";
					} else if (DGPopUpGetSelected (dialogID, POPUP_MRK_TYPE_1) == TYPE_1200x1200) {
						supportingPostPlacingZone.MRK_Type_1_perpendicular = L"120 cm";
						supportingPostPlacingZone.MRK_Type_1_parallel = L"120 cm";
					} else if (DGPopUpGetSelected (dialogID, POPUP_MRK_TYPE_1) == TYPE_1200x2300) {
						supportingPostPlacingZone.MRK_Type_1_perpendicular = L"120 cm";
						supportingPostPlacingZone.MRK_Type_1_parallel = L"230 cm";
					} else {
						supportingPostPlacingZone.MRK_Type_1_perpendicular = L"0 cm";
						supportingPostPlacingZone.MRK_Type_1_parallel = L"0 cm";
					}

					// MRK 규격 2 정보 저장
					if (DGPopUpGetSelected (dialogID, POPUP_MRK_TYPE_2) == TYPE_900x900) {
						supportingPostPlacingZone.MRK_Type_2_perpendicular = L"90 cm";
						supportingPostPlacingZone.MRK_Type_2_parallel = L"90 cm";
					} else if (DGPopUpGetSelected (dialogID, POPUP_MRK_TYPE_2) == TYPE_900x1200) {
						supportingPostPlacingZone.MRK_Type_2_perpendicular = L"90 cm";
						supportingPostPlacingZone.MRK_Type_2_parallel = L"120 cm";
					} else if (DGPopUpGetSelected (dialogID, POPUP_MRK_TYPE_2) == TYPE_900x2300) {
						supportingPostPlacingZone.MRK_Type_2_perpendicular = L"90 cm";
						supportingPostPlacingZone.MRK_Type_2_parallel = L"230 cm";
					} else if (DGPopUpGetSelected (dialogID, POPUP_MRK_TYPE_2) == TYPE_1200x1200) {
						supportingPostPlacingZone.MRK_Type_2_perpendicular = L"120 cm";
						supportingPostPlacingZone.MRK_Type_2_parallel = L"120 cm";
					} else if (DGPopUpGetSelected (dialogID, POPUP_MRK_TYPE_2) == TYPE_1200x2300) {
						supportingPostPlacingZone.MRK_Type_2_perpendicular = L"120 cm";
						supportingPostPlacingZone.MRK_Type_2_parallel = L"230 cm";
					} else {
						supportingPostPlacingZone.MRK_Type_2_perpendicular = L"0 cm";
						supportingPostPlacingZone.MRK_Type_2_parallel = L"0 cm";
					}

					// MRK 규격 3 정보 저장
					if (DGPopUpGetSelected (dialogID, POPUP_MRK_TYPE_3) == TYPE_900x900) {
						supportingPostPlacingZone.MRK_Type_3_perpendicular = L"90 cm";
						supportingPostPlacingZone.MRK_Type_3_parallel = L"90 cm";
					} else if (DGPopUpGetSelected (dialogID, POPUP_MRK_TYPE_3) == TYPE_900x1200) {
						supportingPostPlacingZone.MRK_Type_3_perpendicular = L"90 cm";
						supportingPostPlacingZone.MRK_Type_3_parallel = L"120 cm";
					} else if (DGPopUpGetSelected (dialogID, POPUP_MRK_TYPE_3) == TYPE_900x2300) {
						supportingPostPlacingZone.MRK_Type_3_perpendicular = L"90 cm";
						supportingPostPlacingZone.MRK_Type_3_parallel = L"230 cm";
					} else if (DGPopUpGetSelected (dialogID, POPUP_MRK_TYPE_3) == TYPE_1200x1200) {
						supportingPostPlacingZone.MRK_Type_3_perpendicular = L"120 cm";
						supportingPostPlacingZone.MRK_Type_3_parallel = L"120 cm";
					} else if (DGPopUpGetSelected (dialogID, POPUP_MRK_TYPE_3) == TYPE_1200x2300) {
						supportingPostPlacingZone.MRK_Type_3_perpendicular = L"120 cm";
						supportingPostPlacingZone.MRK_Type_3_parallel = L"230 cm";
					} else {
						supportingPostPlacingZone.MRK_Type_3_perpendicular = L"0 cm";
						supportingPostPlacingZone.MRK_Type_3_parallel = L"0 cm";
					}

					// 설치 층 저장
					BNZeroMemory (&storyInfo, sizeof (API_StoryInfo));
					ACAPI_Environment (APIEnv_GetStorySettingsID, &storyInfo);
					supportingPostPlacingZone.floorInd = storyInfo.data [0][DGPopUpGetSelected (dialogID, POPUP_FLOOR) - 1].index;
					BMKillHandle ((GSHandle *) &storyInfo.data);

					break;
			}
			break;

		case DG_MSG_CLOSE:
			switch (item) {
				case DG_CLOSEBOX:
					break;
			}
			break;

	}

	result = item;

	return	result;
}

// 부재별 레이어 설정
short DGCALLBACK supportingPostPlacerHandler2 (short message, short dialogID, short item, DGUserData /* userData */, DGMessageData /* msgData */)
{
	short	result;
	API_UCCallbackType	ucb;

	switch (message) {
		case DG_MSG_INIT:
			// 다이얼로그 타이틀
			DGSetDialogTitle (dialogID, L"가설재 레이어 선택하기");

			//////////////////////////////////////////////////////////// 아이템 배치 (기본 버튼)
			// 적용 버튼
			DGSetItemText (dialogID, DG_OK, L"확 인");

			// 종료 버튼
			DGSetItemText (dialogID, DG_CANCEL, L"취 소");

			// 체크박스: 레이어 묶음
			DGSetItemText (dialogID, CHECKBOX_LAYER_COUPLING, L"레이어 묶음");
			DGSetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING, TRUE);

			// 레이어 관련 라벨
			DGSetItemText (dialogID, LABEL_LAYER_SETTINGS, L"부재별 레이어 설정");
			DGSetItemText (dialogID, LABEL_LAYER_PERI_SUPPORT, L"PERI 동바리");
			DGSetItemText (dialogID, LABEL_LAYER_MRK, L"MRK");
			DGSetItemText (dialogID, LABEL_LAYER_GT24, L"멍에제");
			DGSetItemText (dialogID, LABEL_LAYER_BEAM, L"산승각");
			DGSetItemText (dialogID, LABEL_LAYER_JOIN, L"결합철물");

			DGAppendDialogItem (dialogID, DG_ITM_BUTTON, DG_BT_ICONTEXT, 0, 120, 730, 160, 25);
			DGSetItemFont (dialogID, BUTTON_AUTOSET, DG_IS_LARGE | DG_IS_PLAIN);
			DGSetItemText (dialogID, BUTTON_AUTOSET, L"레이어 자동 설정");
			DGHideItem (dialogID, BUTTON_AUTOSET);

			// 불필요한 옵션 숨기기
			DGHideItem (dialogID, LABEL_LAYER_6);
			DGHideItem (dialogID, LABEL_LAYER_7);
			DGHideItem (dialogID, LABEL_LAYER_8);
			DGHideItem (dialogID, LABEL_LAYER_9);
			DGHideItem (dialogID, LABEL_LAYER_10);
			DGHideItem (dialogID, LABEL_LAYER_11);
			DGHideItem (dialogID, LABEL_LAYER_12);
			DGHideItem (dialogID, LABEL_LAYER_13);
			DGHideItem (dialogID, LABEL_LAYER_14);
			DGHideItem (dialogID, LABEL_LAYER_15);
			DGHideItem (dialogID, LABEL_LAYER_16);
			DGHideItem (dialogID, LABEL_LAYER_17);
			DGHideItem (dialogID, LABEL_LAYER_18);
			DGHideItem (dialogID, LABEL_LAYER_19);
			DGHideItem (dialogID, LABEL_LAYER_20);
			DGHideItem (dialogID, LABEL_LAYER_21);
			DGHideItem (dialogID, LABEL_LAYER_22);

			DGHideItem (dialogID, USERCONTROL_LAYER_6);
			DGHideItem (dialogID, USERCONTROL_LAYER_7);
			DGHideItem (dialogID, USERCONTROL_LAYER_8);
			DGHideItem (dialogID, USERCONTROL_LAYER_9);
			DGHideItem (dialogID, USERCONTROL_LAYER_10);
			DGHideItem (dialogID, USERCONTROL_LAYER_11);
			DGHideItem (dialogID, USERCONTROL_LAYER_12);
			DGHideItem (dialogID, USERCONTROL_LAYER_13);
			DGHideItem (dialogID, USERCONTROL_LAYER_14);
			DGHideItem (dialogID, USERCONTROL_LAYER_15);
			DGHideItem (dialogID, USERCONTROL_LAYER_16);
			DGHideItem (dialogID, USERCONTROL_LAYER_17);
			DGHideItem (dialogID, USERCONTROL_LAYER_18);
			DGHideItem (dialogID, USERCONTROL_LAYER_19);
			DGHideItem (dialogID, USERCONTROL_LAYER_20);
			DGHideItem (dialogID, USERCONTROL_LAYER_21);
			DGHideItem (dialogID, USERCONTROL_LAYER_22);

			// 유저 컨트롤 초기화
			BNZeroMemory (&ucb, sizeof (ucb));
			ucb.dialogID = dialogID;
			ucb.type	 = APIUserControlType_Layer;
			ucb.itemID	 = USERCONTROL_LAYER_PERI_SUPPORT;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_PERI_SUPPORT, 1);

			ucb.itemID	 = USERCONTROL_LAYER_MRK;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_MRK, 1);

			ucb.itemID	 = USERCONTROL_LAYER_GT24;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_GT24, 1);

			ucb.itemID	 = USERCONTROL_LAYER_BEAM;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_BEAM, 1);

			ucb.itemID	 = USERCONTROL_LAYER_JOIN;
			ACAPI_Interface (APIIo_SetUserControlCallbackID, &ucb, NULL);
			DGSetItemValLong (dialogID, USERCONTROL_LAYER_JOIN, 1);

			break;

		case DG_MSG_CHANGE:
			// 레이어 같이 바뀜
			if (DGGetItemValLong (dialogID, CHECKBOX_LAYER_COUPLING) == 1) {
				long selectedLayer;

				selectedLayer = DGGetItemValLong (dialogID, item);

				for (short i = USERCONTROL_LAYER_PERI_SUPPORT; i <= USERCONTROL_LAYER_JOIN; ++i)
					DGSetItemValLong (dialogID, i, selectedLayer);
			}

			break;

		case DG_MSG_CLICK:
			switch (item) {
				case DG_OK:
					// 레이어 번호 저장
					layerInd_PERI_Support	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_PERI_SUPPORT);
					layerInd_MRK			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_MRK);
					layerInd_GT24			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_GT24);
					layerInd_Sanseunggak	= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_BEAM);
					layerInd_Join			= (short)DGGetItemValLong (dialogID, USERCONTROL_LAYER_JOIN);
					break;

				case DG_CANCEL:
					break;
			}
		case DG_MSG_CLOSE:
			switch (item) {
				case DG_CLOSEBOX:
					break;
			}
	}

	result = item;

	return	result;
}

// 메인 호출 함수
GSErrCode	placeSupportingPostForBeam ()
{
	GSErrCode	err = NoError;
	short	result;

	API_Coord			c;
	API_GetLineType		lineInfo;

	BNZeroMemory (&lineInfo, sizeof (API_GetLineType));
	ClickAPoint ("시작 점을 클릭하십시오.", &c);

	CHCopyC ("끝 점을 클릭하십시오. 라인을 따라 동바리 세트가 배치될 것입니다.", lineInfo.prompt);
	lineInfo.startCoord.x = c.x;
	lineInfo.startCoord.y = c.y;
	err = ACAPI_Interface (APIIo_GetLineID, &lineInfo, NULL);
	if (err != NoError)
		return err;

	double	dx = lineInfo.pos.x - lineInfo.startCoord.x;
	double	dy = lineInfo.pos.y - lineInfo.startCoord.y;

	// 원점과 각도 정보 저장
	supportingPostPlacingZone.oriPos.x = lineInfo.startCoord.x;
	supportingPostPlacingZone.oriPos.y = lineInfo.startCoord.y;
	supportingPostPlacingZone.oriPos.z = 0;
	supportingPostPlacingZone.radAng = atan2 (dy, dx);

	// 멍에제, MRK 정보 입력
	result = DGBlankModalDialog (300, 280, DG_DLG_VGROW | DG_DLG_HGROW, 0, DG_DLG_THICKFRAME, supportingPostPlacerHandler1, 0);

	if (result != DG_OK)
		return err;

	// 레이어 설정하기
	result = DGModalDialog (ACAPI_GetOwnResModule (), 32503, ACAPI_GetOwnResModule (), supportingPostPlacerHandler2, 0);

	// 객체 배치하기
	placePERISupports ();
	placeMRKs ();
	placeGirders ();
	placeBeams ();
	placeJoins ();

	// 그룹화하기
	groupElements (elemList);

	return err;
}

#endif
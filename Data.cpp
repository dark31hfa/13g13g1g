#include "Data.h"
#include<thread>
#include "Esp.h"
#include <algorithm>
Espp* g_pEsp = new Espp();
DWORD Data2::ViewMatrixBase;
DWORD Data2::UWorld = 0;
INT Data2::alivePlayerNum = 0;
INT Data2::aliveTeamNum = 0;
DWORD Data2::PlayerCameraManager = 0;
INT Data2::OnlinePlayer = 0;
INT Data2::GameIDD = 0;
//DWORD Data2::CheckLibs = 0;
BYTE Data2::Visible = 0;
DWORD Data2::MyPlayerWorld = 0;
BOOL Data2::bIsWeaponFiring = 0;
string statusActor;

BYTE ViewMatrixSearch[] =
{
	0x02, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x01
};
AActorPlayer AActorA;
DWORD Data2::GetViewMatrixBase(DWORD StartAddress, DWORD EndAddress)
{
	DWORD ViewMatrixBase = 0;
	std::vector<DWORD_PTR> FoundedBase;
	Utility::MemSearch(ViewMatrixSearch, sizeof(ViewMatrixSearch), StartAddress, EndAddress, 0, 0, FoundedBase);

	for (int i = 0; i < FoundedBase.size(); i++)
	{
		DWORD Cand = FoundedBase[i] - 0x20;
		DWORD Eng = Utility::ReadMemoryEx<DWORD>(Cand) + 0x20;
		DWORD Tmp = Utility::ReadMemoryEx<DWORD>(Eng) + 0x200;

		float v1, v2, v3, v4;
		v1 = Utility::ReadMemoryEx<float>(Tmp + 0x8);
		v2 = Utility::ReadMemoryEx<float>(Tmp + 0x18);
		v3 = Utility::ReadMemoryEx<float>(Tmp + 0x28);
		v4 = Utility::ReadMemoryEx<float>(Tmp + 0x38);

		if (v1 == 0 && v2 == 0 && v3 == 0 && v4 >= 3)
		{
			ViewMatrixBase = Cand;
			FoundedBase.clear();
			break;
		}
	}

	FoundedBase.clear();
	return ViewMatrixBase;
}

float Distance(VECTOR3 myPos, VECTOR3 enPos)
{
	return sqrt((myPos.X - enPos.X) * (myPos.X - enPos.X) + (myPos.Y - enPos.Y) * (myPos.Y - enPos.Y) + (myPos.Z - enPos.Z) * (myPos.Z - enPos.Z));
}

string GetString(DWORD BaseAddress)
{
	char* String = new char[34];
	for (int i = 0; i < 34; i++)
	{
		String[i] = Utility::ReadMemoryEx<char>(BaseAddress + i);
	}

	string Result = String;

	delete[] String;
	return Result;
}

string GetEntityType(DWORD gNames, int id)
{

	auto gname = Utility::ReadMemoryEx<DWORD>(Utility::ReadMemoryEx<DWORD>(gNames) + 0x88);
	auto fNamePtr = Utility::ReadMemoryEx<DWORD>(gname + int(id / 16384) * 0x4);
	auto fName = Utility::ReadMemoryEx<DWORD>(fNamePtr + int(id % 16384) * 0x4);
	auto entryOffset = fName + 0x8;
	auto nameEntry = Utility::ReadMemoryEx<INT16>(entryOffset);

	auto nameLength = nameEntry >> 6;
	char buff[1028];

	if ((uint32_t)nameLength && nameLength > 0)
	{
		Utility::readRaw(LPCVOID(entryOffset), buff, nameLength, 0);

		buff[nameLength] = '\0';

		std::string name_str(buff, nameLength);

		return name_str.c_str();
	}
	else
		return "";

	return "";
}

//std::string GetEntityType3(DWORD gNames, int id)
//{
//
//	auto gname = Utility::ReadMemoryEx<DWORD>(Utility::ReadMemoryEx<DWORD>(Offset::UE4 + 0x7F15A38) + 0x88);
//	auto fNamePtr = Utility::ReadMemoryEx<DWORD>(gname + int(id / 16384) * 0x4);
//	auto fName = Utility::ReadMemoryEx<DWORD>(fNamePtr + int(id % 16384) * 0x4);
//	auto entryOffset = fName + 0x8;
//	auto nameEntry = Utility::ReadMemoryEx<INT16>(entryOffset);
//
//	auto nameLength = nameEntry >> 6;
//	char buff[1028];
//
//	if ((uint32_t)nameLength && nameLength > 0)
//	{
//		ReadProcessMemory(Game::hProcess, LPCVOID(entryOffset), buff, nameLength, 0);
//
//		buff[nameLength] = '\0';
//
//		std::string name_str(buff, nameLength);
//
//		return name_str.c_str();
//	}
//	else
//		return "";
//
//	return "";
//}

string textActor;
string GetEntityType(LONG GNames, LONG Id)
{
	string Result = "";

	DWORD GName = Utility::ReadMemoryEx<DWORD>(GNames);

	if (Id > 0 && Id < 2000000)
	{
		DWORD Page = Id / 16384;
		DWORD Index = Id % 16384;
		DWORD SecPartAddv = Utility::ReadMemoryEx<DWORD>(GName + Page * 4);

		if (SecPartAddv > 0)
		{
			LONG NameAddv = Utility::ReadMemoryEx<DWORD>(SecPartAddv + Index * 4);

			if (NameAddv > 0)
			{
				Result = GetString(NameAddv + 0x8);
			}
		}
	}
	return Result;
}
string GetEntityTypeinfo(LONG Id)
{
	string Result = "";

	auto GName = Utility::ReadMemoryEx<DWORD>(Utility::ReadMemoryEx<DWORD>(Offset::UE4 + Offset::Gname) + 0x88);

	if (Id > 0 && Id < 2000000)
	{
		DWORD Page = Id / 16384;
		DWORD Index = Id % 16384;
		DWORD SecPartAddv = Utility::ReadMemoryEx<DWORD>(GName + Page * 4);

		if (SecPartAddv > 0)
		{
			LONG NameAddv = Utility::ReadMemoryEx<DWORD>(SecPartAddv + Index * 4);

			if (NameAddv > 0)
			{
				Result = GetString(NameAddv + 0x8);
			}
		}
	}

	return Result;
}
struct ObjectName
{
	char Data[64];
};
string GetEntityType2(LONG Id)
{
	auto GName = Utility::ReadMemoryEx<DWORD>(Utility::ReadMemoryEx<DWORD>(Offset::UE4 + Offset::Gname) + 0x88);
	//auto GName = Utility::ReadMemoryEx<DWORD>(Offset::UE4 + 0x7F15A38) + 0x88;
	DWORD NamePtr = Utility::ReadMemoryEx<DWORD>(GName + int(Id / 16384) * 0x4);
	DWORD Name = Utility::ReadMemoryEx<DWORD>(NamePtr + int(Id % 16384) * 0x4);
	ObjectName pBuffer = Utility::ReadMemoryEx<ObjectName>(Name + 0x8);

	return string(pBuffer.Data);
}
std::string GetEntityType3(DWORD gNames, int id)
{

	auto gname = Utility::ReadMemoryEx<DWORD>(Utility::ReadMemoryEx<DWORD>(gNames) + 0x88);
	auto fNamePtr = Utility::ReadMemoryEx<DWORD>(gname + int(id / 16384) * 0x4);
	auto fName = Utility::ReadMemoryEx<DWORD>(fNamePtr + int(id % 16384) * 0x4);
	auto entryOffset = fName + 0x8;
	auto nameEntry = Utility::ReadMemoryEx<INT16>(entryOffset);

	auto nameLength = nameEntry >> 6;
	char buff[1028];

	if ((uint32_t)nameLength && nameLength > 0)
	{
		ReadProcessMemory(Game::hProcess, LPCVOID(entryOffset), buff, nameLength, 0);

		buff[nameLength] = '\0';

		std::string name_str(buff, nameLength);

		return name_str.c_str();
	}
	else
		return "";

	return "";
}
string GetPlayerName(DWORD BaseAddress)
{
	char* String = new char[34];
	for (int i = 0; i < 34; i++)
	{
		String[i] = Utility::ReadMemoryEx<char>(BaseAddress + i * 2);
	}

	string Name = String;

	delete[] String;
	return Name;
}

std::wstring GetPlayerName1(DWORD adds)
{

	wchar_t* temp = new wchar_t[34];
	for (int i = 0; i < 34; i++)
	{
		temp[i] = Utility::ReadMemoryExWSize<wchar_t>(adds + i * 2, sizeof(wchar_t));
	}
	std::wstring ret = temp;
	delete temp;
	return ret;
}
wstring GetPlayerFlag(DWORD BaseAddress)
{
	wchar_t* String = new wchar_t[34];
	for (int i = 0; i < 34; i++)
	{
		String[i] = Utility::ReadMemoryEx<wchar_t>(BaseAddress + i * 2);
	}

	wstring Name = String;

	delete[] String;
	return Name;
}
std::string GetEntity(DWORD gNames, int id)
{
	auto gname = Utility::ReadMemoryEx<DWORD>(gNames);
	auto fNamePtr = Utility::ReadMemoryEx<DWORD>(gname + int(id / 16384) * 0x4);
	auto fName = Utility::ReadMemoryEx<DWORD>(fNamePtr + int(id % 16384) * 0x4);
	auto entryOffset = fName + 0x8;
	auto nameEntry = Utility::ReadMemoryEx<INT16>(entryOffset);

	auto nameLength = nameEntry >> 6;
	char buff[1028];

	if ((uint32_t)nameLength && nameLength > 0)
	{
		Utility::RPM(LPCVOID(entryOffset), buff, nameLength, 0);

		buff[nameLength] = '\0';

		std::string name_str(buff, nameLength);

		return name_str.c_str();
	}
	else
		return "";

	return "";
}
BOOL IsPlayer(string str)
{
	if (str.find("BP_PlayerPawn") != std::string::npos
		|| str.find("PlayerCharacter") != std::string::npos
		|| str.find("PlanET_FakePlayer") != std::string::npos
		|| str.find("BP_PlayerPawn_FM_C") != std::string::npos
		|| str.find("BP_PlayerPawn_C") != std::string::npos
		|| str.find("BP_PlayerPawn_Rune_C") != std::string::npos
		|| str.find("PlayerPawn_Infec_InvisibleZombie_C") != std::string::npos
		|| str.find("PlayerPawn_Infec_Human_C") != std::string::npos
		|| str.find("BP_PlayerPawn_ZombieBase_C") != std::string::npos
		|| str.find("BP_PlayerPawn_TDM_TPP_C") != std::string::npos
		|| str.find("BP_PlayerPawn_FM_Bot_C") != std::string::npos
		|| str.find("BP_PlayerCharacter_SlayTheBot_C") != std::string::npos
		|| str.find("PlanET_FakePlayer_AIPawn_C") != std::string::npos
		|| str.find("BP_PlayerPawn_SI_C") != std::string::npos
		|| str.find("BP_PlayerPawn_Heavy_C") != std::string::npos
		|| str.find("PlayerPawn_Infec_Revenger_C") != std::string::npos
		|| str.find("PlayerPawn_Infec_NormalZombie_C") != std::string::npos
		|| str.find("PlayerPawn_Infec_KingZombie_C") != std::string::npos
		|| str.find("BP_PlayerCharacter_PlanA_C") != std::string::npos
		|| str.find("BP_CharacterModelTaget_C") != std::string::npos
		|| str.find("BP_PlayerPawn_HT_AI_C") != std::string::npos
		//
		//|| str.find("Mob_Zombie92_C") != std::string::npos
		//|| str.find("Mob_Zombie93_C") != std::string::npos
		//|| str.find("Zombie95_01_C") != std::string::npos
		//|| str.find("Mob_Zombie94_C") != std::string::npos
		//|| str.find("BP_SiphonTowerZombie_Boss_C") != std::string::npos
		//|| str.find("BP_SiphonTowerZombie_Normal_C") != std::string::npos


		)
	{
		return TRUE;
	}

	return FALSE;
}

BOOL IsPlayer1(string EntityType)
{
	if (EntityType.find("BP_PlayerPawn") != string::npos)
	{
		return TRUE;
	}
	if (EntityType.find("BP_CharacterModel") != string::npos)
	{
		return TRUE;
	}
	if (EntityType.find("PlayerCharacter") != string::npos)
	{
		return TRUE;
	}

	return FALSE;
}



bool IsDeathbox(std::string classname)
{
	if (classname == "PickUpListWrapperActor")
		return true;
	else
		return false;
	return false;
}
bool IsAirDropBox(std::string classname)
{

	if (classname == "BP_AirDropBox_C" || classname == "BP_AirDropBox_New_C")
		return true;
	else
		return false;
	return false;
}


std::string GetBoxItems(int code)
{
	if (code == 101008)
	{
		Setting::sizecode = Setting::Esp_Item_M762_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_M762_Color[0], Setting::Esp_Item_M762_Color[1], Setting::Esp_Item_M762_Color[2], Setting::Esp_Item_M762_Color[3]);
		return "M762";
	}
	else if (code == 306001)
	{
		Setting::sizecode = Setting::Esp_Item_M762_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_Magnum_Color[0], Setting::Esp_Item_Magnum_Color[1], Setting::Esp_Item_Magnum_Color[2], Setting::Esp_Item_Magnum_Color[3]);
		return "Magnum";
	}

	else if (code == 101003)
	{
		Setting::sizecode = Setting::Esp_Item_SCARL_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_SCARL_Color[0], Setting::Esp_Item_SCARL_Color[1], Setting::Esp_Item_SCARL_Color[2], Setting::Esp_Item_SCARL_Color[3]);
		return "SCAR-L";
	}
	else if (code == 302001)
	{
		Setting::sizecode = Setting::Esp_Item_M762_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_762mm_Color[0], Setting::Esp_Item_762mm_Color[1], Setting::Esp_Item_762mm_Color[2], Setting::Esp_Item_762mm_Color[3]);
		return "7.62";
	}
	else if (code == 303001)
	{
		Setting::sizecode = Setting::Esp_Item_M762_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_556mm_Color[0], Setting::Esp_Item_556mm_Color[1], Setting::Esp_Item_556mm_Color[2], Setting::Esp_Item_556mm_Color[3]);
		return "5.56";
	}
	else if (code == 602004)
	{
		Setting::sizecode = Setting::Esp_Item_M762_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_Frag_Color[0], Setting::Esp_Item_Frag_Color[1], Setting::Esp_Item_Frag_Color[2], Setting::Esp_Item_Frag_Color[3]);
		return "Grenade";
	}
	else if (code == 601006)
	{
		Setting::sizecode = Setting::Esp_Item_M762_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_MedKit_Color[0], Setting::Esp_Item_MedKit_Color[1], Setting::Esp_Item_MedKit_Color[2], Setting::Esp_Item_MedKit_Color[3]);
		return "Medkit";
	}
	else if (code == 101004)
	{
		Setting::sizecode = Setting::Esp_Item_M416_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_M416_Color[0], Setting::Esp_Item_M416_Color[1], Setting::Esp_Item_M416_Color[2], Setting::Esp_Item_M416_Color[3]);
		return "M416";
	}

	else if (code == 101010)
	{
		Setting::sizecode = Setting::Esp_Item_M762_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_G36C_Color[0], Setting::Esp_Item_G36C_Color[1], Setting::Esp_Item_G36C_Color[2], Setting::Esp_Item_G36C_Color[3]);
		return "G36C";
	}

	else if (code == 101006)
	{
		Setting::sizecode = Setting::Esp_Item_M762_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_AUG_Color[0], Setting::Esp_Item_AUG_Color[1], Setting::Esp_Item_AUG_Color[2], Setting::Esp_Item_AUG_Color[3]);
		return "AUG";
	}

	//else if (code == 101101)
	//{
	//	Setting::colorcode = ImColor(Setting::Esp_Item_M762_Color[0], Setting::Esp_Item_M762_Color[1], Setting::Esp_Item_M762_Color[2], Setting::Esp_Item_M762_Color[3]);
	//	return "ASM"; //time
	//}

	else if (code == 101001)
	{
		Setting::sizecode = Setting::Esp_Item_AKM_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_AKM_Color[0], Setting::Esp_Item_AKM_Color[1], Setting::Esp_Item_AKM_Color[2], Setting::Esp_Item_AKM_Color[3]);
		return "AKM";
	}

	else if (code == 101005)
	{
		Setting::sizecode = Setting::Esp_Item_M762_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_Groza_Color[0], Setting::Esp_Item_Groza_Color[1], Setting::Esp_Item_Groza_Color[2], Setting::Esp_Item_Groza_Color[3]);
		return "Groza";
	}

	else if (code == 103003)
	{
		Setting::sizecode = Setting::Esp_Item_M762_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_Awm_Color[0], Setting::Esp_Item_Awm_Color[1], Setting::Esp_Item_Awm_Color[2], Setting::Esp_Item_Awm_Color[3]);
		return "AWM";
	}

	else if (code == 103002)
	{
		Setting::sizecode = Setting::Esp_Item_M762_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_M24_Color[0], Setting::Esp_Item_M24_Color[1], Setting::Esp_Item_M24_Color[2], Setting::Esp_Item_M24_Color[3]);
		return "M24";
	}

	else if (code == 103001)
	{
		Setting::sizecode = Setting::Esp_Item_M762_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_Kar98k_Color[0], Setting::Esp_Item_Kar98k_Color[1], Setting::Esp_Item_Kar98k_Color[2], Setting::Esp_Item_Kar98k_Color[3]);
		return "Kar98k";
	}

	else if (code == 103011)
	{
		Setting::sizecode = Setting::Esp_Item_M762_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_Mosin_Color[0], Setting::Esp_Item_Mosin_Color[1], Setting::Esp_Item_Mosin_Color[2], Setting::Esp_Item_Mosin_Color[3]);
		return "Mosin";
	}

	else if (code == 502002)
	{
		Setting::sizecode = Setting::Esp_Item_M762_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_Helmet2_Color[0], Setting::Esp_Item_Helmet2_Color[1], Setting::Esp_Item_Helmet2_Color[2], Setting::Esp_Item_Helmet2_Color[3]);
		return "Helmet Lv.2";
	}

	else if (code == 502003)
	{
		Setting::sizecode = Setting::Esp_Item_M762_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_Helmet3_Color[0], Setting::Esp_Item_Helmet3_Color[1], Setting::Esp_Item_Helmet3_Color[2], Setting::Esp_Item_Helmet3_Color[3]);
		return "Helmet Lv.3";
	}

	else if (code == 503002)
	{
		Setting::sizecode = Setting::Esp_Item_M762_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_Armor2_Color[0], Setting::Esp_Item_Armor2_Color[1], Setting::Esp_Item_Armor2_Color[2], Setting::Esp_Item_Armor2_Color[3]);
		return "Armor Lv.2";
	}

	else if (code == 503003)
	{
		Setting::sizecode = Setting::Esp_Item_M762_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_Armor3_Color[0], Setting::Esp_Item_Armor3_Color[1], Setting::Esp_Item_Armor3_Color[2], Setting::Esp_Item_Armor3_Color[3]);
		return "Armor Lv.3";
	}

	else if (code == 501005)
	{
		Setting::sizecode = Setting::Esp_Item_M762_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_Bag2_Color[0], Setting::Esp_Item_Bag2_Color[1], Setting::Esp_Item_Bag2_Color[2], Setting::Esp_Item_Bag2_Color[3]);
		return "Bag Lv.2";
	}

	else if (code == 501006)
	{
		Setting::sizecode = Setting::Esp_Item_M762_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_Bag3_Color[0], Setting::Esp_Item_Bag3_Color[1], Setting::Esp_Item_Bag3_Color[2], Setting::Esp_Item_Bag3_Color[3]);
		return "Bag Lv.3";
	}

	else if (code == 203014)
	{
		Setting::sizecode = Setting::Esp_Item_M762_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_x3_Color[0], Setting::Esp_Item_x3_Color[1], Setting::Esp_Item_x3_Color[2], Setting::Esp_Item_x3_Color[3]);
		return "3x scope";

	}

	else if (code == 203004)
	{
		Setting::sizecode = Setting::Esp_Item_M762_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_x4_Color[0], Setting::Esp_Item_x4_Color[1], Setting::Esp_Item_x4_Color[2], Setting::Esp_Item_x4_Color[3]);
		return "4x scope";
	}

	else if (code == 203015)
	{
		Setting::sizecode = Setting::Esp_Item_M762_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_x6_Color[0], Setting::Esp_Item_x6_Color[1], Setting::Esp_Item_x6_Color[2], Setting::Esp_Item_x6_Color[3]);
		return "6x scope";
	}

	else if (code == 203005)
	{
		Setting::sizecode = Setting::Esp_Item_M762_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_x8_Color[0], Setting::Esp_Item_x8_Color[1], Setting::Esp_Item_x8_Color[2], Setting::Esp_Item_x8_Color[3]);
		return "8x scope";
	}

	else if (code == 106007)
	{
		Setting::sizecode = Setting::Esp_Item_M762_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_FlareGun_Color[0], Setting::Esp_Item_FlareGun_Color[1], Setting::Esp_Item_FlareGun_Color[2], Setting::Esp_Item_FlareGun_Color[3]);
		return "Flaregun";
	}

	else if (code == 105001)
	{
		Setting::sizecode = Setting::Esp_Item_M762_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_M249_Color[0], Setting::Esp_Item_M249_Color[1], Setting::Esp_Item_M249_Color[2], Setting::Esp_Item_M249_Color[3]);
		return "M249";
	}

	else if (code == 105002)
	{
		Setting::sizecode = Setting::Esp_Item_M762_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_DP28_Color[0], Setting::Esp_Item_DP28_Color[1], Setting::Esp_Item_DP28_Color[2], Setting::Esp_Item_DP28_Color[3]);
		return "DP28";
	}

	else if (code == 105010)
	{
		Setting::sizecode = Setting::Esp_Item_M762_TextSize;
		Setting::colorcode = ImColor(Setting::Esp_Item_M762_Color[0], Setting::Esp_Item_M762_Color[1], Setting::Esp_Item_M762_Color[2], Setting::Esp_Item_M762_Color[3]);
		return "MG3";
	}

	return "tatti";
}
//std::string scopes(int fov)
//{
//	if (fov == 70)
//		return "Iron";
//	else if (fov == 55)
//		return "1x";
//	else if (fov == 44)
//		return "2x";
//	else if (fov == 26)
//		return "3x";
//	else if (fov == 20)
//		return "4x";
//	else if (fov == 13)
//		return "6x";
//	else if (fov == 11)
//		return "8x";
//	else
//		return "None";
//
//	return "";
//}

std::string scopes(int fov)
{
	if (fov == 70)
		return "Iron";
	else if (fov == 55)
		return "RedDot";
	else if (fov == 44)
		return "X2";
	else if (fov == 26)//27
		return "X3";
	else if (fov == 20)
		return "X4";
	else if (fov == 13)
		return "X6";
	else if (fov == 11)
		return "X8";
	else
		return "None";

	return "";
}
std::string PlayerScope(int fov)
{
	if (fov == 70)
		return "";
	else if (fov == 55)
		return "1x";
	else if (fov == 44)
		return "2x";
	else if (fov == 26)
		return "3x";
	else if (fov == 20)
		return "4x";
	else if (fov == 13)
		return "6x";
	else if (fov == 11)
		return "8x";
	else
		return "";
	return "";
}
std::string PlayerWeapon(std::string str)
{
	if (str == "BP_Sniper_AWM_C")
		return "AWM";
	else if (str == "BP_Sniper_QBU_C")
		return "QBU";
	else if (str == "BP_Sniper_SLR_C")
		return "SLR";
	else if (str == "BP_Sniper_SKS_C")
		return "SKS";
	else if (str == "BP_Sniper_Mini14_C")
		return "Mini";
	else if (str == "BP_Sniper_M24_C")
		return "M24";
	else if (str == "BP_Sniper_Kar98k_C")
		return "Kar98";
	else if (str == "BP_Sniper_VSS_C")
		return "VSS";
	else if (str == "BP_Sniper_Win94_C")
		return "Win94";
	else if (str == "BP_Sniper_Mosin_C")
		return "Mosin";
	else if (str == "BP_Sniper_MK12_C")
		return "MK12";
	else if (str == "BP_Rifle_AUG_C")
		return "AUG";
	else if (str == "BP_Rifle_M762_C")
		return "M762";
	else if (str == "BP_Rifle_SCAR_C")
		return "SCAR";
	else if (str == "BP_Rifle_M416_C")
		return "M416";
	else if (str == "BP_Rifle_M16A4_C")
		return "M16A4";
	else if (str == "BP_Rifle_Mk47_C")
		return "Mk47";
	else if (str == "BP_Rifle_G36_C")
		return "G36";
	else if (str == "BP_Rifle_QBZ_C")
		return "QBZ";
	else if (str == "BP_Rifle_AKM_C")
		return "AKM";
	else if (str == "BP_Rifle_Groza_C")
		return "Groza";
	else if (str == "BP_Other_DP28_C")
		return "DP28";
	else if (str == "BP_Other_M249_C")
		return "M249";
	else if (str == "BP_MachineGun_P90_C")
		return "P90";
	else if (str == "BP_ShotGun_S12K_C")
		return "S12K";
	else if (str == "BP_ShotGun_DP12_C")
		return "DBS";
	else if (str == "BP_ShotGun_M1014_C")
		return "BP_ShotGun_M1014_C";
	else if (str == "BP_ShotGun_S686_C")
		return "S686";
	else if (str == "BP_ShotGun_S1897_C")
		return "S1897";
	else if (str == "BP_ShotGun_SawedOff_C")
		return "SawedOff";
	else if (str == "BP_MachineGun_PP19_C")
		return "PP19";
	else if (str == "BP_MachineGun_TommyGun_C")
		return "TommyGu";
	else if (str == "BP_MachineGun_MP5K_C")
		return "MP5K";
	else if (str == "BP_MachineGun_UMP9_C")
		return "UMP9";
	else if (str == "BP_MachineGun_Vector_C")
		return "Vector";
	else if (str == "BP_MachineGun_Uzi_C")
		return "Uzi";
	else if (str == "BP_Pistol_Flaregun_C")
		return "Flaregun";
	else if (str == "BP_Pistol_R1895_C")
		return "R1895";
	else if (str == "BP_Pistol_Vz61_C")
		return "Vz61";
	else if (str == "BP_Pistol_P92_C")
		return "P92";
	else if (str == "BP_Pistol_P18C_C")
		return "P18C";
	else if (str == "BP_Pistol_R45_C")
		return "R45";
	else if (str == "BP_Pistol_P1911_C")
		return "P1911";
	else if (str == "BP_Pistol_DesertEagle_C")
		return "DesertEagle";
	else if (str == "BP_WEP_Mk14_C")
		return "Mk14";
	else if (str == "BP_WEP_Sickle_C")
		return "BP_WEP_Sickle_C";
	else if (str == "BP_WEP_Machete_C")
		return "BP_WEP_Machete_C";
	else if (str == "BP_WEP_Cowbar_C")
		return "BP_WEP_Cowbar_C";
	else if (str == "BP_WEP_Pan_C")
		return "BP_WEP_Pan_C";
	else if (str == "BP_Other_CrossBow_C")
		return "BP_Other_CrossBow_C";
	else if (str == "BP_WEP_Zombie59_Gloves_C")
		return "BP_WEP_Zombie59_Gloves_C";
	else if (str == "BP_Grenade_Shoulei_Weapon_C")
		return "BP_Grenade_Shoulei_Weapon_C";
	else if (str == "BP_Grenade_Smoke_Weapon_C")
		return "BP_Grenade_Smoke_Weapon_C";
	else if (str == "BP_Grenade_Apple_Weapon_C")
		return "BP_Grenade_Apple_Weapon_C";
	else if (str == "BP_Grenade_Burn_Weapon_C")
		return "BP_Grenade_Burn_Weapon_C";
	else if (str == "")
		return "Fist";
	else
		return "";
	return "";
}

string GetGrenadeType(string classname)
{
	if (classname.find("BP_Grenade_Burn_C") != string::npos)
		return "Burn_Grenade";
	if (classname.find("BP_Grenade_Shoulei_C") != string::npos)
		return "Frag_Grenade";
	return "tatti";
}

string GetItemType(string Name)
{
	if (Setting::Esp_Item_Awm) if (Name.find("BP_Sniper_AWM_Wrapper_C") != string::npos) return "AWM";
	if (Setting::Esp_Item_QBU) if (Name.find("BP_Sniper_QBU_Wrapper_C") != string::npos) return "QBU";
	if (Setting::Esp_Item_SLR) if (Name.find("BP_Sniper_SLR_Wrapper_C") != string::npos) return "SLR";
	if (Setting::Esp_Item_SKS) if (Name.find("BP_Sniper_SKS_Wrapper_C") != string::npos) return "SKS";
	if (Setting::Esp_Item_Mini14) if (Name.find("BP_Sniper_Mini14_Wrapper_C") != string::npos) return "Mini-14";
	if (Setting::Esp_Item_M24) if (Name.find("BP_Sniper_M24_Wrapper_C") != string::npos) return "M24";
	if (Setting::Esp_Item_Kar98k) if (Name.find("BP_Sniper_Kar98k_Wrapper_C") != string::npos) return "Kar98k";
	if (Setting::Esp_Item_VSS) if (Name.find("BP_Sniper_VSS_Wrapper_C") != string::npos) return "VSS";
	if (Setting::Esp_Item_Win94) if (Name.find("BP_Sniper_Win94_Wrapper_C") != string::npos) return "Win94";
	if (Setting::Esp_Item_Mosin) if (Name.find("BP_Sniper_Mosin_Wrapper_C") != string::npos) return "Mosin";
	if (Setting::Esp_Item_M762) if (Name.find("BP_Rifle_M762_Wrapper_C") != string::npos) return "M762";
	if (Setting::Esp_Item_SCARL) if (Name.find("BP_Rifle_SCAR_Wrapper_C") != string::npos) return "SCAR-L";
	if (Setting::Esp_Item_M416) if (Name.find("BP_Rifle_M416_Wrapper_C") != string::npos) return "M416";
	if (Setting::Esp_Item_M16A4) if (Name.find("BP_Rifle_M16A4_Wrapper_C") != string::npos) return "M16A4";
	if (Setting::Esp_Item_Mk47Mutant) if (Name.find("BP_Rifle_Mk47_Wrapper_C") != string::npos) return "Mk47";
	if (Setting::Esp_Item_G36C) if (Name.find("BP_Rifle_G36_Wrapper_C") != string::npos) return "G36C";
	if (Setting::Esp_Item_QBZ) if (Name.find("BP_Rifle_QBZ_Wrapper_C") != string::npos) return "QBZ";
	if (Setting::Esp_Item_AKM) if (Name.find("BP_Rifle_AKM_Wrapper_C") != std::string::npos) return "AKM";
	if (Setting::Esp_Item_Groza) if (Name.find("BP_Rifle_Groza_Wrapper_C") != std::string::npos) return "Groza";
	if (Setting::Esp_Item_AUG) if (Name.find("BP_Rifle_AUG_Wrapper_C") != std::string::npos) return "AUG";
	if (Setting::Esp_Item_S12K) if (Name.find("BP_ShotGun_S12K_Wrapper_C") != std::string::npos) return "S12K";
	if (Setting::Esp_Item_M1014) if (Name.find("BP_ShotGun_M1014_Wrapper_C") != std::string::npos) return "M1014";
	if (Setting::Esp_Item_DBS) if (Name.find("BP_ShotGun_DP12_Wrapper_C") != std::string::npos) return "DBS";
	if (Setting::Esp_Item_S686) if (Name.find("BP_ShotGun_S686_Wrapper_C") != std::string::npos) return "S686";
	if (Setting::Esp_Item_S1897) if (Name.find("BP_ShotGun_S1897_Wrapper_C") != std::string::npos) return "S1897";
	if (Setting::Esp_Item_SawedOff) if (Name.find("BP_ShotGun_SawedOff_Wrapper_C") != std::string::npos) return "SawedOff";
	if (Setting::Esp_Item_PP19) if (Name.find("BP_MachineGun_PP19_Wrapper_C") != string::npos) return "PP19";
	if (Setting::Esp_Item_TommyGun) if (Name.find("BP_MachineGun_TommyGun_Wrapper_C") != string::npos) return "Tommy Gun";
	if (Setting::Esp_Item_MP5K) if (Name.find("BP_MachineGun_MP5K_Wrapper_C") != string::npos) return "MP5K";
	if (Setting::Esp_Item_UMP9) if (Name.find("BP_MachineGun_UMP9_Wrapper_C") != string::npos) return "UMP9";
	if (Setting::Esp_Item_Vector) if (Name.find("BP_MachineGun_Vector_Wrapper_C") != string::npos) return "Vector";
	if (Setting::Esp_Item_Uzi) if (Name.find("BP_MachineGun_Uzi_Wrapper_C") != string::npos) return "Uzi";
	if (Setting::Esp_Item_R1895) if (Name.find("BP_Pistol_R1895_Wrapper_C") != string::npos) return "R1895";
	if (Setting::Esp_Item_Vz61) if (Name.find("BP_Pistol_Vz61_Wrapper_C") != string::npos) return "Vz61";
	if (Setting::Esp_Item_P92) if (Name.find("BP_Pistol_P92_Wrapper_C") != string::npos) return "P92";
	if (Setting::Esp_Item_P18C) if (Name.find("BP_Pistol_P18C_Wrapper_C") != string::npos) return "P18C";
	if (Setting::Esp_Item_R45) if (Name.find("BP_Pistol_R45_Wrapper_C") != string::npos) return "R45";
	if (Setting::Esp_Item_P1911) if (Name.find("BP_Pistol_P1911_Wrapper_C") != string::npos) return "P1911";
	if (Setting::Esp_Item_DesertEagle) if (Name.find("BP_Pistol_DesertEagle_Wrapper_C") != string::npos) return "DesertEagle";
	if (Setting::Esp_Item_Mk14) if (Name.find("BP_WEP_Mk14_Pickup_C") != string::npos) return "Mk14";
	if (Setting::Esp_Item_762mm) if (Name.find("BP_Ammo_762mm_Pickup_C") != string::npos) return "7.62";
	if (Setting::Esp_Item_45ACP) if (Name.find("BP_Ammo_45ACP_Pickup_C") != string::npos) return "45ACP";
	if (Setting::Esp_Item_556mm) if (Name.find("BP_Ammo_556mm_Pickup_C") != string::npos) return "5.56";
	if (Setting::Esp_Item_9mm) if (Name.find("BP_Ammo_9mm_Pickup_C") != string::npos) return "9mm";
	if (Setting::Esp_Item_Magnum) if (Name.find("BP_Ammo_300Magnum_Pickup_C") != std::string::npos) return "Magnum";
	if (Setting::Esp_Item_12Guage) if (Name.find("BP_Ammo_12Guage_Pickup_C") != std::string::npos) return "12Guage";
	if (Setting::Esp_Item_BP_QK_Mid_FlashHider_Pickup_C) if (Name.find("BP_QK_Mid_FlashHider_Pickup_C") != std::string::npos) return "Flash Hider (SMG)";
	if (Setting::Esp_Item_BP_QK_Large_FlashHider_Pickup_C) if (Name.find("BP_QK_Large_FlashHider_Pickup_C") != std::string::npos) return "Flash Hider (AR)";
	if (Setting::Esp_Item_BP_QK_Large_Compensator_Pickup_C) if (Name.find("BP_QK_Large_Compensator_Pickup_C") != std::string::npos) return "Compensator (AR)";
	if (Setting::Esp_Item_BP_QK_Mid_Compensator_Pickup_C) if (Name.find("BP_QK_Mid_Compensator_Pickup_C") != std::string::npos) return "Compensator (SMG)";
	if (Setting::Esp_Item_BP_QK_Sniper_FlashHider_Pickup_C) if (Name.find("BP_QK_Sniper_FlashHider_Pickup_C") != std::string::npos) return "Flash Hider (Sniper)";
	if (Setting::Esp_Item_BP_QK_Mid_Suppressor_Pickup_C) if (Name.find("BP_QK_Mid_Suppressor_Pickup_C") != std::string::npos) return "Suppressor (SMG)";
	if (Setting::Esp_Item_BP_QT_Sniper_Pickup_C) if (Name.find("BP_QT_Sniper_Pickup_C") != std::string::npos) return "Chekpad (Sniper)";
	if (Setting::Esp_Item_BP_QT_A_Pickup_C) if (Name.find("BP_QT_A_Pickup_C") != std::string::npos) return "Tactical Stock";
	if (Setting::Esp_Item_BP_QK_DuckBill_Pickup_C) if (Name.find("BP_QK_DuckBill_Pickup_C") != std::string::npos) return "Duckbill";
	if (Setting::Esp_Item_BP_QK_Choke_Pickup_C) if (Name.find("BP_QK_Choke_Pickup_C") != std::string::npos) return "Choke";
	if (Setting::Esp_Item_BP_QT_UZI_Pickup_C) if (Name.find("BP_QT_UZI_Pickup_C") != std::string::npos) return "Stock";
	if (Setting::Esp_Item_BP_QK_Sniper_Compensator_Pickup_C) if (Name.find("BP_QK_Sniper_Compensator_Pickup_C") != std::string::npos) return "Compensator (Sniper)";
	if (Setting::Esp_Item_BP_QK_Sniper_Suppressor_Pickup_C) if (Name.find("BP_QK_Sniper_Suppressor_Pickup_C") != std::string::npos) return "Suppressor (Sniper)";
	if (Setting::Esp_Item_BP_QK_Large_Suppressor_Pickup_C) if (Name.find("BP_QK_Large_Suppressor_Pickup_C") != std::string::npos) return "Suppressor (AR)";
	if (Setting::Esp_Item_BP_DJ_Sniper_EQ_Pickup_C) if (Name.find("BP_DJ_Sniper_EQ_Pickup_C") != std::string::npos) return "Extended Quickdraw Mag (Sniper)";
	if (Setting::Esp_Item_BP_DJ_Mid_E_Pickup_C) if (Name.find("BP_DJ_Mid_E_Pickup_C") != std::string::npos) return "Extended Mag (SMG)";
	if (Setting::Esp_Item_BP_DJ_Mid_Q_Pickup_C) if (Name.find("BP_DJ_Mid_Q_Pickup_C") != std::string::npos) return "Quickdraw Mag (SMG)";
	if (Setting::Esp_Item_BP_DJ_Mid_EQ_Pickup_C) if (Name.find("BP_DJ_Mid_EQ_Pickup_C") != std::string::npos) return "Extended Quickdraw Mag (SMG)";
	if (Setting::Esp_Item_BP_DJ_Sniper_E_Pickup_C) if (Name.find("BP_DJ_Sniper_E_Pickup_C") != std::string::npos) return "Extended Mag (Sniper)";
	if (Setting::Esp_Item_BP_DJ_Sniper_Q_Pickup_C) if (Name.find("BP_DJ_Sniper_Q_Pickup_C") != std::string::npos) return "Quickdraw Mag (Sniper)";
	if (Setting::Esp_Item_BP_DJ_Large_Q_Pickup_C) if (Name.find("BP_DJ_Large_Q_Pickup_C") != std::string::npos) return "Quickdraw Mag (AR)";
	if (Setting::Esp_Item_BP_DJ_Large_EQ_Pickup_C) if (Name.find("BP_DJ_Large_EQ_Pickup_C") != std::string::npos) return "Extended Quickdraw Mag (AR)";
	if (Setting::Esp_Item_BP_DJ_Large_E_Pickup_C) if (Name.find("BP_DJ_Large_E_Pickup_C") != std::string::npos) return "Extended Mag (AR)";
	if (Setting::Esp_Item_BP_ZDD_Sniper_Pickup_C) if (Name.find("BP_ZDD_Sniper_Pickup_C") != std::string::npos) return "Mermilik";
	if (Setting::Esp_Item_BP_WB_ThumbGrip_Pickup_C) if (Name.find("BP_WB_ThumbGrip_Pickup_C") != std::string::npos) return "ThumbGrip";
	if (Setting::Esp_Item_BP_WB_LightGrip_Pickup_C) if (Name.find("BP_WB_LightGrip_Pickup_C") != std::string::npos) return "LightGrip";
	if (Setting::Esp_Item_BP_WB_HalfGrip_Pickup_C) if (Name.find("BP_WB_HalfGrip_Pickup_C") != std::string::npos) return "HalfGrip";
	if (Setting::Esp_Item_BP_WB_Vertical_Pickup_C) if (Name.find("BP_WB_Vertical_Pickup_C") != std::string::npos) return "Vertical Foregrip";
	if (Setting::Esp_Item_BP_WB_Angled_Pickup_C) if (Name.find("BP_WB_Angled_Pickup_C") != std::string::npos) return "Angled Foregrip";
	if (Setting::Esp_Item_BP_WB_Lasersight_Pickup_C) if (Name.find("BP_WB_Lasersight_Pickup_C") != string::npos) return "Lasersight";

	if (Setting::Esp_Item_BP_WB_Lasersight_Pickup_C) if (Name.find("GoldenTokenWrapper_C") != string::npos) return "GoldenTokenWrapper_C";

	if (Setting::Esp_Item_BP_WEP_Sickle_Pickup_C) if (Name.find("BP_WEP_Sickle_Pickup_C") != string::npos) return "Sickle";
	if (Setting::Esp_Item_BP_WEP_Machete_Pickup_C) if (Name.find("BP_WEP_Machete_Pickup_C") != string::npos) return "Machete";
	if (Setting::Esp_Item_BP_WEP_Cowbar_Pickup_C) if (Name.find("BP_WEP_Cowbar_Pickup_C") != string::npos) return "Levye";
	if (Setting::Esp_Item_BP_WEP_Pan_Pickup_C) if (Name.find("BP_WEP_Pan_Pickup_C") != string::npos) return "Pan";


	if (Setting::Esp_Item_Holo) if (Name.find("BP_MZJ_QX_Pickup_C") != string::npos) return "Holo";
	if (Setting::Esp_Item_RedDot) if (Name.find("BP_MZJ_HD_Pickup_C") != string::npos) return "Red Dot";
	if (Setting::Esp_Item_x2) if (Name.find("BP_MZJ_2X_Pickup_C") != string::npos) return "2x scope";
	if (Setting::Esp_Item_x3) if (Name.find("BP_MZJ_3X_Pickup_C") != string::npos) return "3x scope";
	if (Setting::Esp_Item_x4) if (Name.find("BP_MZJ_4X_Pickup_C") != string::npos) return "4x scope";
	if (Setting::Esp_Item_x6) if (Name.find("BP_MZJ_6X_Pickup_C") != string::npos) return "6x scope";
	if (Setting::Esp_Item_x8) if (Name.find("BP_MZJ_8X_Pickup_C") != string::npos) return "8x scope";


	if (Setting::Esp_Item_DP28) if (Name.find("BP_Other_DP28_Wrapper_C") != string::npos) return "DP28";
	if (Setting::Esp_Item_CrossBow) if (Name.find("BP_Other_CrossBow_Wrapper_C") != string::npos) return "CrossBow";
	if (Setting::Esp_Item_M249) if (Name.find("BP_Other_M249_Wrapper_C") != std::string::npos)return "M249";


	if (Setting::Esp_Item_Helmet1) if (Name.find("PickUp_BP_Helmet_Lv1") != string::npos) return "Helmet lv.1";
	if (Setting::Esp_Item_Helmet2) if (Name.find("PickUp_BP_Helmet_Lv2") != string::npos) return "Helmet lv.2";
	if (Setting::Esp_Item_Helmet3) if (Name.find("PickUp_BP_Helmet_Lv3") != string::npos) return "Helmet lv.3";


	if (Setting::Esp_Item_Armor1) if (Name.find("PickUp_BP_Armor_Lv1") != string::npos) return "Armor lv.1";
	if (Setting::Esp_Item_Armor2) if (Name.find("PickUp_BP_Armor_Lv2") != string::npos) return "Armor lv.2";
	if (Setting::Esp_Item_Armor3) if (Name.find("PickUp_BP_Armor_Lv3") != string::npos) return "Armor lv.3";



	if (Setting::Esp_Item_Bag1) if (Name.find("PickUp_BP_Bag_Lv1") != string::npos) return "Bag lv.1";
	if (Setting::Esp_Item_Bag2) if (Name.find("PickUp_BP_Bag_Lv2") != string::npos) return "Bag lv.2";
	if (Setting::Esp_Item_Bag3) if (Name.find("PickUp_BP_Bag_Lv3") != string::npos) return "Bag lv.3";



	if (Setting::Esp_Item_Mk12) if (Name.find("BP_Other_MK12_Wrapper_C") != std::string::npos)return "MK12";
	if (Setting::Esp_Item_FlareGun) if (Name.find("BP_Pistol_Flaregun_Wrapper_C") != string::npos) return "Flare Gun";
	if (Setting::Esp_Item_Flare) if (Name.find("BP_Ammo_Flare_Pickup_C") != string::npos) return "Flare";



	if (Setting::Esp_Item_MedKit) if (Name.find("FirstAidbox_Pickup_C") != string::npos) return "Med Kit";
	if (Setting::Esp_Item_FirstAidKit) if (Name.find("Firstaid_Pickup_C") != string::npos) return "First Aid Kit";
	if (Setting::Esp_Item_Painkiller) if (Name.find("Pills_Pickup_C") != string::npos) return "Painkiller";
	if (Setting::Esp_Item_EnergyDrink) if (Name.find("Drink_Pickup_C") != string::npos) return "Energy Drink";
	if (Setting::Esp_Item_AdrenalineSyringe) if (Name.find("Injection_Pickup_C") != string::npos) return "Adrenaline Syringe";
	if (Setting::Esp_Item_Bandage) if (Name.find("Bandage_Pickup_C") != string::npos) return "Bandage";



	if (Setting::Esp_Item_Frag) if (Name.find("BP_Grenade_Shoulei_Weapon_Wrapper_C") != string::npos) return "Frag Grenade";
	if (Setting::Esp_Item_molotov) if (Name.find("BP_Grenade_Burn_Weapon_Wrapper_C") != string::npos) return "Molotof Grenade";
	if (Setting::Esp_Item_Smoke) if (Name.find("BP_Grenade_Smoke_Weapon_Wrapper_C") != string::npos) return "Smoke Grenade";
	if (Setting::Esp_Item_tun) if (Name.find("BP_Grenade_tun_Weapon_Wrapper_C") != string::npos) return "Tun Grenade";

	//if (Setting::smokewarning) if (Name.find("BP_Grenade_Smoke_C") != string::npos) return "Smoke_Grenade";
	//if (Setting::molotovwarning) 	if (Name.find("BP_Grenade_Burn_C") != std::string::npos)return "Burn_Grenade";
	//if (Setting::fragwarning) if (Name.find("BP_Grenade_Shoulei_C") != string::npos) return "Frag_Grenade";
	//if (Setting::tunwarning) if (Name.find("BP_Grenade_tun_C") != string::npos) return "Tun_Grenade";

	//if (Setting::apple) 	if (Name.find("BP_Grenade_Apple_Weapon_Wrapper_C") != std::string::npos)return "Apple";
	if (Setting::Esp_Item_GasCan) if (Name.find("GasCan_Destructible_Pickup_C") != string::npos) return "GasCan";
	if (Setting::Esp_Item_AirDrop) if (Name.find("BP_AirDropBox_C") != string::npos) return "Air Drop";
	if (Setting::Esp_Item_AirDrop) if (Name.find("BP_AirDropBox_New_C") != string::npos) return "Air Drop";
	if (Setting::Esp_Item_DeadBox) if (Name.find("PickUpListWrapperActor") != string::npos) return "Crate Box";
	//if (Setting::Esp_Item_DeadBox) if (Name.find("Skill_UseBike_C") != string::npos) return "Skill_UseBike_C";
	//if (Setting::Esp_Item_DeadBox) if (Name.find("Skill_UseBike_B_C") != string::npos) return "Skill_UseBike_B_C";






	return "Unknown";
}
//std::string GetBoxItemByboth(DWORD code, std::string classname)
//{
//	if (classname.find("PlayerDeadInventoryBox_C") != std::string::npos)
//		return "Crate";
//	if (code == 103003 || classname.find("BP_Sniper_AWM_Wrapper_C") != std::string::npos)
//		return "AWM";
//	if (code == 103010 || classname.find("BP_Sniper_QBU_Wrapper_C") != std::string::npos)
//		return "QBU";
//	if (code == 103009 || classname.find("BP_Sniper_SLR_Wrapper_C") != std::string::npos)
//		return "SLR";
//	if (code == 103004 || classname.find("BP_Sniper_SKS_Wrapper_C") != std::string::npos)
//		return "SKS";
//	if (code == 103006 || classname.find("BP_Sniper_Mini14_Wrapper_C") != std::string::npos)
//		return "Mini14";
//	if (code == 103002 || classname.find("BP_Sniper_M24_Wrapper_C") != std::string::npos)
//		return "M24";
//	if (code == 103001 || classname.find("BP_Sniper_Kar98k_Wrapper_C") != std::string::npos)
//		return "Kar98k";
//	if (code == 103005 || classname.find("BP_Sniper_VSS_Wrapper_C") != std::string::npos)
//		return "VSS";
//	if (code == 103008 || classname.find("BP_Sniper_Win94_Wrapper_C") != std::string::npos)
//		return "Win94";
//	if (code == 101008 || classname.find("BP_Rifle_M762_Wrapper_C") != std::string::npos)
//		return "M762";
//	if (code == 101003 || classname.find("BP_Rifle_SCAR_Wrapper_C") != std::string::npos)
//		return "SCAR-L";
//	if (code == 101004 || classname.find("BP_Rifle_M416_Wrapper_C") != std::string::npos)
//		return "M416";
//	if (code == 101002 || classname.find("BP_Rifle_M16A4_Wrapper_C") != std::string::npos)
//		return "M16A-4";
//	if (code == 101009 || classname.find("BP_Rifle_Mk47_Wrapper_C") != std::string::npos)
//		return "Mk47 Mutant";
//	if (code == 101010 || classname.find("BP_Rifle_G36_Wrapper_C") != std::string::npos)
//		return "G36C";
//	if (code == 101007 || classname.find("BP_Rifle_QBZ_Wrapper_C") != std::string::npos)
//		return "QBZ";
//	if (code == 210700 || classname.find("BP_Rifle_AKM_Wrapper_C") != std::string::npos)
//		return "AKM";
//	if (code == 101005 || classname.find("BP_Rifle_Groza_Wrapper_C") != std::string::npos)
//		return "Groza";
//	if (code == 101006 || classname.find("BP_Rifle_AUG_Wrapper_C") != std::string::npos)
//		return "AUG_A3";
//	if (code == 104003 || classname.find("BP_ShotGun_S12K_Wrapper_C") != std::string::npos)
//		return "S12K";
//	if (code == 104004 || classname.find("BP_ShotGun_DP12_Wrapper_C") != std::string::npos)
//		return "DBS";
//	if (code == 104001 || classname.find("BP_ShotGun_S686_Wrapper_C") != std::string::npos)
//		return "S686";
//	if (code == 104002 || classname.find("BP_ShotGun_S1897_Wrapper_C") != std::string::npos)
//		return "S1897";
//	if (code == 106006 || classname.find("BP_ShotGun_SawedOff_Wrapper_C") != std::string::npos)
//		return "SawedOff";
//	if (code == 102005 || classname.find("BP_MachineGun_PP19_Wrapper_C") != std::string::npos)
//		return "PP19 Bizon";
//	if (code == 102004 || classname.find("BP_MachineGun_TommyGun_Wrapper_C") != std::string::npos)
//		return "TommyGun";
//	if (code == 102007 || classname.find("BP_MachineGun_MP5K_Wrapper_C") != std::string::npos)
//		return "MP5K";
//	if (code == 102002 || classname.find("BP_MachineGun_UMP9_Wrapper_C") != std::string::npos)
//		return "UMP9";
//	if (code == 102003 || classname.find("BP_MachineGun_Vector_Wrapper_C") != std::string::npos)
//		return "Vector";
//	if (code == 102001 || classname.find("BP_MachineGun_Uzi_Wrapper_C") != std::string::npos)
//		return "Uzi";
//	if (code == 106003 || classname.find("BP_Pistol_R1895_Wrapper_C") != std::string::npos)
//		return "R1895";
//	if (code == 106008 || classname.find("BP_Pistol_Vz61_Wrapper_C") != std::string::npos)
//		return "Vz61";
//	if (code == 106001 || classname.find("BP_Pistol_P92_Wrapper_C") != std::string::npos)
//		return "P92";
//	if (code == 106004 || classname.find("BP_Pistol_P18C_Wrapper_C") != std::string::npos)
//		return "P18C";
//	if (code == 106005 || classname.find("BP_Pistol_R45_Wrapper_C") != std::string::npos)
//		return "R45";
//	if (code == 106002 || classname.find("BP_Pistol_P1911_Wrapper_C") != std::string::npos)
//		return "P1911";
//	if (code == 106010 || classname.find("BP_Pistol_DesertEagle_Wrapper_C") != std::string::npos)
//		return "DesertEagle";
//	if (code == 108003 || classname.find("BP_WEP_Sickle_Pickup_C") != std::string::npos)
//		return "Sickle";
//	if (code == 108001 || classname.find("BP_WEP_Machete_Pickup_C") != std::string::npos)
//		return "Machete";
//	if (code == 107001 || classname.find("BP_WEP_Cowbar_Pickup_C") != std::string::npos)
//		return "Cross Bow";
//	if (code == 108004 || classname.find("BP_WEP_Pan_Pickup_C") != std::string::npos)
//		return "Pan";
//	if (code == 103007 || classname.find("BP_WEP_Mk14_Pickup_C") != std::string::npos)
//		return "Mk14";
//	if (code == 302001 || classname.find("BP_Ammo_762mm_Pickup_C") != std::string::npos)
//		return "7.62";
//	if (code == 305001 || classname.find("BP_Ammo_45ACP_Pickup_C") != std::string::npos)
//		return "45ACP";
//	if (code == 303001 || classname.find("BP_Ammo_556mm_Pickup_C") != std::string::npos)
//		return "5.56";
//	if (code == 301001 || classname.find("BP_Ammo_9mm_Pickup_C") != std::string::npos)
//		return "9mm";
//	if (code == 306001 || classname.find("BP_Ammo_300Magnum_Pickup_C") != std::string::npos)
//		return "300MAGNUM";
//	if (code == 304001 || classname.find("BP_Ammo_12Guage_Pickup_C") != std::string::npos)
//		return "12Guage";
//	if (code == 307001 || classname.find("BP_Ammo_Bolt_Pickup_C") != std::string::npos)
//		return "Arrows";
//	if (code == 201004 || classname.find("BP_QK_Mid_FlashHider_Pickup_C") != std::string::npos)
//		return "Alev Gizl (Haf. Mak..)";
//	if (code == 201010 || classname.find("BP_QK_Large_FlashHider_Pickup_C") != std::string::npos)
//		return "Alev Gizl (Oto.)";
//	if (code == 201009 || classname.find("BP_QK_Large_Compensator_Pickup_C") != std::string::npos)
//		return "Otomatik Kompensator";
//	if (code == 201004 || classname.find("BP_QK_Mid_Compensator_Pickup_C") != std::string::npos)
//		return "Kompensator (Haf.Mak.)";
//	if (code == 205002 || classname.find("BP_QT_A_Pickup_C") != std::string::npos)
//		return "Taktik Dipcik";
//	if (code == 201012 || classname.find("BP_QK_DuckBill_Pickup_C") != std::string::npos)
//		return "Duckbill (Pompalı)";
//	if (code == 201005 || classname.find("BP_QK_Sniper_FlashHider_Pickup_C") != std::string::npos)
//		return "Alev Gizl. Sniper";
//	if (code == 201006 || classname.find("BP_QK_Mid_Suppressor_Pickup_C") != std::string::npos)
//		return "Susturucu (Haf. Mak. Tabanca)";
//	if (code == 205003 || classname.find("BP_QT_Sniper_Pickup_C") != std::string::npos)
//		return "Chekpad Sniper";
//	if (code == 201001 || classname.find("BP_QK_Choke_Pickup_C") != std::string::npos)
//		return "Choke";
//	if (code == 205001 || classname.find("BP_QT_UZI_Pickup_C") != std::string::npos)
//		return "Dipcik (Micro UZI)";
//	if (code == 201003 || classname.find("BP_QK_Sniper_Compensator_Pickup_C") != std::string::npos)
//		return "Sniper Kompensator";
//	if (code == 201007 || classname.find("BP_QK_Sniper_Suppressor_Pickup_C") != std::string::npos)
//		return "Suppressor Sniper";
//	if (code == 201011 || classname.find("BP_QK_Large_Suppressor_Pickup_C") != std::string::npos)
//		return "Suppressor AR";
//	if (code == 204009 || classname.find("BP_DJ_Sniper_EQ_Pickup_C") != std::string::npos)
//		return "Sniper Extended";
//	if (code == 204004 || classname.find("BP_DJ_Mid_E_Pickup_C") != std::string::npos)
//		return "Uz.Haf.Sarjor";
//	if (code == 204005 || classname.find("BP_DJ_Mid_Q_Pickup_C") != std::string::npos)
//		return "Hc.Haf.Sarjor";
//	if (code == 204007 || classname.find("BP_DJ_Sniper_E_Pickup_C") != std::string::npos)
//		return "Uz.Snip.Sarjor";
//	if (code == 204008 || classname.find("BP_DJ_Sniper_Q_Pickup_C") != std::string::npos)
//		return "Hc.Snip.Sarjor";
//	if (code == 204012 || classname.find("BP_DJ_Large_Q_Pickup_C") != std::string::npos)
//		return "Hc.Oto.Sarjor";
//	if (code == 204013 || classname.find("BP_DJ_Large_EQ_Pickup_C") != std::string::npos)
//		return "Exteded AR";
//	if (code == 204011 || classname.find("BP_DJ_Large_E_Pickup_C") != std::string::npos)
//		return "Uz.Oto.Sarjor";
//	if (code == 204006 || classname.find("BP_DJ_Mid_EQ_Pickup_C") != std::string::npos)
//		return "Hc.Uz.Haf.Sarjor";
//	if (code == 205004 || classname.find("BP_ZDD_Crossbow_Q_Pickup_C") != std::string::npos)
//		return "Sadak (Arrow)";
//	if (code == 204014 || classname.find("BP_ZDD_Sniper_Pickup_C") != std::string::npos)
//		return "Mermilik";
//	if (code == 203005 || classname.find("BP_MZJ_8X_Pickup_C") != std::string::npos)
//		return "8x";
//	if (code == 203003 || classname.find("BP_MZJ_2X_Pickup_C") != std::string::npos)
//		return "2x";
//	if (code == 203001 || classname.find("BP_MZJ_HD_Pickup_C") != std::string::npos)
//		return "Lazer";
//	if (code == 203014 || classname.find("BP_MZJ_3X_Pickup_C") != std::string::npos)
//		return "3X";
//	if (code == 203002 || classname.find("BP_MZJ_QX_Pickup_C") != std::string::npos)
//		return "Holo";
//	if (code == 203015 || classname.find("BP_MZJ_6X_Pickup_C") != std::string::npos)
//		return "6x";
//	if (code == 203004 || classname.find("BP_MZJ_4X_Pickup_C") != std::string::npos)
//		return "4x";
//	if (code == 105002 || classname.find("BP_Other_DP28_Wrapper_C") != std::string::npos)
//		return "DP28";
//	if (code == 107001 || classname.find("BP_Other_CrossBow_Wrapper_C") != std::string::npos)
//		return "CrossBow";
//	if (code == 105001 || classname.find("BP_Other_M249_Wrapper_C") != std::string::npos)
//		return "M249";
//	if (code == 501006 || classname.find("PickUp_BP_Bag_Lv3_C") != std::string::npos)
//		return "Bag Lv.3";
//	if (code == 501006 || classname.find("PickUp_BP_Bag_Lv3_B_C") != std::string::npos)
//		return "Bag Lv.3";
//	if (code == 501004 || classname.find("PickUp_BP_Bag_Lv1_C") != std::string::npos)
//		return "Bag Lv.1";
//	if (code == 501004 || classname.find("PickUp_BP_Bag_Lv1_B_C") != std::string::npos)
//		return "Bag Lv.1";
//	if (code == 501005 || classname.find("PickUp_BP_Bag_Lv2_C") != std::string::npos)
//		return "Bag Lv.2";
//	if (code == 501005 || classname.find("PickUp_BP_Bag_Lv2_B_C") != std::string::npos)
//		return "Bag Lv.2";
//	if (code == 503002 || classname.find("PickUp_BP_Armor_Lv2_C") != std::string::npos)
//		return "Armour Lv.2";
//	if (code == 503002 || classname.find("PickUp_BP_Armor_Lv2_B_C") != std::string::npos)
//		return "Armour Lv.2";
//	if (code == 503001 || classname.find("PickUp_BP_Armor_Lv1_C") != std::string::npos)
//		return "Armour Lv.1";
//	if (code == 503001 || classname.find("PickUp_BP_Armor_Lv1_B_C") != std::string::npos)
//		return "Armour Lv.1";
//	if (code == 503003 || classname.find("PickUp_BP_Armor_Lv3_C") != std::string::npos)
//		return "Armour Lv.3";
//	if (code == 503003 || classname.find("PickUp_BP_Armor_Lv3_B_C") != std::string::npos)
//		return "Armour Lv.3";
//	if (code == 502002 || classname.find("PickUp_BP_Helmet_Lv2_C") != std::string::npos)
//		return "Helmet Lv.2";
//	if (code == 502002 || classname.find("PickUp_BP_Helmet_Lv2_B_C") != std::string::npos)
//		return "Helmet Lv.2";
//	if (code == 502001 || classname.find("PickUp_BP_Helmet_Lv1_C") != std::string::npos)
//		return "Helmet Lv.1";
//	if (code == 502001 || classname.find("PickUp_BP_Helmet_Lv1_B_C") != std::string::npos)
//		return "Helmet Lv.1";
//	if (code == 502003 || classname.find("PickUp_BP_Helmet_Lv3_C") != std::string::npos)
//		return "Helmet Lv.3";
//	if (code == 502003 || classname.find("PickUp_BP_Helmet_Lv3_B_C") != std::string::npos)
//		return "Helmet Lv.3";
//	if (code == 0 && classname.find("BP_VH_Buggy_2_C") != std::string::npos)
//		return "Buggy";
//	if (code == 0 && classname.find("BP_VH_Buggy_3_C") != std::string::npos)
//		return "Buggy";
//	if (code == 0 && classname.find("BP_VH_Tuk_1_C") != std::string::npos)
//		return "Tuk";
//	if (code == 602004 || classname.find("BP_Grenade_Shoulei_Weapon_Wrapper_C") != std::string::npos)
//		return "Grenade";
//	if (code == 0 && classname.find("BP_Grenade_Shoulei_C") != std::string::npos)
//		return "Bomb!";
//	if (code == 602002 || classname.find("BP_Grenade_Smoke_Weapon_Wrapper_C") != std::string::npos)
//		return "Smoke";
//	if (code == 602003 || classname.find("BP_Grenade_Burn_Weapon_Wrapper_C") != std::string::npos)
//		return "Molotof";
//	if (code == 0 && classname.find("BP_Grenade_Burn_C") != std::string::npos)
//		return "Burn!";
//	if (code == 602005 || classname.find("BP_Grenade_Apple_Weapon_Wrapper_C") != std::string::npos)
//		return "Apple";
//	if (code == 601003 || classname.find("Pills_Pickup_C") != std::string::npos)
//		return "Painkiller";
//	if (code == 601002 || classname.find("Injection_Pickup_C") != std::string::npos)
//		return "Adrenaline Syringe";
//	if (code == 601001 || classname.find("Drink_Pickup_C") != std::string::npos)
//		return "Energy Drink";
//	if (code == 601005 || classname.find("Firstaid_Pickup_C") != std::string::npos)
//		return "FirstaidKit";
//	if (code == 601004 || classname.find("Bandage_Pickup_C") != std::string::npos)
//		return "Bandage";
//	if (code == 0 && classname.find("BP_PlayerPawn_C") != std::string::npos)
//		return "BP_PlayerPawn_C";
//	if (code == 0 && classname.find("BP_PlayerPawn_ZNQ_C") != std::string::npos)
//		return "BP_PlayerPawn_ZNQ_C";
//	if (code == 202006 || classname.find("BP_WB_ThumbGrip_Pickup_C") != std::string::npos)
//		return "Thumb　Grip";
//	if (code == 202007 || classname.find("BP_WB_Lasersight_Pickup_C") != std::string::npos)
//		return "Laser Sight";
//	if (code == 202001 || classname.find("BP_WB_Angled_Pickup_C") != std::string::npos)
//		return "Angled";
//	if (code == 202004 || classname.find("BP_WB_LightGrip_Pickup_C") != std::string::npos)
//		return "Light Grip";
//	if (code == 0 && classname.find("BP_WB_HalfGrip_Pickup_C") != std::string::npos)
//		return "Half Grip";
//	if (code == 202002 || classname.find("BP_WB_Vertical_Pickup_C") != std::string::npos)
//		return "Vertical Grip";
//	if (code == 0 && classname.find("VH_Motorcycle_C") != std::string::npos)
//		return "Motor";
//	if (code == 0 && classname.find("VH_Motorcycle_1_C") != std::string::npos)
//		return "Motor";
//	if (code == 0 && classname.find("Mirado_open_4_C") != std::string::npos)
//		return "Mirado Open";
//	if (code == 0 && classname.find("VH_Dacia_C") != std::string::npos)
//		return "Toros";
//	if (code == 0 && classname.find("VH_Dacia_1_C") != std::string::npos)
//		return "Toros";
//	if (code == 0 && classname.find("VH_Dacia_4_C") != std::string::npos)
//		return "Toros";
//	if (code == 0 && classname.find("Rony_01_C") != std::string::npos)
//		return "Rony";
//	if (code == 0 && classname.find("VH_Snowmobile_C") != std::string::npos)
//		return "Snowmobile";
//	if (code == 0 && classname.find("Mirado_close_3_C") != std::string::npos)
//		return "Mirado Blue";
//	if (code == 0 && classname.find("LadaNiva_01_C") != std::string::npos)
//		return "Lada Niva";
//	if (code == 0 && classname.find("VH_Scooter_C") != std::string::npos)
//		return "Scooter";
//	if (code == 0 && classname.find("VH_BRDM_C") != std::string::npos)
//		return "Tank";
//	if (code == 0 && classname.find("PickUp_02_C") != std::string::npos)
//		return "PickUp";
//	if (code == 0 && classname.find("VH_MiniBus_01_C") != std::string::npos)
//		return "MiniBus";
//	if (code == 0 && classname.find("VH_MotorcycleCart_C") != std::string::npos)
//		return "Motor 3Teker";
//	if (code == 0 && classname.find("VH_MotorcycleCart_1_C") != std::string::npos)
//		return "Motor 3Teker";
//	if (code == 0 && classname.find("VH_Snowbike_C") != std::string::npos)
//		return "Snowbike";
//	if (code == 0 && classname.find("VH_PG117_C") != std::string::npos)
//		return "Boat";
//	if (code == 0 && classname.find("VH_UAZ01_C") != std::string::npos)
//		return "UAZ1";
//	if (code == 0 && classname.find("VH_UAZ02_C") != std::string::npos)
//		return "UAZ2";
//	if (code == 0 && classname.find("VH_UAZ03_C") != std::string::npos)
//		return "UAZ2";
//	if (code == 0 && classname.find("VH_UAZ04_C") != std::string::npos)
//		return "UAZ2";
//	if (code == 0 && classname.find("AquaRail_1_C") != std::string::npos)
//		return "JetSki";
//	if (code == 106007 || classname.find("BP_Pistol_Flaregun_Wrapper_C") != std::string::npos)
//		return "Flaregun";
//	if (code == 0 && classname.find("BP_AirDropBox_C") != std::string::npos)
//		return "AirDrop";
//	if (code == 0 && classname.find("BP_AirDropPlane_C") != std::string::npos)
//		return "Plane";
//	if (code == 0 && classname.find("PlayerDeadInventoryBox_C") != std::string::npos)
//		return "Player Box";
//	if (code == 603001 || classname.find("GasCan_Destructible_Pickup_C") != std::string::npos)
//		return "Gas Can";
//	if (code == 0 && classname.find("PickUpListWrapperActor") != std::string::npos)
//		return "Create Box";
//	if (code == 0 && classname.find("VH_Dacia_2_C") != std::string::npos)
//		return "Toros";
//	if (code == 0 && classname.find("VH_Dacia_3_C") != std::string::npos)
//		return "Toros";
//	if (code == 3000312 || classname.find("BP_GameCoin_Pickup_C") != std::string::npos)
//		return "GameCoin";
//	if (code == 0 && classname.find("BP_BlindBoxMachine_C") != std::string::npos)
//		return "BlindBoxMachine";
//	if (code == 0 && classname.find("BP_MiniGameMachine_C") != std::string::npos)
//		return "MiniGameMachine";
//	if (code == 0 && classname.find("BP_Grenade_ColorBall_C") != std::string::npos)
//		return "ColorBall";
//	if (code == 0 && classname.find("AirDropListWrapperActor") != std::string::npos)
//		return "AirDrop";
//	if (code == 601006 || classname.find("FirstAidbox_Pickup_C") != std::string::npos)
//		return "Medkit";
//	if (code == 308001 || classname.find("BP_Ammo_Flare_Pickup_C") != std::string::npos)
//		return "Flaregun";
//	if (code == 501003 || classname.find("PickUp_BP_Bag_Lv3_Inbox_C") != std::string::npos)
//		return "Bag Lv.3";
//	if (code == 501002 || classname.find("PickUp_BP_Bag_Lv2_Inbox_C") != std::string::npos)
//		return "Bag Lv.2";
//	if (code == 501001 || classname.find("PickUp_BP_Bag_Lv1_Inbox_C") != std::string::npos)
//		return "Bag Lv.1";
//	if (code == 201002 || classname.find("BP_QK_Mid_Compensator_Inbox_C") != std::string::npos)
//		return "Kompensator (Haf.Mak.)";
//	if (code == 502005 || classname.find("PickUp_BP_Helmet_Lv2_Inbox_C") != std::string::npos)
//		return "Helmet Lv.2";
//	if (code == 403989 || classname.find("PickUp_BP_Ghillie_4_C") != std::string::npos)
//		return "Suit - Arctic";
//	if (code == 403045 || classname.find("PickUp_BP_Ghillie_1_C") != std::string::npos)
//		return "Suit - Woodland";
//	if (code == 403187 || classname.find("PickUp_BP_Ghillie_2_C") != std::string::npos)
//		return "Suit - Desert";
//	if (code == 403188 || classname.find("PickUp_BP_Ghillie_3_C") != std::string::npos)
//		return "Suit - Desert";
//	return "tatti";
//}
bool IsBox(std::string classname)
{
	if (classname.find("BP_AirDropBox_C") != std::string::npos)
		return true;
	if (classname.find("BP_AirDropBox_New_C") != std::string::npos)
		return true;
	if (classname.find("PickUpListWrapperActor") != std::string::npos || classname.find("AirDropListWrapperActor") != std::string::npos)
		return true;

	return false;
}
string GetVehicleType(string Name)
{
	if (Name.find("VH_BRDM_C") != std::string::npos) return "BRDM";
	if (Name.find("PickUp_BP_Bike_2_C") != std::string::npos) return "PickUp_BP_Bike_2_C";
	if (Name.find("Skill_UseBike_B_C") != std::string::npos) return "Skill_UseBike_B_C";
	if (Name.find("Skill_UseBike_C") != std::string::npos) return "Skill_UseBike_C";
	if (Name.find("VH_Scooter_C") != std::string::npos) return "Scooter";
	if (Name.find("VH_Motorcycle_1_C") != std::string::npos) return "Motorcycle";
	if (Name.find("VH_MotorcycleCart_1_C") != std::string::npos) return "MotorcycleCart";
	if (Name.find("VH_Snowbike_C") != std::string::npos) return "Snowbike";
	if (Name.find("VH_Snowmobile_C") != std::string::npos) return "Snowmobile";
	if (Name.find("BP_VH_Tuk_1_C") != std::string::npos) return "Tuk Tuk";
	if (Name.find("BP_VH_Buggy_2_C") != std::string::npos) return "Buggy";
	if (Name.find("Mirado_close_3_C") != std::string::npos) return "Mirado";
	if (Name.find("Mirado_open_4_C") != std::string::npos) return "Mirado";
	if (Name.find("BP_VH_Bigfoot_C") != std::string::npos) return "Monster Track";
	if (Name.find("PickUp_07_C") != std::string::npos) return "PickUp";
	if (Name.find("VH_Dacia_C") != std::string::npos) return "Dacia";
	if (Name.find("VH_UTV_C") != std::string::npos) return "UTV";
	if (Name.find("Rony_01_C") != std::string::npos) return "Rony";
	if (Name.find("UAZ") != std::string::npos) return "UAZ";
	if (Name.find("VH_MiniBus_01_C") != std::string::npos) return "Bus";
	if (Name.find("VH_ATV1_C") != std::string::npos) return "ATV";
	if (Name.find("BP_AirDropPlane_C") != std::string::npos) return "Plane";
	if (Name.find("PG117") != std::string::npos) return "PG117";
	if (Name.find("AquaRail") != std::string::npos) return "Aquarail";
	if (Name.find("CoupeRB") != std::string::npos) return "Coupe RB";
	if (Name.find("ModelY") != std::string::npos) return "Tesla";
	if (Name.find("G-38Anti-GravityMotorcycle") != std::string::npos) return "Gravity";
	//if (Name.find("Glider") != std::string::npos) return "Glider";
	return "Unknown";
}





DWORD Data2::NetDriver;
DWORD Data2::LocalPlayer;
DWORD Data2::LocalController;
DWORD Data2::PIPE;
DWORD Data2::PlayerController;
DWORD Data2::EntityAddress = 0;
vector<cardata> Data2::vehicledatar = {};
vector<BombAlert> Data2::BombAlertList = {};
vector<Itemline> Data2::ItemListline = {};
vector<impItem> Data2::impItemList = {};
vector<Tracking> Data2::vehicledata = {};
//std::string getNameFromId(int ID) {
//	DWORD64 fNamePtr = mem->Read<DWORD64>(GNamesOffset + (ID / 0x4000) * 8);
//	DWORD64 fName = mem->Read<DWORD64>(fNamePtr + 8 * (ID % 0x4000));
//	char name[64];
//	ZeroMemory(name, sizeof(name));
//	if (ReadProcessMemory(global->hProcess, (LPVOID)(fName + 16), name, sizeof(name) - 2, NULL) != 0)
//	{
//		return std::string(name);
//	}
//	else {
//		return std::string("FAIL");
//	}
//}
//
//// Dump all GNames
//std::ofstream myfile;
//myfile.open("Gnames.txt");
//
//for (int i = 0; i < 200000; i++)
//{
//	if (getNameFromId(i) != "FAIL")
//	{
//		myfile << i << "  |  " << getNameFromId(i) << "\n";
//	}
//}
//
//myfile.close();
std::string playerstatus2(int GetEnemyState)
{
	switch (GetEnemyState)
	{
	case 520:
	case 544:
	case 656:
	case 521:
	case 528:
		return "Aiming";
		break;

	default:
		return "";
		break;
	}
}
std::string playerstatus(int GetEnemyState)
{
	switch (GetEnemyState)
	{
	case 0:
		return "AFK";
		break;
	case 1:
		return "In Water";
		break;
	case 268435464:
		return "Play Emotion";
		break;
	case 8:
		return "Stand";
		break;
	case 520:
	case 544:
	case 656:
	case 521:
	case 528:
		return "Aiming";
		break;
	case 1680:
	case 1672:
	case 1673:
	case 1032:
	case 1544:
	case 1545:
	case 1033:
		return "Peek";
		break;
	case 9:
		return "Walking";
		break;
	case 11:
		return "Running";
		break;
	case 4194304:
		return "Swimming";
		break;
	case 32784:
		return "Reviving";
		break;
	case 16777224:
		return "Climbing";
		break;
	case 8200:
	case 8208:
		return "Punching";
		break;
	case 16:
	case 17:
	case 19:
		return "Crouch";
		break;
	case 32:
	case 33:
	case 35:
	case 5445:
	case 762:
		return "Snake";
		break;
	case 72:
	case 73:
	case 75:
		return "Jumping";
		break;
	case 264:
	case 272:
	case 273:
	case 288:
	case 265:
	case 329:
		return "Reloading";
		break;
	case 137:
	case 144:
	case 201:
	case 145:
	case 160:
	case 649:
	case 648:
	case 1160:
	case 1161:
	case 1169:
		return "Firing";
		break;
	case 131070:
	case 131071:
	case 131072:
	case 131073:
	case 131075:
	case 131074:
		return "Knocked";
		break;
	case 33554440:
	case 524296:
	case 1048584:
	case 524288:
		return "Driving";
		break;
	case 16392:
	case 16393:
	case 16401:
	case 16416:
	case 16417:
	case 16457:
	case 16400:
	case 17401:
	case 17417:
	case 17425:
	case 17424:
		return "Throwing Bomb";
		break;
	default:
		return "";
		break;
	}
}
DWORD dGett(DWORD addr) {
	DWORD buff;
	ReadProcessMemory(Game::hProcess, (LPCVOID)addr, &buff, sizeof(DWORD), NULL);
	return buff;
}
INT iGet(INT addr) {

	INT buff;
	ReadProcessMemory(Game::hProcess, (LPCVOID)addr, &buff, sizeof(INT), NULL);
	return buff;

}
std::vector<std::string> TotalPlayerAiming = {};
ImVec4 to_vec42(float r, float g, float b, float a)
{
	return ImVec4(r / 255.0, g / 255.0, b / 255.0, a / 255.0);
}
FLOAT GetDistance(VECTOR3 to, VECTOR3 from)
{
	float deltaX = to.X - from.X;
	float deltaY = to.Y - from.Y;
	float deltaZ = to.Z - from.Z;

	return (float)sqrt(deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ);

}
template <typename I> std::string n2hexstr(I w, size_t hex_len = sizeof(I) << 1) {
	static const char* digits = "0123456789ABCDEF";
	std::string rc(hex_len, '0');
	for (size_t i = 0, j = (hex_len - 1) * 4; i < hex_len; ++i, j -= 4)
		rc[i] = digits[(w >> j) & 0x0f];
	return rc;
}
FRotator Data2::ToRotator(VECTOR3 local, VECTOR3 target) {
	VECTOR3 rotation;
	rotation.X = local.X - target.X;
	rotation.Y = local.Y - target.Y;
	rotation.Z = local.Z - target.Z;
	FRotator newViewAngle = { 0 };
	newViewAngle.Pitch = std::atan2(rotation.Z, std::sqrt(rotation.X * rotation.X + rotation.Y * rotation.Y)) * 180.0f / 3.1415926535897;
	newViewAngle.Yaw = std::atan2(rotation.Y, rotation.X) * 180.0f / 3.1415926535897;
	newViewAngle.Roll = (float)0.f;
	return newViewAngle;
}

VECTOR3 SubVec(VECTOR3 Src, VECTOR3 Dst)
{
	VECTOR3 Result;
	Result.X = Src.X - Dst.X;
	Result.Y = Src.Y - Dst.Y;
	Result.Z = Src.Z - Dst.Z;
	return Result;
}
float MagVec(VECTOR3 Vec)
{
	return sqrtf(Vec.X * Vec.X + Vec.Y * Vec.Y + Vec.Z * Vec.Z);
}
float GetDistVec(VECTOR3 Src, VECTOR3 Dst)
{
	VECTOR3 Result = SubVec(Src, Dst);
	return MagVec(Result);
}







DWORD GETUE4()
{
	if (Utility::ReadMemoryEx<BYTE>(0x200) == 0x01)
	{
		int libue4 = Utility::ReadMemoryEx<int>(Offset::UE4Pointer);
		return libue4;
	}
	else
	{
		int libue4 = Utility::ReadMemoryEx<int>(Offset::UE4Pointer);
		return libue4;
	}
}


//DWORD GETUE4BASE()
//{
//	BYTE libUE4Header[] = { 0x7F,0x45,0x4C,0x46,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x00,0x28,0x00,0x01,0x00,0x00,0x00,0x00,0xF0,0xF1,0x01,0x34,0x00,0x00,0x00,0xC0,0x6E,0xF1,0x07,0x00,0x02,0x00,0x05,0x34,0x00,0x20,0x00,0x0D,0x00,0x28,0x00 };
//	std::vector<DWORD_PTR> Game_UE4;
//	Utility::MemSearch(libUE4Header, sizeof(libUE4Header), 0x30000000, 0x6F000000, 0, 0, Game_UE4);
//	if (Game_UE4.size() != 0)
//	{
//		return Game_UE4[0];
//	}
//	return Game_UE4[0];
//}

BOOL Data2::TEST_VIS_TRACK = 0;

VECTOR2 static ClampAngle(VECTOR2 qaAng)
{
	VECTOR2 ret;
	ret.X = qaAng.X;
	ret.Y = qaAng.Y;

	if (qaAng.X > 89.0f && qaAng.X <= 180.0f)
		ret.X = 89.0f;

	if (qaAng.X > 180.0f)
		ret.X = qaAng.X - 360.0f;

	if (qaAng.X < -89.0f)
		ret.X = -89.0f;

	if (qaAng.Y > 180.0f)
		ret.Y = qaAng.Y - 360.0f;

	if (qaAng.Y < -180.0f)
		ret.Y = qaAng.Y + 360.0f;

	//ret.Z = 0;

	return ret;
}

VECTOR2 ToRotatorrrrr(VECTOR3 local, VECTOR3 target)
{
	FLOAT xDif = target.X - local.X;
	FLOAT yDif = target.Y - local.Y;
	FLOAT zDif = target.Z - local.Z;

	VECTOR2 AimRotation = { 0 };
	float Hyp = sqrt(xDif * xDif + yDif * yDif);
	AimRotation.X = atan2(zDif, Hyp) * 180.0f / 3.1415926535897f;
	AimRotation.Y = atan2(yDif, xDif) * 180.0f / 3.1415926535897f;

	return AimRotation;
}

std::vector<DWORD> TempEntityList = {};
std::vector<DWORD> TempEntityList2 = {};

void Data2::Cache()
{
	if (Offset::UE4 == NULL)
		Offset::UE4 = Utility::ReadMemoryEx<int>(Offset::UE4Pointer);
	if (Offset::UE4 != 0) {
	}

	std::vector<DWORD> TempList = {};
	std::vector<DWORD> TempList2 = {};
	for (;;)
	{
		DWORD uWorlds = Utility::ReadMemoryEx<DWORD>(Utility::ReadMemoryEx<DWORD>(Offset::UE4 + Offset::Gworld) + 0x3C);
		if (uWorlds != 0)
		{
			DWORD Level = Utility::ReadMemoryEx<DWORD>(uWorlds + Offset::PersistentLevel);
			if (Level != 0)
			{
				DWORD entityEntry = Utility::ReadMemoryEx<DWORD>(Level + Offset::EntityList);
				INT entityCount = Utility::ReadMemoryEx<INT>(Level + Offset::EntityCount);
				if (entityCount > 1028)
					entityCount = 1028;

				for (int i = 0; i < entityCount; i++)
				{
					DWORD entityAddv = Utility::ReadMemoryEx<DWORD>(entityEntry + i * 4);
					if (!entityAddv)
						continue;
					if (entityAddv == (DWORD)nullptr || entityAddv == -1 || entityAddv == NULL)
						continue;
					int entityStruct = Utility::ReadMemoryEx<int>(entityAddv + 16);
					string entityType = GetEntityType(Offset::UE4 + Offset::Gname, entityStruct);
					if (entityType == "")
						continue;
					if (entityType == "None")
						continue;
					if (GetAsyncKeyState(VK_F10))
					{
						TempList.push_back(entityAddv);
						TempList2.push_back(entityAddv);

					}
					else
					{
						if (IsPlayer(entityType))
						{
							TempList.push_back(entityAddv);
						}

						string VehicleType = GetVehicleType(entityType);
						if (VehicleType.find("Unknown") == std::string::npos)
						{
							TempList.push_back(entityAddv);
						}

						string GrType = GetGrenadeType(entityType);
						if (GrType.find("tatti") == std::string::npos)
						{
							TempList.push_back(entityAddv);
						}

						string ItemType = GetItemType(entityType);
						if (ItemType.find("Unknown") == std::string::npos)
						{
							TempList2.push_back(entityAddv);
						}
					}
				}
			}
		}



		TempEntityList.clear();
		TempEntityList = TempList;
		TempList.clear();


		TempEntityList2.clear();
		TempEntityList2 = TempList2;
		TempList2.clear();

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
}

std::vector<AActor> Data2::AActorList = {};
std::vector<Vehicle> Data2::VehicleList = {};
std::vector<A_Grenade> Data2::GrenadeList = {};
void Data2::Cache2()
{
	INT MyTeamId = 9999;
	for (;;)
	{
		std::vector<AActor> AActorList = {};
		std::vector<Vehicle> VehicleList = {};
		std::vector<A_Grenade> Grenades = {};

		DWORD uWorlds = Utility::ReadMemoryEx<DWORD>(Utility::ReadMemoryEx<DWORD>(Offset::UE4 + Offset::Gworld) + 0x3C);  //// doubt
		if (uWorlds != 0)
		{
			Data2::NetDriver = Utility::ReadMemoryEx<DWORD>(uWorlds + Offset::NetDriver);
			if (Data2::NetDriver != 0)
			{
				DWORD NetConnection = Utility::ReadMemoryEx<DWORD>(Data2::NetDriver + Offset::ServerConnection);
				if (NetConnection != 0)
				{
					PlayerController = Utility::ReadMemoryEx<DWORD>(NetConnection + Offset::PlayerController);
					if (PlayerController != 0)
					{
						LocalPlayer = Utility::ReadMemoryEx<DWORD>(PlayerController + Offset::AcknowledgedPawn);
						LocalController = Utility::ReadMemoryEx<DWORD>(Data2::LocalPlayer + Offset::STExtraPlayerCharacter::STPlayerController);

						unsigned int mapUiMarkManager = Utility::ReadMemoryEx<UINT>(PlayerController + Offset::BP_MapUIMarkManager);
						unsigned int pExtraGameState = Utility::ReadMemoryEx<UINT>(mapUiMarkManager + Offset::pExtraGameState);
						alivePlayerNum = Utility::ReadMemoryEx<UINT>(pExtraGameState + Offset::AlivePlayerNum);
						aliveTeamNum = Utility::ReadMemoryEx<UINT>(pExtraGameState + Offset::AliveTeamNum);
						OnlinePlayer = Utility::ReadMemoryEx<UINT>(pExtraGameState + Offset::OnlinePlayer);
						GameIDD = Utility::ReadMemoryEx<UINT>(pExtraGameState + 0xb50);//uint64 GameID;//[Offset:
						MyPlayerWorld = Utility::ReadMemoryEx<DWORD>(LocalPlayer + Offset::RootComponent);
						bIsWeaponFiring = Utility::ReadMemoryEx<bool>(LocalPlayer + Offset::bIsWeaponFiring);

						if (PlayerController != 0)
						{
							PlayerCameraManager = Utility::ReadMemoryEx<DWORD>(PlayerController + Offset::PlayerCameraManager);
							if (PlayerCameraManager != 0)
							{
								CameraCacheEntry222 = Utility::ReadMemoryEx<FMinimalViewInfo>(PlayerCameraManager + Offset::CameraCache + 0x10);
								CameraCache = Utility::ReadMemoryEx<CameraCacheEntry>(PlayerCameraManager + Offset::CameraCache);
							}
						}

						for (int i = 0; i < TempEntityList.size(); i++)
						{

							DWORD entityAddv = TempEntityList[i];
							if (!entityAddv)
								continue;
							if (entityAddv == (DWORD)nullptr || entityAddv == -1 || entityAddv == NULL)
								continue;
							int entityStruct = Utility::ReadMemoryEx<int>(entityAddv + 0x10);
							string entityType = GetEntityType(Offset::UE4 + Offset::Gname, entityStruct);
							if (entityType == "")
								continue;
							if (entityType == "None")
								continue;
							if (IsPlayer(entityType))
							{
								bool IsDead = Utility::ReadMemoryEx<bool>(entityAddv + Offset::bDead);
								if (IsDead == true)
									continue;
								//bool Status = Utility::ReadMemoryEx<int>(entityAddv + Offset::CurrentStates);
								//if (Status == 0)
								//	continue;
								AActor AActor = {};
								DWORD playerWorld = Utility::ReadMemoryEx<DWORD>(entityAddv + Offset::RootComponent);
								if (playerWorld != 0)
								{
									//AActor.Location = Utility::ReadMemoryEx<VECTOR3>(playerWorld + Offset::RelativeLocation);
									AActor.Position = Utility::ReadMemoryEx<VECTOR3>(playerWorld + Offset::RelativeLocation);
									AActor.RelativeRotation = Utility::ReadMemoryEx<FRotator>(playerWorld + Offset::RelativeRotation);
									//AActor.Velocity = Utility::ReadMemoryEx<VECTOR3>(Utility::ReadMemoryEx<DWORD>(entityAddv + Offset::CharacterMovement) + Offset::LastUpdateVelocity);
									AActor.Velocity = Utility::ReadMemoryEx<VECTOR3>(playerWorld + 0x1a0);//Vector ComponentVelocity;//[Offset: 0x1a0, Size: 12]
								}


								//int dis = CameraCacheEntry.Location.Distance(AActor.Location) / 100.f;
								//if (dis > 700)
								//	continue;

								AActor.Address = entityAddv;
								AActor.TeamId = Utility::ReadMemoryEx<int>(entityAddv + Offset::TeamId);
								AActor.Health = Utility::ReadMemoryEx<FLOAT>(entityAddv + Offset::Health);
								AActor.HealthMax = Utility::ReadMemoryEx<FLOAT>(entityAddv + Offset::HealthMax);
								AActor.IsBot = Utility::ReadMemoryEx<BOOL>(entityAddv + Offset::IsBot);
								AActor.Status = Utility::ReadMemoryEx<INT>(entityAddv + Offset::CurrentStates);
								AActor.ScopeFov = Utility::ReadMemoryEx<FLOAT>(entityAddv + Offset::ScopeFov);
								AActor.KnockedHealth = Utility::ReadMemoryEx<FLOAT>(entityAddv + Offset::KnockHealth);



								if (Setting::plr_wpn)
								{
									std::string wep = PlayerWeapon(GetEntityType2(Utility::ReadMemoryEx<DWORD>(Utility::ReadMemoryEx<DWORD>(entityAddv + Offset::CurrentReloadWeapon2) + 16)));
									if (wep == "")
									{
										wep = "Fist";
									}
									AActor.weapon = wep;
								}


								AActor.Nation = GetPlayerFlag(Utility::ReadMemoryEx<DWORD>(entityAddv + Offset::Nation));
								AActor.Name = GetPlayerName1(Utility::ReadMemoryEx<DWORD>(entityAddv + Offset::Name));
								AActor.NameT = entityType;
								AActor.PlayerId = GetPlayerName(Utility::ReadMemoryEx<DWORD>(entityAddv + Offset::PlayerUID));


								AActor.PlayerDead = Utility::ReadMemoryEx<bool>(entityAddv + Offset::bDead);
								AActor.VISIBLE = Utility::ReadMemoryEx<BOOL>(entityAddv + 0x1D2);
								if (AActor.VISIBLE == 1)
								{
									AActor.VISIBLE2 = 1;
								}
								else
								{
									AActor.VISIBLE2 = 0;
								}


								if (AActor.Address == LocalPlayer)
								{
									MyTeamId = AActor.TeamId;
									continue;
								}
								if (AActor.TeamId == MyTeamId) continue;


								if (Setting::Enenmyaim)
								{
									VECTOR2 RelativeRotation = Utility::ReadMemoryEx<VECTOR2>(playerWorld + Offset::RelativeRotation);
									AActor.RelativeRotationnnn = ClampAngle(RelativeRotation);
									DWORD LocalRoot = Utility::ReadMemoryEx<DWORD>(LocalPlayer + Offset::RootComponent);
									VECTOR3 MyPositionnnn = Utility::ReadMemoryEx<VECTOR3>(LocalRoot + Offset::Position);
									VECTOR3 MyHEad = MyPositionnnn;
									VECTOR3 EnemyHed = AActor.Position;
									auto Angle = ToRotatorrrrr(EnemyHed, MyHEad);
									AActor.ToRotatorToMe = ClampAngle(Angle);
									float num = AActor.ToRotatorToMe.Y - AActor.RelativeRotationnnn.Y;
									AActor.isAimingAtMe = false;
									if (num <= 6.0f)
									{
										if (num >= -6.0f)
										{
											AActor.isAimingAtMe = true;
										}
									}
								}


								DWORD MeshAddv = Utility::ReadMemoryEx<DWORD>(AActor.Address + Offset::Mesh);
								if (!MeshAddv)continue;
								DWORD BodyAddv = MeshAddv + Offset::BodyAddv;
								if (!BodyAddv)continue;
								DWORD BoneAddv = Utility::ReadMemoryEx<DWORD>(MeshAddv + Offset::MinLOD) + 48;
								if (!BoneAddv)continue;
								if (BoneAddv)
								{
									AActor.HeadPos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 5 * 48);
									if (!AActor.HeadPos.X)continue;
									AActor.ChestPos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 4 * 48);
									if (!AActor.ChestPos.X)continue;
									AActor.PelvisPos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 1 * 48);
									AActor.lSholderPos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 11 * 48);
									AActor.rSholderPos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 32 * 48);
									AActor.lElbowPos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 12 * 48);
									AActor.rElbowPos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 33 * 48);
									AActor.lWristPos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 63 * 48);
									AActor.rWristPos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 62 * 48);
									AActor.lThighPos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 52 * 48);
									AActor.rThighPos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 56 * 48);
									AActor.lKneePos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 53 * 48);
									AActor.rKneePos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 57 * 48);
									AActor.lAnklePos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 54 * 48);
									AActor.rAnklePos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 58 * 48);
									AActorList.push_back(AActor);
									continue;
								}
								else { continue; }
							}

							if (Setting::Vehicle)
							{
								string VehicleType = GetVehicleType(entityType);
								if (VehicleType.find("Unknown") == std::string::npos)
								{
									Vehicle Vehicle = { };
									DWORD playerWorld = Utility::ReadMemoryEx<DWORD>(entityAddv + Offset::RootComponent);
									if (playerWorld != 0)
									{
										Vehicle.Position = Utility::ReadMemoryEx<VECTOR3>(playerWorld + Offset::RelativeLocation);
									}

									//int dis = CameraCacheEntry222.Location.Distance(Vehicle.Position) / 100.f;
									//if (dis > 12)
									//	continue;

									DWORD VehicleCommonComponent = Utility::ReadMemoryEx<DWORD>(entityAddv + Offset::VehicleCommon);
									if (VehicleCommonComponent != 0)
									{
										Vehicle.Name = VehicleType;
										Vehicle.hp = (int)(Utility::ReadMemoryEx<float>(VehicleCommonComponent + Offset::HP));
										Vehicle.oil = (float)(Utility::ReadMemoryEx<float>(VehicleCommonComponent + Offset::Fuel) * 100 / Utility::ReadMemoryEx<float>(VehicleCommonComponent + Offset::FuelMax));
										if (Utility::ReadMemoryEx<float>(entityAddv + Offset::lastForwardSpeed) > 8 || Utility::ReadMemoryEx<float>(entityAddv + Offset::lastForwardSpeed) < -8)
										{
											Vehicle.driving = true;
										}
										else
										{
											Vehicle.driving = false;
										}

										if (Vehicle.hp == 0 || Vehicle.hp < 0 || Vehicle.oil < 0 || Vehicle.oil > 101)
										{
											continue;
										}
										else
										{
											VehicleList.push_back(Vehicle);
										}
									}
								}
							}

							if (Setting::Warning)
							{
								string GrType = GetGrenadeType(entityType);
								if (GrType.find("tatti") == std::string::npos)
								{
									A_Grenade Grenade = { };
									DWORD playerWorld = Utility::ReadMemoryEx<DWORD>(entityAddv + Offset::RootComponent);
									if (playerWorld != 0)
									{
										Grenade.Position = Utility::ReadMemoryEx<VECTOR3>(playerWorld + Offset::RelativeLocation);
									}

									//int dis = CameraCacheEntry.Location.Distance(Grenade.Position) / 100.f;
									//if (dis > 100)
									//	continue;

									Grenade.Name = GrType;

									Grenades.push_back(Grenade);
								}
							}

						}
					}
				}
			}

		}
		Data2::AActorList.clear();
		Data2::VehicleList.clear();
		Data2::GrenadeList.clear();

		Data2::AActorList = AActorList;
		Data2::VehicleList = VehicleList;
		Data2::GrenadeList = Grenades;

		AActorList.clear();
		VehicleList.clear();
		Grenades.clear();

		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}


std::vector<Item> Data2::ItemList = {};
void Data2::Cache3()
{
	for (;;)
	{
		std::vector<Item> ItemList = {};
		for (int i = 0; i < TempEntityList2.size(); i++)
		{
			DWORD entityAddv = TempEntityList2[i];
			int entityStruct = Utility::ReadMemoryEx<int>(entityAddv + 0x10);
			std::string entityType = GetEntityType2(entityStruct);

			if (Setting::Item)
			{
				if (GetAsyncKeyState(VK_F10))
				{
					string ItemType = entityType;

					Item Item = { };
					DWORD playerWorld = Utility::ReadMemoryEx<DWORD>(entityAddv + Offset::RootComponent);
					if (playerWorld != 0)
					{
						Item.Position = Utility::ReadMemoryEx<VECTOR3>(playerWorld + Offset::RelativeLocation);
					}

					//int dis = CameraCacheEntry.Location.Distance(AActor.origin) / 100.f;
					//if (dis > 100) continue;

					Item.Address = entityAddv;
					Item.Name = ItemType;
					ItemList.push_back(Item);

				}
				else
				{
					string ItemType = GetItemType(entityType);
					if (ItemType.find("Unknown") == std::string::npos)
					{
						Item Item = { };
						DWORD playerWorld = Utility::ReadMemoryEx<DWORD>(entityAddv + Offset::RootComponent);
						if (playerWorld != 0)
						{
							Item.Position = Utility::ReadMemoryEx<VECTOR3>(playerWorld + Offset::RelativeLocation);
						}

						//int dis = CameraCacheEntry.Location.Distance(AActor.origin) / 100.f;
						//if (dis > 100) continue;

						Item.Address = entityAddv;
						Item.Name = ItemType;
						ItemList.push_back(Item);
					}
				}
			}

		}

		Data2::ItemList.clear();

		Data2::ItemList = ItemList;

		ItemList.clear();

		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}
}



//void Data2::Cache3()
//{
//	vector<Item> ItemList = {};
//	vector<Vehicle> VehicleList = {};
//	for (;;)
//	{
//		BYTE LIB_VIS = Utility::ReadMemoryEx<BYTE>(Offset::UE4 + 0x200);
//		if (LIB_VIS == 0x01)
//		{
//			Setting::vislibfound = true;
//		}
//		else
//		{
//			Setting::vislibfound = false;
//		}
//
//		for (int i = 0; i < TempEntityList.size(); i++)
//		{
//			DWORD entityAddv = TempEntityList[i];
//			int entityStruct = Utility::ReadMemoryEx<int>(entityAddv + 0x10);
//			std::string entityType = GetEntityType2(entityStruct);
//			DWORD RootComponent = Utility::ReadMemoryEx<DWORD>(entityAddv + Offset::RootComponent);
//
//			if (Setting::Item)
//			{
//				string ItemType = GetItemType(entityType);
//				if (ItemType.find("Unknown") == std::string::npos)
//				{
//					Item Item;
//					Item.Address = entityAddv;
//					Item.Name = ItemType;
//					Item.Position = Utility::ReadMemoryEx<VECTOR3>(RootComponent + Offset::Position);
//					ItemList.push_back(Item);
//				}
//			}
//
//			if (Setting::Vehicle)
//			{
//				string VehicleType = GetVehicleType(entityType);
//				if (VehicleType.find("Unknown") == std::string::npos)
//				{
//					Vehicle Vehicle;
//					Vehicle.Name = VehicleType;
//					Vehicle.Position = Utility::ReadMemoryEx<VECTOR3>(RootComponent + Offset::Position);
//					DWORD veh = Utility::ReadMemoryEx<DWORD>(entityAddv + Offset::VehicleCommon);
//					float HP = Utility::ReadMemoryEx<float>(veh + Offset::HP);
//					Vehicle.hp = (int)(HP);
//					float Fuel = Utility::ReadMemoryEx<float>(veh + Offset::Fuel);
//					float FuelMax = Utility::ReadMemoryEx<float>(veh + Offset::FuelMax);
//					Vehicle.oil = (float)(Fuel * 100 / FuelMax);
//					float speed = Utility::ReadMemoryEx<float>(entityAddv + Offset::lastForwardSpeed);
//					if (speed > 8 || speed < -8)
//					{
//						Vehicle.driving = true;
//					}
//					else
//					{
//						Vehicle.driving = false;
//					}
//
//					if (Vehicle.hp == 0 || Vehicle.hp < 0 || Vehicle.oil < 0 || Vehicle.oil > 101)
//					{
//						continue;
//					}
//					else
//					{
//						VehicleList.push_back(Vehicle);
//					}
//				}
//			}
//		}
//
//		Data2::ItemList.clear();
//		Data2::VehicleList.clear();
//
//		Data2::ItemList = ItemList;
//		Data2::VehicleList = VehicleList;
//
//		ItemList.clear();
//		VehicleList.clear();
//
//		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//	}
//}


//void Data2::Cache3()
//{
//	vector<Item> ItemList = {};
//	vector<Vehicle> VehicleList = {};
//	while (true)
//	{
//		DWORD UWorld = Utility::ReadMemoryEx<DWORD>(Utility::ReadMemoryEx<DWORD>(Offset::UE4 + 0x82467D4) + 0x3C);
//		DWORD PersistentLevel = Utility::ReadMemoryEx<DWORD>(UWorld + Offset::PersistentLevel);
//		DWORD EntityList = Utility::ReadMemoryEx<DWORD>(PersistentLevel + Offset::EntityList);
//		INT EntityCount = Utility::ReadMemoryEx<INT>(PersistentLevel + Offset::EntityCount);
//		BYTE LIB_VIS = Utility::ReadMemoryEx<BYTE>(Offset::UE4 + 0x200);
//		if (LIB_VIS == 0x01)
//		{
//			Setting::vislibfound = true;
//		}
//		else
//		{
//			Setting::vislibfound = false;
//		}
//
//		for (int i = 0; i < EntityCount; ++i)
//		{
//			EntityAddress = Utility::ReadMemoryEx<DWORD>(EntityList + i * 4);
//			DWORD EntityStruct = Utility::ReadMemoryEx<DWORD>(EntityAddress + 0x10);
//			string EntityType = GetEntityType2(EntityStruct);
//			DWORD RootComponent = Utility::ReadMemoryEx<DWORD>(EntityAddress + Offset::RootComponent);
//
//			if (Setting::Item)
//			{
//				string ItemType = GetItemType(EntityType);
//				if (ItemType.find("Unknown") == std::string::npos)
//				{
//					Item Item;
//					Item.Address = EntityAddress;
//					Item.Name = ItemType;
//					Item.Position = Utility::ReadMemoryEx<VECTOR3>(RootComponent + Offset::Position);
//					ItemList.push_back(Item);
//				}
//			}
//
//			//if (Setting::deadbox)
//			//{
//			//	if (IsBox(EntityType))
//			//	{
//			//		g_pEsp->lootbox.push_back(g_pEsp->lbox);
//			//		DWORD count = Utility::ReadMemoryEx<DWORD>(EntityAddress + 1672);
//			//		if (count > 30)
//			//			count = 30;
//			//		g_pEsp->lbox.boxitem.clear();
//			//		if (count > 0)
//			//		{
//			//			long itemBase = Utility::ReadMemoryEx<DWORD>(EntityAddress + 1668);
//			//			long itemAddv;
//			//			for (int t = 0; t < count; t++)
//			//			{
//			//				itemAddv = itemBase + t * 48;
//			//				std::string bitm = GetBoxItems(Utility::ReadMemoryEx<DWORD>(itemAddv + 0x4));
//			//				if (bitm.find("tatti") == std::string::npos)
//			//				{
//			//					Itemb temo;
//			//					bitm.append("  x").append(std::to_string(Utility::ReadMemoryEx<DWORD>(itemAddv + 0x18)));
//			//					g_pEsp->lbox.Position = Utility::ReadMemoryEx<VECTOR3>(Utility::ReadMemoryEx<DWORD>(EntityAddress + Offset::RootComponent) + Offset::Position);
//			//					temo.colorcod = Setting::colorcode;
//			//					temo.Displayname = bitm;
//			//					temo.Size = Setting::sizecode;
//			//					g_pEsp->lbox.boxitem.push_back(temo);
//			//				}
//			//			}
//			//		}
//			//	}
//			//}
//
//			if (Setting::Vehicle)
//			{
//				string VehicleType = GetVehicleType(EntityType);
//				if (VehicleType.find("Unknown") == std::string::npos)
//				{
//					Vehicle Vehicle;
//					Vehicle.Name = VehicleType;
//					Vehicle.Position = Utility::ReadMemoryEx<VECTOR3>(RootComponent + Offset::Position);
//					DWORD veh = Utility::ReadMemoryEx<DWORD>(EntityAddress + Offset::VehicleCommon);
//					float HP = Utility::ReadMemoryEx<float>(veh + Offset::HP);
//					Vehicle.hp = (int)(HP);
//					float Fuel = Utility::ReadMemoryEx<float>(veh + Offset::Fuel);
//					float FuelMax = Utility::ReadMemoryEx<float>(veh + Offset::FuelMax);
//					Vehicle.oil = (float)(Fuel * 100 / FuelMax);
//					float speed = Utility::ReadMemoryEx<float>(EntityAddress + Offset::lastForwardSpeed);
//					if (speed > 8 || speed < -8)
//					{
//						Vehicle.driving = true;
//					}
//					else
//					{
//						Vehicle.driving = false;
//					}
//
//					if (Vehicle.hp == 0 || Vehicle.hp < 0 || Vehicle.oil < 0 || Vehicle.oil > 101)
//					{
//						continue;
//					}
//					else
//					{
//						VehicleList.push_back(Vehicle);
//					}
//				}
//			}
//
//		}
//
//		Data2::ItemList.clear();
//		Data2::VehicleList.clear();
//
//		Data2::ItemList = ItemList;
//		Data2::VehicleList = VehicleList;
//
//		ItemList.clear();
//		VehicleList.clear();
//
//		std::this_thread::sleep_for(std::chrono::milliseconds(300));
//	}
//}


//void Data2::QueryThread()
//{
//	if (Offset::UE4 == NULL)
//		Offset::UE4 = GETUE4BASE();//Utility::ReadMemoryEx<int>(0xE0C3868);
//	if (Offset::UE4 != 0) {
//	}
//
//	INT MyTeamId = 9999;
//	vector<AActor> AActorList = {};
//	vector<Item> ItemList = {};
//	vector<Vehicle> VehicleList = {};
//	while (true)
//	{
//		Sleep(30);
//		DWORD UWorld = Utility::ReadMemoryEx<DWORD>(Utility::ReadMemoryEx<DWORD>(Offset::UE4 + 0x82467D4) + 0x3C);
//		DWORD PersistentLevel = Utility::ReadMemoryEx<DWORD>(UWorld + Offset::PersistentLevel);
//		NetDriver = Utility::ReadMemoryEx<DWORD>(UWorld + Offset::NetDriver);
//		DWORD ServerConnection = Utility::ReadMemoryEx<DWORD>(NetDriver + Offset::ServerConnection);
//		PlayerController = Utility::ReadMemoryEx<DWORD>(ServerConnection + Offset::PlayerController);
//		LocalPlayer = Utility::ReadMemoryEx<DWORD>(PlayerController + Offset::AcknowledgedPawn);
//		LocalController = Utility::ReadMemoryEx<DWORD>(LocalPlayer + Offset::STExtraPlayerCharacter::STPlayerController);
//		//CharacterMovementComponent = Utility::ReadMemoryEx<DWORD>(LocalPlayer + Offset::CharacterMovement);
//		BYTE LIB_VIS = Utility::ReadMemoryEx<BYTE>(Offset::UE4 + 0x200);
//		if (LIB_VIS == 0x01)
//		{
//			Setting::vislibfound = true;
//		}
//		else
//		{
//			Setting::vislibfound = false;
//		}
//
//
//		unsigned int mapUiMarkManager = Utility::ReadMemoryEx<UINT>(PlayerController + Offset::BP_MapUIMarkManager);
//		unsigned int pExtraGameState = Utility::ReadMemoryEx<UINT>(mapUiMarkManager + Offset::pExtraGameState);
//		Data2::alivePlayerNum = Utility::ReadMemoryEx<UINT>(pExtraGameState + Offset::AlivePlayerNum);
//		Data2::aliveTeamNum = Utility::ReadMemoryEx<UINT>(pExtraGameState + Offset::AliveTeamNum);
//		Data2::OnlinePlayer = Utility::ReadMemoryEx<UINT>(pExtraGameState + Offset::OnlinePlayer);
//		Data2::GameIDD = Utility::ReadMemoryEx<UINT>(pExtraGameState + 0xb40);//uint64 GameID;//[Offset:
//		Data2::MyPlayerWorld = Utility::ReadMemoryEx<DWORD>(Data2::LocalPlayer + Offset::RootComponent);
//		Data2::bIsWeaponFiring = Utility::ReadMemoryEx<bool>(Data2::LocalPlayer + Offset::bIsWeaponFiring);
//
//		DWORD EntityList = Utility::ReadMemoryEx<DWORD>(PersistentLevel + Offset::EntityList);
//		INT EntityCount = Utility::ReadMemoryEx<INT>(PersistentLevel + Offset::EntityCount);
//		//if (EntityCount > 1028)
//		//	EntityCount = 1028;
//
//		for (int i = 0; i < EntityCount; ++i)
//		{
//			//std::this_thread::sleep_for(std::chrono::milliseconds(1));
//			EntityAddress = Utility::ReadMemoryEx<DWORD>(EntityList + i * 4);
//			//if (!EntityAddress)
//			//	continue;
//			//if (EntityAddress == (DWORD)nullptr || EntityAddress == -1 || EntityAddress == NULL)
//			//	continue;
//			DWORD EntityStruct = Utility::ReadMemoryEx<DWORD>(EntityAddress + 0x10);
//			string EntityType = GetEntityType2(EntityStruct);
//			//if (EntityType == "")
//			//	continue;
//			//if (EntityType == "None")
//			//	continue;
//			DWORD RootComponent = Utility::ReadMemoryEx<DWORD>(EntityAddress + Offset::RootComponent);
//
//
//			/*cout << EntityType; "";
//			cout << "\n";*/
//
//
//			if (Setting::Item)
//			{
//				string ItemType = GetItemType(EntityType);
//				if (ItemType.find("Unknown") == std::string::npos)
//				{
//					Item Item;
//					Item.Address = EntityAddress;
//					Item.Name = ItemType;
//					Item.Position = Utility::ReadMemoryEx<VECTOR3>(RootComponent + Offset::Position);
//					ItemList.push_back(Item);
//				}
//			}
//
//			//if (Setting::deadbox)
//			//{
//			//	if (IsBox(EntityType))
//			//	{
//			//		g_pEsp->lootbox.push_back(g_pEsp->lbox);
//			//		DWORD count = Utility::ReadMemoryEx<DWORD>(EntityAddress + 1672);
//			//		if (count > 30)
//			//			count = 30;
//			//		g_pEsp->lbox.boxitem.clear();
//			//		if (count > 0)
//			//		{
//			//			long itemBase = Utility::ReadMemoryEx<DWORD>(EntityAddress + 1668);
//			//			long itemAddv;
//			//			for (int t = 0; t < count; t++)
//			//			{
//			//				itemAddv = itemBase + t * 48;
//			//				std::string bitm = GetBoxItems(Utility::ReadMemoryEx<DWORD>(itemAddv + 0x4));
//			//				if (bitm.find("tatti") == std::string::npos)
//			//				{
//			//					Itemb temo;
//			//					bitm.append("  x").append(std::to_string(Utility::ReadMemoryEx<DWORD>(itemAddv + 0x18)));
//			//					g_pEsp->lbox.Position = Utility::ReadMemoryEx<VECTOR3>(Utility::ReadMemoryEx<DWORD>(EntityAddress + Offset::RootComponent) + Offset::Position);
//			//					temo.colorcod = Setting::colorcode;
//			//					temo.Displayname = bitm;
//			//					temo.Size = Setting::sizecode;
//			//					g_pEsp->lbox.boxitem.push_back(temo);
//			//				}
//			//			}
//			//		}
//			//	}
//			//}
//
//			if (Setting::Vehicle)
//			{
//				string VehicleType = GetVehicleType(EntityType);
//				if (VehicleType.find("Unknown") == std::string::npos)
//				{
//					Vehicle Vehicle;
//					Vehicle.Name = VehicleType;
//					Vehicle.Position = Utility::ReadMemoryEx<VECTOR3>(RootComponent + Offset::Position);
//					DWORD veh = Utility::ReadMemoryEx<DWORD>(EntityAddress + Offset::VehicleCommon);
//					float HP = Utility::ReadMemoryEx<float>(veh + Offset::HP);
//					Vehicle.hp = (int)(HP);
//					float Fuel = Utility::ReadMemoryEx<float>(veh + Offset::Fuel);
//					float FuelMax = Utility::ReadMemoryEx<float>(veh + Offset::FuelMax);
//					Vehicle.oil = (float)(Fuel * 100 / FuelMax);
//					float speed = Utility::ReadMemoryEx<float>(EntityAddress + Offset::lastForwardSpeed);
//					if (speed > 8 || speed < -8)
//					{
//						Vehicle.driving = true;
//					}
//					else
//					{
//						Vehicle.driving = false;
//					}
//
//					if (Vehicle.hp == 0 || Vehicle.hp < 0 || Vehicle.oil < 0 || Vehicle.oil > 101)
//					{
//						continue;
//					}
//					else
//					{
//						VehicleList.push_back(Vehicle);
//					}
//				}
//			}
//
//			if (IsPlayer(EntityType))
//			{
//				AActor AActor;
//				AActor.PlayerDead = Utility::ReadMemoryEx<bool>(EntityAddress + Offset::bDead);
//				if (!AActor.PlayerDead == true)
//				{
//					AActor.Address = EntityAddress;
//					AActor.Velocity = Utility::ReadMemoryEx<VECTOR3>(Utility::ReadMemoryEx<DWORD>(AActor.Address + Offset::CharacterMovement) + Offset::LastUpdateVelocity);
//					AActor.TeamId = Utility::ReadMemoryEx<INT>(EntityAddress + Offset::TeamId);
//					AActor.Position = Utility::ReadMemoryEx<VECTOR3>(RootComponent + Offset::Position);
//
//					AActor.VISIBLE = Utility::ReadMemoryEx<BOOL>(EntityAddress + 0x1D2);
//					if (AActor.VISIBLE == 1)
//					{
//						AActor.VISIBLE2 = 1;
//					}
//					else
//					{
//						AActor.VISIBLE2 = 0;
//					}
//
//					if (AActor.Address == LocalPlayer)
//					{
//						MyTeamId = AActor.TeamId;
//						continue;
//					}
//					if (AActor.TeamId == MyTeamId) continue;
//
//					AActor.Status = Utility::ReadMemoryEx<INT>(EntityAddress + Offset::CurrentStates);
//					//CurrentReloadWeapon = Utility::ReadMemoryEx<DWORD>(EntityAddress + Offset::CurrentReloadWeapon);
//					//ShootWeaponEntity = Utility::ReadMemoryEx<DWORD>(CurrentReloadWeapon + Offset::ShootWeaponEntity);
//					//STExtraShootWeaponComponent = Utility::ReadMemoryEx<DWORD>(CurrentReloadWeapon + Offset::ShootWeaponComponent);
//					//STExtraShootWeapon = Utility::ReadMemoryEx<DWORD>(STExtraShootWeaponComponent + Offset::OwnerShootWeapon);
//					//AActor.RelativeRotation = Utility::ReadMemoryEx<FRotator>(EntityAddress + Offset::RelativeRotation);
//					//AActor.Location = Utility::ReadMemoryEx<VECTOR3>(EntityAddress + Offset::RelativeLocation);
//					AActor.Nation = GetPlayerFlag(Utility::ReadMemoryEx<DWORD>(EntityAddress + Offset::Nation));
//					AActor.ScopeFov = Utility::ReadMemoryEx<INT>(EntityAddress + Offset::ScopeFov);
//					AActor.Name = GetPlayerName1(Utility::ReadMemoryEx<DWORD>(EntityAddress + Offset::Name));
//					AActor.IsBot = Utility::ReadMemoryEx<BOOL>(EntityAddress + Offset::IsBot);
//					AActor.Health = Utility::ReadMemoryEx<FLOAT>(EntityAddress + Offset::Health);
//					AActor.HealthMax = Utility::ReadMemoryEx<FLOAT>(EntityAddress + Offset::HealthMax);
//					AActor.KnockedHealth = Utility::ReadMemoryEx<FLOAT>(EntityAddress + Offset::KnockHealth);
//
//					if (Setting::Enenmyaim)
//					{
//						VECTOR2 RelativeRotation = Utility::ReadMemoryEx<VECTOR2>(RootComponent + Offset::RelativeRotation);
//						AActor.RelativeRotationnnn = ClampAngle(RelativeRotation);
//						DWORD LocalRoot = Utility::ReadMemoryEx<DWORD>(LocalPlayer + Offset::RootComponent);
//						VECTOR3 MyPositionnnn = Utility::ReadMemoryEx<VECTOR3>(LocalRoot + Offset::Position);
//						VECTOR3 MyHEad = MyPositionnnn;
//						VECTOR3 EnemyHed = AActor.Position;
//						auto Angle = ToRotatorrrrr(EnemyHed, MyHEad);
//						AActor.ToRotatorToMe = ClampAngle(Angle);
//						float num = AActor.ToRotatorToMe.Y - AActor.RelativeRotationnnn.Y;
//						AActor.isAimingAtMe = false;
//						if (num <= 6.0f)
//						{
//							if (num >= -6.0f)
//							{
//								AActor.isAimingAtMe = true;
//							}
//						}
//					}
//
//
//					//string Player_Scope = scopes((int)AActor.ScopeFov);
//					//int testfuckenscope = Utility::ReadMemoryEx<FLOAT>(Data2::LocalPlayer + Offset::ScopeFov);
//					//cout << testfuckenscope; "";
//					//cout << "\n";
//					if (Setting::plr_wpn)
//					{
//						std::string wep = PlayerWeapon(GetEntityType2(Utility::ReadMemoryEx<DWORD>(Utility::ReadMemoryEx<DWORD>(EntityAddress + Offset::CurrentReloadWeapon2) + 16)));
//						if (wep == "")
//						{
//							wep = "Fist";
//						}
//						AActor.weapon = wep;
//					}
//
//					DWORD MeshAddv = Utility::ReadMemoryEx<DWORD>(AActor.Address + Offset::Mesh);
//					if (!MeshAddv)continue;
//					DWORD BodyAddv = MeshAddv + Offset::BodyAddv;
//					if (!BodyAddv)continue;
//					DWORD BoneAddv = Utility::ReadMemoryEx<DWORD>(MeshAddv + Offset::MinLOD) + 48;
//					if (!BoneAddv)continue;
//					if (BoneAddv)
//					{
//						AActor.HeadPos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 5 * 48);
//						if (!AActor.HeadPos.X)continue;
//						AActor.ChestPos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 4 * 48);
//						if (!AActor.ChestPos.X)continue;
//						AActor.PelvisPos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 1 * 48);
//						AActor.lSholderPos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 11 * 48);
//						AActor.rSholderPos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 32 * 48);
//						AActor.lElbowPos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 12 * 48);
//						AActor.rElbowPos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 33 * 48);
//						AActor.lWristPos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 63 * 48);
//						AActor.rWristPos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 62 * 48);
//						AActor.lThighPos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 52 * 48);
//						AActor.rThighPos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 56 * 48);
//						AActor.lKneePos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 53 * 48);
//						AActor.rKneePos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 57 * 48);
//						AActor.lAnklePos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 54 * 48);
//						AActor.rAnklePos = Algorithm::GetBoneWorldPosition(BodyAddv, BoneAddv + 58 * 48);
//						AActorList.push_back(AActor);
//						continue;
//					}
//					else { continue; }
//				}
//				else { continue; }
//			}
//		}
//
//		Data2::AActorList.clear();
//		Data2::ItemList.clear();
//		Data2::VehicleList.clear();
//
//		Data2::AActorList = AActorList;
//		Data2::ItemList = ItemList;
//		Data2::VehicleList = VehicleList;
//
//		AActorList.clear();
//		ItemList.clear();
//		VehicleList.clear();
//	}
//}


void Data2::TotalSpeed()
{
	while (true)
	{
		Sleep(50);
		if (Data2::NetDriver > 0 && Data2::LocalPlayer > 0)
		{
			if (Data2::MyPlayerWorld > 0)
			{
				VECTOR3 previousPositionn = Utility::ReadMemoryEx<VECTOR3>(Data2::MyPlayerWorld + Offset::RelativeLocation);
				previousPositionn.Z = 0;

				Sleep(100);

				VECTOR3 currentPositionn = Utility::ReadMemoryEx<VECTOR3>(Data2::MyPlayerWorld + Offset::RelativeLocation);
				currentPositionn.Z = 0;

				double deltaDd = GetDistVec(currentPositionn, previousPositionn);

				double playerSpeeed;

				playerSpeeed = deltaDd;

				double PlayeeerSpeedinM = playerSpeeed / 10;

				Setting::MoveSpeed = PlayeeerSpeedinM;
			}
		}
	}
}

void Data2::TotalDistance()
{
	while (true)
	{
		if (Data2::NetDriver > 0 && Data2::LocalPlayer > 0)
		{
			if (Data2::MyPlayerWorld > 0)
			{
				VECTOR3 Position = Utility::ReadMemoryEx<VECTOR3>(Data2::MyPlayerWorld + Offset::RelativeLocation);
				Position.Z = 0;

				Sleep(100);
				VECTOR3 Position2 = Utility::ReadMemoryEx<VECTOR3>(Data2::MyPlayerWorld + Offset::RelativeLocation);
				Position2.Z = 0;

				double deltaa = GetDistVec(Position2, Position);

				double Distance;

				Distance += deltaa;

				double PlayerDis = Distance / 100;

				Setting::totaldistance = PlayerDis;
			}
		}
		else
		{
			Setting::totaldistance = 0;
		}
	}
}

float Data2::DistanceTravelled = 0;
float Data2::GameInfovelocity = 0;

void Data2::GameInformation()
{
	int time = 100;
	vector<float> avgVelo;
	float v1 = 0;
	float v2 = 0;
	while (true)
	{
		if (Data2::NetDriver > 0 && Data2::LocalPlayer > 0)
		{
			if (Data2::MyPlayerWorld > 0)
			{
				VECTOR3 Pi = Utility::ReadMemoryEx<VECTOR3>(Data2::MyPlayerWorld + Offset::RelativeLocation);
				Pi.Z = 0;
				Sleep(time);
				VECTOR3 Pf = Utility::ReadMemoryEx<VECTOR3>(Data2::MyPlayerWorld + Offset::RelativeLocation);
				Pf.Z = 0;

				float displacement = ((GetDistVec(Pf, Pi)) / 100);//metre
				float velocity = ((displacement * 1000) / time);//m/s        
				Data2::DistanceTravelled += displacement;
				if (avgVelo.size() < 5)
				{
					avgVelo.push_back(velocity);
				}
				else
				{
					double maxii = *max_element(avgVelo.begin(), avgVelo.end());
					Data2::GameInfovelocity = maxii;
					avgVelo.clear();
				}
				Data2::GameInfovelocity = velocity;
			}
		}
		else
		{
			Sleep(200);
		}
	}
}
void Data2::ColorTime()
{
	for (;;)
	{
		if (Data2::NetDriver > 0)
		{
			int g2temp = Setting::colortime;
			Setting::colortime = g2temp + 1;
			if (Setting::colortime == 3)
			{
				Setting::colortime = 0;
			}
		}
		else
		{
			Setting::colortime = 0;
		}
		Sleep(200);
	}
}
#pragma once

#include "AK/SoundEngine/Common/AkSoundEngine.h"
#include <thread>
#include <chrono>
#include <AK/SoundEngine/Common/IAkStreamMgr.h>
#include <AK/Tools/Common/AkPlatformFuncs.h>
#include "AK/SoundEngine/Common/AkStreamMgrModule.h"
#include <AK/SoundEngine/Common/AkModule.h>
#include "AkFilePackageLowLevelIOBlocking.h"
#include "AK/Plugin/AkVorbisDecoderFactory.h"
#include <random>
#include <fstream>

#define DllExport extern "C" __declspec(dllexport)

namespace Wwise_Player
{
	struct Name_Playing_ID
	{
		std::string Event_Name = "";
		unsigned int Event_ID = 0;
		int Container_ID = 0;
		int Play_ID = 0;
		float Volume = 1.0f;
		AkGameObjectID Object_ID = 0;
	};
	DllExport int* __stdcall Wwise_Get_End_Event_Container_ID();
	DllExport int __stdcall Wwise_Get_End_Event_Count();
	DllExport int __stdcall Wwise_Get_Playing_Event_Count();
	DllExport bool __stdcall Wwise_Init(const char* Init_BNK, int Listener_Index, double Init_Volume = 1.0);
	DllExport bool __stdcall Wwise_Load_Bank(const char* Stream_BNK);
	DllExport bool __stdcall Wwise_Play_Name(const char* Name, int Container_ID, double Volume);
	DllExport unsigned long __stdcall Wwise_Play_ObjectID(const char* Name, unsigned int ObjectID);
	DllExport bool __stdcall Wwise_Play_ID(unsigned int Event_ID, int Container_ID, double Volume = -1);
	DllExport bool __stdcall Wwise_Stop_Name(const char* Name);
	DllExport bool __stdcall Wwise_Stop_ID(unsigned int Name);
	DllExport bool __stdcall Wwise_Stop_Container_ID(int Container_ID);
	DllExport void __stdcall Wwise_Stop_Object_ID(unsigned int GameObject_ID, unsigned long Playing_ID);
	DllExport void __stdcall Wwise_Stop_All();
	DllExport bool __stdcall Wwise_Pause_All(bool IsRenderAudio);
	DllExport bool __stdcall Wwise_Play_All();
	DllExport int __stdcall Wwise_Get_Position_Name(const char* Name);
	DllExport int __stdcall Wwise_Get_Position_ID(unsigned int Event_ID);
	DllExport int __stdcall Wwise_Get_Position_Container_ID(int Container_ID);
	DllExport bool __stdcall Wwise_Set_Position_Percent_Name(const char* Name, float Percent);
	DllExport bool __stdcall Wwise_Set_Position_Percent_ID(unsigned int Event_ID, float Percent);
	DllExport bool __stdcall Wwise_Set_Position_Percent_Container_ID(int Container_ID, float Percent);
	DllExport bool __stdcall Wwise_Set_Position_Second_Name(const char* Name, int Position);
	DllExport bool __stdcall Wwise_Set_Position_Second_ID(unsigned int Event_ID, int Position);
	DllExport bool __stdcall Wwise_Set_Position_Second_Container_ID(int Container_ID, int Position);
	DllExport bool __stdcall Wwise_Set_Volume_Name(const char* Name, double Volume);
	DllExport bool __stdcall Wwise_Set_Volume_ID(unsigned int EventID, double Volume);
	DllExport bool __stdcall Wwise_Set_Volume_Container_ID(int Container_ID, double Volume);
	DllExport bool __stdcall Wwise_Set_Volume_All(double Volume);
	DllExport double __stdcall Wwise_Get_Volume_Name(const char* Name);
	DllExport double __stdcall Wwise_Get_Volume_ID(unsigned int Event_ID);
	DllExport double __stdcall Wwise_Get_Volume_Container_ID(int Container_ID);
	DllExport bool __stdcall Wwise_Set_State(const char* State_Parent_Name, const char* State_Child_Name);
	DllExport bool __stdcall Wwise_Set_RTPC(const char* RTPC_Name, float Value);
	DllExport double* __stdcall Wwise_Get_Volume_All();
	DllExport int* __stdcall Wwise_Get_All_Container_ID();
	DllExport int __stdcall Wwise_Get_Max_Length_Name(const char* Name);
	DllExport int __stdcall Wwise_Get_Max_Length_ID(unsigned int Event_ID);
	DllExport int __stdcall Wwise_Get_Max_Length_Container_ID(int Container_ID);
	DllExport void _stdcall Wwise_Set_Listener_Position(float X, float Y, float Z, float OriFrontX, float OriFrontY, float OriFrontZ, float OriTopX, float OriTopY, float OriTopZ);
	DllExport void _stdcall Wwise_Set_Object_Position(unsigned int Object_ID, float X, float Y, float Z, float OriFrontX, float OriFrontY, float OriFrontZ, float OriTopX, float OriTopY, float OriTopZ);
	DllExport void _stdcall Wwise_Set_Object_Positions(unsigned int Object_ID, int Count, float* X, float* Y, float* Z, float* OriFrontX, float* OriFrontY, float* OriFrontZ,
		float* OriTopX, float* OriTopY, float* OriTopZ);
	DllExport void __stdcall Wwise_Dispose();
	DllExport bool __stdcall Wwise_IsInited();
	DllExport unsigned int __stdcall Wwise_Get_Result_Index();

	DllExport char* HashToChar(int length, unsigned int shortID);
	DllExport int GetHashLength();

	class Utilities final
	{
	public:
		static bool Increment(std::vector<unsigned char>& array, int i);
		static bool Increment(unsigned char from, unsigned char& result);
		static void ZeroFrom(std::vector<unsigned int>& array, int i);
	};
	class FNV_Hash_Class
	{
	private:
		static constexpr unsigned int OffsetBasis = 2166136261;
		static constexpr unsigned int Prime = 16777619;
		std::vector<unsigned int> _hashes;
		void Initialize(int length);
	public:
		void Bruteforce(int Length, unsigned int match);
		std::vector<unsigned char> _bytes;
		unsigned char returnLastByte = 0x2f;
	private:
		static unsigned int Hash(std::vector<unsigned char>& array, std::vector<unsigned int>& hashes, int length);
	};

}
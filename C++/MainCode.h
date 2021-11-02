#pragma once

#include "AK/SoundEngine/Common/AkSoundEngine.h"
#include <thread>
#include <chrono>
#include <AK/SoundEngine/Common/AkMemoryMgr.h>
#include <AK/SoundEngine/Common/IAkStreamMgr.h>
#include <AK/Tools/Common/AkPlatformFuncs.h>
#include "AK/SoundEngine/Common/AkStreamMgrModule.h"
#include <AK/SoundEngine/Common/AkModule.h>
#include "AkFilePackageLowLevelIOBlocking.h"
#include "AK/Plugin/AkVorbisDecoderFactory.h"
#include <random>
#include "string"

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
	DllExport void __stdcall Wwise_Set_Path(const char* Base_Dir_Path);
	DllExport bool __stdcall Wwise_Play_Name(const char* Name, int Container_ID, double Volume = -1);
	DllExport bool __stdcall Wwise_Play_ID(unsigned int Event_ID, int Container_ID, double Volume = -1);
	DllExport bool __stdcall Wwise_Stop_Name(const char* Name);
	DllExport bool __stdcall Wwise_Stop_ID(unsigned int Name);
	DllExport bool __stdcall Wwise_Stop_Container_ID(int Container_ID);
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
	DllExport double* __stdcall Wwise_Get_Volume_All();
	DllExport int* __stdcall Wwise_Get_All_Container_ID();
	DllExport int __stdcall Wwise_Get_Max_Length_Name(const char* Name);
	DllExport int __stdcall Wwise_Get_Max_Length_ID(unsigned int Event_ID);
	DllExport int __stdcall Wwise_Get_Max_Length_Container_ID(int Container_ID);
	DllExport void __stdcall Wwise_Dispose();
	DllExport bool __stdcall Wwise_IsInited();
	DllExport unsigned int __stdcall Wwise_Get_Result_Index();
}
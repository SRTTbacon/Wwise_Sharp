#include "MainCode.h"

namespace Wwise_Player
{
    CAkFilePackageLowLevelIOBlocking g_lowLevelIO;
    AkBankID BNK_ID = 0;
    std::vector<Name_Playing_ID> Playing_List;
    std::vector<int> End_Event_ID;
    int Listener_Index = 1;
    float Set_Volume = 1.0f;
    AKRESULT Last_Result = AKRESULT::AK_Success;
    bool __stdcall Wwise_Init(const char* Init_BNK, int Listener_Index, double Init_Volume)
    {
        //メモリマネージャーの初期化
        AkMemSettings memSettings;
        AK::MemoryMgr::GetDefaultSettings(memSettings);
        Last_Result = AK::MemoryMgr::Init(&memSettings);
        if (Last_Result != AK_Success)
            return false;
        //ストリームマネージャーの初期化
        AkStreamMgrSettings stmSettings;
        AK::StreamMgr::GetDefaultSettings(stmSettings);
        if (!AK::StreamMgr::Create(stmSettings))
            return false;
        //IOBlockingの初期化
        AkDeviceSettings deviceSettings;
        AK::StreamMgr::GetDefaultDeviceSettings(deviceSettings);
        Last_Result = g_lowLevelIO.Init(deviceSettings);
        if (Last_Result != AK_Success)
            return false;
        //サウンドエンジンの初期化
        AkInitSettings initSettings;
        AkPlatformInitSettings platformInitSettings;
        AK::SoundEngine::GetDefaultInitSettings(initSettings);
        AK::SoundEngine::GetDefaultPlatformInitSettings(platformInitSettings);
        Last_Result = AK::SoundEngine::Init(&initSettings, &platformInitSettings);
        if (Last_Result != AK_Success)
            return false;
        AK::StreamMgr::SetCurrentLanguage(AKTEXT("English(US)"));
        //Init.bnkを読み込む
        AK::SoundEngine::LoadBank(Init_BNK, BNK_ID);
        AK::SoundEngine::RenderAudio();
        Wwise_Player::Listener_Index = Listener_Index;
        Set_Volume = (float)Init_Volume;
        return true;
    }
    //.bnkファイルをロード(親であるInit.bnkが初期化時と異なる場合はロードできません多分)
    bool __stdcall Wwise_Load_Bank(const char* Stream_BNK)
    {
        Last_Result = AK::SoundEngine::LoadBank(Stream_BNK, BNK_ID);
        if (Last_Result == AK_Success)
        {
            AK::SoundEngine::RenderAudio();
            return true;
        }
        return false;
    }
    void End_Event(AkCallbackType type, AkCallbackInfo* info)
    {
        int Index = -1;
        for (int Number = 0; Number < (int)Playing_List.size(); Number++)
            if (Playing_List[Number].Object_ID == info[0].gameObjID)
                Index = Number;
        if (Index != -1)
        {
            End_Event_ID.push_back(Playing_List[Index].Container_ID);
            Playing_List.erase(Playing_List.begin() + Index);
        }
    }
    int* _stdcall Wwise_Get_End_Event_Container_ID()
    {
        if (End_Event_ID.size() == 0)
            return new int[0];
        int* Temp = new int[End_Event_ID.size()];
        for (unsigned int i = 0; i < End_Event_ID.size(); i++)
            Temp[i] = End_Event_ID[i];
        End_Event_ID.clear();
        return Temp;
    }
    int _stdcall Wwise_Get_End_Event_Count()
    {
        return (int)End_Event_ID.size();
    }
    int _stdcall Wwise_Get_Playing_Event_Count()
    {
        return (int)Playing_List.size();
    }
    //イベント名で再生
    bool __stdcall Wwise_Play_Name(const char* Name, int Container_ID, double Volume)
    {
        //IDを指定(ランダム)
        if (Volume != -1 && Volume < 0)
            Volume = 0;
        else if (Volume > 1)
            Volume = 1;
        AkGameObjectID Object_ID = 0;
        std::random_device rnd;
        std::mt19937 mt(rnd());
        std::uniform_int_distribution<> rand100(10, 1000);
        Object_ID = rand100(mt);
        AK::SoundEngine::RegisterGameObj(Object_ID);
        AK::SoundEngine::SetDefaultListeners(&Object_ID, Listener_Index);
        if (Volume == -1)
            AK::SoundEngine::SetGameObjectOutputBusVolume(Object_ID, Object_ID, Set_Volume);
        else
            AK::SoundEngine::SetGameObjectOutputBusVolume(Object_ID, Object_ID, (float)Volume);
        int Play_ID = AK::SoundEngine::PostEvent(Name, Object_ID, AK_EnableGetSourcePlayPosition | AK_EndOfEvent, End_Event, NULL, NULL, NULL, Container_ID + 1);
        if (Play_ID != 0)
        {
            Name_Playing_ID Temp;
            Temp.Event_Name = Name;
            Temp.Event_ID = 0;
            Temp.Play_ID = Play_ID;
            Temp.Container_ID = Container_ID;
            Temp.Object_ID = Object_ID;
            if (Volume == -1)
                Temp.Volume = Set_Volume;
            else
                Temp.Volume = (float)Volume;
            Playing_List.push_back(Temp);
            AK::SoundEngine::RenderAudio();
            return true;
        }
        return false;
    }
    //イベントIDで再生
    bool __stdcall Wwise_Play_ID(unsigned int Event_ID, int Container_ID, double Volume)
    {
        //IDを指定(ランダム)
        if (Volume != -1 && Volume < 0)
            Volume = 0;
        else if (Volume > 1)
            Volume = 1;
        AkGameObjectID Object_ID = 0;
        std::random_device rnd;
        std::mt19937 mt(rnd());
        std::uniform_int_distribution<> rand100(10, 1000);
        Object_ID = rand100(mt);
        AK::SoundEngine::RegisterGameObj(Object_ID);
        AK::SoundEngine::SetDefaultListeners(&Object_ID, Listener_Index);
        if (Volume == -1)
            AK::SoundEngine::SetGameObjectOutputBusVolume(Object_ID, Object_ID, Set_Volume);
        else
            AK::SoundEngine::SetGameObjectOutputBusVolume(Object_ID, Object_ID, (float)Volume);
        int Play_ID = AK::SoundEngine::PostEvent(Event_ID, Object_ID, AK_EnableGetSourcePlayPosition | AK_EndOfEvent, End_Event, NULL, NULL, NULL, Container_ID + 1);
        if (Play_ID != 0)
        {
            Name_Playing_ID Temp;
            Temp.Event_Name = "";
            Temp.Event_ID = Event_ID;
            Temp.Play_ID = Play_ID;
            Temp.Container_ID = Container_ID;
            Temp.Object_ID = Object_ID;
            if (Volume == -1)
                Temp.Volume = Set_Volume;
            else
                Temp.Volume = (float)Volume;
            Playing_List.push_back(Temp);
            AK::SoundEngine::RenderAudio();
            return true;
        }
        return false;
    }
    //イベント名で停止
    bool __stdcall Wwise_Stop_Name(const char* Name)
    {
        bool IsStopped = false;
        for (int Number = 0; Number < (int)Playing_List.size(); Number++)
        {
            if ((int)Playing_List.size() <= Number)
                break;
            if (Playing_List[Number].Event_Name == Name)
            {
                AK::SoundEngine::StopPlayingID(Playing_List[Number].Play_ID);
                AK::SoundEngine::UnregisterGameObj(Playing_List[Number].Object_ID);
                Playing_List.erase(Playing_List.begin() + Number);
                Number--;
                IsStopped = true;
            }
        }
        AK::SoundEngine::RenderAudio();
        return IsStopped;
    }
    //イベントIDで停止
    bool __stdcall Wwise_Stop_ID(unsigned int Event_ID)
    {
        bool IsStopped = false;
        for (int Number = 0; Number < (int)Playing_List.size(); Number++)
        {
            if ((int)Playing_List.size() <= Number)
                break;
            if (Playing_List[Number].Event_ID == Event_ID)
            {
                AK::SoundEngine::StopPlayingID(Playing_List[Number].Play_ID);
                AK::SoundEngine::UnregisterGameObj(Playing_List[Number].Object_ID);
                Playing_List.erase(Playing_List.begin() + Number);
                Number--;
                IsStopped = true;
            }
        }
        AK::SoundEngine::RenderAudio();
        return IsStopped;
    }
    //コンテナIDで停止
    bool __stdcall Wwise_Stop_Container_ID(int Container_ID)
    {
        bool IsStopped = false;
        for (int Number = 0; Number < (int)Playing_List.size(); Number++)
        {
            if ((int)Playing_List.size() <= Number)
                break;
            if (Playing_List[Number].Container_ID == Container_ID)
            {
                AK::SoundEngine::StopPlayingID(Playing_List[Number].Play_ID);
                AK::SoundEngine::UnregisterGameObj(Playing_List[Number].Object_ID);
                Playing_List.erase(Playing_List.begin() + Number);
                Number--;
                IsStopped = true;
            }
        }
        AK::SoundEngine::RenderAudio();
        return IsStopped;
    }
    void _stdcall Wwise_Stop_All()
    {
        Playing_List.clear();
        AK::SoundEngine::StopAll();
        AK::SoundEngine::RenderAudio();
    }
    bool _stdcall Wwise_Pause_All(bool IsRenderAudio)
    {
        Last_Result = AK::SoundEngine::Suspend(IsRenderAudio);
        if (Last_Result == AK_Success)
            return true;
        return false;
    }
    bool _stdcall Wwise_Play_All()
    {
        Last_Result = AK::SoundEngine::WakeupFromSuspend();
        if (Last_Result == AK_Success)
            return true;
        return false;
    }
    //イベント名の再生位置を取得
    //複数ある場合は最初に一致したサウンドの再生位置が取得されます
    int __stdcall Wwise_Get_Position_Name(const char* Name)
    {
        for (int Number = 0; Number < (int)Playing_List.size(); Number++)
        {
            if (Playing_List[Number].Event_Name == Name)
            {
                AkTimeMs uPosition = 0;
                Last_Result = AK::SoundEngine::GetSourcePlayPosition(Playing_List[Number].Play_ID, &uPosition);
                if (Last_Result == AK_Success)
                    return uPosition;
                return 0;
            }
        }
        return 0;
    }
    //イベントIDの再生位置を取得
    int  __stdcall Wwise_Get_Position_ID(unsigned int Event_ID)
    {
        for (int Number = 0; Number < (int)Playing_List.size(); Number++)
        {
            if (Playing_List[Number].Event_ID == Event_ID)
            {
                AkTimeMs uPosition = 0;
                Last_Result = AK::SoundEngine::GetSourcePlayPosition(Playing_List[Number].Play_ID, &uPosition);
                if (Last_Result == AK_Success)
                    return uPosition;
                return 0;
            }
        }
        return 0;
    }
    //コンテナIDの再生位置を取得
    int  __stdcall Wwise_Get_Position_Container_ID(int Container_ID)
    {
        for (int Number = 0; Number < (int)Playing_List.size(); Number++)
        {
            if (Playing_List[Number].Container_ID == Container_ID)
            {
                AkTimeMs uPosition = 0;
                Last_Result = AK::SoundEngine::GetSourcePlayPosition(Playing_List[Number].Play_ID, &uPosition);
                if (Last_Result == AK_Success)
                    return uPosition;
                return 0;
            }
        }
        return 0;
    }
    bool _stdcall Wwise_Set_Volume_Name(const char* Name, double Volume)
    {
        if (Volume != -1 && Volume <= 0)
            Volume = 0.001;
        else if (Volume > 1)
            Volume = 1;
        bool IsOK = false;
        for (int Number = 0; Number < (int)Playing_List.size(); Number++)
        {
            if (Playing_List[Number].Event_Name == Name)
            {
                Last_Result = AK::SoundEngine::SetGameObjectOutputBusVolume(Playing_List[Number].Object_ID, Playing_List[Number].Object_ID, (float)Volume);
                if (Last_Result == AK_Success)
                {
                    Playing_List[Number].Volume = (float)Volume;
                    IsOK = true;
                }
            }
        }
        AK::SoundEngine::RenderAudio();
        return IsOK;
    }
    bool _stdcall Wwise_Set_Volume_ID(unsigned int Event_ID, double Volume)
    {
        if (Volume != -1 && Volume <= 0)
            Volume = 0.001;
        else if (Volume > 1)
            Volume = 1;
        bool IsOK = false;
        for (int Number = 0; Number < (int)Playing_List.size(); Number++)
        {
            if (Playing_List[Number].Event_ID == Event_ID)
            {
                Last_Result = AK::SoundEngine::SetGameObjectOutputBusVolume(Playing_List[Number].Object_ID, Playing_List[Number].Object_ID, (float)Volume);
                if (Last_Result == AK_Success)
                {
                    Playing_List[Number].Volume = (float)Volume;
                    IsOK = true;
                }
            }
        }
        AK::SoundEngine::RenderAudio();
        return IsOK;
    }
    bool _stdcall Wwise_Set_Volume_Container_ID(int Container_ID, double Volume)
    {
        if (Volume != -1 && Volume <= 0)
            Volume = 0.001;
        else if (Volume > 1)
            Volume = 1;
        bool IsOK = false;
        for (int Number = 0; Number < (int)Playing_List.size(); Number++)
        {
            if (Playing_List[Number].Container_ID == Container_ID)
            {
                Last_Result = AK::SoundEngine::SetGameObjectOutputBusVolume(Playing_List[Number].Object_ID, Playing_List[Number].Object_ID, (float)Volume);
                if (Last_Result == AK_Success)
                {
                    Playing_List[Number].Volume = (float)Volume;
                    IsOK = true;
                }
            }
        }
        AK::SoundEngine::RenderAudio();
        return IsOK;
    }
    bool __stdcall Wwise_Set_Volume_All(double Volume)
    {
        if (Volume != -1 && Volume <= 0)
            Volume = 0.001;
        else if (Volume > 1)
            Volume = 1;
        bool IsOK = false;
        for (int Number = 0; Number < (int)Playing_List.size(); Number++)
        {
            Last_Result = AK::SoundEngine::SetGameObjectOutputBusVolume(Playing_List[Number].Object_ID, Playing_List[Number].Object_ID, (float)Volume);
            if (Last_Result == AK_Success)
            {
                Playing_List[Number].Volume = (float)Volume;
                IsOK = true;
            }
        }
        AK::SoundEngine::RenderAudio();
        return IsOK;
    }
    double _stdcall Wwise_Get_Volume_Name(const char* Name)
    {
        for (int Number = 0; Number < (int)Playing_List.size(); Number++)
            if (Playing_List[Number].Event_Name == Name)
                return Playing_List[Number].Volume;
        return 1;
    }
    double _stdcall Wwise_Get_Volume_ID(unsigned int Event_ID)
    {
        for (int Number = 0; Number < (int)Playing_List.size(); Number++)
            if (Playing_List[Number].Event_ID == Event_ID)
                return Playing_List[Number].Volume;
        return 1;
    }
    double _stdcall Wwise_Get_Volume_Container_ID(int Container_ID)
    {
        for (int Number = 0; Number < (int)Playing_List.size(); Number++)
            if (Playing_List[Number].Container_ID == Container_ID)
                return Playing_List[Number].Volume;
        return 1;
    }
    double* _stdcall Wwise_Get_Volume_All()
    {
        if (Playing_List.size() == 0)
            return new double[0];
        double* Temp = new double[Wwise_Get_Playing_Event_Count()];
        for (int Number = 0; Number < (int)Playing_List.size(); Number++)
            Temp[Number] = Playing_List[Number].Volume;
        return Temp;
    }
    int* _stdcall Wwise_Get_All_Container_ID()
    {
        if (Playing_List.size() == 0)
            return new int[0];
        int* Temp = new int[Wwise_Get_Playing_Event_Count()];
        for (int Number = 0; Number < (int)Playing_List.size(); Number++)
            Temp[Number] = Playing_List[Number].Container_ID;
        return Temp;
    }
    bool _stdcall Wwise_Set_Position_Percent_Name(const char* Name, float Percent)
    {
        for (int Number = 0; Number < (int)Playing_List.size(); Number++)
        {
            if (Playing_List[Number].Event_Name == Name)
            {
                Last_Result = AK::SoundEngine::SeekOnEvent(Name, Playing_List[Number].Object_ID, Percent);
                if (Last_Result == AK_Success)
                {
                    AK::SoundEngine::RenderAudio();
                    return true;
                }
            }
        }
        return false;
    }
    bool _stdcall Wwise_Set_Position_Percent_ID(unsigned int Event_ID, float Percent)
    {
        for (int Number = 0; Number < (int)Playing_List.size(); Number++)
        {
            if (Playing_List[Number].Event_ID == Event_ID)
            {
                Last_Result = AK::SoundEngine::SeekOnEvent(Event_ID, Playing_List[Number].Object_ID, Percent);
                if (Last_Result == AK_Success)
                {
                    AK::SoundEngine::RenderAudio();
                    return true;
                }
            }
        }
        return false;
    }
    bool _stdcall Wwise_Set_Position_Percent_Container_ID(int Container_ID, float Percent)
    {
        for (int Number = 0; Number < (int)Playing_List.size(); Number++)
        {
            if (Playing_List[Number].Container_ID == Container_ID)
            {
                Last_Result = AK::SoundEngine::SeekOnEvent(Playing_List[Number].Event_ID, Playing_List[Number].Object_ID, Percent);
                if (Last_Result == AK_Success)
                {
                    AK::SoundEngine::RenderAudio();
                    return true;
                }
            }
        }
        return false;
    }
    bool _stdcall Wwise_Set_Position_Second_Name(const char* Name, int Position)
    {
        for (int Number = 0; Number < (int)Playing_List.size(); Number++)
        {
            if (Playing_List[Number].Event_Name == Name)
            {
                Last_Result = AK::SoundEngine::SeekOnEvent(Name, Playing_List[Number].Object_ID, (long)Position);
                if (Last_Result == AK_Success)
                {
                    AK::SoundEngine::RenderAudio();
                    return true;
                }
            }
        }
        return false;
    }
    bool _stdcall Wwise_Set_Position_Second_ID(unsigned int ID, int Position)
    {
        for (int Number = 0; Number < (int)Playing_List.size(); Number++)
        {
            if (Playing_List[Number].Event_ID == ID)
            {
                Last_Result = AK::SoundEngine::SeekOnEvent(ID, Playing_List[Number].Object_ID, (long)Position);
                if (Last_Result == AK_Success)
                {
                    AK::SoundEngine::RenderAudio();
                    return true;
                }
            }
        }
        return false;
    }
    bool _stdcall Wwise_Set_Position_Second_Container_ID(int Container_ID, int Position)
    {
        for (int Number = 0; Number < (int)Playing_List.size(); Number++)
        {
            if (Playing_List[Number].Container_ID == Container_ID)
            {
                Last_Result = AK::SoundEngine::SeekOnEvent(Playing_List[Number].Event_ID, Playing_List[Number].Object_ID, (long)Position);
                if (Last_Result == AK_Success)
                {
                    AK::SoundEngine::RenderAudio();
                    return true;
                }
            }
        }
        return false;
    }
    int _stdcall Wwise_Get_Max_Length_Name(const char* Name)
    {
        AK::SoundEngine::Suspend();
        int Before_Pos = Wwise_Get_Position_Name(Name);
        for (int Number = 0; Number < (int)Playing_List.size(); Number++)
        {
            if (Playing_List[Number].Event_Name == Name)
            {
                Last_Result = AK::SoundEngine::SeekOnEvent(Name, Playing_List[Number].Object_ID, 0.999f);
                if (Last_Result != AK_Success)
                    return 0;
            }
        }
        AK::SoundEngine::RenderAudio();
        int After_Pos = 0;
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
            int Temp_Pos = Wwise_Get_Position_Name(Name);
            if (Temp_Pos == After_Pos)
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
            After_Pos = Wwise_Get_Position_Name(Name);
        }
        for (int Number = 0; Number < (int)Playing_List.size(); Number++)
        {
            if (Playing_List[Number].Event_Name == Name)
            {
                Last_Result = AK::SoundEngine::SeekOnEvent(Name, Playing_List[Number].Object_ID, (long)Before_Pos);
                if (Last_Result != AK_Success)
                    return 0;
            }
        }
        AK::SoundEngine::WakeupFromSuspend();
        AK::SoundEngine::RenderAudio();
        return After_Pos;
    }
    int _stdcall Wwise_Get_Max_Length_ID(unsigned int Event_ID)
    {
        AK::SoundEngine::Suspend();
        int Before_Pos = Wwise_Get_Position_ID(Event_ID);
        for (int Number = 0; Number < (int)Playing_List.size(); Number++)
        {
            if (Playing_List[Number].Event_ID == Event_ID)
            {
                Last_Result = AK::SoundEngine::SeekOnEvent(Event_ID, Playing_List[Number].Object_ID, 0.999f);
                if (Last_Result != AK_Success)
                    return 0;
            }
        }
        AK::SoundEngine::RenderAudio();
        int After_Pos = 0;
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
            int Temp_Pos = Wwise_Get_Position_ID(Event_ID);
            if (Temp_Pos == After_Pos)
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
            After_Pos = Wwise_Get_Position_ID(Event_ID);
        }
        for (int Number = 0; Number < (int)Playing_List.size(); Number++)
        {
            if (Playing_List[Number].Event_ID == Event_ID)
            {
                Last_Result = AK::SoundEngine::SeekOnEvent(Event_ID, Playing_List[Number].Object_ID, (long)Before_Pos);
                if (Last_Result != AK_Success)
                    return 0;
            }
        }
        AK::SoundEngine::WakeupFromSuspend();
        AK::SoundEngine::RenderAudio();
        return After_Pos;
    }
    int _stdcall Wwise_Get_Max_Length_Container_ID(int Container_ID)
    {
        AK::SoundEngine::Suspend();
        int Index = -1;
        for (int Number = 0; Number < (int)Playing_List.size(); Number++)
            if (Playing_List[Number].Container_ID == Container_ID)
            {
                Index = Number;
                break;
            }
        if (Index == -1)
            return 0;
        int Before_Pos = Wwise_Get_Position_Container_ID(Container_ID);
        Last_Result = AK::SoundEngine::SeekOnEvent(Playing_List[Index].Event_ID, Playing_List[Index].Object_ID, 0.999f);
        if (Last_Result != AK_Success)
            return 0;
        AK::SoundEngine::RenderAudio();
        int After_Pos = 0;
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
            int Temp_Pos = Wwise_Get_Position_Container_ID(Container_ID);
            if (Temp_Pos == After_Pos)
                break;
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
            After_Pos = Wwise_Get_Position_Container_ID(Container_ID);
        }
        Last_Result = AK::SoundEngine::SeekOnEvent(Playing_List[Index].Event_ID, Playing_List[Index].Object_ID, (long)Before_Pos);
        if (Last_Result != AK_Success)
            return 0;
        AK::SoundEngine::WakeupFromSuspend();
        AK::SoundEngine::RenderAudio();
        return After_Pos;
    }
    //解放(以降上にあるメゾットたちは使用できません)
    void __stdcall Wwise_Dispose()
    {
        AK::SoundEngine::StopAll();
        AK::SoundEngine::UnregisterAllGameObj();
        AK::SoundEngine::ClearBanks();
        AK::SoundEngine::Term();
        g_lowLevelIO.Term();
        if (AK::IAkStreamMgr::Get())
            AK::IAkStreamMgr::Get()->Destroy();
        AK::MemoryMgr::Term();
        Playing_List.clear();
    }
    //初期化されているかを取得
    bool __stdcall Wwise_IsInited()
    {
        return AK::SoundEngine::IsInitialized();
    }
    unsigned int _stdcall Wwise_Get_Result_Index()
    {
        return Last_Result;
    }
}
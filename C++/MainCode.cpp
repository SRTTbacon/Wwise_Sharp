#include "MainCode.h"
#include <locale.h>

namespace Wwise_Player
{
    CAkFilePackageLowLevelIOBlocking g_lowLevelIO;
    AkBankID BNK_ID = 0;
    AkGameObjectID Listener = 0;
    std::vector<Name_Playing_ID> Playing_List;
    std::vector<int> End_Event_ID;
    int Listener_Index = 1;
    float Set_Volume = 1.0f;
    AKRESULT Last_Result = AKRESULT::AK_Success;
    FNV_Hash_Class hashClass;

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
        AK::SoundEngine::RegisterGameObj(Listener, "Default Listener");
        AK::SoundEngine::SetDefaultListeners(&Listener, 1);
        AK::SoundEngine::RenderAudio();
        Wwise_Player::Listener_Index = Listener_Index;
        Set_Volume = (float)Init_Volume;
        return true;
    }
    //.bnkファイルをロード(親であるInit.bnkが初期化時と異なる場合はロードできません多分)
    bool __stdcall Wwise_Load_Bank(const char* Stream_BNK)
    {
        std::ifstream ifs(Stream_BNK, std::ios_base::in | std::ios_base::binary);
        if (ifs.fail()) {
            return false;
        }
        std::istreambuf_iterator<char> it_ifs_begin(ifs);
        std::istreambuf_iterator<char> it_ifs_end{};
        std::vector<char> input_data(it_ifs_begin, it_ifs_end);
        if (ifs.fail()) {
            ifs.close();
            return false;
        }
        Last_Result = AK::SoundEngine::LoadBankMemoryCopy(input_data.data(), (unsigned long)input_data.size(), BNK_ID);
        ifs.close();
        input_data.clear();
        //Last_Result = AK::SoundEngine::LoadBank(Stream_BNK, BNK_ID);
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
            Volume = 0.001;
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
    unsigned long __stdcall Wwise_Play_ObjectID(const char* Name, unsigned int ObjectID)
    {
        AkGameObjectID Object_ID = ObjectID;
        AK::SoundEngine::RegisterGameObj(Object_ID);
        if (unsigned long a = AK::SoundEngine::PostEvent(Name, Object_ID) != 0)
        {
            AK::SoundEngine::RenderAudio();
            return a;
        }
        return 0;
    }
    //イベントIDで再生
    bool __stdcall Wwise_Play_ID(unsigned int Event_ID, int Container_ID, double Volume)
    {
        //IDを指定(ランダム)
        if (Volume != -1 && Volume < 0)
            Volume = 0.001;
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
    void __stdcall Wwise_Stop_Object_ID(unsigned int GameObject_ID, unsigned long Playing_ID)
    {
        AK::SoundEngine::StopPlayingID(Playing_ID);
        AK::SoundEngine::UnregisterGameObj(GameObject_ID);
        AK::SoundEngine::RenderAudio();
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
    bool __stdcall Wwise_Set_State(const char* State_Parent_Name, const char* State_Child_Name)
    {
        Last_Result = AK::SoundEngine::SetState(State_Parent_Name, State_Child_Name);
        bool IsOK = Last_Result == AK_Success ? true : false;
        AK::SoundEngine::RenderAudio();
        return IsOK;
    }
    bool __stdcall Wwise_Set_RTPC(const char* RTPC_Name, float Value)
    {
        if (Value < 0)
            Value = 0.0f;
        Last_Result = AK::SoundEngine::SetRTPCValue(RTPC_Name, Value);
        bool IsOK = Last_Result == AK_Success ? true : false;
        AK::SoundEngine::RenderAudio();
        return IsOK;
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
    //オブジェクトの位置を設定
    void _stdcall Wwise_Set_Listener_Position(float X, float Y, float Z, float OriFrontX, float OriFrontY, float OriFrontZ, float OriTopX, float OriTopY, float OriTopZ)
    {
        AkSoundPosition Pos{};
        Pos.Set(X, Y, Z, OriFrontX, OriFrontY, OriFrontZ, OriTopX, OriTopY, OriTopZ);
        Last_Result = AK::SoundEngine::SetPosition(Listener, Pos);
    }
    void _stdcall Wwise_Set_Object_Position(unsigned int Object_ID, float X, float Y, float Z, float OriFrontX, float OriFrontY, float OriFrontZ, float OriTopX, float OriTopY, float OriTopZ)
    {
        AkSoundPosition Pos{};
        Pos.Set(X, Y, Z, OriFrontX, OriFrontY, OriFrontZ, OriTopX, OriTopY, OriTopZ);
        Last_Result = AK::SoundEngine::SetPosition(Object_ID, Pos);
    }
    void _stdcall Wwise_Set_Object_Positions(unsigned int Object_ID, int Count, float* X, float* Y, float* Z, float* OriFrontX, float* OriFrontY, float* OriFrontZ,
        float* OriTopX, float* OriTopY, float* OriTopZ)
    {
        std::vector<float> VecX(X, X + Count);
        std::vector<float> VecY(Y, Y + Count);
        std::vector<float> VecZ(Z, Z + Count);
        std::vector<float> VecFrontX(OriFrontX, OriFrontX + Count);
        std::vector<float> VecFrontY(OriFrontY, OriFrontY + Count);
        std::vector<float> VecFrontZ(OriFrontZ, OriFrontZ + Count);
        std::vector<float> VecTopX(OriTopX, OriTopX + Count);
        std::vector<float> VecTopY(OriTopY, OriTopY + Count);
        std::vector<float> VecTopZ(OriTopZ, OriTopZ + Count);
        AkSoundPosition* Pos_List = new AkSoundPosition[Count];
        for (int i = 0; i < Count; i++)
            Pos_List[i].Set(VecX[i], VecY[i], VecZ[i], VecFrontX[i], VecFrontY[i], VecFrontZ[i], VecTopX[i], VecTopY[i], VecTopZ[i]);
        Last_Result = AK::SoundEngine::SetMultiplePositions(Object_ID, Pos_List, Count);
    }
    //解放(Wwise_Init()を実行するまで上にあるメゾットたちは使用できません)
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

    char* _stdcall HashToChar(int length, unsigned int shortID)
    {
        hashClass.Bruteforce(length, shortID);
        hashClass._bytes.push_back(hashClass.returnLastByte);
        //HANDLE file = CreateFile(AKTEXT("Test.dat"), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
        //DWORD dwActualWrite;
        //WriteFile(file, hashClass._bytes.data(), hashClass._bytes.size(), &dwActualWrite, NULL);
        //WriteFile(file, hashClass.returnLastByte, hashClass._bytes.size(), &dwActualWrite, NULL);
        //std::ofstream binFile("Test.dat", std::ios::out | std::ios::binary);
        //binFile.write((char*)hashClass._bytes.data(), hashClass._bytes.size());
        //binFile.close();
        return (char*)hashClass._bytes.data();
    }

    int _stdcall GetHashLength()
    {
        return (int)hashClass._bytes.size();
    }

    void FNV_Hash_Class::Initialize(int length)
    {
        _bytes.clear();
        _hashes.clear();
        _bytes = std::vector<unsigned char>(length - 1);
        _hashes = std::vector<unsigned int>(length - 1);
        _bytes[0] = 0x60;
        for (auto i = 1; i < _bytes.size(); i++)
        {
            _bytes[i] = 0x5f;
        }
    }

    void FNV_Hash_Class::Bruteforce(int Length, unsigned int match)
    {
        Initialize(Length);
        while (true)
        {
            int depth = (int)_bytes.size() - 1;
            while (Utilities::Increment(_bytes, depth))
            {
                depth--;
                if (depth == -1)
                {
                    return;
                }
            }
            Utilities::ZeroFrom(_hashes, depth);
            unsigned char lastByte = 0x2f;
            unsigned int tempHash = Hash(_bytes, _hashes, (int)_bytes.size()) * Prime;
            unsigned char nextByte;
            while (!Utilities::Increment(lastByte, nextByte))
            {
                lastByte = nextByte;
                unsigned int result = tempHash;
                result ^= lastByte;
                if (result == match)
                {
                    returnLastByte = lastByte;
                    return;
                }
            }
        }
    }

    unsigned int FNV_Hash_Class::Hash(std::vector<unsigned char>& array, std::vector<unsigned int>& hashes, int length)
    {
        if (length > 1)
        {
            unsigned int hash = hashes[static_cast<std::vector<std::seed_seq::result_type, std::allocator<std::seed_seq::result_type>>::size_type>(length) - 1];
            if (hash > 0)
            {
                return hash;
            }
            else
            {
                hash = Hash(array, hashes, length - 1) * Prime;
                hash ^= array[static_cast<std::vector<unsigned char, std::allocator<unsigned char>>::size_type>(length) - 1];
                hashes[static_cast<std::vector<std::seed_seq::result_type, std::allocator<std::seed_seq::result_type>>::size_type>(length) - 1] = hash;
                return hash;
            }
        }
        else
        {
            unsigned int hash = OffsetBasis;
            hash *= Prime;
            hash ^= array[0];
            hashes[0] = hash;
            return hash;
        }
    }

    bool Utilities::Increment(std::vector<unsigned char>& array, int i)
    {
        auto result = static_cast<unsigned char>(array[i] + 1);
        if (result == 0x3a)
        {
            array[i] = 0x61;
            return false;
        }
        else if (result == 0x7b)
        {
            if (i > 0)
            {
                array[i] = 0x5f;
                return false;
            }
            else
            {
                array[i] = 0x61;
                return true;
            }
        }
        else if (result == 0x60)
        {
            array[i] = 0x30;
            return true;
        }
        array[i] = result;
        return false;
    }

    bool Utilities::Increment(unsigned char from, unsigned char& result)
    {
        result = static_cast<unsigned char>(from + 1);
        if (result == 0x3a)
        {
            result = 0x61;
            return false;
        }
        else if (result == 0x7b)
        {
            result = 0x5f;
            return false;
        }
        else if (result == 0x60)
        {
            result = 0x30;
            return true;
        }
        return false;
    }

    void Utilities::ZeroFrom(std::vector<unsigned int>& array, int i)
    {
        while (i < array.size())
        {
            array[i] = 0;
            i++;
        }
    }

}
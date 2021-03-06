# Wwise_Sharp<br>
C#からWwiseの機能を使用できるライブラリです。<br>
<br>
# ---使用方法---<br>
ステップ1:Wwise_Player.csをプロジェクトに追加<br>
ステップ2:using WoTB_Voice_Mod_Createrをどこかのクラスに追加<br>
ステップ3:Wwise_Player.Init(Init.bnkの場所)を起動時に呼び出す<br>
ステップ4:Wwise_Player.Load_Bank(.bnkの場所)を実行<br>
ステップ5:Wwise_Player.Play(イベント名またはイベントID, 識別ID)を実行するとイベントが再生されます(識別IDはどんな数字でも構いません。詳しくは---各機能---をご覧ください)<br>
<br>
# ---注意事項---<br>
・.exeが生成される場所に、付属しているWwise_Player.dllを設置する必要があります<br>
・すべての関数をテストしているわけではないので、バグがあるかもしれません<br>
<br>
# ---各機能(関数)---<br>
※注意1:イベント名またはイベントIDが引数のPlay()を除く関数は、Container_ID(コンテナID)も対応しています。<br>
※注意2:イベントIDはuintで、Container_ID(コンテナID)はintです。間違えれば正常に処理されない可能性がありますのでお気をつけて。<br>
<br>
・bool Init(...) = Wwiseの初期化(2回以上実行する場合は、必ず実行する前にDispose()を呼ぶ必要があります)。 | 戻り値:成功したらtrue<br>
Listener_Indexは基本は1で、.bnk内のBus設定が特殊な場合1以外を指定できます。Set_Init_VolumeはInit()内でSet_Volumeを実行するときの値です。<br>
<br>
・bool Load_Bank(...) = .bnkファイルを追加します。この関数を実行しても、既にロードされている.bnkファイルを失うことはありません。 | 戻り値:ロードできたらtrue<br>
<br>
・List<int> Get_End_Event_List() = 戻り値:終了したイベントのコンテナID(Play()関数で指定するID)のリスト。イベント終了を感知する場合は定期的にこの関数を呼び出す必要があります。<br>
  <br>
・bool Play(...) = イベント名またはイベントIDを指定してイベントを再生させます。2つ目の引数は、イベント終了検知用のIDで、自由に決めて大丈夫です。 | 戻り値:再生可能であればtrue<br>
  <br>
・bool Stop(...) = 再生中のイベントを停止させます。引数ありで特定のイベントのみに適応され、引数なしですべてのイベントに適応されます。 | 戻り値:停止できるイベントがあればtrue(引数なしの場合は戻り値なし)<br>
  <br>
・bool Pause_All() = 再生中のすべてのイベントを一時停止させます。今のところ特定のイベントのみ適応というのはできません。 | 戻り値:一時停止できればtrue<br>
  <br>
・bool Play_All() = 一時停止中のすべてのイベントを再生させます。今のところ特定のイベントのみ適応というのはできません。 | 戻り値:再生できればtrue<br>
  <br>
・int Get_Position(...) = 指定したイベント名またはイベントIDの現在の再生時間(ミリ秒)を取得します。 | 戻り値:ミリ秒単位の再生時間<br>
  <br>
・bool Set_Position(..., float Percent) = 指定したイベント名またはイベントIDの再生時間を0～1の範囲で移動させます。(1を指定するとループでない限りイベントは終了します) | 戻り値:時間を移動できればtrue<br>
  <br>
・bool Set_Position(..., int Position) = 指定したイベント名またはイベントIDの再生時間をミリ秒単位で移動させます。 | 戻り値:時間を移動できればtrue<br>
  <br>
・int Get_Max_Length(...) = 指定したイベント名またはイベントIDのサウンドの長さをミリ秒単位で取得します。(イベント内にコンテナが複数ある場合は---備考---を参照してください) | 戻り値:サウンドの長さ<br>
  <br>
・bool Set_Volume(...(引数が2つのもの)) = 指定したイベント名またはイベントIDの音量を0～1の範囲で変更 | 戻り値:音量が変更されたイベントがあればtrue<br>
  <br>
・bool Set_Volume(...(引数が1つのもの)) = 再生中のすべてのイベントの音量を0～1の範囲で変更 | 戻り値:音量が変更されたイベントがあればtrue<br>
  <br>
・double Get_Volume(...) = 指定したイベント名またはイベントIDの音量を取得<br>
  <br>
・List<Container_und_Volume>　Get_Volume() = 再生中のすべてのイベントの音量を取得 | 戻り値:音量とコンテナIDが入ったクラスの配列<br>
  <br>
・void Dispose() = Wwiseを初期状態に戻す。(以降使用するにはInit()を呼ぶ必要があります。)<br>
  <br>
・bool IsInited() = Wwiseが正常に起動したかを取得 | 戻り値:問題なければtrue<br>
  <br>
・uint Get_Result_Index() = 最新のWwiseのログのインデックスを取得(enum AKRESULTに詳細が書かれています) | 戻り値:ログのインデックス<br>
  <br>
# ---備考---
・int Get_Max_Length()は、イベント内の1つのサウンドの長さを取得して値を返します。なので、イベント内に複数のコンテナが存在する場合、どのサウンドの長さを取得するのかがわかりません。<br>
また、Get_Max_Length()を実行すると、0.3秒ほど再生が止まります。(本来できないので、無理やり取得しようとしているため)<br>
面白いことに、サウンドの最大値を超えた状態？(よくわかりません)でもう一度関数を呼ぶと、別のコンテナの長さが取得できます。(イベント内にコンテナが複数ある場合に限る)<br>
  <br>
・Set_Volume()は0を指定しても実際には0.001となります。(Wwiseの仕様上、0を指定するとイベントが終了してしまうのだ！)
　<br>
・すべての関数をテストしているわけではないので、予期しない動作をするところがあるかもしれません。

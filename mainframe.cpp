#include <iostream>
#include <fstream>
#include <chrono>
#include <moon/wx/bitmap.hpp>
#include <moon/file.hpp>
#include <miniaudio/miniaudio.h>
#include <curl/curl.h>

#include "mainframe.h"
#include "all_strings.h"
#include "sinalmusic.h"
#include "logindialog.hpp"
#include "dlgnomusic.h"

#include "images/delete_icon.xpm"
#include "images/edit_icon.xpm"
#include "images/uta_icon_large.xpm"
#include "images/play_icon.xpm"
#include "images/share_icon.xpm"

/*#ifndef __arm__
#ifdef DEBUG
#define digitalRead(x) true
#else
#define digitalRead(x) false
#endif
#define digitalWrite(x,y)
#define HIGH 1
#define LOW 0
#endif*/

static int gpioports[]={RPI_V2_GPIO_P1_31,RPI_V2_GPIO_P1_33,RPI_V2_GPIO_P1_35};

#define wxAbortError(message) wxMessageBox(message, wxString::FromUTF8(STR_Error), wxICON_ERROR, this); return;
#define wxLabel(parent, label) new wxStaticText(parent, wxID_ANY, label)
#define STR2(x) #x
#define STR(x) STR2(x)
#define INPUT_FILE_DETAIL(root,x) root #x
#define CMakeRelative(x) INPUT_FILE_DETAIL(CMakeListsPath "/", x)
#ifdef __arm__
#define INCBIN(name, file) \
    __asm__(".section .rodata\n" \
            ".global incbin_" STR(name) "_start\n" \
            ".type incbin_" STR(name) "_start, object\n" \
            ".balign 16\n" \
            "incbin_" STR(name) "_start:\n" \
            ".incbin \"" file "\"\n" \
            \
            ".global incbin_" STR(name) "_end\n" \
            ".type incbin_" STR(name) "_end, object\n" \
            ".balign 1\n" \
            "incbin_" STR(name) "_end:\n" \
            ".byte 0\n" \
            \
            ".global incbin_" STR(name) "_size\n" \
            ".type incbin_" STR(name) "_size, object\n" \
            ".balign 16\n" \
            "incbin_" STR(name) "_size:\n" \
            ".int incbin_" STR(name) "_end - incbin_" STR(name) "_start\n" \
    ); \
    extern "C" const __attribute__((aligned(16))) void* incbin_ ## name ## _start; \
    extern "C" const void* incbin_ ## name ## _end; \
    extern "C" const unsigned int incbin_ ## name ## _size;
#else
#define INCBIN(name, file) \
    __asm__(".section .rodata\n" \
            ".global incbin_" STR(name) "_start\n" \
            ".type incbin_" STR(name) "_start, @object\n" \
            ".balign 16\n" \
            "incbin_" STR(name) "_start:\n" \
            ".incbin \"" file "\"\n" \
            \
            ".global incbin_" STR(name) "_end\n" \
            ".type incbin_" STR(name) "_end, @object\n" \
            ".balign 1\n" \
            "incbin_" STR(name) "_end:\n" \
            ".byte 0\n" \
            \
            ".global incbin_" STR(name) "_size\n" \
            ".type incbin_" STR(name) "_size, @object\n" \
            ".balign 16\n" \
            "incbin_" STR(name) "_size:\n" \
            ".int incbin_" STR(name) "_end - incbin_" STR(name) "_start\n" \
    ); \
    extern "C" const __attribute__((aligned(16))) void* incbin_ ## name ## _start; \
    extern "C" const void* incbin_ ## name ## _end; \
    extern "C" const unsigned int incbin_ ## name ## _size;
#endif

INCBIN(sirene,CMakeRelative(SinaleiroMusical/sound/sirene.mp3));
INCBIN(dingo,CMakeRelative(SinaleiroMusical/sound/dingo.mp3));

char* _str_list[quant_strings];
std::string sBuffer;
wxSize m_comboSize={-1,-1};
int margem=2;
const wxWindowID ID_TIMER=wxNewId();
enum Col{
 COL_Hour,
 COL_Days,
 COL_Time,
 COL_File,
 COL_Actions
};
static wxBitmap s_editIcon=wxBitmap(editIcon_XPM);
static wxBitmap s_deleteIcon=wxBitmap(deleteIcon_XPM);
static wxBitmap s_playIcon=wxBitmap(playIcon_XPM);
static wxBitmap s_stopIcon=wxBitmap(stopIcon_XPM);
static wxBitmap s_shareIcon=wxBitmap(shareIcon_XPM);

class wxGridCellActionsRenderer: public wxGridCellRenderer{
protected:
 bool m_showPlay=false;
public:
 wxGridCellActionsRenderer(bool showPlay=false)
  : m_showPlay(showPlay),wxGridCellRenderer(){

 }
 virtual wxGridCellRenderer* Clone() const override{
  wxGridCellActionsRenderer* renderer=new wxGridCellActionsRenderer(m_showPlay);
  return renderer;
 }

 virtual void Draw(wxGrid& grid,wxGridCellAttr& attr,wxDC& dc,const wxRect& rect,int row,int col,bool isSelected) override{
  wxPoint point=rect.GetPosition();
  point.x+=4;
  point.y+=4;
  if(m_showPlay) {
   dc.DrawBitmap(s_playIcon,point);
   point.x+=28;
   dc.DrawBitmap(s_stopIcon,point);
   point.x+=28;
  }
  dc.DrawBitmap(s_editIcon,point);
  point.x+=28;
  dc.DrawBitmap(s_deleteIcon,point);
  point.x+=28;
  if(m_showPlay) {
   dc.DrawBitmap(s_shareIcon,point);
  }
 }

 virtual wxSize GetBestSize(wxGrid& grid,wxGridCellAttr& attr,wxDC& dc,int row,int col) override{
  return {m_showPlay?148:60,32};
 }
};

class dlgChooseLang:public wxDialog{
public:
 dlgChooseLang(wxWindow* parent,int index);
 void on_btok_clicked();
private:
 wxComboBox* combobox;
 wxButton* btn_close;
 void OnDestroy(wxWindowDestroyEvent& event);
 DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(dlgChooseLang,wxDialog)
EVT_WINDOW_DESTROY(dlgChooseLang::OnDestroy)
END_EVENT_TABLE()

void dlgChooseLang::OnDestroy(wxWindowDestroyEvent& event){
 event.Skip();
}

void dlgChooseLang::on_btok_clicked(){
 MainFrame::mn_language=combobox->GetCurrentSelection();
 MainFrame::lestrings(true);
 wxString voice_path=SinaleiroMusical::GetDataPath("lang.txt");
 wxFile file;
 if(file.Create(voice_path,true))
 {
  if(file.Open(voice_path,wxFile::write))
  {
   file.Write(wxString::FromDouble((double)MainFrame::mn_language));
  }
 }
}

dlgChooseLang::dlgChooseLang(wxWindow* parent,int index):wxDialog(parent,wxID_ANY,wxString::FromUTF8(STR_ChooseLanguage),wxDefaultPosition,wxSize(200,100)){
 wxArrayString cbitems(0);
 for(int i=0;i<NUM_LANG_SUPPORT;i++)cbitems.Add(wxString::FromUTF8(lang_descr[i]));
 wxBoxSizer* szV=new wxBoxSizer(wxVERTICAL);
 combobox=new wxComboBox(this,wxID_ANY,wxT(""),wxPoint(-1,-1),wxSize(-1,-1),cbitems);
 combobox->SetSelection(index);
 wxBoxSizer* button_sizer=new wxBoxSizer(wxHORIZONTAL);
 szV->Add(combobox,0,wxALL,10);
 btn_close=new wxButton(this,wxID_OK,wxT("Ok"),wxPoint(-1,-1),wxSize(-1,-1));
 button_sizer->Add(btn_close,0,wxALL,10);
 szV->Add(button_sizer,0,wxALIGN_CENTER);
 SetIcon(wxNullIcon);
 this->SetSizer(szV);
 szV->SetSizeHints(this);
 Center();
}

class TabManagerFrame: public wxFrame {
private:
 GtkSwitch* m_switchs[AlarmTypeCount]{nullptr};
 GtkButton* m_button=nullptr;
public:
 TabManagerFrame(): wxFrame(nullptr,wxID_ANY,wxString::FromUTF8(STR_Tabmanager)) {
  std::bitset<AlarmTypeCount> tabs=SinaleiroMusical::GetAvailableTabs();
  GtkWidget* listBox=gtk_list_box_new();
  gtk_list_box_set_selection_mode(GTK_LIST_BOX(listBox),GtkSelectionMode::GTK_SELECTION_NONE);
  gtk_list_box_set_header_func(GTK_LIST_BOX(listBox),TabManagerFrame::HeaderFunction,nullptr,nullptr);
  wxNativeWindow* nativeListBox=new wxNativeWindow(this,wxID_ANY,listBox);
  g_object_unref(listBox);
  const char* tabNames[AlarmTypeCount]{STR_Alarms,STR_DailyCommunications,STR_ScheduledCommunications,STR_QuickWarnings,STR_Security};
  for(int i=0; i<AlarmTypeCount; ++i) {
   GtkWidget* row=gtk_list_box_row_new();
   gtk_widget_set_can_focus(row,TRUE);
   gtk_list_box_row_set_selectable(GTK_LIST_BOX_ROW(row),FALSE);
   gtk_list_box_row_set_activatable(GTK_LIST_BOX_ROW(row),FALSE);
   GtkWidget* hbox=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
   GtkWidget* label1=gtk_label_new(tabNames[i]);
   gtk_widget_set_margin_start(label1,12);
   gtk_widget_set_margin_end(label1,12);
   gtk_widget_set_margin_bottom(label1,6);
   gtk_widget_set_margin_top(label1,6);
   gtk_label_set_xalign(GTK_LABEL(label1),0);
   GtkWidget* switchHandle=gtk_switch_new();
   GtkSwitch* sw=GTK_SWITCH(switchHandle);
   g_signal_connect(switchHandle,"state_set",G_CALLBACK(TabManagerFrame::SwitchChanged),this);
   if(tabs[i]) {
    gtk_switch_set_state(sw,true);
   }
   m_switchs[i]=sw;
   gtk_widget_set_margin_start(switchHandle,12);
   gtk_widget_set_margin_end(switchHandle,12);
   gtk_widget_set_margin_bottom(switchHandle,12);
   gtk_widget_set_margin_top(switchHandle,12);
   gtk_container_add(GTK_CONTAINER(row),hbox);
   gtk_box_pack_start(GTK_BOX(hbox),label1,TRUE,TRUE,0);
   gtk_box_pack_start(GTK_BOX(hbox),switchHandle,FALSE,TRUE,0);
   gtk_container_add(GTK_CONTAINER(nativeListBox->GetHandle()),row);
   gtk_widget_show_all(row);
  }
  GtkWidget* row=gtk_list_box_row_new();
  gtk_widget_set_can_focus(row,TRUE);
  gtk_list_box_row_set_selectable(GTK_LIST_BOX_ROW(row),FALSE);
  gtk_list_box_row_set_activatable(GTK_LIST_BOX_ROW(row),FALSE);
  GtkWidget* hbox=gtk_box_new(GTK_ORIENTATION_HORIZONTAL,0);
  GtkWidget* label1=gtk_label_new(wxString::FromUTF8(STR_Language));
  gtk_widget_set_margin_start(label1,12);
  gtk_widget_set_margin_end(label1,12);
  gtk_widget_set_margin_bottom(label1,6);
  gtk_widget_set_margin_top(label1,6);
  gtk_label_set_xalign(GTK_LABEL(label1),0);
  GtkWidget* btlang=gtk_button_new_with_label("...");
  GtkButton* sw=GTK_BUTTON(btlang);
  g_signal_connect(btlang,"clicked",G_CALLBACK(TabManagerFrame::btlangclicked),this);
  m_button=sw;
  gtk_widget_set_margin_start(btlang,12);
  gtk_widget_set_margin_end(btlang,12);
  gtk_widget_set_margin_bottom(btlang,12);
  gtk_widget_set_margin_top(btlang,12);
  gtk_container_add(GTK_CONTAINER(row),hbox);
  gtk_box_pack_start(GTK_BOX(hbox),label1,TRUE,TRUE,0);
  gtk_box_pack_start(GTK_BOX(hbox),btlang,FALSE,TRUE,0);
  gtk_container_add(GTK_CONTAINER(nativeListBox->GetHandle()),row);
  gtk_widget_show_all(row);
  wxBoxSizer* buttons_sizer=new wxBoxSizer(wxHORIZONTAL);
  Bind(wxEVT_BUTTON,[this](wxCommandEvent& event) {
   this->Close();
   event.Skip();
   },buttons_sizer->Add(new wxButton(this,wxID_ANY,wxString::FromUTF8(STR_Cancel)))->GetWindow()->GetId());
  Bind(wxEVT_BUTTON,[this](wxCommandEvent& event) {
   this->Close();
   MainFrame* mainFrame=(MainFrame*)wxTheApp->GetTopWindow();
   mainFrame->SetTitle(wxString::FromUTF8(STR_UtaMusicalAlarm));
   wxString title=mainFrame->GetTitle();
   mainFrame->CreateGUIControls();
   },buttons_sizer->Add(new wxButton(this,wxID_ANY,wxString::FromUTF8(STR_Apply)))->GetWindow()->GetId());
  wxBoxSizer* rootSizer=new wxBoxSizer(wxVERTICAL);
  rootSizer->Add(nativeListBox,1,wxEXPAND|wxALL,1);
  rootSizer->Add(buttons_sizer,0,wxALL,4);
  SetSizer(rootSizer);
  DoGetBestSize();
  gtk_widget_show_all((GtkWidget*)this);
 }

 static void btlangclicked(GtkButton* button,gpointer user_data) {
  dlgChooseLang* dlglang;
  dlglang=new dlgChooseLang((wxWindow*)user_data,MainFrame::mn_language);
  if(dlglang->ShowModal()==wxID_OK){
   dlglang->on_btok_clicked();
  }
  delete  dlglang;
 }

 static void HeaderFunction(GtkListBoxRow* row,GtkListBoxRow* before,gpointer user_data) {
  GtkWidget* current;
  if(!before) {
   gtk_list_box_row_set_header(row,nullptr);
   return;
  }
  current=gtk_list_box_row_get_header(row);
  if(!current) {
   current=gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
   gtk_widget_show(current);
   gtk_list_box_row_set_header(row,current);
  }
 }

 static gboolean SwitchChanged(GtkSwitch* sw,gboolean state,gpointer user_data) {
  TabManagerFrame* self=(TabManagerFrame*)user_data;
  std::bitset<AlarmTypeCount> tabs;
  if(std::find(std::begin(self->m_switchs),std::end(self->m_switchs),sw)!=std::end(self->m_switchs)) {
   for(int i=0; i<AlarmTypeCount; ++i) {
    if(self->m_switchs[i]==sw) {
     tabs[i]=state;
    }
    else {
     tabs[i]=gtk_switch_get_state(self->m_switchs[i]);
    }
   }
   unsigned long tabsBits=tabs.to_ulong();
   Moon::File::WriteAllBytes(SinaleiroMusical::GetDataPath("tabs.bin").ToStdString(),&tabsBits,sizeof(unsigned long));
  }
  return FALSE;
 }
};

class UtaStaticBitmap: public wxStaticBitmap{
public:
 UtaStaticBitmap(wxWindow* parent): wxStaticBitmap(parent,wxID_ANY,wxBitmap(utaIconLarge_XPM)) {
  Bind(wxEVT_LEFT_DCLICK,[parent](wxMouseEvent& event) {
   LoginDialog dialog(parent);
   if(dialog.ShowModal()==wxID_OK) {
    TabManagerFrame* frame=new TabManagerFrame();
    frame->Centre();
    frame->Show();
   }
   event.Skip();
   });
 }
};

int secondOfDay(wxDateTime& dt) {
 return (dt.GetHour()*3600)+(dt.GetMinute()*60)+dt.GetSecond();
}

void data_callback(ma_device* pDevice,void* pOutput,const void* pInput,ma_uint32 frameCount){
 ma_bool32 isLooping=MA_TRUE;
 ma_decoder* pDecoder=(ma_decoder*)pDevice->pUserData;
 if(pDecoder==NULL) {
  return;
 }
 ma_data_source_read_pcm_frames(pDecoder,pOutput,frameCount,NULL,isLooping);
 (void)pInput;
}

void data_callback_loop(ma_device* pDevice,void* pOutput,const void* pInput,ma_uint32 frameCount){
 ma_bool32 isLooping=MA_TRUE;
 ma_decoder* pDecoder=(ma_decoder*)pDevice->pUserData;
 if(pDecoder==NULL) {
  return;
 }
 ma_data_source_read_pcm_frames(pDecoder,pOutput,frameCount,NULL,isLooping);
 (void)pInput;
}

int GetStreamSize(Aws::IOStream* stream){
 stream->seekg(0,std::ios::beg);
 int begin=stream->tellg();
 stream->seekg(0,std::ios::end);
 int end=stream->tellg();
 stream->seekg(0,std::ios::beg);

 return end-begin;
}

char* GetStreamBytes(Aws::IOStream* stream){
 int size=GetStreamSize(stream);
 char* bytes=new char[size];
 stream->read(bytes,size);

 return bytes;
}

class QuickAlarmEditDialog: public wxDialog{
public:
 QuickAlarmEditDialog(wxWindow* parent,QuickAlarm* alarm)
  : wxDialog(parent,wxID_ANY,wxString::FromUTF8(STR_EditAlarm)),m_alarm(alarm){
  mainframe=(MainFrame*)parent;
  m_alarm=alarm;
  CreateGUIControls();
 }
private:
 MainFrame* mainframe;
 QuickAlarm* m_alarm;
 wxTextCtrl* m_textSpeech;
 wxTextCtrl* m_textTitle;
 wxOwnerDrawnComboBox* m_voiceCombo;
private:
 void CreateGUIControls() {
  wxArrayString languages;
  for(const std::pair<wxString,int>& language:s_languages) {
   languages.push_back(language.first);
  }
  m_textTitle=new wxTextCtrl(this,wxID_ANY,m_alarm->title,wxDefaultPosition,wxDefaultSize);
  m_textSpeech=new wxTextCtrl(this,wxID_ANY,m_alarm->description,wxDefaultPosition,wxSize(300,150),wxTE_MULTILINE);
  wxButton* convertButton=new wxButton(this,wxID_ANY,wxString::FromUTF8(STR_Save));
  m_voiceCombo=new wxOwnerDrawnComboBox(this,wxID_ANY,m_alarm->voice,wxDefaultPosition,wxDefaultSize,mainframe->m_voiceOptions,wxCB_READONLY);
  convertButton->Bind(wxEVT_BUTTON,[this](wxCommandEvent& event) {
   wxString title=m_textTitle->GetValue();
   if(title.empty()) {
    wxMessageBox(wxString::FromUTF8(STR_Thetitlecannotbeempty),wxString::FromUTF8(STR_Error),wxICON_ERROR);
    return;
   }
   wxString path=wxFileName(SinaleiroMusical::GetMusicFolder(QuickAlarmType),title,L"mp3").GetFullPath();
   if(wxFileExists(path)&&title!=m_alarm->title) {
    wxMessageBox(wxString::FromUTF8(STR_Warningtitleexists),wxString::FromUTF8(STR_Error),wxICON_ERROR);
    return;
   }
   wxString oldpath=wxFileName(SinaleiroMusical::GetMusicFolder(QuickAlarmType),m_alarm->title,L"mp3").GetFullPath();
   if(wxFileExists(oldpath))wxRemoveFile(oldpath);
   wxString speech=m_textSpeech->GetValue().ToStdString();
   Aws::Polly::Model::SynthesizeSpeechRequest speechRequest;
   wxString voice_with_language=m_voiceCombo->GetValue();
   wxString voice=voice_with_language.substr(0,voice_with_language.find('-')-1);
   speechRequest.SetVoiceId(Aws::Polly::Model::VoiceIdMapper::GetVoiceIdForName(voice.ToStdString()));
   speechRequest.SetOutputFormat(Aws::Polly::Model::OutputFormat::mp3);
   speechRequest.SetText(speech);
   Aws::Polly::Model::SynthesizeSpeechOutcome result=mainframe->m_pollyClient->SynthesizeSpeech(speechRequest);
   if(result.IsSuccess()) {
    Aws::IOStream& audio_stream=result.GetResult().GetAudioStream();
    Aws::OFStream   voice_file;
    Aws::IOStream* audio_stream1=&result.GetResult().GetAudioStream();
    // Get the bytes from the stream and save them into a file
    voice_file.open("./polly.mp3",std::ios::out|std::ios::binary);
    voice_file.write(GetStreamBytes(audio_stream1),GetStreamSize(audio_stream1));
    voice_file.close();
    audio_stream.seekg(0,std::ios::beg);
    size_t begin=audio_stream.tellg();
    audio_stream.seekg(0,std::ios::end);
    size_t end=audio_stream.tellg();
    audio_stream.seekg(0,std::ios::beg);
    size_t size=end-begin;
    uint8_t* bytes=new uint8_t[size];
    audio_stream.read((char*)bytes,size);
    std::vector<uint8_t> pcmFrames;
    PlayerApi::ConvertToPCMFrames(&incbin_dingo_start,incbin_dingo_size,pcmFrames);
    float* data=(float*)pcmFrames.data();
    float* dataEnd=(float*)(pcmFrames.data()+pcmFrames.size());
    while(data<dataEnd) {
     *data=*data*0.5f;
     data++;
    }
    PlayerApi::ConvertToPCMFrames(bytes,size,pcmFrames);
    ma_encoder_config encoderConfig=ma_encoder_config_init(ma_resource_format_wav,ma_format_f32,2,44100);
    ma_encoder encoder;
    ma_encoder_init_file(path.ToStdString().c_str(),&encoderConfig,&encoder);
    int totalFramesWritten=0;
    int bytesWritten=0;
    int outputChannels=2;
    int sampleSize=4;
    int pcmFrameCount=pcmFrames.size()/sampleSize/outputChannels;
    while(totalFramesWritten<pcmFrameCount) {
     int framesWritten=ma_encoder_write_pcm_frames(&encoder,pcmFrames.data()+bytesWritten,pcmFrameCount);
     bytesWritten+=outputChannels*sampleSize*framesWritten;
     totalFramesWritten+=framesWritten;
    }
    ma_encoder_uninit(&encoder);
    SinaleiroMusical::AppendQuickAlarm(title,speech,voice_with_language,m_alarm->title);
    m_alarm->title=title;
    mainframe->UpdateGrid(QuickAlarmType);
    sBuffer.clear();
    m_textSpeech->SetValue(L"");
    m_textTitle->SetValue(L"");
    wxMessageBox(wxString::FromUTF8(STR_Filesuccessfullyconverted),wxString::FromUTF8(STR_Information),wxICON_INFORMATION);
   }
   else {
    wxMessageBox(wxString::FromUTF8(STR_Errorconvertingfile),wxString::FromUTF8(STR_Error),wxICON_EXCLAMATION);
   }
   EndModal(wxOK);
   event.Skip();
   });
  wxBoxSizer* sizer=new wxBoxSizer(wxVERTICAL);
  sizer->Add(m_textTitle,0,wxEXPAND|wxBOTTOM,4);
  sizer->Add(m_textSpeech,1,wxEXPAND|wxBOTTOM,4);
  sizer->Add(m_voiceCombo,0,wxEXPAND|wxBOTTOM,4);
  sizer->AddStretchSpacer(0);
  sizer->Add(convertButton,0,wxALIGN_CENTER|wxBOTTOM,4);
  SetSizer(sizer);
  if(GetSizer()){
   GetSizer()->SetSizeHints(this);
  }
  Centre();
 }
};

class QuickAlarmExportDialog: public wxDialog {
public:
 QuickAlarmExportDialog(): wxDialog(nullptr,wxID_ANY,wxString::FromUTF8(STR_Exportquickwarning)) {
  CreateGUIControls();
 }
private:
 wxChoice* m_tabSelection;
private:
 void CreateGUIControls() {
  wxArrayString abas;
  abas.push_back(wxString::FromUTF8(STR_Alarms));
  abas.push_back(wxString::FromUTF8(STR_DailyCommunications));
  abas.push_back(wxString::FromUTF8(STR_ScheduledCommunications));
  abas.push_back(wxString::FromUTF8(STR_Security));
  m_tabSelection=new wxChoice(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,abas,wxCB_READONLY);
  m_tabSelection->SetSelection(AlarmType::Alarm);
  wxBoxSizer* rootSizer=new wxBoxSizer(wxVERTICAL);
  rootSizer->Add(new wxStaticText(this,wxID_ANY,wxString::FromUTF8(STR_Selectthetab)),0,wxALL,4);
  rootSizer->Add(m_tabSelection,0,wxALL,4);
  wxBoxSizer* buttonsSizer=new wxBoxSizer(wxHORIZONTAL);
  buttonsSizer->Add(new wxButton(this,wxID_CANCEL,wxString::FromUTF8(STR_Cancel)),1,wxLEFT|wxRIGHT,margem);
  buttonsSizer->Add(new wxButton(this,wxID_OK,wxString::FromUTF8(STR_Export)),1,wxLEFT|wxRIGHT,margem);
  buttonsSizer->GetItem((size_t)0)->GetWindow()->SetLabel(wxString::FromUTF8(STR_Cancel));
  rootSizer->Add(buttonsSizer,0,wxEXPAND,0);
  SetSizerAndFit(rootSizer);
  SetBackgroundColour(wxColour(255,255,255));
 }
public:
 AlarmType GetSelectedTab() {
  int selection=m_tabSelection->GetSelection();
  switch(selection) {
  case AlarmType::QuickAlarmType:
   return AlarmType::Security;
   break;
  default: return (AlarmType)selection;
  }
 }
};

class AlarmeDialog: public wxDialog{
private:
 AlarmType m_type;
 bool      m_ok=true;
public:
 AlarmeDialog(wxWindow* parent,Alarme& alarme,const AlarmType& type): wxDialog(parent,wxID_ANY,SinaleiroMusical::AlarmTypeToString(type)),m_alarme(alarme),m_type(type){
  wxDateTime now=wxDateTime::Now();
  if(type==AlarmType::Security)
   m_startTime=now;
  else {
   m_endTime=now;
   if(type!=AlarmType::DailyRelease)
    m_alarme.m_norepeat=1;
   m_endTime.Add(wxTimeSpan(0,0,m_alarme.m_duration*m_alarme.m_norepeat));//*m_alarme.m_norepeat
  }
  bool looping=type==AlarmType::Security||AlarmType::Alarm;
  wxString music=alarme.m_music;
  wxString path;
  if(type==AlarmType::Alarm) {
   if(music==wxString::FromUTF8(STR_Random)) {
    auto& musics=SinaleiroMusical::GetMusics(type);
    if(musics.size()==1)
     path=SinaleiroMusical::GetMusicPath(musics.back().name(),type);
    else {
     wxString random=RandomMusic(type);
     path=SinaleiroMusical::GetMusicPath(random,type);
    }
   }
   else if(music==L"Input 4") {
    alarme.m_naNF=true;
    bcm2835_gpio_set(RPI_V2_GPIO_P1_37);
   }
   else {
    path=SinaleiroMusical::GetMusicPath(music,type);
    if(!wxFileExists(path)) {
     path=SinaleiroMusical::GetMusicPath(RandomMusic(type),type);
    }
   }
  }
  else
   path=SinaleiroMusical::GetMusicPath(music,type);
  if(path.empty()) {
   if(type==AlarmType::Alarm)
    PlayMusic(&incbin_sirene_start,incbin_sirene_size,looping,music==L"Input 4"?0.0:1.0);
   else {
    m_ok=false;
    return;
   }
  }
  else {
   if(!wxFileExists(path)||!PlayMusic(path.ToStdString(),looping)) {
    if(type==AlarmType::Alarm) {
     PlayMusic(&incbin_sirene_start,incbin_sirene_size,looping,1.0);
    }
    else {
     m_ok=false;
     return;
    }
   }
  }
  CreateGUIControls();
  m_timer.Start(200);
  m_timer.Bind(wxEVT_TIMER,&AlarmeDialog::OnTimer,this);
  srand(time(nullptr));
 }
private:
 Alarme& m_alarme;
 wxStaticText* m_durationLabel=nullptr;
 wxTimer m_timer;
 wxDateTime m_endTime;
 wxDateTime m_startTime;
 wxString m_musicPath;
 bool m_musicState=false;
 ma_decoder m_decoder;
 ma_device m_device;
 bool m_playing=false;
 bool m_errorPlaying=false;
 wxDECLARE_EVENT_TABLE();
private:
 const wxString& RandomMusic(const AlarmType& type){
  auto& musics=SinaleiroMusical::m_musics[type];
  int random=rand();
  int index=random%musics.size();
  return musics[index].name();
 }
public:
 bool IsOk(){
  return m_ok;
 }

 static bool CheckMusic(const wxString& music,const AlarmType& type) {
  if((music==wxString::FromUTF8(STR_Random)||music==L"Input 4")&&type==AlarmType::Alarm)
   return true;
  wxString path=SinaleiroMusical::SinaleiroMusical::GetMusicPath(music,type);
  return wxFileExists(path);
 }
private:
 bool PlayMusic(const void* data,size_t size,bool looping,float vol){
  ma_result result;
  if(ma_decoder_init_memory(data,size,NULL,&m_decoder)!=MA_SUCCESS)
   return false;
  return PlayMusic(looping,vol);
 }

 bool PlayMusic(const std::string& path,bool looping){
  ma_result result;
  ma_device_config deviceConfig;
  if(ma_decoder_init_file(path.c_str(),NULL,&m_decoder)!=MA_SUCCESS)
   return false;
  return PlayMusic(looping,1.0);
 }

 bool PlayMusic(bool looping,float vol){
  ma_device_config deviceConfig;
  deviceConfig=ma_device_config_init(ma_device_type_playback);
  deviceConfig.playback.format=m_decoder.outputFormat;
  deviceConfig.playback.channels=m_decoder.outputChannels;
  deviceConfig.sampleRate=m_decoder.outputSampleRate;
  if(looping)
   deviceConfig.dataCallback=data_callback_loop;
  else
   deviceConfig.dataCallback=data_callback;
  deviceConfig.pUserData=&m_decoder;
  if(ma_device_init(NULL,&deviceConfig,&m_device)!=MA_SUCCESS){
   ma_decoder_uninit(&m_decoder);
   return false;
  }
  ma_device_set_master_volume(&m_device,vol);
  if(ma_device_start(&m_device)!=MA_SUCCESS){
   ma_device_uninit(&m_device);
   ma_decoder_uninit(&m_decoder);
   return false;
  }
  m_playing=true;
  return true;
 }

 void StopMusic(){
  m_alarme.m_firedToday=true;
  if(!m_playing)
   return;
  bcm2835_gpio_clr(RPI_V2_GPIO_P1_37);
  ma_device_uninit(&m_device);
  ma_decoder_uninit(&m_decoder);
 }

 void UpdateDurationLabel(){
  if(!m_durationLabel) return;
  wxDateTime now=wxDateTime::Now();
  wxTimeSpan diff;
  if(m_alarme.m_type==AlarmType::Security)
   diff=now-m_startTime;
  else
   diff=m_endTime-now;
  auto seconds=diff.GetSeconds().GetValue();
  if(m_alarme.m_type!=AlarmType::Security) {
   if(seconds<=5&&seconds>=0)
    m_durationLabel->SetForegroundColour(*wxRED);
   else if(seconds<0){
    StopMusic();
    EndModal(wxID_OK);
   }
  }
  m_durationLabel->SetLabel(std::to_string(seconds));
 }

 void CreateGUIControls(){
  wxBoxSizer* rootSizer=new wxBoxSizer(wxVERTICAL);
  rootSizer->AddSpacer(15);
  m_durationLabel=new wxStaticText(this,wxID_ANY,wxEmptyString);
  UpdateDurationLabel();
  wxFont font=m_durationLabel->GetFont();
  font.SetPointSize(20);
  m_durationLabel->SetFont(font);
  wxString timeElapsedLabel=m_alarme.m_type==AlarmType::Security?wxString::FromUTF8(STR_Elapsedtime):wxString::FromUTF8(STR_Timeleft);
  rootSizer->Add(wxLabel(this,timeElapsedLabel),0,wxALIGN_CENTER_HORIZONTAL);
  rootSizer->Add(m_durationLabel,0,wxALIGN_CENTER_HORIZONTAL);
  if(m_type!=AlarmType::Alarm) {
   int flags=wxALIGN_CENTER_HORIZONTAL;
   if(m_type==AlarmType::Security) flags|=wxLEFT|wxRIGHT;
   rootSizer->Add(wxLabel(this,wxString::FromUTF8(STR_Announcement)),0,flags,80);
   rootSizer->Add(wxLabel(this,m_alarme.m_message),0,wxALIGN_CENTER_HORIZONTAL);
  }
  rootSizer->AddSpacer(20);
  if(m_type!=AlarmType::Security) {
   rootSizer->Add(new wxStaticText(this,wxID_ANY,wxString::FromUTF8(STR_alarmwillcloseauto),wxDefaultPosition,wxDefaultSize,wxTE_CENTRE|wxTE_WORDWRAP),0,wxLEFT|wxRIGHT,10);
   rootSizer->Add(new wxStaticText(this,wxID_ANY,wxString::FromUTF8(STR_endoftheduration),wxDefaultPosition,wxDefaultSize,wxTE_CENTRE|wxTE_WORDWRAP),0,wxALIGN_CENTER_HORIZONTAL);
  }
  rootSizer->AddSpacer(4);
  wxBoxSizer* buttonsSizer=new wxBoxSizer(wxHORIZONTAL);
  buttonsSizer->Add(new wxButton(this,wxID_OK,wxString::FromUTF8(STR_Mute)),1,wxLEFT|wxRIGHT,margem);
  rootSizer->Add(buttonsSizer,0,wxEXPAND,0);
  rootSizer->AddSpacer(4);
  SetSizerAndFit(rootSizer);
 }

 void OnTimer(wxTimerEvent& event){
  UpdateDurationLabel();
  event.Skip();
 }

 void OnOkButton(wxCommandEvent& event){
  StopMusic();
  event.Skip();
 }
};

class EditAlarmeDialog: public wxDialog{
public:
 EditAlarmeDialog(wxWindow* parent,const Alarme& alarme,const AlarmType& type): wxDialog(parent,wxID_ANY,wxString::FromUTF8(STR_Edit)),m_type(type){
  const char* alarmnames[AlarmTypeCount]{STR_Alarms,STR_DailyCommunications,STR_ScheduledCommunications,STR_QuickWarnings,STR_Security};
  this->SetLabel(wxString::FromUTF8(STR_Edit)+L" "+wxString::FromUTF8(alarmnames[type]));
  m_alarme=alarme;
  CreateGUIControls();
 }
private:
 Alarme m_alarme;
 AlarmType m_type;
 TabControls m_tab;
private:
 void CreateGUIControls(){
  m_tab.rootPanel=new wxPanel(this,wxID_ANY);
  MainFrame* mainFrame=(MainFrame*)GetParent();
  wxBoxSizer* addSizer=mainFrame->CreateAddSizer(m_tab,m_type,true);
  if(m_type!=AlarmType::Alarm)
   m_tab.name->SetValue(m_alarme.m_message);
  m_tab.timePicker->SetValue(m_alarme.m_time);
  if(m_tab.timePicker2)
   m_tab.timePicker2->SetValue(m_alarme.m_lastTime);
  if(m_tab.fullTime)
   m_tab.fullTime->SetValue(m_alarme.m_24Hours);
  if(GTK_IS_SWITCH(m_tab.na_nf))
   gtk_switch_set_active(GTK_SWITCH(m_tab.na_nf),m_alarme.m_naNF);
  if(m_tab.port)
   m_tab.port->SetSelection(m_alarme.m_port);
  std::vector<int> selections;
  for(int i=0; i<7; ++i) {
   if(m_alarme.m_days[i])
    selections.push_back(i);
  }
  if(m_type==AlarmType::ScheduledRelease) {
   wxDatePickerCtrl* datePicker=(wxDatePickerCtrl*)m_tab.days;
   datePicker->SetValue(m_alarme.m_time);
  }
  else if(m_type!=AlarmType::Security) {
   wxCheckChoice* checkChoice=(wxCheckChoice*)m_tab.days;
   checkChoice->SetSelections(selections);
  }
  if(m_type==AlarmType::Alarm||m_type==AlarmType::DailyRelease) {
   m_tab.time->SetValue(m_type==AlarmType::Alarm?m_alarme.m_duration:m_alarme.m_norepeat);
  }
  m_tab.music->SetSelection(m_tab.music->FindString(m_alarme.m_music,true));
  m_tab.rootPanel->SetSizer(addSizer);
  wxBoxSizer* rootSizer=new wxBoxSizer(wxVERTICAL);
  rootSizer->Add(m_tab.rootPanel,0,wxEXPAND);
  wxSizer* btszr=CreateButtonSizer(wxOK|wxCANCEL);
  rootSizer->Add(btszr,0,wxALIGN_RIGHT|wxTOP,10);
  rootSizer->AddSpacer(4);
  SetSizerAndFit(rootSizer);
  rootSizer->SetSizeHints(this);
 }
public:
 wxString GetAlarme(Alarme& alarme){
  return ((MainFrame*)GetParent())->GetAlarmFromTab(m_tab,alarme,m_type);
 }
};

wxBEGIN_EVENT_TABLE(MainFrame,wxFrame)
EVT_TIMER(ID_TIMER,MainFrame::OnTimer)
wxEND_EVENT_TABLE();

wxBEGIN_EVENT_TABLE(AlarmeDialog,wxDialog)
EVT_BUTTON(wxID_OK,AlarmeDialog::OnOkButton)
wxEND_EVENT_TABLE();

MainFrame::MainFrame(wxWindow* parent,wxWindowID id): wxFrame(parent,id,wxString::FromUTF8(STR_UtaMusicalAlarm),wxDefaultPosition,wxDefaultSize,
 wxMINIMIZE_BOX|wxMAXIMIZE_BOX|wxRESIZE_BORDER|wxSYSTEM_MENU),m_timer(this,ID_TIMER){
 Bind(wxEVT_CLOSE_WINDOW,[this](wxCloseEvent& event) {
  wxMessageBox(wxString::FromUTF8(STR_dontcanclose),wxString::FromUTF8(STR_Information),wxICON_WARNING);
  event.Veto();
  });

#ifdef __arm__
 if(!bcm2835_init()){
  printf("bcm2835_init failed. Are you running as root?\n");
 }
 bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_31,BCM2835_GPIO_PUD_UP);
 bcm2835_gpio_fsel(RPI_V2_GPIO_P1_31,BCM2835_GPIO_FSEL_INPT);

 bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_33,BCM2835_GPIO_PUD_UP);
 bcm2835_gpio_fsel(RPI_V2_GPIO_P1_33,BCM2835_GPIO_FSEL_INPT);

 bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_35,BCM2835_GPIO_PUD_UP);
 bcm2835_gpio_fsel(RPI_V2_GPIO_P1_35,BCM2835_GPIO_FSEL_INPT);

 bcm2835_gpio_set_pud(RPI_V2_GPIO_P1_37,BCM2835_GPIO_PUD_UP);
 bcm2835_gpio_fsel(RPI_V2_GPIO_P1_37,BCM2835_GPIO_FSEL_OUTP);
 bcm2835_gpio_clr(RPI_V2_GPIO_P1_37);
#endif
 Aws::SDKOptions options;
 Aws::InitAPI(options);
 Aws::Auth::AWSCredentials credentials;
 credentials.SetAWSAccessKeyId(Aws::String("AKIARPVQWU3AMUIY23DV"));
 credentials.SetAWSSecretKey(Aws::String("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"));
 m_pollyClient=std::shared_ptr<Aws::Polly::PollyClient>(new Aws::Polly::PollyClient(credentials,"PollySample::Main"));
 m_textManager=Aws::TextToSpeech::TextToSpeechManager::Create(m_pollyClient);
 m_textManagerVoices=m_textManager->ListAvailableVoices();
 wxArrayString vozesportuga,vozesespanhol,vozesingles;
 wxString secvoice,voicefirst;
 for(const Aws::Http::HeaderValuePair& voice:m_textManagerVoices){
  secvoice=wxString::Format("%s - %s",voice.first,voice.second);
  secvoice=secvoice.AfterLast(wxUniChar(' '));
  voicefirst=wxString(voice.first);
  voicefirst.Replace(L"ê",L"e");
  voicefirst.Replace(L"é",L"e");
  voicefirst.Replace(L"ó",L"o");
  if(secvoice==L"Portuguese")
   vozesportuga.push_back(wxString::Format("%s - %s",voicefirst,voice.second));
  else if(secvoice==L"Spanish")
   vozesespanhol.push_back(wxString::Format("%s - %s",voicefirst,voice.second));
  else if(secvoice==L"English")
   vozesingles.push_back(wxString::Format("%s - %s",voicefirst,voice.second));
 }
 for(const wxString& voice:vozesportuga)
  m_voiceOptions.push_back(voice);
 for(const wxString& voice:vozesespanhol)
  m_voiceOptions.push_back(voice);
 for(const wxString& voice:vozesingles)
  m_voiceOptions.push_back(voice);
 CreateGUIControls();
 m_timer.Start(200);
}

void MainFrame::UpdateDateAndTime(){
 TabControls& controls=m_tabs[m_selectedTab];
 wxDateTime now=wxDateTime::Now();
 controls.clock->SetLabel(wxString::Format(L"%02i/%02i/%02i %02i:%02i:%02i",(int)now.GetDay(),(int)now.GetMonth()+1,now.GetYear(),(int)now.GetHour(),(int)now.GetMinute(),(int)now.GetSecond()));
}

bool MainFrame::CheckAlarme(const Alarme& alarme,const AlarmType& type){
 if(alarme.m_firedToday)
  return false;
 wxDateTime now=wxDateTime::Now();
 if(type==AlarmType::ScheduledRelease) {
  if(alarme.m_time.GetDay()!=now.GetDay()||alarme.m_time.GetMonth()!=now.GetMonth()||alarme.m_time.GetYear()!=now.GetYear())
   return false;
 }
 else {
  int day_week=now.GetWeekDay();
  if(day_week==wxDateTime::WeekDay::Sun)
   day_week=6;
  else
   day_week--;
  if(!alarme.m_days[day_week])
   return false;
 }
 if(alarme.m_time.GetHour()!=now.GetHour())
  return false;
 if(alarme.m_time.GetMinute()!=now.GetMinute())
  return false;
 int alarme_second=alarme.m_time.GetSecond();
 int now_second=now.GetSecond();
 if(alarme_second>=now_second)
  return false;
 return true;
}

size_t MainFrame::CheckAlarmes(const AlarmType& type){
 auto& alarmes=SinaleiroMusical::GetAlarmes(type);
 for(size_t i=0; i<alarmes.size(); ++i){
  if(CheckAlarme(alarmes[i],type))
   return i;
 }
 return std::string::npos;
}

void MainFrame::TestAlarmesFiredToday(const AlarmType& type){
 auto& alarmes=SinaleiroMusical::GetAlarmes(type);
 wxDateTime now=wxDateTime::Now();
 for(size_t i=0; i<alarmes.size(); ++i){
  if(alarmes[i].m_firedToday){
   if(now.GetMinute()>alarmes[i].m_time.GetMinute())
    alarmes[i].m_firedToday=false;
  }
 }
}

void MainFrame::TestAlarmes(){
 for(int i=0; i<3; ++i) {
  AlarmType type=(AlarmType)i;
  size_t pos=CheckAlarmes(type);
  if(pos!=std::string::npos) {
   auto& alarmes=SinaleiroMusical::GetAlarmes(type);
   AlarmeDialog dialog(this,alarmes[pos],type);
   if(dialog.IsOk())
    dialog.ShowModal();
   else
    alarmes[pos].m_firedToday=true;
   break;
  }
 }
 for(int i=0; i<3; ++i) {
  AlarmType type=(AlarmType)i;
  TestAlarmesFiredToday(type);
 }
}

IMPLEMENT_DYNAMIC_CLASS(dlgNomusic,wxDialog)

BEGIN_EVENT_TABLE(dlgNomusic,wxDialog)
EVT_BUTTON(ID_BUTTON,dlgNomusic::OnButtonClick)
END_EVENT_TABLE()

dlgNomusic::dlgNomusic(){
 Init();
}

dlgNomusic::dlgNomusic(wxWindow* parent,wxWindowID id,const wxString& caption,const wxPoint& pos,const wxSize& size,long style){
 Init();
 Create(parent,id,caption,pos,size,style);
}

bool dlgNomusic::Create(wxWindow* parent,wxWindowID id,const wxString& caption,const wxPoint& pos,const wxSize& size,long style){
 SetExtraStyle(wxWS_EX_VALIDATE_RECURSIVELY|wxWS_EX_BLOCK_EVENTS);
 wxDialog::Create(parent,id,caption,pos,size,style);
 CreateControls();
 if(GetSizer())
  GetSizer()->SetSizeHints(this);
 Centre();
 return true;
}

dlgNomusic::~dlgNomusic(){
}

void dlgNomusic::Init(){
 st1=NULL;
 btok=NULL;
}

void dlgNomusic::CreateControls(){
 dlgNomusic* itemDialog1=this;
 itemBoxSizer2=new wxBoxSizer(wxVERTICAL);
 itemDialog1->SetSizer(itemBoxSizer2);
 st1=new wxStaticText(itemDialog1,wxID_STATIC,wxString::FromUTF8(STR_audiofileswarning),wxDefaultPosition,wxDefaultSize,wxALIGN_CENTRE);
 itemBoxSizer2->Add(st1,0,wxALIGN_CENTER_HORIZONTAL|wxALL,10);
}

void dlgNomusic::OnButtonClick(wxCommandEvent& event){
 EndModal(ID_BUTTON);
}

void MainFrame::OnTimer(wxTimerEvent& event){
 //On ARM, for some reason, wxMessageBox crashes the app on OnInit()
 static int shotCount=0;
 if(shotCount==2) {
  std::vector<const wxString*> not_founds_tabs[AlarmTypeCount];
  for(int i=0; i<AlarmTypeCount; ++i) {
   if(i==AlarmType::QuickAlarmType) continue;
   for(const Alarme& alarm:SinaleiroMusical::GetAlarmes((AlarmType)i)) {
    if(i==AlarmType::Alarm) {
     if(alarm.m_music==wxString::FromUTF8(STR_Random)||alarm.m_music==L"Input 4") continue;
    }
    bool found=false;
    for(const Music& music:SinaleiroMusical::GetMusics((AlarmType)i)) {
     if(alarm.m_music==music.name()) {
      found=true;
      break;
     }
    }
    if(!found) {
     not_founds_tabs[i].push_back(&alarm.m_music);
    }
   }
  }
  wxString musics;
  size_t i=0;
  dlgNomusic* dlg=new dlgNomusic(this);
  for(const auto& not_founds:not_founds_tabs) {
   if(not_founds.size()) {
    musics+=wxString::Format(L"----- %s ------\n\n",SinaleiroMusical::AlarmTypeToString((AlarmType)i));
    wxBoxSizer* itemBoxSizer4=new wxBoxSizer(wxHORIZONTAL);
    dlg->itemBoxSizer2->Add(itemBoxSizer4,1,wxGROW|wxALL,margem);
    wxStaticLine* itemStaticLine5=new wxStaticLine(dlg,wxID_STATIC,wxDefaultPosition,wxDefaultSize,wxLI_HORIZONTAL);
    itemBoxSizer4->Add(itemStaticLine5,1,wxGROW|wxALL,10);
    wxStaticText* sinais=new wxStaticText(dlg,wxID_STATIC,SinaleiroMusical::AlarmTypeToString((AlarmType)i),wxDefaultPosition,wxDefaultSize,0);
    itemBoxSizer4->Add(sinais,0,wxALIGN_CENTER_VERTICAL|wxALL,margem);
    wxStaticLine* itemStaticLine7=new wxStaticLine(dlg,wxID_STATIC,wxDefaultPosition,wxDefaultSize,wxLI_HORIZONTAL);
    itemBoxSizer4->Add(itemStaticLine7,1,wxGROW|wxALL,10);
    for(const wxString* msc:not_founds) {
     musics=*msc+wxT(".mp3");
     wxStaticText* file=new wxStaticText(dlg,wxID_STATIC,musics,wxDefaultPosition,wxDefaultSize,0);
     dlg->itemBoxSizer2->Add(file,0,wxALIGN_CENTER_HORIZONTAL|wxLEFT|wxRIGHT|wxBOTTOM,10);
    }
   }
   ++i;
  }
  dlg->btok=new wxButton(dlg,ID_BUTTON,wxT("Ok"),wxDefaultPosition,wxDefaultSize,0);
  dlg->btok->SetName(wxT("btok"));
  dlg->itemBoxSizer2->Add(dlg->btok,0,wxALIGN_CENTER_HORIZONTAL|wxALL,margem);
  if(dlg->GetSizer())
   dlg->GetSizer()->SetSizeHints(dlg);
  dlg->Centre();
  if(musics.size()) {
   wxString format=wxString::FromUTF8(STR_audiofileswarning)+L"\n";
   format+=musics;
   dlg->ShowModal();
  }
 }
 shotCount++;
 auto& alarms=SinaleiroMusical::GetAlarmes(AlarmType::Security);
 for(int i=0; i<alarms.size(); ++i) {
  if(bcm2835_gpio_lev(gpioports[alarms[i].m_port]+gpioStart)==alarms[i].m_naNF) {
   if(!alarms[i].m_firedToday&&alarms[i].m_active) {
    wxDateTime now=wxDateTime::Now();
    int secondsNow=secondOfDay(now);
    if((secondsNow>=secondOfDay(alarms[i].m_time))||alarms[i].m_24Hours) {
     if((secondsNow<secondOfDay(alarms[i].m_lastTime))||alarms[i].m_24Hours) {
      wxString music=alarms[i].m_music;
      if(!music.empty()) {
       AlarmeDialog dialog(this,alarms[i],AlarmType::Security);
       dialog.ShowModal();
      }
     }
    }
   }
  }
  else {
   alarms[i].m_firedToday=false;
  }
 }
 if(m_selectedTab==AlarmType::Security)
  UpdateStatusText();
 TestAlarmes();
 event.Skip();
}

void MainFrame::OnSetTime(wxCommandEvent& event){
 event.Skip();
}

void MainFrame::OnPageChanged(wxBookCtrlEvent& event){
 event.Skip();
 for(auto& pair:m_tabMap) {
  if(pair.second==event.GetSelection())
   m_selectedTab=pair.first;
 }
 if(!m_tabs[m_selectedTab].rootPanel) return;
 if(m_firstTabChange)
  if(m_selectedTab!=QuickAlarmType&&m_selectedTab!=Security)
   m_tabs[m_selectedTab].rootPanel->Layout();
 m_firstTabChange=false;
 UpdateStatusText();
 UpdateGridSize(m_selectedTab);
 if(m_selectedTab==QuickAlarmType)
  UpdateGrid(m_selectedTab);
}

void MainFrame::DoUpdateWindowUI(wxUpdateUIEvent& event){
 if(event.GetSetEnabled())
  Enable(event.GetEnabled());
 if(event.GetSetText()){
  if(event.GetText()!=GetTitle())
   SetTitle(event.GetText());
 }
}

void MainFrame::OnGridSize(wxSizeEvent& event){
 UpdateGridSize(m_selectedTab);
 event.Skip();
}

void MainFrame::OnGridMouse(wxMouseEvent& event){
 event.Skip();
 wxPoint point=event.GetPosition();
 TabControls& tab=m_tabs[m_selectedTab];
 wxGrid* grid=tab.grid;
 AlarmType type=(AlarmType)m_selectedTab;
 point=grid->CalcUnscrolledPosition(point);
 wxGridCellCoords coords=grid->XYToCell(point);
 wxString typeName;
 switch(type){
 case AlarmType::Alarm:
  typeName=wxString::FromUTF8(STR_Alarm);
  break;
 case AlarmType::DailyRelease:
  typeName=wxString::FromUTF8(STR_DailyCommunication);
  break;
 case AlarmType::ScheduledRelease:
  typeName=wxString::FromUTF8(STR_ScheduledCommunication);
  break;
 case QuickAlarmType:
  typeName=wxString::FromUTF8(STR_QuickWarning);
  break;
 case Security:
  typeName=wxString::FromUTF8(STR_Event);
  break;
 default:
  break;
 }
 int actionsCols=0;
 if(type==QuickAlarmType) actionsCols=3;
 else if(type==Security) actionsCols=7;
 else if(type==AlarmType::DailyRelease) actionsCols=5;
 else actionsCols=COL_Actions;
 wxRect rect=grid->CellToRect(coords);
 point.x-=rect.x;
 point.y-=rect.y;
 if(event.GetButton()==wxMOUSE_BTN_LEFT&&coords.GetCol()==actionsCols) {
  //When left down and the tooltip is shown, it don't gets hidden.
  //This makes sure the tooltip is disabled.
  grid->GetGridWindow()->SetToolTip(wxEmptyString);
  if(coords.GetRow()==-1||coords.GetCol()==-1)
   return;
  int button=point.x/28;
  if(point.y<4||point.y > 16+4)
   button=-1;
  std::vector<QuickAlarm>& quickAlarms=SinaleiroMusical::GetQuickAlarms();
  if(type==QuickAlarmType) {
   if(button==0) {
    delete m_speechApi;
    wxString speechPath=SinaleiroMusical::GetMusicPath(quickAlarms[coords.GetRow()].title,AlarmType::QuickAlarmType);
    m_speechApi=new PlayerApi(speechPath);
    if(!m_speechApi->IsOk()||!m_speechApi->Play()) {
     wxMessageBox(wxString::Format(wxString::FromUTF8(STR_Errorplayingfile)+L" %s.",speechPath),wxString::FromUTF8(STR_Error),wxICON_ERROR);
     delete m_speechApi;
     m_speechApi=nullptr;
    }
   }
   else if(button==1) {
    delete m_speechApi;
    m_speechApi=nullptr;
   }
   else if(button==2) {
    QuickAlarmEditDialog dialog(this,&quickAlarms[coords.GetRow()]);
    dialog.ShowModal();
   }
   else if(button==3) {
    wxMessageDialog dialog(this,wxString::Format(wxString::FromUTF8(STR_wanttodeletethis)+L" %s?",typeName),wxString::FromUTF8(STR_Information),wxICON_WARNING|wxYES_NO);
    dialog.SetYesNoLabels(wxMessageDialog::ButtonLabel(wxString::FromUTF8(STR_Yes)),wxMessageDialog::ButtonLabel(wxString::FromUTF8(STR_No)));
    if(dialog.ShowModal()!=wxID_YES) {
     return;
    }
    wxFileName fn;
    fn.SetPath(SinaleiroMusical::GetMusicFolder(QuickAlarmType));
    fn.SetName(quickAlarms[coords.GetRow()].title);
    fn.SetExt(L"mp3");
    wxString file=fn.GetFullPath();
    if(wxFileExists(file))
     wxRemoveFile(file);
    quickAlarms.erase(coords.GetRow()+quickAlarms.begin());
    SinaleiroMusical::Save(AlarmType::QuickAlarmType);
    UpdateGrid(m_selectedTab);
   }
   else if(button==4) {
    QuickAlarmExportDialog dialog;
    if(dialog.ShowModal()==wxID_OK) {
     AlarmType tab=dialog.GetSelectedTab();
     wxString newPath=SinaleiroMusical::GetMusicPath(quickAlarms[coords.GetRow()].title,tab);
     wxString oldPath=SinaleiroMusical::GetMusicPath(quickAlarms[coords.GetRow()].title,AlarmType::QuickAlarmType);
     if(!wxCopyFile(oldPath,newPath)) {
      wxMessageBox(wxString::FromUTF8(STR_Errorcopyingfile),wxString::FromUTF8(STR_Error),wxICON_ERROR,this);
     }
     else {
      wxMessageBox(wxString::FromUTF8(STR_Filecopiedsuccess),wxString::FromUTF8(STR_Information),wxICON_INFORMATION,this);
     }
    }
   }
  }
  else {
   if(button==0) {
    auto& alarmes=SinaleiroMusical::GetAlarmes((AlarmType)m_selectedTab);
    //ToDo: Fix EditAlarmDialog unknown destruction bug when calling wxMessageBox.
    Alarme alarme;
    wxString result;
    EditAlarmeDialog dialog(nullptr,alarmes[coords.GetRow()],m_selectedTab);
    if(dialog.ShowModal()==wxID_CANCEL)
     return;
    result=dialog.GetAlarme(alarme);
    if(!result.empty())
     wxMessageBox(result,wxString::FromUTF8(STR_Error),wxICON_ERROR);
    else {
     auto other_alarmes=alarmes;
     other_alarmes.erase(coords.GetRow()+other_alarmes.begin());
     if(CheckAlarmConflict(alarme,other_alarmes))
      return;
     for(int i=0; i<3; ++i) {
      AlarmType type=(AlarmType)i;
      if(type!=m_selectedTab) {
       if(CheckAlarmConflict(alarme,SinaleiroMusical::GetAlarmes(type))) {
        return;
       }
      }
     }
     alarmes[coords.GetRow()]=alarme;
     SinaleiroMusical::SortAlarmes((AlarmType)m_selectedTab);
     SinaleiroMusical::Save((AlarmType)m_selectedTab);
     UpdateGrid(m_selectedTab);
    }
   }
   else if(button==1) {
    wxMessageDialog dialog(this,wxString::Format(wxString::FromUTF8(STR_wanttodeletethis)+L" %s?",typeName),wxString::FromUTF8(STR_Information),wxICON_WARNING|wxYES_NO);
    dialog.SetYesNoLabels(wxMessageDialog::ButtonLabel(wxString::FromUTF8(STR_Yes)),wxMessageDialog::ButtonLabel(wxString::FromUTF8(STR_No)));
    if(dialog.ShowModal()!=wxID_YES) {
     return;
    }
    auto& alarmes=SinaleiroMusical::GetAlarmes((AlarmType)m_selectedTab);
    alarmes.erase(alarmes.begin()+coords.GetRow());
    SinaleiroMusical::SortAlarmes((AlarmType)m_selectedTab);
    SinaleiroMusical::Save((AlarmType)m_selectedTab);
    UpdateGrid(m_selectedTab);
   }
  }
 }
 else if(event.Moving()) {
  if(coords==m_lastCell)
   return;
  m_lastCell=coords;
  if(coords.GetCol()==COL_File) {
   if(grid->GetCellTextColour(coords.GetRow(),coords.GetCol())==*wxRED)
    grid->GetGridWindow()->SetToolTip(wxString::FromUTF8(STR_Musicnotfound));
   else
    grid->GetGridWindow()->SetToolTip(wxEmptyString);
  }
  else if(coords.GetCol()==actionsCols) {
   //In this case, the grid has sub-buttons, so it have to be updated again.
   m_lastCell={-1,-1};
   int button=point.x/20;
   if(point.y<4||point.y > 16+4)
    button=-1;
   switch(m_selectedTab) {
   case QuickAlarmType:
    switch(button){
    case 0:
     grid->GetGridWindow()->SetToolTip(wxString::FromUTF8(STR_Edit));
     break;
    case 1:
     grid->GetGridWindow()->SetToolTip(wxString::FromUTF8(STR_Delete));
     break;
    case 2:
     grid->GetGridWindow()->SetToolTip(wxString::FromUTF8(STR_Play));
     break;
    case 3:
     grid->GetGridWindow()->SetToolTip(wxString::FromUTF8(STR_StopPlayback));
     break;
    case 4:
     grid->GetGridWindow()->SetToolTip(wxString::FromUTF8(STR_Exporttoothertab));
     break;
    default:
     grid->GetGridWindow()->SetToolTip(wxEmptyString);
     break;
    }
    break;
   default:
    switch(button){
    case 0:
     grid->GetGridWindow()->SetToolTip(wxString::FromUTF8(STR_Edit));
     break;
    case 1:
     grid->GetGridWindow()->SetToolTip(wxString::FromUTF8(STR_Delete));
     break;
    default:
     grid->GetGridWindow()->SetToolTip(wxEmptyString);
     break;
    }
    break;
   }
  }
  else
   grid->GetGridWindow()->SetToolTip(wxEmptyString);
 }
}

int gridSizes[AlarmTypeCount][9]{
 {1,80,70,-1,1,0,0,0,0},
 {-1,1,80,70,-1,1,0,0,0},
 {-1,1,80,-1,1,0,0,0,0},
 {300,-1,1,1,0,0,0,0,0},
 {-1,125,125,64,1,-1,64,1,0}
};

void MainFrame::UpdateGridSize(int n) {
 if(m_tabMap.find((AlarmType)n)==m_tabMap.end()) return;
 const TabControls& tab=m_tabs[n];
 wxGrid* grid=tab.grid;
 AlarmType type=(AlarmType)n;
 int width=tab.grid->GetSize().GetWidth() /* - wxSystemSettings::GetMetric(wxSYS_VSCROLL_X) */;
 int itemCount=0;
 int colIndex=0;
 grid->AutoSizeRows();
 while(gridSizes[n][colIndex]) {
  int size=gridSizes[n][colIndex];
  if(size==0)
   break;
  else if(size==-1)
   itemCount++;
  else if(size==1) {
   grid->AutoSizeColumn(colIndex);
   int colWidth=grid->GetColSize(colIndex);
   width-=colWidth;
  }
  else {
   int colWidth=grid->GetColSize(colIndex);
   colWidth=std::max(colWidth,size);
   width-=colWidth;
   grid->SetColSize(colIndex,colWidth);
  }
  ++colIndex;
 }
 if(width<0)
  return;
 colIndex=0;
 while(gridSizes[n][colIndex]) {
  if(gridSizes[n][colIndex]==-1)
   grid->SetColSize(colIndex,width/itemCount);
  ++colIndex;
 }
}

void MainFrame::OnInsertClick(wxCommandEvent& event){
 event.Skip();
 Alarme alarme;
 wxString result=GetAlarmFromTab(m_tabs[m_selectedTab],alarme,(AlarmType)m_selectedTab);
 if(!result.empty()) {
  wxMessageBox(result,wxString::FromUTF8(STR_Error),wxICON_ERROR);
  return;
 }
 auto& alarmes=SinaleiroMusical::GetAlarmes((AlarmType)m_selectedTab);
 for(int i=0; i<3; ++i) {
  AlarmType type=(AlarmType)i;
  if(CheckAlarmConflict(alarme,SinaleiroMusical::GetAlarmes(type))) {
   return;
  }
 }
 alarmes.push_back(alarme);
 SinaleiroMusical::SortAlarmes((AlarmType)m_selectedTab);
 SinaleiroMusical::Save((AlarmType)m_selectedTab);
 wxDateTime now=wxDateTime::Now();
 now.SetHour(12);
 now.SetMinute(0);
 now.SetSecond(0);
 m_tabs[m_selectedTab].timePicker->SetValue(now);
 if(m_tabs[m_selectedTab].name)
  m_tabs[m_selectedTab].name->SetValue(wxEmptyString);
 if(m_selectedTab==AlarmType::Alarm) {
  m_tabs[m_selectedTab].music->SetSelection(0);
  m_tabs[m_selectedTab].time->SetValue(60);
 }
 else
  m_tabs[m_selectedTab].music->SetSelection(-1);
 wxMessageBox(wxString::FromUTF8(STR_Alarmsuccessadded),L"",wxICON_INFORMATION);
 UpdateGrid(m_selectedTab);
}

void MainFrame::OnListenClick(wxCommandEvent& event){
 event.Skip();
 Alarme alarme;
 wxString result=GetAlarmFromTab(m_tabs[m_selectedTab],alarme,(AlarmType)m_selectedTab);
 if(!result.empty()) {
  wxMessageBox(result,wxString::FromUTF8(STR_Error),wxICON_ERROR);
  return;
 }
 AlarmeDialog dialog(this,alarme,m_selectedTab);
 dialog.ShowModal();
}

void MainFrame::OnStatusSize(wxSizeEvent& event){
 UpdateStatusText();
 event.Skip();
}

void MainFrame::OnMusicDropDown(wxCommandEvent& event){
 TabControls& controls=m_tabs[m_selectedTab];
 auto& musics=SinaleiroMusical::GetMusics(m_selectedTab);
 musics.clear();
 wxString music=SinaleiroMusical::AlarmTypeToString(m_selectedTab);
 SinaleiroMusical::LoadMusics(m_selectedTab);
 controls.music->Clear();
 if(m_selectedTab==AlarmType::Alarm) {
  controls.music->AppendString(wxString::FromUTF8(STR_Random));
  controls.music->AppendString(L"Input 4");
 }
 for(int i=0; i<musics.size(); ++i)
  controls.music->AppendString(musics[i].name());
 SinaleiroMusical::UpdateMusics(m_selectedTab);
 UpdateGrid(m_selectedTab);
 event.Skip();
}

void MainFrame::UpdateStatusText(){
 wxString status;
 if(m_selectedTab==AlarmType::Security) {
  int activeTabs=m_tabs[AlarmType::Security].grid->GetNumberRows();
  wxString input=wxString::FromUTF8(STR_Event);
  wxString format=L"%i %s "+wxString::FromUTF8(STR_active);
  if(activeTabs!=1) {
   input+=L's';
   format+=L's';
  }
  status=wxString::Format(format,activeTabs,input);
 }
 else {
  TabControls& tab=m_tabs[m_selectedTab];
  bool isAviso=m_selectedTab;
  int rows=tab.grid->GetNumberRows();
  wxString format=L"%i %s "+wxString::FromUTF8(STR_registered);
  wxString aviso=wxString::FromUTF8(STR_warning);
  wxString alarme=wxString::FromUTF8(STR_Alarm);
  if(rows!=1){
   format.append('s');
   aviso.append('s');
   alarme.append('s');
  }
  status=wxString::Format(format,rows,isAviso?aviso:alarme);
 }
 m_statusText->SetLabel(status);
 int widths[]{-1,m_statusText->GetSize().GetWidth()+4};
 m_frameStatusBar->SetStatusWidths(2,widths);
 wxRect rect;
 if(m_frameStatusBar->GetFieldRect(1,rect))
  m_statusText->SetPosition(rect.GetPosition());
}

wxBoxSizer* MainFrame::CreateAddSizer(TabControls& tab,AlarmType type,bool edit){
 if(type==AlarmType::DailyRelease||type==AlarmType::ScheduledRelease||type==AlarmType::Security) {
  tab.name=new wxTextCtrl(tab.rootPanel,wxID_ANY);
  wxSize size=tab.name->GetSize();
  //size.x *= 2;
 // tab.name->SetSize(wxSize(100,-1));
 // tab.name->SetMinSize(wxSize(100,-1));
 }
 tab.timePicker=new TimePicker(tab.rootPanel,wxID_ANY,wxDefaultDateTime);
 wxArrayString days;
 days.push_back(wxString::FromUTF8(STR_Mon));
 days.push_back(wxString::FromUTF8(STR_Tue));
 days.push_back(wxString::FromUTF8(STR_Wed));
 days.push_back(wxString::FromUTF8(STR_Thu));
 days.push_back(wxString::FromUTF8(STR_Fri));
 days.push_back(wxString::FromUTF8(STR_Sat));
 days.push_back(wxString::FromUTF8(STR_Sun));
 wxArrayString options;
 auto& musics=SinaleiroMusical::m_musics[type];
 if(type==AlarmType::Alarm) {
  options.push_back(wxString::FromUTF8(STR_Random));
  options.push_back(L"Input 4");
 }
 for(const auto& music:musics)
  options.push_back(music.name());
 if(type==AlarmType::Alarm||type==AlarmType::DailyRelease)
  tab.days=(wxWindow*)new wxCheckChoice(tab.rootPanel,wxID_ANY,wxDefaultPosition,wxDefaultSize,days);
 else if(type!=AlarmType::Security)
  tab.days=(wxWindow*)new wxDatePickerCtrl(tab.rootPanel,wxID_ANY,wxDateTime::Now());
 if(type==AlarmType::Alarm||type==AlarmType::DailyRelease)
  tab.time=new wxSpinCtrl(tab.rootPanel,wxID_ANY,type==AlarmType::Alarm?L"60":L"1",wxDefaultPosition,wxDefaultSize,wxSP_ARROW_KEYS|wxSP_VERTICAL,type==AlarmType::Alarm?5:1,255);
 tab.music=new ComboBox(tab.rootPanel,options);
 if(m_comboSize==wxSize(-1,-1))
  m_comboSize={tab.days->GetMinSize().x*2 /* 2*/,tab.days->GetMinSize().y};
 //tab.music->SetMaxSize( m_comboSize );
 tab.music->SetMinSize(m_comboSize);
 wxBoxSizer* nameSizer=nullptr;
 wxString nameLabel=type==AlarmType::Security?wxString::FromUTF8(STR_Event):wxString::FromUTF8(STR_Announcement);
 if(tab.name) {
  nameSizer=new wxBoxSizer(wxVERTICAL);
  nameSizer->Add(wxLabel(tab.rootPanel,nameLabel) /*, 0, wxALIGN_CENTER_HORIZONTAL */);
  nameSizer->Add(tab.name,1,wxEXPAND);
 }
 wxString hourText=wxString::FromUTF8(STR_Time);
 wxBoxSizer* hourSizer=new wxBoxSizer(wxVERTICAL);
 hourSizer->Add(wxLabel(tab.rootPanel,type==AlarmType::Security?hourText+L" "+wxString::FromUTF8(STR_initial):hourText));
 hourSizer->Add(tab.timePicker);
 wxBoxSizer* finalHourSizer=nullptr;
 if(type==AlarmType::Security) {
  finalHourSizer=new wxBoxSizer(wxVERTICAL);
  tab.timePicker2=new TimePicker(tab.rootPanel,wxID_ANY,wxDefaultDateTime);
  finalHourSizer->Add(wxLabel(tab.rootPanel,hourText+L" "+wxString::FromUTF8(STR_end)));
  finalHourSizer->Add(tab.timePicker2);
 }
 wxBoxSizer* fullTimeSizer=nullptr;
 if(type==AlarmType::Security) {
  fullTimeSizer=new wxBoxSizer(wxVERTICAL);
  tab.fullTime=new wxCheckBox(tab.rootPanel,wxID_ANY,wxEmptyString);
  fullTimeSizer->Add(wxLabel(tab.rootPanel,L"24h"));
  fullTimeSizer->Add(tab.fullTime,0,wxALIGN_CENTER_HORIZONTAL);
 }
 wxBoxSizer* daysSizer=nullptr;
 if(type!=AlarmType::Security) {
  daysSizer=new wxBoxSizer(wxVERTICAL);
  daysSizer->Add(wxLabel(tab.rootPanel,wxString::FromUTF8(STR_Schedule)));
  daysSizer->Add(tab.days);
 }
 wxBoxSizer* timeSizer=nullptr;
 if(type==AlarmType::Alarm||type==AlarmType::DailyRelease) {
  timeSizer=new wxBoxSizer(wxVERTICAL);
  timeSizer->Add(wxLabel(tab.rootPanel,type==AlarmType::Alarm?wxString::FromUTF8(STR_Duration):wxString::FromUTF8(STR_Loops)));
  timeSizer->Add(tab.time);
 }
 wxBoxSizer* fileSizer=new wxBoxSizer(wxVERTICAL);
 fileSizer->Add(wxLabel(tab.rootPanel,wxString::FromUTF8(STR_File)) /* , 0, wxALIGN_CENTER_HORIZONTAL */);
 fileSizer->Add(tab.music);
 wxBoxSizer* na_nfSizer=nullptr;
 if(type==AlarmType::Security) {
  na_nfSizer=new wxBoxSizer(wxVERTICAL);
  tab.na_nf=gtk_switch_new();
  wxNativeWindow* na_nfWin=new wxNativeWindow(tab.rootPanel,wxID_ANY,tab.na_nf);
  na_nfWin->Disown();
  na_nfSizer->Add(wxLabel(tab.rootPanel,L"NA/NF"));
  na_nfSizer->Add(na_nfWin,1);
  if(tab.na_nf!=na_nfWin->GetHandle())
   wxMessageBox(L"Erro. O handle do switch é diferente do handle nativo.",L"Erro",wxICON_ERROR);
 }
 wxBoxSizer* inputSizer=nullptr;
 if(type==AlarmType::Security) {
  inputSizer=new wxBoxSizer(wxVERTICAL);
  wxArrayString options;
  if(edit){
   for(int i=0; i<gpioPorts-1; i++)
   {
    options.push_back(wxString::Format(L"Input %i",i+1));
   }
  }
  else
   options=GetAvailablePorts();
  tab.port=new ComboBox(tab.rootPanel,options);
  tab.port->Bind(wxEVT_COMBOBOX_DROPDOWN,[this,tab,edit](wxCommandEvent& event) {
   wxArrayString options;
   if(edit){
    for(int i=0; i<gpioPorts-1; i++)
     options.push_back(wxString::Format(L"Input %i",i+1));
   }
   else
    options=GetAvailablePorts();
   tab.port->Clear();
   tab.port->Append(options);
   event.Skip();
   });
  tab.port->SetMinSize({90,30});
  inputSizer->Add(wxLabel(tab.rootPanel,L"Input"));
  inputSizer->Add(tab.port);
 }
 wxBoxSizer* addSizer=new wxBoxSizer(wxHORIZONTAL);
 if(nameSizer)
  addSizer->Add(nameSizer,1,wxALIGN_BOTTOM|wxALL,margem);
 addSizer->Add(hourSizer,0,wxALIGN_BOTTOM|wxALL,margem);
 if(finalHourSizer)
  addSizer->Add(finalHourSizer,0,wxALIGN_BOTTOM|wxALL,margem);
 if(daysSizer)
  addSizer->Add(daysSizer,0,wxALIGN_BOTTOM|wxALL,margem);
 if(timeSizer)
  addSizer->Add(timeSizer,0,wxALIGN_BOTTOM|wxALL,margem);
 if(fullTimeSizer)
  addSizer->Add(fullTimeSizer,0,wxALIGN_BOTTOM|wxALL,margem);
 addSizer->Add(fileSizer,0,wxALIGN_BOTTOM|wxALL,margem);
 if(na_nfSizer)
  addSizer->Add(na_nfSizer,0,wxALIGN_BOTTOM|wxALL,margem);
 if(inputSizer)
  addSizer->Add(inputSizer,0,wxALIGN_BOTTOM|wxALL,margem);
 return addSizer;
}

TabControls MainFrame::CreateTab(AlarmType type,const wchar_t* label,bool select){
 if(type==QuickAlarmType)
  return CreateQuickAlarmTab(label,select);
 TabControls tab;
 tab.rootPanel=new wxPanel(m_tabControl);
 m_tabControl->AddPage(tab.rootPanel,label,select);
 wxBoxSizer* addSizer=CreateAddSizer(tab,type,false);
 tab.music->Bind(wxEVT_COMBOBOX_DROPDOWN,&MainFrame::OnMusicDropDown,this);
 tab.insert=new wxButton(tab.rootPanel,wxID_ANY,wxString::FromUTF8(STR_Include));
 tab.insert->Bind(wxEVT_BUTTON,&MainFrame::OnInsertClick,this);
 addSizer->Add(tab.insert,0,wxALIGN_BOTTOM|wxALL,margem);
 if(type!=AlarmType::Security) {
  tab.listen=new wxButton(tab.rootPanel,wxID_ANY,wxString::FromUTF8(STR_Listen));
  addSizer->Add(tab.listen,0,wxALIGN_BOTTOM|wxALL,margem);
  tab.listen->Bind(wxEVT_BUTTON,&MainFrame::OnListenClick,this);
 }
 bool name=false;
 if(type==AlarmType::DailyRelease||type==AlarmType::ScheduledRelease)
  name=true;
 tab.grid=new wxGrid(tab.rootPanel,wxID_ANY);
 tab.grid->Bind(wxEVT_SIZE,&MainFrame::OnGridSize,this);
 tab.grid->GetGridWindow()->Bind(wxEVT_LEFT_DOWN,&MainFrame::OnGridMouse,this);
 tab.grid->GetGridWindow()->Bind(wxEVT_MOTION,&MainFrame::OnGridMouse,this);
 tab.grid->CreateGrid(0,(type==AlarmType::DailyRelease?6:5)+((type==AlarmType::Security)*3));
 tab.grid->Bind(wxEVT_GRID_CELL_CHANGED,[this](wxGridEvent& event) {
  if(m_selectedTab==AlarmType::Security) {
   if(event.GetCol()==6) {
    wxString value=m_tabs[m_selectedTab].grid->GetCellValue(event.GetRow(),event.GetCol());
    if(value.empty())
     value=L"0";
    auto& alarms=SinaleiroMusical::GetAlarmes(AlarmType::Security);
    alarms[event.GetRow()].m_active=std::stoi(value.ToStdWstring());
   }
  }
  event.Skip();
  });
 tab.grid->HideRowLabels();
#ifndef __WXMSW__
 tab.grid->UseNativeColHeader();
#endif
 int colIndex=0;
 if(type==AlarmType::DailyRelease||type==AlarmType::ScheduledRelease)
  tab.grid->SetColLabelValue(colIndex++,wxString::FromUTF8(STR_Announcement));
 else if(type==AlarmType::Security)
  tab.grid->SetColLabelValue(colIndex++,wxString::FromUTF8(STR_Event));
 wxString secondColLabel=wxString::FromUTF8(STR_Time);
 tab.grid->SetColLabelValue(colIndex++,secondColLabel+(type==AlarmType::Security?L" "+wxString::FromUTF8(STR_initial):L""));
 if(type==AlarmType::Security) {
  tab.grid->SetColLabelValue(colIndex++,secondColLabel+L" "+wxString::FromUTF8(STR_end));
  tab.grid->SetColLabelValue(colIndex++,L"NA/NF");
  tab.grid->SetColLabelValue(colIndex++,wxString::FromUTF8(STR_Gate));
 }
 else
  tab.grid->SetColLabelValue(colIndex++,wxString::FromUTF8(STR_Schedule));
 if(type==AlarmType::Alarm)
  tab.grid->SetColLabelValue(colIndex++,wxString::FromUTF8(STR_Duration));
 if(type==AlarmType::DailyRelease)
  tab.grid->SetColLabelValue(colIndex++,wxString::FromUTF8(STR_Loops));

 tab.grid->SetColLabelValue(colIndex++,wxString::FromUTF8(STR_File));
 if(type==AlarmType::Security)
  tab.grid->SetColLabelValue(colIndex++,wxString::FromUTF8(STR_active));
 tab.grid->SetColLabelValue(colIndex++,wxString::FromUTF8(STR_Actions));
 tab.grid->UseNativeColHeader();
 tab.grid->SetColLabelAlignment(wxALIGN_LEFT,wxALIGN_CENTER);
 wxBoxSizer* addLogoSizer=new wxBoxSizer(wxHORIZONTAL);
 addLogoSizer->Add(addSizer,1,wxEXPAND,5);
 addLogoSizer->Add(new UtaStaticBitmap(tab.rootPanel),0,wxALL,margem);
 wxBoxSizer* rootPanelSizer=new wxBoxSizer(wxVERTICAL);
 rootPanelSizer->Add(addLogoSizer,0,wxEXPAND);
 //rootPanelSizer->AddSpacer(16);
 rootPanelSizer->Add(new wxStaticLine(tab.rootPanel,wxID_ANY),0,wxEXPAND,0);
 rootPanelSizer->AddSpacer(4);
 rootPanelSizer->Add(tab.grid,1,wxEXPAND);
 tab.rootPanel->SetSizer(rootPanelSizer);
 return tab;
}

TabControls MainFrame::CreateQuickAlarmTab(const wchar_t* label,bool select) {
 TabControls tab;
 tab.rootPanel=new wxPanel(m_tabControl);
 m_tabControl->AddPage(tab.rootPanel,label,select);
 wxBoxSizer* quickAlarmSizer=new wxBoxSizer(wxVERTICAL);
 quickAlarmSizer->Add(new wxStaticText(tab.rootPanel,wxID_ANY,wxString::FromUTF8(STR_QuickWarning)),0,wxTOP|wxRIGHT|wxLEFT,15);
 quickAlarmSizer->AddSpacer(4);
 m_textTitle=new wxTextCtrl(tab.rootPanel,wxID_ANY);
 quickAlarmSizer->Add(m_textTitle,0,wxRIGHT|wxLEFT|wxEXPAND,15);
 quickAlarmSizer->AddSpacer(4);
 m_textSpeech=new wxTextCtrl(tab.rootPanel,wxID_ANY,wxEmptyString,wxDefaultPosition,wxDefaultSize,wxTE_MULTILINE);
 m_textSpeech->SetMaxSize({-1,200});
 m_textSpeech->SetMaxLength(500);
 m_textSpeech->Bind(wxEVT_TEXT,[this](wxCommandEvent& event){
  size_t used=m_textSpeech->GetValue().size();
  if(used>500){
   m_textSpeech->ChangeValue(m_textSpeech->GetValue().SubString(0,499));
   m_textSpeech->SetInsertionPointEnd();
  }
  else
   m_speechButton->Enable(true);
  event.Skip();
  });
 quickAlarmSizer->Add(m_textSpeech,1,wxEXPAND|wxRIGHT|wxLEFT,15);
 quickAlarmSizer->AddSpacer(4);
 m_speechButton=new wxButton(tab.rootPanel,wxID_ANY,wxString::FromUTF8(STR_Convert));
 m_speechButton->Bind(wxEVT_BUTTON,[this](wxCommandEvent& event) {
  wxString title=m_textTitle->GetValue();
  if(title.empty()){
   wxMessageBox(wxString::FromUTF8(STR_Thetitlecannotbeempty),wxString::FromUTF8(STR_Error),wxICON_ERROR);
   return;
  }
  wxString path=wxFileName(SinaleiroMusical::GetMusicFolder(QuickAlarmType),title,L"mp3").GetFullPath();
  if(wxFileExists(path)){
   wxMessageBox(wxString::FromUTF8(STR_Warningtitleexists),wxString::FromUTF8(STR_Error),wxICON_ERROR);
   return;
  }
  wxString speech=m_textSpeech->GetValue().ToStdString();
  Aws::Polly::Model::SynthesizeSpeechRequest speechRequest;
  wxString voice_with_language=m_voiceCombo->GetValue();
  wxString voice=voice_with_language.substr(0,voice_with_language.find('-')-1);
  wxString voice_path=SinaleiroMusical::GetDataPath("voice.txt");
  wxFile file;
  if(file.Create(voice_path,true))
   if(file.Open(voice_path,wxFile::write))
    file.Write(voice_with_language);
  speechRequest.SetVoiceId(Aws::Polly::Model::VoiceIdMapper::GetVoiceIdForName(voice.ToStdString()));
  speechRequest.SetOutputFormat(Aws::Polly::Model::OutputFormat::mp3);
  speechRequest.SetText(speech);
  Aws::Polly::Model::SynthesizeSpeechOutcome result=m_pollyClient->SynthesizeSpeech(speechRequest);
  if(result.IsSuccess()) {
   Aws::IOStream& audio_stream=result.GetResult().GetAudioStream();
   Aws::OFStream   voice_file;
   Aws::IOStream* audio_stream1=&result.GetResult().GetAudioStream();
   // Get the bytes from the stream and save them into a file
   voice_file.open("./polly.mp3",std::ios::out|std::ios::binary);
   voice_file.write(GetStreamBytes(audio_stream1),GetStreamSize(audio_stream1));
   voice_file.close();
   audio_stream.seekg(0,std::ios::beg);
   size_t begin=audio_stream.tellg();
   audio_stream.seekg(0,std::ios::end);
   size_t end=audio_stream.tellg();
   audio_stream.seekg(0,std::ios::beg);
   size_t size=end-begin;
   uint8_t* bytes=new uint8_t[size];
   audio_stream.read((char*)bytes,size);
   std::vector<uint8_t> pcmFrames;
   PlayerApi::ConvertToPCMFrames(&incbin_dingo_start,incbin_dingo_size,pcmFrames);
   float* data=(float*)pcmFrames.data();
   float* dataEnd=(float*)(pcmFrames.data()+pcmFrames.size());
   while(data<dataEnd) {
    *data=*data*0.5f;
    data++;
   }
   PlayerApi::ConvertToPCMFrames(bytes,size,pcmFrames);
   ma_encoder_config encoderConfig=ma_encoder_config_init(ma_resource_format_wav,ma_format_f32,2,44100);
   ma_encoder encoder;
   ma_encoder_init_file(path.ToStdString().c_str(),&encoderConfig,&encoder);
   int totalFramesWritten=0;
   int bytesWritten=0;
   int outputChannels=2;
   int sampleSize=4;
   int pcmFrameCount=pcmFrames.size()/sampleSize/outputChannels;
   while(totalFramesWritten<pcmFrameCount) {
    int framesWritten=ma_encoder_write_pcm_frames(&encoder,pcmFrames.data()+bytesWritten,pcmFrameCount);
    bytesWritten+=outputChannels*sampleSize*framesWritten;
    totalFramesWritten+=framesWritten;
   }
   ma_encoder_uninit(&encoder);
   SinaleiroMusical::AppendQuickAlarm(title,speech,voice_with_language,wxEmptyString);
   UpdateGrid(QuickAlarmType);
   sBuffer.clear();
   m_textSpeech->SetValue(L"");
   m_textTitle->SetValue(L"");
   wxMessageBox(wxString::FromUTF8(STR_Filesuccessfullyconverted),wxString::FromUTF8(STR_Information),wxICON_INFORMATION);
  }
  else
   wxMessageBox(wxString::FromUTF8(STR_Errorconvertingfile),wxString::FromUTF8(STR_Error),wxICON_EXCLAMATION);
  event.Skip();
  });
 wxString path=SinaleiroMusical::GetDataPath("voice.txt");
 wxFile file;
 wxString voice;
 if(wxFileExists(path))
  if(file.Open(path,wxFile::read))
   file.ReadAll(&voice);
 m_voiceCombo=new wxOwnerDrawnComboBox(tab.rootPanel,wxID_ANY,m_voiceOptions[0],wxDefaultPosition,wxDefaultSize,m_voiceOptions,wxCB_READONLY);
 if(!voice.empty()){
  int index=m_voiceCombo->FindString(voice,true);
  if(index!=-1)
   m_voiceCombo->SetSelection(index);
 }
 wxBoxSizer* convertSizer=new wxBoxSizer(wxHORIZONTAL);
 convertSizer->AddStretchSpacer(1);
 convertSizer->Add(m_voiceCombo,0,wxRIGHT,15);
 convertSizer->Add(m_speechButton,0,wxRIGHT,15);
 quickAlarmSizer->Add(convertSizer,1,wxEXPAND);
 tab.grid=new wxGrid(tab.rootPanel,wxID_ANY);
 tab.grid->Bind(wxEVT_SIZE,&MainFrame::OnGridSize,this);
 tab.grid->GetGridWindow()->Bind(wxEVT_LEFT_DOWN,&MainFrame::OnGridMouse,this);
 tab.grid->GetGridWindow()->Bind(wxEVT_MOTION,&MainFrame::OnGridMouse,this);
 tab.grid->CreateGrid(0,4);
 tab.grid->HideRowLabels();
 tab.grid->SetColLabelAlignment(wxALIGN_LEFT,wxALIGN_CENTER);
#ifndef __WXMSW__
 tab.grid->UseNativeColHeader();
#endif        
 tab.grid->SetColLabelValue(0,wxString::FromUTF8(STR_Title));
 tab.grid->SetColLabelValue(1,wxString::FromUTF8(STR_Message));
 tab.grid->SetColLabelValue(2,wxString::FromUTF8(STR_Voice));
 tab.grid->SetColLabelValue(3,wxString::FromUTF8(STR_Actions));
 wxBoxSizer* horizontalSizer=new wxBoxSizer(wxHORIZONTAL);
 horizontalSizer->Add(quickAlarmSizer,1,wxEXPAND,0);
 horizontalSizer->Add(new UtaStaticBitmap(tab.rootPanel),0,wxALL,margem);
 wxBoxSizer* rootSizer=new wxBoxSizer(wxVERTICAL);
 rootSizer->Add(horizontalSizer,1,wxEXPAND,0);
 rootSizer->Add(tab.grid,1,wxEXPAND);
 tab.rootPanel->SetSizer(rootSizer);
 return tab;
}

wxString MainFrame::GetAlarmFromTab(TabControls& tab,Alarme& alarme,const AlarmType& type){
 alarme.m_type=type;
 if(tab.name) {
  wxString message=tab.name->GetValue();
  alarme.m_message=message;
 }
 if(tab.music->GetSelection()==-1)
  return wxString::FromUTF8(STR_Pleasechooseasong);
 wxDateTime time=tab.timePicker->GetValue();
 alarme.m_time=wxDateTime(time.GetHour(),time.GetMinute(),time.GetSecond());
 if(type==AlarmType::Security) {
  time=tab.timePicker2->GetValue();
  alarme.m_lastTime=wxDateTime(time.GetHour(),time.GetMinute(),time.GetSecond());
  alarme.m_24Hours=tab.fullTime->GetValue();
 }
 if(type==AlarmType::ScheduledRelease) {
  wxDatePickerCtrl* datePicker=(wxDatePickerCtrl*)tab.days;
  wxDateTime date=datePicker->GetValue();
  alarme.m_time.SetDay(date.GetDay());
  alarme.m_time.SetMonth(date.GetMonth());
  alarme.m_time.SetYear(date.GetYear());
 }
 else if(type!=AlarmType::Security) {
  auto selections=((wxCheckChoice*)tab.days)->GetSelections();
  if(selections.empty()) {
   return wxString::FromUTF8(STR_Pleasechooseweekday);
  }
  std::bitset<8> days(0);
  for(const auto& selection:selections)
   days[selection]=true;
  alarme.m_days=days;
 }
 alarme.m_music=tab.music->GetString(tab.music->GetSelection());
 if(type==AlarmType::Alarm){
  alarme.m_duration=tab.time->GetValue();
  alarme.m_norepeat=1;
 }
 else {
  for(const Music& music:SinaleiroMusical::m_musics[type])
   if(alarme.m_music==music.name()){
    alarme.m_duration=music.duration();
    alarme.m_norepeat=music.norepeat();
   }
  if(type==AlarmType::DailyRelease)
   alarme.m_norepeat=tab.time->GetValue();
 }
 if(type==AlarmType::Security) {
  wxString selectedPort=tab.port->GetStringSelection();
  if(selectedPort.empty())
   return wxString::FromUTF8(STR_Pleasechooseaninput);
  selectedPort.erase(0,6);
  int port=std::stoi(selectedPort.ToStdWstring());
  alarme.m_port=port-1;
  tab.port->SetSelection(-1);
  alarme.m_naNF=gtk_switch_get_state(GTK_SWITCH(tab.na_nf));
 }
 return wxEmptyString;
}

bool MainFrame::CheckAlarmConflict(const Alarme& alarme,const std::vector<Alarme>& alarmes){
 for(const Alarme& other:alarmes){
  if(alarme.conflict(other)) {
   wxMessageBox(wxString::FromUTF8(STR_AlarmConflictWarning),wxString::FromUTF8(STR_Error),wxICON_ERROR);
   return true;
  }
 }
 return false;
}

void MainFrame::UpdateGrid(const AlarmType& type){
 if(m_tabMap.find(type)==m_tabMap.end())
  return;
 const TabControls& tab=m_tabs[type];
 if(type==QuickAlarmType) {
  SinaleiroMusical::LoadQuickAlarms();
  auto quickAlarms=SinaleiroMusical::GetQuickAlarms();
  if(tab.grid->GetNumberRows()>quickAlarms.size())
   tab.grid->DeleteRows(0,tab.grid->GetNumberRows()-quickAlarms.size());
  else if(tab.grid->GetNumberRows()<quickAlarms.size())
   tab.grid->AppendRows(quickAlarms.size()-tab.grid->GetNumberRows());
  for(int i=0; i<quickAlarms.size(); ++i) {
   tab.grid->SetCellValue(i,0,quickAlarms[i].title);
   tab.grid->SetCellValue(i,1,quickAlarms[i].description);
   tab.grid->SetCellFitMode(i,1,wxGridFitMode::Ellipsize());
   tab.grid->SetCellValue(i,2,quickAlarms[i].voice);
   tab.grid->SetCellRenderer(i,3,new wxGridCellActionsRenderer(true));
   for(int col=0; col<tab.grid->GetNumberCols(); ++col)
    tab.grid->SetReadOnly(i,col);
  }
  UpdateGridSize(AlarmType::QuickAlarmType);
  return;
 }
 auto& alarmes=SinaleiroMusical::GetAlarmes(type);
 if(tab.grid->GetNumberRows()>alarmes.size())
  tab.grid->DeleteRows(0,tab.grid->GetNumberRows()-alarmes.size());
 else if(tab.grid->GetNumberRows()<alarmes.size())
  tab.grid->AppendRows(alarmes.size()-tab.grid->GetNumberRows());
 for(int i=0; i<alarmes.size(); ++i) {
  wxString days;
  if(type==AlarmType::ScheduledRelease)
   days=wxString::Format("%i/%i/%i",alarmes[i].m_time.GetDay(),(int)alarmes[i].m_time.GetMonth()+1,alarmes[i].m_time.GetYear());
  else if(type!=AlarmType::Security) {
   wxString days_week[7]={wxString::FromUTF8(STR_Mon),wxString::FromUTF8(STR_Tue),wxString::FromUTF8(STR_Wed),wxString::FromUTF8(STR_Thu),wxString::FromUTF8(STR_Fri),wxString::FromUTF8(STR_Sat),wxString::FromUTF8(STR_Sun)};
   std::bitset<8> bitset=alarmes[i].m_days;
   for(int i=0; i<7; ++i) {
    if(bitset[i]) {
     days+=days_week[i];
     bitset[i]=false;
     if(bitset.any())
      days+=", ";
     else
      break;
    }
   }
  }
  int colIndex=0;
  if(type!=AlarmType::Alarm)
   tab.grid->SetCellValue(i,colIndex++,alarmes[i].m_message);
  if(type==AlarmType::Security&&alarmes[i].m_24Hours) {
   tab.grid->SetCellValue(i,colIndex,L"----------");
   tab.grid->SetCellAlignment(i,colIndex++,wxALIGN_CENTER,wxALIGN_CENTER);
   tab.grid->SetCellValue(i,colIndex,L"----------");
   tab.grid->SetCellAlignment(i,colIndex++,wxALIGN_CENTER,wxALIGN_CENTER);
  }
  else {
   tab.grid->SetCellValue(i,colIndex++,wxString::Format(L"%02i:%02i:%02i",(int)alarmes[i].m_time.GetHour(),alarmes[i].m_time.GetMinute(),alarmes[i].m_time.GetSecond()));
   if(type==AlarmType::Security) {
    tab.grid->SetCellValue(i,colIndex++,wxString::Format(L"%02i:%02i:%02i",(int)alarmes[i].m_lastTime.GetHour(),alarmes[i].m_lastTime.GetMinute(),alarmes[i].m_lastTime.GetSecond()));
   }
  }
  const wchar_t* na_nf[2]{L"NA",L"NF"};
  if(type==AlarmType::Security)
   tab.grid->SetCellValue(i,colIndex++,na_nf[alarmes[i].m_naNF]);
  else
   tab.grid->SetCellValue(i,colIndex++,days);
  if(type==AlarmType::Alarm)
   tab.grid->SetCellValue(i,colIndex++,std::to_string((uint)alarmes[i].m_duration));
  else if(type==AlarmType::DailyRelease)
   tab.grid->SetCellValue(i,colIndex++,std::to_string((uint)alarmes[i].m_norepeat));
  else if(type==AlarmType::Security)
   tab.grid->SetCellValue(i,colIndex++,wxString::Format(L"Input %zu",1+alarmes[i].m_port));
  tab.grid->SetCellValue(i,colIndex,alarmes[i].m_music);
  if(AlarmeDialog::CheckMusic(alarmes[i].m_music,type))
   tab.grid->SetCellTextColour(i,colIndex++,*wxBLACK);
  else
   tab.grid->SetCellTextColour(i,colIndex++,*wxRED);
  if(type==AlarmType::Security) {
   //tab.grid->SetCellAlignment(i, colIndex, wxALIGN_CENTER, wxALIGN_CENTER);
   tab.grid->SetCellRenderer(i,colIndex,new wxGridCellBoolRenderer());
   tab.grid->SetCellEditor(i,colIndex,new wxGridCellBoolEditor());
   tab.grid->SetCellValue(i,colIndex++,std::to_string(alarmes[i].m_active));
  }
  tab.grid->SetCellRenderer(i,colIndex++,new wxGridCellActionsRenderer());
  for(int col=0; col<tab.grid->GetNumberCols(); ++col) {
   if((type==AlarmType::Security&&col!=6)||type!=AlarmType::Security)
    tab.grid->SetReadOnly(i,col);
   tab.grid->SetCellFitMode(i,col,wxGridFitMode::Ellipsize());
  }
 }
 if(type==AlarmType::Alarm)
  tab.grid->AutoSizeColumn(1);
 else
  tab.grid->AutoSizeColumn(2);
 UpdateGridSize(type);
 UpdateStatusText();
 tab.grid->Refresh();
}

void MainFrame::CreateGUIControls(){
 if(!m_frameStatusBar) {
  CreateStatusBar(2);
  m_frameStatusBar->Bind(wxEVT_SIZE,&MainFrame::OnStatusSize,this);
  m_statusText=new wxStaticText(m_frameStatusBar,wxID_ANY,wxEmptyString);
  SetStatusText(wxString::FromUTF8(STR_UtaAlwayslookoutsigns),0);
 }
 memset(m_tabs,0,sizeof(TabControls)*AlarmTypeCount);
 if(m_tabControl) {
  if(!m_tabControl->Destroy()) {
   wxMessageBox(wxString::FromUTF8(STR_Failedrecreatetabs),wxString::FromUTF8(STR_Error),wxICON_ERROR,this);
   return;
  }
 }
 m_tabControl=new wxNotebook(this,wxID_ANY,wxDefaultPosition,wxDefaultSize,wxBK_DEFAULT);
 m_tabMap.clear();
 wxString tabNames[AlarmTypeCount]{wxString::FromUTF8(STR_Alarms),wxString::FromUTF8(STR_DailyCommunications),wxString::FromUTF8(STR_ScheduledCommunications),wxString::FromUTF8(STR_QuickWarnings),wxString::FromUTF8(STR_Security)};
 bool foundTab=false;
 int index=0;
 auto tabs=SinaleiroMusical::GetAvailableTabs();
 for(int i=0; i<AlarmTypeCount; ++i) {
  if(tabs[i]) {
   AlarmType type=(AlarmType)i;
   m_tabs[i]=CreateTab(type,tabNames[i],false);
   m_tabMap.insert({type,index});
   if(!foundTab) {
    m_selectedTab=type;
    m_tabControl->SetSelection(index);
   }
   foundTab=true;
   UpdateGrid(type);
   ++index;
  }
 }
 if(!foundTab) {
  wxMessageBox(wxString::FromUTF8(STR_Atleastonetab),wxString::FromUTF8(STR_Error),wxICON_ERROR);
  wxTheApp->ExitMainLoop();
  wxRemoveFile(SinaleiroMusical::GetDataPath("tabs.bin"));
  exit(EXIT_FAILURE);
 }
 m_tabControl->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED,&MainFrame::OnPageChanged,this);
 UpdateStatusText();
 Layout();
}

wxArrayString MainFrame::GetAvailablePorts() {
 wxArrayString availablePorts;
 for(int i=0; i<gpioPorts-1; ++i) {
  bool portAvailable=true;
  for(Alarme& event:SinaleiroMusical::GetAlarmes(AlarmType::Security)) {
   if(event.m_port==i) {
    portAvailable=false;
    break;
   }
  }
  if(portAvailable)
   availablePorts.push_back(wxString::Format(L"Input %i",i+1));
 }
 return availablePorts;
}

/*
 * MMMALHub.cpp
 *
 *  Created on: Jul 12, 2013
 *      Author: Ji-Yu
 */

#include "MMMALHub.h"
#include "MMMALShutter.h"
#include "MMMALLightPath.h"
#include "MMMALNosepiece.h"
#include "MMMALFocus.h"
#include "MMMALMirrorUnit.h"
#include "MMDeviceConstants.h"
#include "MalMicroscope.h"
#include "MMMALAutofocus.h"
#include "MMMALLamp.h"

namespace MMMAL {

   const char * const MMMALHub::DeviceName_ = "IXHub";
   const char * const MMMALHub::Description_ = "Olympus IX microscope hub";
   const char * const MMMALHub::Keyword_Frame_Version_ = "Frame Version";
   const char * const MMMALHub::Keyword_Focus_Version_ = "Focus Version";
   const char * const MMMALHub::Keyword_UCB_Version_ = "UCB Version";
   const char * const MMMALHub::Keyword_Config_Path = "INI File";
   const char * const MMMALHub::Keyword_ZDC_Config_Path = "ZDC INI File";

   MMMALHub::MMMALHub():
               initialized_(false),
               pMAL_(NULL),
               cubeBusy_(false),
               nosepieceBusy_(false),
               prismBusy_(false),
               bottomPortBusy_(false),
               lampBusy_(false),
               focusBusy_(false),
               afBusy_(false),
               afStatus_(MAL_MS_AF_OFF),
               autofocusDev_(NULL),
               focusDev_(NULL),
               lightPathDev_(NULL),
               mirrorUnitDev_(NULL),
               nosepieceDev_(NULL),
               epiShutterDev_(NULL),
               diaShutterDev_(NULL),
               lampDev_(NULL)
   {
      InitializeDefaultErrorMessages();

      memset(configuration_, 0, sizeof(configuration_));
      memset(escapePos_, 0, sizeof(escapePos_));

      CreateProperty(MM::g_Keyword_Name, DeviceName_, MM::String, true);
      CreateProperty(MM::g_Keyword_Description, Description_, MM::String, true);

      char path[MM::MaxStrLength];
      if ( GetConfigFilePath(path) != DEVICE_OK) 
      {
         strcpy(path, "C:\\Program Files\\IX2-BSW");
      }

      CreateProperty(Keyword_ZDC_Config_Path, path, MM::String, false);
      strcat(path, "\\List");
      CreateProperty(Keyword_Config_Path, path, MM::String, false);
   }

   MMMALHub::~MMMALHub()
   {
      Shutdown();
      ClearInstalledDevices();
   }

   int MMMALHub::TranslateMalError(MALRESULT malResult) const
   {
      int ret;
      switch(malResult)
      {
      case MAL_OK:
         ret = DEVICE_OK;
         break;
      case MALERR_NEEDED_OTHER_PARAM_BEFORE:
      case MALERR_INSUFFICIENT_PARAMETERS:
         ret = DEVICE_INVALID_INPUT_PARAM;
         break;
      case MALERR_NOT_ENOUGH_RESOURCE:
         ret = DEVICE_OUT_OF_MEMORY;
         break;
      case MALERR_NOTSUPPORTED:
      case MALERR_NOTACCEPTEDNOW:
         ret = DEVICE_NOT_SUPPORTED;
         break;
      case MALERR_TIMEOUT:
      case MALERR_TIMEOUT_SEND:
      case MALERR_TIMEOUT_RECEIVE:
         ret = DEVICE_SERIAL_TIMEOUT;
         break;
      case MALERR_SEND:
      case MALERR_RECEIVE:
      case MALERR_IOCTL:
         ret = DEVICE_SERIAL_COMMAND_FAILED;
         break;
      default:
         ret = DEVICE_ERR;
         break;
      }
      return ret;
   }

   void MMMALHub::LogError(std::string src, long errorcode, void* pv) const
   {
      std::stringstream ss;

      if (pv != NULL) // detail error strin in pv
      {
         ss << src << " - error: " << (char *) pv ;
      }
      else
      {
         ss << src << " - error: " << errorcode;
      }

      LogMessage(ss.str(), false);
   }

   void MMMALHub::GetName(char * pszName) const
   {
      GetProperty(MM::g_Keyword_Name, pszName);
   }

   int MMMALHub::Shutdown()
   {
      initialized_ = false;

      if (pMAL_ != NULL)  
      {
         ExitMALObject();
         Sleep(500); //FIXME implement a proper locking 
         ReleaseMALObject();
      }

      return DEVICE_OK;
   }

   int MMMALHub::GetConfigFilePath(char *path, size_t len)
   {
      HKEY hkey;
      DWORD dwType = REG_SZ;
      DWORD valueSize = len;
      LPCTSTR subkey = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IX2.exe";

      if (RegOpenKey(HKEY_LOCAL_MACHINE, subkey, &hkey) != ERROR_SUCCESS)
      {
         return DEVICE_ERR;
      }

      if ( RegQueryValueEx(hkey, TEXT("Path"), NULL, &dwType, (LPBYTE)path, &valueSize) != ERROR_SUCCESS)
      {
         return DEVICE_ERR;
      }

      return DEVICE_OK;
   }

   void MMMALHub::SetMirrorUnitDevice(MMMALMirrorUnit* dev)
   {
      mirrorUnitDev_ = dev;
   }

   void MMMALHub::SetNosepieceDevice(MMMALNosepiece* dev)
   {
      nosepieceDev_ = dev;
   }

   void MMMALHub::SetLightPathDevice(MMMALLightPath* dev)
   {
      lightPathDev_ = dev;
   }

   void MMMALHub::SetLampDevice(MMMALLamp* dev)
   {
      lampDev_ = dev;
   }

   void MMMALHub::SetShutterDevice(MMMALShutter* dev)
   {
      char buf[MM::MaxStrLength];
      dev->GetName(buf);
      if (strncmp(buf, MMMALShutter::DeviceNameDIA_, MM::MaxStrLength) == 0)
      {
         diaShutterDev_ = dev;
      }
      else 
      {
         epiShutterDev_ = dev;
      }
   }

   void MMMALHub::SetFocusDevice(MMMALFocus* dev)
   {
      focusDev_ = dev;
   }

   void MMMALHub::SetAutofocusDevice(MMMALAutofocus* dev)
   {
      autofocusDev_ = dev;
   }

   int MMMALHub::CreateMALObject()
   {
      long arg[1];
      arg[0] = ID_RS232C;

      MALRESULT malResult = malCreate(&pMAL_, ID_IX, 1L, arg);

      return TranslateMalError(malResult);
   }

   int MMMALHub::InitMALObject()
   {
      int result;
      MALRESULT malResult;

      if (pMAL_ == NULL)
      {
         return DEVICE_NOT_CONNECTED;
      }

      LONG_PTR arg[13];
      memset(arg, 0, sizeof(arg));

      arg[C_TX_BUFFER_SIZE] = 512;
      arg[C_RX_BUFFER_SIZE] = 512;
      arg[L_TX_TIMEOUT] = 5000000; // 5 sec
      arg[L_RX_TIMEOUT] = 10000000; // 10 sec
      arg[DEVICE_ADDRESS] = 1;
      arg[N_EOS_TYPE] = EOS_CRLF;
      arg[RS232C_BAUDRATE] = 19200;
      arg[RS232C_F_PARITY] = 1;
      arg[RS232C_PARITY] = EVENPARITY;
      arg[RS232C_STOP_BITS] = TWOSTOPBITS;
      arg[RS232C_BYTE_SIZE] = 8;
      arg[RS232C_FLOW] = FLOW_RTSCTS;
      arg[12] = 0;

      malResult = malInitialize(pMAL_, 13, arg);
      if (malResult != MAL_OK)
      {
         return DEVICE_NOT_CONNECTED;
      }

      char configPath[MM::MaxStrLength];
      GetProperty(Keyword_Config_Path, configPath);

      malResult = malReadConfigurationData(pMAL_, MAL_MS_UNIT_MAIN, 0, configPath);
      if (malResult != MAL_OK)
      {
         return TranslateMalError(malResult);
      }

      malResult = malSetRemoteStatus(pMAL_, 0, MAL_MS_ALLUNITS, MAL_MS_REMOTE);
      if (malResult != MAL_OK)
      {
         return TranslateMalError(malResult);
      }

      result = DetectDeviceStatus();
      if (result != DEVICE_OK)
      {
         return result;
      }

      GetProperty(Keyword_ZDC_Config_Path, configPath);
      if (HasZDC())
      {
         malResult = malReadConfigurationData(pMAL_, MAL_MS_UNIT_ZDC, 0, configPath);
         if (malResult != MAL_OK)
         {
            return TranslateMalError(malResult);
         }

      } else if (HasZDC2())
      {
         malResult = malReadConfigurationData(pMAL_, MAL_MS_UNIT_ZDC2, 0, configPath);
         if (malResult != MAL_OK)
         {
            return TranslateMalError(malResult);
         }
      }

      return RegisterButtonNotices(MAL_ON);
   }


   int MMMALHub::RegisterButtonNotices(MAL_STATUS status)
   {
      static long buttonList[] = {0, 1, 2, 3, 4, 9, 11, 24, 25};

      for (long i = 0; i < sizeof(buttonList)/sizeof(long) ; i ++)
      {
         MALRESULT malResult = malSetButtonNoticeStatus(pMAL_, buttonList[i] , status);
         if (malResult != MAL_OK)
         {
            return TranslateMalError(malResult);
         }
      }

      return DEVICE_OK;

   }

   int MMMALHub::ExitMALObject()
   {
      if (pMAL_ == NULL)
      {
         return DEVICE_OK;
      }

      MALRESULT malResult;

      RegisterButtonNotices(MAL_OFF);

      malResult = malSetFocusPolling(pMAL_, MAL_OFF, 0);
      if (malResult != MAL_OK)
      {
         return TranslateMalError(malResult);
      }

      malResult = malSetRemoteStatus(pMAL_, 0, MAL_MS_ALLUNITS, MAL_MS_LOCAL);

      return TranslateMalError(malResult);
   }

   int MMMALHub::ReleaseMALObject()
   {
      if (pMAL_ == NULL)
      {
         return DEVICE_OK;
      }

      MALRESULT malResult;
      if ((malResult = malRelease(pMAL_, 0, NULL)) != MAL_OK)
      {
         return TranslateMalError(malResult);
      }

      pMAL_ = NULL;

      return DEVICE_OK;
   }

   int MMMALHub::DetectDeviceStatus()
   {
      MALRESULT malResult;
      if (pMAL_ == NULL) 
      {
         return DEVICE_NOT_CONNECTED;
      }

      long nargs;
      malResult = malQueryComponentCategory(pMAL_, &nargs, configuration_);

      if (malResult != MAL_OK)
      {
         return TranslateMalError(malResult);
      }

      if (configuration_[0] != 1 || configuration_[1] != 1)
      {
         return DEVICE_NOT_CONNECTED;
      }

      return DEVICE_OK;
   }

   int MMMALHub::Initialize()
   {
      int result;
      MALRESULT malResult;

      if (initialized_)
      {
         return DEVICE_OK;
      }

      if ((result = CreateMALObject()) != DEVICE_OK)
      {
         return result;
      }

      if ((result = InitMALObject()) != DEVICE_OK)
      {
         return result;
      }

      malResult = malRegisterCallback(pMAL_, (FPMALCALLBACK) PreMALCallback, this);
      if (result != MAL_OK)
      {
         return TranslateMalError(malResult);
      }

      result = InitializeFocus();
      if (result != DEVICE_OK) 
      {
         return result;
      }

      result = UpdateMirrorUnitState();
      if (result != DEVICE_OK)
      {
         return result;
      }
      result = UpdateShutterState();
      if (result != DEVICE_OK)
      {
         return result;
      }
      result = UpdateNosepieceState();
      if (result != DEVICE_OK)
      {
         return result;
      }
      result = UpdateLightPathState();
      if (result != DEVICE_OK)
      {
         return result;
      }
      result = UpdateLampState();
      if (result != DEVICE_OK)
      {
         return result;
      }

      char pszVersion[MM::MaxStrLength];
      long strLen = MM::MaxStrLength;

      if (malGetMicroscopeVersionString(pMAL_, MS_FRAME_FIRM_VERSION, &strLen, pszVersion) == MAL_OK)
      {
         CreateProperty(Keyword_Frame_Version_, pszVersion, MM::String, true);
      }

      if (malGetMicroscopeVersionString(pMAL_, MS_FOCUS_FIRM_VERSION, &strLen, pszVersion) == MAL_OK)
      {
         CreateProperty(Keyword_Focus_Version_, pszVersion, MM::String, true);
      }

      if (malGetMicroscopeVersionString(pMAL_, MS_UCB_FIRM_VERSION, &strLen, pszVersion) == MAL_OK)
      {
         CreateProperty(Keyword_UCB_Version_, pszVersion, MM::String, true);
      }

      initialized_ = true;

      return DEVICE_OK;
   }

   MM::DeviceDetectionStatus MMMALHub::DetectDevice(void)
   {
      if ( Initialize() == DEVICE_OK)
      {
         return MM::CanCommunicate;
      }

      return MM::Misconfigured;
   }

   int MMMALHub::DetectInstalledDevices()
   {
      if ( DetectDevice() != MM::CanCommunicate)
      {
         return DEVICE_ERR;
      }

      AddInstalledDevice(new MMMALLightPath());
      AddInstalledDevice(new MMMALMirrorUnit());
      AddInstalledDevice(new MMMALFocus());
      AddInstalledDevice(new MMMALNosepiece());
      AddInstalledDevice(new MMMALLamp());
      AddInstalledDevice(new MMMALShutter(MMMALShutter::DeviceNameDIA_));
      AddInstalledDevice(new MMMALShutter(MMMALShutter::DeviceNameEPI_));

      if (HasZDC() || HasZDC2())
      {
         AddInstalledDevice(new MMMALAutofocus());
      }

      return DEVICE_OK;
   }

   int MMMALHub::EscapeNosepiece()
   {
      MALRESULT malResult = MAL_OK;
      UpdateNosepieceState();

      if ((malResult = malStopMovingSample(pMAL_, 0, AXIS_Z)) != MAL_OK)
      {
         return TranslateMalError(malResult);
      }

      if (escapePos_[nosepiecePosition_ - 1] == 0)
      { // escape
         escapePos_[nosepiecePosition_ - 1] = focusPos_;
         malResult = malSetSamplePosition(pMAL_,1, AXIS_Z, 10000);
         Sleep(1);
      }
      else
      { // recover
         malResult = malSetSamplePosition(pMAL_, 1, AXIS_Z,escapePos_[nosepiecePosition_ - 1]);
         escapePos_[nosepiecePosition_ - 1] = 0;
         Sleep(1);
      }

      return TranslateMalError(malResult);
   }

   void * MMMALHub::GetMALObject() const
   {
      return pMAL_;
   }

   int MMMALHub::UpdateShutterState()
   {
      int result = DEVICE_OK;
      MALRESULT malResult;
      MAL_STATUS status;
      if ((malResult = malGetShutterStatus(pMAL_, MICROSCOPE_DIA1, &status)) != MAL_OK)
      {
         return TranslateMalError(malResult);
      }
      if (status != diaShutterState_)
      {
         diaShutterState_ = status;
//         if (diaShutterDev_ != NULL)
//         {
//            diaShutterDev_->StateChanged();
//         }
      }

      if ((malResult = malGetShutterStatus(pMAL_, MICROSCOPE_EPI1, &status)) != MAL_OK)
      {
         return TranslateMalError(malResult);
      }
      if (status != epiShutterState_)
      {
         epiShutterState_ = status;
//         if (epiShutterDev_ != NULL)
//         {
//            epiShutterDev_->StatusChanged();
//         }
      }

      return result;
   }

   int MMMALHub::SetShutterState(MAL_IOTARGET channel, bool open)
   {
      MAL_STATUS status = open ? MAL_OPEN : MAL_CLOSE;

      if (channel == MICROSCOPE_DIA1 || channel == MICROSCOPE_EPI1)
      {
         MALRESULT malResult = malSetShutterStatus(pMAL_, 1, channel, status); //FIXME synchronous call

         if (malResult != MAL_OK)
         {
            //UpdateShutterStatus();
            return TranslateMalError(malResult);
         }

         if (channel == MICROSCOPE_DIA1)
         {
            //diaShutterBusy_ = true;
            diaShutterState_ = status;
         }
         else
         {
            //epiShutterBusy_ = true;
            epiShutterState_ = status;
         }
      }

      return DEVICE_OK;
   }

   bool MMMALHub::IsShutterOpen(MAL_IOTARGET channel) const
   {
      if (channel == MICROSCOPE_DIA1 )
      {
         return (diaShutterState_ == MAL_OPEN);
      }
      else
      {
         return (epiShutterState_ == MAL_OPEN);
      }
   }

   bool MMMALHub::GetShutterBusy(MAL_IOTARGET channel)
   {
      Sleep(1);
      return false;  // don't know how to implement which shutter is busy
//      if (channel == MICROSCOPE_DIA1 )
//      {
//         return diaShutterBusy_;
//      }
//      else
//      {
//         return epiShutterBusy_;
//      }
   }


   long MMMALHub::GetMirrorUnitNPositions() const
   {
      return configuration_[6];
   }

   int MMMALHub::UpdateMirrorUnitState()
   {
      MALRESULT malResult;
      MAL_MS_CUBEPOSITION pos;

      malResult = malGetCubePosition(pMAL_, &pos);

      if (malResult != MAL_OK)
      {
         return TranslateMalError(malResult);
      }

      cubePosition_ = (int)(pos - MAL_MS_CUBEPOS0);

      return DEVICE_OK;
   }

   int MMMALHub::GetMirrorUnitPosition() const
   {
      return cubePosition_;
   }

   int MMMALHub::SetMirrorUnitPosition(int pos)
   {
      MALRESULT malResult;

      malResult = malSetCubePosition(pMAL_, 1, (MAL_MS_CUBEPOSITION)(pos + MAL_MS_CUBEPOS0));
      if (malResult != MAL_OK)
      {
         return TranslateMalError(malResult);
      }

      cubeBusy_ = true;
      cubePosition_ = pos;
      return DEVICE_OK;
   }

   bool MMMALHub::GetMirrorUnitBusy() const
   {
      return cubeBusy_;
   }

   long MMMALHub::GetNosepieceNPositions() const
   {
      return configuration_[2];
   }

   int MMMALHub::UpdateNosepieceState()
   {
      MAL_MS_REVOLVERPOSITION pos;
      MALRESULT malResult;

      if ((malResult = malGetRevolverPosition(pMAL_, &pos)) != MAL_OK)
      {
         return TranslateMalError(malResult);
      }

      nosepiecePosition_= pos - MAL_MS_REVOLVERPOS0;

      return DEVICE_OK;
   }

   int MMMALHub::GetNosepiecePosition() const
   {
      return nosepiecePosition_;
   }

   bool MMMALHub::GetNosepieceBusy() const
   {
      return nosepieceBusy_;
   }

   int MMMALHub::SetNosepiecePostion(int pos)
   {
      MALRESULT malResult;

      malResult = malSetRevolverPosition(pMAL_, 1, (MAL_MS_REVOLVERPOSITION)(pos + MAL_MS_REVOLVERPOS0));

      if (malResult != MAL_OK)
      {
         return TranslateMalError(malResult);
      }

      nosepieceBusy_ = true;
      nosepiecePosition_ = pos;

      return DEVICE_OK;
   }

   long MMMALHub::GetLightPathNPositions() const
   {
      return ( HasBottomPort() ? 4L : 2L);
   }

   bool MMMALHub::HasBottomPort() const
   {
      return configuration_[8] > 0;
   }

   int MMMALHub::UpdateLightPathState()
   {
      MALRESULT malResult;
      MAL_STATUS status;

      int oldState = lightPathState_;
      lightPathState_ = 0;

      if (HasBottomPort())
      {
         malResult = malGetBottomPortStatus(pMAL_, &status);
         if (malResult != MAL_OK)
         {
            return TranslateMalError(malResult);
         }
         if (status == MAL_OPEN)
         {
            lightPathState_ = 2;
         }
      }

      malResult = malGetPrismStatus(pMAL_, &status);
      if (malResult != MAL_OK)
      {
         return TranslateMalError(malResult);
      }
      malSetSwitchLEDStatus(pMAL_, 0, status == MAL_BI ? MAL_ON : MAL_OFF);
      malSetSwitchLEDStatus(pMAL_, 1, status == MAL_SIDE ? MAL_ON : MAL_OFF);

      lightPathState_ += (status == MAL_BI) ? 1 : 2;
      if (lightPathState_ != oldState)
      {
         if (lightPathDev_ != NULL)
         {
            lightPathDev_->StateChanged();
         }
      }

      return DEVICE_OK;
   }

   int MMMALHub::GetLightPathState() const
   {
      return lightPathState_;
   }

   bool MMMALHub::GetLightPathBusy() const
   {
      return bottomPortBusy_ || prismBusy_;
   }

   int MMMALHub::SetLightPathPosition(int pos)
   {
      MALRESULT malResult;

      if (HasBottomPort() && ((pos > 2 && lightPathState_ <= 2) || (pos <= 2 && lightPathState_ > 2)))
      {
         malResult = malSetBottomPortStatus(pMAL_, 1, pos > 2 ? MAL_OPEN : MAL_CLOSE);
         if (malResult != MAL_OK)
         {
            return TranslateMalError(malResult);
         }
         bottomPortBusy_ = true;
      }

      if ( (pos % 2) != (lightPathState_ % 2))
      {
         malResult = malSetPrismStatus(pMAL_, 1, pos % 2 == 1 ? MAL_BI : MAL_SIDE);
         if (malResult != MAL_OK)
         {
            return TranslateMalError(malResult);
         }
         prismBusy_ = true;
      }

      lightPathState_ = pos;

      return DEVICE_OK;
   }

   ULONG MMMALHub::GetLampVoltage() const
   {
      return lampVoltage_;
   }

   int MMMALHub::GetLampVoltageRange(ULONG* min, ULONG* max)
   {
      static ULONG ulMin = -1;
      static ULONG ulMax = -1;
      MALRESULT malResult = MAL_OK;

      if (ulMin == -1)
      {
         malResult = malGetLampVoltageRange(pMAL_, MICROSCOPE_DIA1, &ulMin, &ulMax);
      }

      * min = ulMin;
      * max = ulMax;

      return TranslateMalError(malResult);
   }

   int MMMALHub::UpdateLampState()
   {
      ULONG voltage;
      MALRESULT malResult = malGetLampVoltage(pMAL_, MICROSCOPE_DIA1, &voltage);
      if (malResult != MAL_OK)
      {
         return TranslateMalError(malResult);
      }

      if (voltage != lampVoltage_)
      {
         lampVoltage_ = voltage;
         if (lampDev_ != NULL)
         {
            lampDev_->VoltageChanged();
         }
      }

      return DEVICE_OK;
   }

   int MMMALHub::SetLampVoltage(ULONG voltage)
   {
      MALRESULT malResult = malSetLampVoltage(pMAL_, 1, MICROSCOPE_DIA1, voltage);
      lampVoltage_ = voltage;
      lampBusy_ = true;
      return TranslateMalError(malResult);
   }

   bool MMMALHub::GetLampBusy() const
   {
      return lampBusy_;
   }

   int MMMALHub::SwitchLampVoltage()
   {
      static ULONG LampVolt = 0;

      ULONG volt = GetLampVoltage();
      if (volt == 0)
      {
         return SetLampVoltage(LampVolt);
      }
      else
      {
         LampVolt = volt;
         return SetLampVoltage(0);
      }
   }

   bool MMMALHub::HasZDC() const
   {
      return (configuration_[3] == 4);
   }

   bool MMMALHub::HasZDC2() const
   {
      return (configuration_[3] == 6);
   }

   int MMMALHub::InitializeFocus()
   {
      MALRESULT malResult;
      int result;

      if ((malResult = malInitializeSamplePosition(pMAL_, 0, AXIS_Z)) != MAL_OK)
      {
         return TranslateMalError(malResult);
      }

      if ((malResult = malSetJogStatus(pMAL_, MAL_MS_JOG1, MAL_ON)) != MAL_OK)
      {
         return TranslateMalError(malResult);
      }

      fineJogStep_ = false;
      if ((malResult = malSetJogStepSize(pMAL_, MAL_MS_JOG1, -2)) != MAL_OK)
      {
         return TranslateMalError(malResult);
      }

      if ((malResult = malSetControlJog(pMAL_, MAL_MS_JOG1)) != MAL_OK)
      {
         return TranslateMalError(malResult);
      }

      if ((malResult = malSetObservation(pMAL_, 0, MAL_MS_OBS_FL_DIC)) != MAL_OK) // needed for autofocus
      {      
         return TranslateMalError(malResult);
      }

      if ((malResult = malSetFocusPolling(pMAL_, MAL_ON, 300L)) != MAL_OK)
      {
         return TranslateMalError(malResult);
      }

      if (HasZDC() || HasZDC2()) 
      {
         if ((malResult = malSetOffsetLensAutoMovementStatus(pMAL_, MAL_ON)) != MAL_OK)
         {
            return TranslateMalError(malResult);
         }
      }

      focusPos_ = -1;

      result = UpdateFocusPosition();

      return result;
   }

   LONGLONG MMMALHub::GetFocusPosition()
   {
      return focusPos_;
   }

   int MMMALHub::UpdateFocusPosition()
   {
      int ret = DEVICE_OK;
      LONGLONG pos;
      MALRESULT malResult;

      if ( (malResult = malGetSamplePosition(pMAL_, AXIS_Z, &pos)) == MAL_OK )
      {
         if (pos != focusPos_)
         {
            focusPos_ = pos;
            if (focusDev_ != NULL) 
            {
               ret = focusDev_->PositionChanged();
            }
         }
      } 

      return TranslateMalError(malResult);
   }

   int MMMALHub::SetFocusPosition(LONGLONG pos)
   {
      MALRESULT malResult;

      malResult = malSetSamplePosition(pMAL_, 1, AXIS_Z, pos);

      if (malResult != MAL_OK)
      {
         return TranslateMalError(malResult);
      }

      focusBusy_ = true;
      focusPos_ = pos;

      return DEVICE_OK;
   }

   bool MMMALHub::GetFocusDriverBusy() const
   {
      return focusBusy_;
   }

   int MMMALHub::GetFocusLimits(LONGLONG* nearLimit, LONGLONG *farLimit)
   {
      MALRESULT malResult;

      malResult = malGetSampleFarLimit(pMAL_, AXIS_Z, farLimit);
      if (malResult != MAL_OK) 
      {
         return TranslateMalError(malResult);
      }

      malResult = malGetSampleNearLimit(pMAL_, AXIS_Z, nearLimit);
      if (malResult != MAL_OK) 
      {
         return TranslateMalError(malResult);
      }

      return DEVICE_OK;
   }

   int MMMALHub::SetFocusLimits(LONGLONG nearLimit, LONGLONG farLimit)
   {
      MALRESULT malResult;

      malResult = malSetSampleFarLimit(pMAL_, AXIS_Z, farLimit);
      if (malResult != MAL_OK) 
      {
         return TranslateMalError(malResult);
      }

      malResult = malSetSampleFarLimit(pMAL_, AXIS_Z, nearLimit);
      if (malResult != MAL_OK) 
      {
         return TranslateMalError(malResult);
      }

      return DEVICE_OK;
   }

   bool MMMALHub::GetAutofocusBusy() const
   {
      return afBusy_;
   }

   MAL_MS_AFSTATUS MMMALHub::GetAFStatus() const
   {
      return afStatus_;
   }

   int MMMALHub::UpdateAFStatus()
   {
      int result = DEVICE_OK;
      MALRESULT malResult;

      MAL_MS_AFSTATUS status;
      malResult = malGetAFStatus(pMAL_, &status);
      if (malResult != MAL_OK)
      {
         return TranslateMalError(malResult);
      }

      if (afStatus_ != status)
      {
         malSetSwitchLEDStatus(pMAL_, 3, status == MAL_MS_AF_FOCUS ? MAL_ON : MAL_OFF);

         afStatus_ = status;
         if (autofocusDev_ != NULL) 
         {
            result = autofocusDev_->AFStatusChanged();
         }
      }

      return result;
   }

   int MMMALHub::SetAFStatus(MAL_MS_AFSTATUS status)
   {
      MALRESULT malResult;

      malSetAFSearchCenterPosition(pMAL_, GetFocusPosition());

      malResult = malSetAFStatus(pMAL_, 1, status);
      if (malResult != MAL_OK)
      {
         return TranslateMalError(malResult);
      }

      afBusy_ = true;
      Sleep(1); // force asynchronise AF to start 

      return DEVICE_OK;
   }

   int MMMALHub::GetAFOffset(long *offset) const
   {
      MALRESULT malResult;

      malResult = malGetOffsetLensOffsetDistance(
            pMAL_,
            (MAL_MS_REVOLVERPOSITION)(nosepiecePosition_+MAL_MS_REVOLVERPOS0),
            offset
      );

      return TranslateMalError(malResult);
   }

   int MMMALHub::SetAFOffset(long offset)
   {
      MALRESULT malResult;

      malResult = malSetOffsetLensOffsetDistance(
            pMAL_,
            (MAL_MS_REVOLVERPOSITION)(nosepiecePosition_+MAL_MS_REVOLVERPOS0),
            offset
      );

      return TranslateMalError(malResult);
   }

   int MMMALHub::GetAFSearchRange(LONGLONG* range) const
   {
      MALRESULT malResult;

      malResult = malGetAFSearchZone(pMAL_, range);

      return TranslateMalError(malResult);
   }

   int MMMALHub::SetAFSearchRange(LONGLONG range)
   {
      MALRESULT malResult;

      malResult = malSetAFSearchZone(pMAL_, range);

      return TranslateMalError(malResult);
   }

   int MMMALHub::SwitchAF()
   {
      int ret = DEVICE_OK;

      if (HasZDC())
      {
         ret = SetAFStatus(MAL_MS_AF_SHOT);
      }
      else if (HasZDC2())
      {
         if (GetAFStatus() == MAL_MS_AF_OFF)
         {
            ret = SetAFStatus(MAL_MS_AF_ON);
         }
         else
         {
            ret = SetAFStatus(MAL_MS_AF_OFF);
         }
      }

      return ret;
   }

   MALRESULT MMMALHub::PreMALCallback(MAL_MS_MESSAGE msg, long wParam, long lParam, void* pv, void* pCaller, void* pOwner)
   {
      return ((MMMALHub*)pOwner)->MALCallback(msg, wParam, lParam, pv, pCaller);
   }

   MALRESULT MMMALHub::MALCallback(MAL_MS_MESSAGE msg, long wParam, long lParam, void* pv, void* pCaller)
   {
      switch (wParam)
      {
      case MAL_MS_EVT_CUBEPOSITION: // malSetShutterStatus
         cubeBusy_ = false;
         UpdateMirrorUnitState();
         break;
      case MAL_MS_EVT_REVOLVERPOSITION: // malSetShutterStatus
         nosepieceBusy_ = false;
         UpdateNosepieceState();
         break;
      case MAL_MS_EVT_BPORTSTATUS:
         bottomPortBusy_ = false;
         UpdateLightPathState();
         break;
      case MAL_MS_EVT_PRISMSTATUS:
         prismBusy_ = false;
         UpdateLightPathState();
         break;
      case MAL_MS_EVT_LAMPVOLTAGE:
         lampBusy_=  false;
         UpdateLampState();
         break;
      case MAL_MS_EVT_SAMPLEPOSITION:
         focusBusy_ = false;
         UpdateFocusPosition();
         break;
      case  MAL_MS_EVT_AFSTATUS:
         afBusy_ = false;
         UpdateAFStatus();
         break;
      case MAL_MS_EVT_SHUTTERSTATUS:
         break;
      default:
         break;
      }

      if (msg == ME_MS_ERR || msg == ME_MS_COMM_ERR || msg == ME_MS_TXTIMEOUT || msg == ME_MS_RXTIMEOUT)
      {
         const char * source;
         switch (wParam)
         {
         case MAL_MS_EVT_CUBEPOSITION:
            source = "Mirror Unit";
            break;
         case MAL_MS_EVT_REVOLVERPOSITION:
            source = "Nosepiece";
            break;
         case MAL_MS_EVT_BPORTSTATUS:
            source = "Bottom Port";
            break;
         case MAL_MS_EVT_PRISMSTATUS:
            source = "Prism";
            break;
         case MAL_MS_EVT_SAMPLEPOSITION:
            source = "Focus";
            break;
         case MAL_MS_EVT_AFSTATUS:
            source = "ZDC";
            break;
         case MAL_MS_EVT_LAMPVOLTAGE:
            source = "Lamp";
            break;
         case MAL_MS_EVT_SHUTTERSTATUS:
            source = "Shutter";
            break;
         default:
            source = "Unknown";
            break;
         }

         LogError(source, lParam, pv);
         return MAL_OK;
      }

      if (wParam == MAL_IX_EVT_NOTICE_FOCUSPOLLING)
      {
         if (msg == ME_MS_NOTICE_OK)
         {
            if (autofocusDev_ != NULL)
            {
               UpdateAFStatus();
            }
            if (! GetFocusDriverBusy())
            {
               UpdateFocusPosition();
            }
         }
         else 
         {
            LogError("Focus polling", lParam, pv);
         }
      } 

      if (wParam == MAL_IX_EVT_NOTICE_SWITCHON || wParam == MAL_IX_EVT_NOTICE_SWITCHOFF)
      {
         ULONG min, max;
         if (msg == ME_MS_NOTICE_OK)
         {
            switch (lParam)
            {
            case 0:
               malSetPrismStatus(pMAL_, 1, wParam == MAL_IX_EVT_NOTICE_SWITCHON ? MAL_BI : MAL_SIDE);
               Sleep(1);
               break;
            case 2:
               if (! GetAutofocusBusy())
               {
                  SwitchAF();
               }
               break;
            case 1:
               SwitchLampVoltage();
            case 3:
               GetLampVoltageRange(&min, &max);
               if (lampVoltage_ + 200 <= max)
               {
                  SetLampVoltage(lampVoltage_ + 200);
                  Sleep(1);
               }
               break;
            case 4:
               GetLampVoltageRange(&min, &max);
               if (lampVoltage_ - 200 >= min)
               {
                  SetLampVoltage(lampVoltage_ - 100);
                  Sleep(1);
               }
               break;
            case 9:
               fineJogStep_ = ! fineJogStep_;
               malSetJogStepSize(pMAL_, MAL_MS_JOG1, fineJogStep_ ? -1 : -2);
               break;
            case 11:
               EscapeNosepiece();
               break;
            case 24:
               SetShutterState(MICROSCOPE_DIA1, ! IsShutterOpen(MICROSCOPE_DIA1));
               break;
            case 25:
               SetShutterState(MICROSCOPE_EPI1, ! IsShutterOpen(MICROSCOPE_EPI1));
            default:
               break;
            }
         }
      }

      return MAL_OK;
   }
}

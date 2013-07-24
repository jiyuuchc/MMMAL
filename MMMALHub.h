/*
 * MMMalHub.h
 *
 *  Created on: Jul 12, 2013
 *      Author: Ji-Yu
 */

#ifndef MMMALHUB_H_
#define MMMALHUB_H_

#include <windows.h>
#include <iostream>
#include "MAL/include/MalMicroscope.h"

#include "MMDevice.h"
#include "MMDeviceConstants.h"
#include "DeviceBase.h"

namespace MMMAL {
   class MMMALFocus;
   class MMMALAutofocus;
   class MMMALMirrorUnit;
   class MMMALLightPath;
   class MMMALNosepiece;
   class MMMALShutter;
   class MMMALLamp;

   class MMMALHub : public HubBase<MMMALHub>
   {
   public:
      static const char * const DeviceName_;
      static const char * const Description_;
      static const char * const Keyword_Frame_Version_;
      static const char * const Keyword_Focus_Version_;
      static const char * const Keyword_UCB_Version_;
      static const char * const Keyword_Config_Path;
      static const char * const Keyword_ZDC_Config_Path;

      MMMALHub();
      ~MMMALHub();

      // MM::Device
      bool Busy() {return false; }
      int Initialize();
      int Shutdown();
      void GetName(char *pszName) const;

      // MM:hub
      MM::DeviceDetectionStatus DetectDevice(void);
      int DetectInstalledDevices();

      void * GetMALObject() const;
      void SetMirrorUnitDevice(MMMALMirrorUnit* dev);
      void SetNosepieceDevice(MMMALNosepiece* dev);
      void SetLightPathDevice(MMMALLightPath* dev);
      void SetShutterDevice(MMMALShutter* dev);
      void SetLampDevice(MMMALLamp* dev);
      void SetFocusDevice(MMMALFocus* dev);
      void SetAutofocusDevice(MMMALAutofocus* dev);

      //Shutter functions
      int UpdateShutterState();
      int SetShutterState(MAL_IOTARGET channel, bool bOpen);
      bool IsShutterOpen(MAL_IOTARGET channel) const;
      bool IsShutterBusy(MAL_IOTARGET channel);

      // mirro unit
      long GetMirrorUnitNPositions() const;
      int UpdateMirrorUnitState();
      int GetMirrorUnitPosition() const;
      int SetMirrorUnitPosition(int pos);
      bool IsMirrorUnitBusy() const;

      //nosepiece
      long GetNosepieceNPositions() const;
      int UpdateNosepieceState();
      int GetNosepiecePosition() const;
      bool IsNosepieceBusy() const;
      int SetNosepiecePosition(int pos);

      //lightpath
      long GetLightPathNPositions() const;
      int UpdateLightPathState();
      int GetLightPathState() const;
      bool IsLightPathBusy() const;
      int SetLightPathState(int pos);
      bool HasBottomPort() const;

      //Lamp
      ULONG GetLampVoltage() const;
      int UpdateLampState();
      int SetLampVoltage(ULONG voltage);
      bool IsLampBusy() const;
      int GetLampVoltageRange(ULONG* min, ULONG* max);
      int SwitchLampOnOff();

      //focus
      bool HasZDC() const;
      bool HasZDC2() const;
      int InitializeFocus();
      LONGLONG GetFocusPosition();
      int UpdateFocusPosition();
      int SetFocusPosition(LONGLONG pos);
      bool IsFocusBusy() const;
      int GetFocusLimits(LONGLONG* nearLimit, LONGLONG *farLimit);
      int SetFocusLimits(LONGLONG nearLimit, LONGLONG farLimit);
      
      bool IsAFBusy() const;
      MAL_MS_AFSTATUS GetAFStatus() const;
      int SetAFStatus(MAL_MS_AFSTATUS status);
      int GetAFOffset(long *offset) const;
      int UpdateAFStatus();
      int SetAFOffset(long offset);
      int GetAFSearchRange(LONGLONG* range)const;
      int SetAFSearchRange(LONGLONG range);
      int SwitchAF();

   protected:
      bool fineJogStep_;
      int CreateMALObject();
      int InitMALObject();
      int ExitMALObject();
      int ReleaseMALObject();
      int DetectDeviceStatus();
      int RegisterButtonNotices(MAL_STATUS status);
      int EscapeNosepiece();

      static MALRESULT MALAPI PreMALCallback(MAL_MS_MESSAGE msg, long wParam, long lParam, void* pv, void* pCaller, void* pOwner);
      MALRESULT MALCallback(MAL_MS_MESSAGE msg, long wParam, long lParam, void* pv, void* pCaller);
      int TranslateMalError(MALRESULT malResult) const;

   private:
      void LogError(std::string src, long errorcode, void* pv) const;
      int GetConfigFilePath(char* path, size_t len = MM::MaxStrLength);

      void * pMAL_;
      bool initialized_;
      long configuration_[12];

      //subdevices
      MMMALAutofocus* autofocusDev_;
      MMMALFocus* focusDev_;
      MMMALLightPath* lightPathDev_;
      MMMALMirrorUnit* mirrorUnitDev_;
      MMMALNosepiece* nosepieceDev_;
      MMMALShutter* epiShutterDev_;
      MMMALShutter* diaShutterDev_;
      MMMALLamp* lampDev_;

      //shutter
      MAL_STATUS epiShutterState_;
      MAL_STATUS diaShutterState_;
      //      bool epiShutterBusy_;
      //      bool diaShutterBusy_;

      //Lamp
      ULONG lampVoltage_;
      bool lampBusy_;

      //MU
      int cubePosition_;
      bool cubeBusy_;

      //nosepiece
      int nosepiecePosition_;
      bool nosepieceBusy_;

      //lightpath
      int lightPathState_;
      bool prismBusy_;
      bool bottomPortBusy_;

      //focus
      LONGLONG focusPos_;
      bool focusBusy_;
      MAL_MS_AFSTATUS afStatus_;
      bool afBusy_;
      long afOffset_;
   };

}

#endif /* MMMALHUB_H_ */



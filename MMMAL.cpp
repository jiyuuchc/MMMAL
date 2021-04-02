/*
 * MMMAL.cpp
 *
 *  Created on: Jul 12, 2013
 *      Author: Ji-Yu
 */

#include "MMMALHub.h"
#include "MMMALShutter.h"
#include "MMMALMirrorUnit.h"
#include "MMMALLightPath.h"
#include "MMMALNosepiece.h"
#include "MMMALFocus.h"
#include "MMMALAutofocus.h"
#include "MMMALLamp.h"
#include "MMDeviceConstants.h"

using namespace MMMAL;

// windows DLL entry code
#ifdef WIN32
BOOL APIENTRY DllMain(HANDLE /*hModule*/,  DWORD  ul_reason_for_call, LPVOID /*lpReserved*/)
{ 
   switch (ul_reason_for_call) {
   case DLL_PROCESS_ATTACH:
      break;
   case DLL_THREAD_ATTACH:
      break;
   case DLL_THREAD_DETACH:
      break;
   case DLL_PROCESS_DETACH:
      break;
   }
   return TRUE;
}
#endif

MODULE_API void InitializeModuleData()
{
   RegisterDevice(MMMALHub::DeviceName_, MM::HubDevice, MMMALHub::Description_);
   RegisterDevice(MMMALShutter::DeviceNameDIA_, MM::ShutterDevice, "Olympus IX DIA Shutter");
   RegisterDevice(MMMALShutter::DeviceNameEPI_, MM::ShutterDevice, "Olympus IX EPI Shutter");
   RegisterDevice(MMMALMirrorUnit::DeviceName_, MM::StateDevice, MMMALMirrorUnit::Description_);
   RegisterDevice(MMMALLightPath::DeviceName_, MM::StateDevice, MMMALLightPath::Description_);
   RegisterDevice(MMMALNosepiece::DeviceName_, MM::StateDevice, MMMALNosepiece::Description_);
   RegisterDevice(MMMALFocus::DeviceName_, MM::StageDevice, MMMALFocus::Description_);
   RegisterDevice(MMMALAutofocus::DeviceName_, MM::AutoFocusDevice, MMMALAutofocus::Description_);
   RegisterDevice(MMMALLamp::DeviceName_, MM::GenericDevice, MMMALLamp::Description_);

   // AddAvailableDeviceName(g_IX71Lamp, "IX71 Halogen Lamp");
}

MODULE_API MM::Device* CreateDevice(const char* deviceName)
{
   if (deviceName == NULL)
      return NULL;

   if (strcmp(deviceName, MMMALHub::DeviceName_) == 0)
   {
      return new MMMALHub();
   }

   if (
         strcmp(deviceName, MMMALShutter::DeviceNameDIA_) == 0 ||
         strcmp(deviceName, MMMALShutter::DeviceNameEPI_) == 0 )
   {
       return  new MMMALShutter(deviceName);
   }

   if (strcmp(deviceName, MMMALMirrorUnit::DeviceName_) == 0)
   {
       return  new MMMALMirrorUnit();
   }

   if (strcmp(deviceName, MMMALNosepiece::DeviceName_) == 0)
   {
       return  new MMMALNosepiece();
   }

   if (strcmp(deviceName, MMMALLightPath::DeviceName_) == 0)
   {
       return  new MMMALLightPath();
   }

   if (strcmp(deviceName, MMMALFocus::DeviceName_) == 0)
   {
       return  new MMMALFocus();
   }

   if (strcmp(deviceName, MMMALAutofocus::DeviceName_) == 0)
   {
      return new MMMALAutofocus();
   }

   if (strcmp(deviceName, MMMALLamp::DeviceName_) == 0)
   {
      return new MMMALLamp();
   }

   return NULL;
}

MODULE_API void DeleteDevice(MM::Device* pDevice)
{
   delete pDevice;
}

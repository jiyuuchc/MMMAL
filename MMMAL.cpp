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
   AddAvailableDeviceName(MMMALHub::DeviceName_, MMMALHub::Description_);
   AddAvailableDeviceName(MMMALShutter::DeviceNameDIA_, "Olympus IX DIA Shutter");
   AddAvailableDeviceName(MMMALShutter::DeviceNameEPI_, "Olympus IX EPI Shutter");
   AddAvailableDeviceName(MMMALMirrorUnit::DeviceName_, MMMALMirrorUnit::Description_);
   AddAvailableDeviceName(MMMALLightPath::DeviceName_, MMMALLightPath::Description_);
   AddAvailableDeviceName(MMMALNosepiece::DeviceName_, MMMALNosepiece::Description_);
   AddAvailableDeviceName(MMMALFocus::DeviceName_, MMMALFocus::Description_);
   AddAvailableDeviceName(MMMALAutofocus::DeviceName_, MMMALAutofocus::Description_);
   AddAvailableDeviceName(MMMALLamp::DeviceName_, MMMALLamp::Description_);

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

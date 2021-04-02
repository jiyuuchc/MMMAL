/*
 * MMMALShutter.cpp
 *
 *  Created on: Jul 13, 2013
 *      Author: Ji-Yu
 */

#include "MMMALShutter.h"

namespace MMMAL
{

   const char * const MMMALShutter::DeviceNameDIA_ = "IXShutterDIA";
   const char * const MMMALShutter::DeviceNameEPI_ = "IXShutterEPI";

   MMMALShutter::MMMALShutter(const char * deviceName): initialized_(false)
   {
      InitializeDefaultErrorMessages();

      CreateProperty(MM::g_Keyword_Name, deviceName, MM::String, true);
      channel_ = strcmp(deviceName, DeviceNameDIA_) == 0 ? MICROSCOPE_DIA1 : MICROSCOPE_EPI1;

      if (channel_ == MICROSCOPE_DIA1)
      {
         CreateProperty(MM::g_Keyword_Description, "Olympus IX DIA Shutter", MM::String, true);
      } else
      {
         CreateProperty(MM::g_Keyword_Description, "Olympus IX EPI Shutter", MM::String, true);
      }

   }

   MMMALShutter::~MMMALShutter()
   {
      Shutdown();
   }

   int MMMALShutter::Initialize()
   {
      int ret = DEVICE_OK;

      if (initialized_)
         return DEVICE_OK;

      
      hub_ = static_cast<MMMALHub*>(GetParentHub());
      if (hub_ == NULL)
      {
         return DEVICE_ERR;
      }

      hub_->SetShutterDevice(this);

      initialized_ = true;
      return ret;
   }

   int MMMALShutter::Shutdown()
   {
      initialized_ = false;
      return DEVICE_OK;
   }

   void MMMALShutter::GetName(char *pszName) const
   {
      GetProperty(MM::g_Keyword_Name, pszName);
   }

   bool MMMALShutter::Busy()
   {
      return hub_->IsShutterBusy(channel_);
   }

   int MMMALShutter::SetOpen(bool open)
   {
      return hub_->SetShutterState(channel_, open);
   }

   int MMMALShutter::GetOpen(bool& open)
   {
      open =  hub_->IsShutterOpen(channel_);

      return DEVICE_OK;
   }

   int MMMALShutter::Fire(double interval)
   {
      return DEVICE_UNSUPPORTED_COMMAND;
   }

   int MMMALShutter::StatusChanged()
   {
      int ret = DEVICE_OK;

      if (IsCallbackRegistered())
      {
         // ret = GetCoreCallback()->OnStatusChanged(this);
      }

      return ret;
   }
}

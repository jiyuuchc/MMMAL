/*
 * MMMALLamp.cpp
 *
 *  Created on: Jul 19, 2013
 *      Author: Ji-Yu
 */

#include "MMMALLamp.h"

namespace MMMAL
{
   const char * const MMMALLamp::DeviceName_ = "IXLamp";
   const char * const MMMALLamp::Description_ = "Olympus IX Lamp";
   const char * const MMMALLamp::Keyword_Voltage_ = "Voltage";

   MMMALLamp::MMMALLamp() : initialized_(false), hub_(NULL)
   {
      InitializeDefaultErrorMessages();

      CreateProperty(MM::g_Keyword_Name, DeviceName_, MM::String, true); // Name
      CreateProperty(MM::g_Keyword_Description, Description_ , MM::String, true); // Description
   }

   MMMALLamp::~MMMALLamp()
   {
      Shutdown();
   }

   int MMMALLamp::Initialize()
   {
      int ret = DEVICE_OK;

      if (initialized_)
      {
         return DEVICE_OK;
      }

      hub_ = static_cast<MMMALHub*>(GetParentHub());
      if (hub_ == NULL)
      {
         return DEVICE_COMM_HUB_MISSING;
      }

      hub_->SetLampDevice(this);

      ULONG pos = hub_->GetLampVoltage();
      ULONG vMin, vMax;

      hub_->GetLampVoltageRange(&vMin, &vMax);

      CPropertyAction* pAct = new CPropertyAction (this, &MMMALLamp::OnVoltage);
      ret = CreateProperty(Keyword_Voltage_, CDeviceUtils::ConvertToString((long)pos), MM::Integer, false, pAct);
      if (ret != DEVICE_OK)
      {
         return ret;
      }

      SetPropertyLimits(Keyword_Voltage_, (long)vMin, (long)vMax);

      initialized_ = true;

      return ret;
   }

   int MMMALLamp::Shutdown()
   {
      if (hub_ != NULL)
      {
         hub_->SetLampDevice(NULL);
      }
      initialized_ = false;
      return DEVICE_OK;
   }

   void MMMALLamp::GetName(char *pszName) const
   {
      GetProperty(MM::g_Keyword_Name, pszName);
   }

   bool MMMALLamp::Busy()
   {
      return hub_->GetLampBusy();
   }

   int MMMALLamp::OnVoltage(MM::PropertyBase* pProp, MM::ActionType eAct)
   {
      int ret = DEVICE_OK;

      if (eAct == MM::BeforeGet)
      {
         ULONG v = hub_->GetLampVoltage();
         pProp->Set((long) v);
      }
      else if (eAct == MM::AfterSet)
      {
         long voltage;
         pProp->Get(voltage);
         hub_->SetLampVoltage((ULONG) voltage);
      }

      return ret;
   }

   int MMMALLamp::VoltageChanged()
   {
      int ret = DEVICE_OK;

      if (IsCallbackRegistered())
      {
         ULONG voltage = hub_->GetLampVoltage();
         ret = GetCoreCallback()->OnPropertyChanged(this, Keyword_Voltage_, CDeviceUtils::ConvertToString((long)voltage));
      }

      return ret;
   }
}

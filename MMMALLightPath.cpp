/*
 * MMMALLightPath.cpp
 *
 *  Created on: Jul 13, 2013
 *      Author: Ji-Yu
 */

#include "MMMALLightPath.h"

namespace MMMAL
{

   const char * const MMMALLightPath::DeviceName_ = "IXLightPath";
   const char * const MMMALLightPath::Description_ = "Olympus IX LightPath";

   MMMALLightPath::MMMALLightPath():
         initialized_(false)
   {
      InitializeDefaultErrorMessages();

      CreateProperty(MM::g_Keyword_Name, DeviceName_, MM::String, true); // Name
      CreateProperty(MM::g_Keyword_Description, Description_, MM::String, true); // Description
   }

   MMMALLightPath::~MMMALLightPath()
   {
      Shutdown();
   }

   int MMMALLightPath::Shutdown()
   {
      initialized_ = false;
      return DEVICE_OK;
   }

   void MMMALLightPath::GetName(char *pszName) const
   {
      GetProperty(MM::g_Keyword_Name, pszName);
   }

   bool MMMALLightPath::Busy()
   {
      return hub_->GetLightPathBusy();
   }

   unsigned long MMMALLightPath::GetNumberOfPositions() const
   {
      return hub_->GetLightPathNPositions();
   };

   int MMMALLightPath::Initialize()
   {
      int ret = DEVICE_OK;

      if (initialized_)
      {
         return DEVICE_OK;
      }

      
      hub_ = static_cast<MMMALHub*>(GetParentHub());
      if (hub_ == NULL)
      {
         return DEVICE_ERR;
      }

      hub_->SetLightPathDevice(this);

      int pos = hub_->GetLightPathState();

      CPropertyAction* pAct = new CPropertyAction (this, &MMMALLightPath::OnState);
      ret = CreateProperty(MM::g_Keyword_State, CDeviceUtils::ConvertToString((long)pos), MM::Integer, false, pAct);
      if (ret != DEVICE_OK)
      {
         return ret;
      }

      SetPropertyLimits(MM::g_Keyword_State, 1, GetNumberOfPositions());
      
      pAct = new CPropertyAction (this, &MMMALLightPath::OnLabel);
      ret = CreateProperty(MM::g_Keyword_Label, "", MM::String, false, pAct);

      SetPositionLabel(1L, "Eyepiece");
      SetPositionLabel(2L, "Camera");
      if (GetNumberOfPositions() > 2) 
      {
         SetPositionLabel(3L, "Bottom Port");
         SetPositionLabel(3L, "Camer + Bottom Port");
      }
      initialized_ = true;
      return ret;
   }


   int MMMALLightPath::OnState(MM::PropertyBase* pProp, MM::ActionType eAct)
   {
      int ret = DEVICE_OK;

      if (eAct == MM::BeforeGet)
      {
         pProp->Set((long)(hub_->GetLightPathState()));
      }
      else if (eAct == MM::AfterSet) {
         long pos;
         pProp->Get(pos);

         assert((unsigned long) pos <= GetNumberOfPositions() && pos > 0);

         ret = hub_->SetLightPathPosition((int)pos);

      }

      return ret;
   }

   int MMMALLightPath::StateChanged()
   {
      int ret = DEVICE_OK;

      if (IsCallbackRegistered())
      {
         int state = hub_->GetLightPathState();
         
         ret = GetCoreCallback()->OnPropertyChanged(this, MM::g_Keyword_State, CDeviceUtils::ConvertToString(state));

         char buf[MM::MaxStrLength];
         GetPositionLabel(state, buf);
         ret = GetCoreCallback()->OnPropertyChanged(this, MM::g_Keyword_Label, buf);
         //std::clog << "Lightpath state changed " << state << std::endl;
      }

      return ret;
   }
}

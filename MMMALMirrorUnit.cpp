/*
 * MMMALMirrorUnit.cpp
 *
 *  Created on: Jul 13, 2013
 *      Author: Ji-Yu
 */

#include "MMMALMirrorUnit.h"

namespace MMMAL
{

   const char * const MMMALMirrorUnit::DeviceName_ = "IXMirrorUnit";
   const char * const MMMALMirrorUnit::Description_ = "Olympus IX Mirror Unit";
   const char * const MMMALMirrorUnit::Keyword_N_Positions_ = "NumOfPositions";

   MMMALMirrorUnit::MMMALMirrorUnit():initialized_(false)
   {
      InitializeDefaultErrorMessages();

      CreateProperty(MM::g_Keyword_Name, DeviceName_, MM::String, true); // Name
      CreateProperty(MM::g_Keyword_Description, Description_ , MM::String, true); // Description
   }

   MMMALMirrorUnit::~MMMALMirrorUnit()
   {
      Shutdown();
   }

   int MMMALMirrorUnit::Shutdown()
   {
      if (hub_ != NULL)
      {
         hub_->SetMirrorUnitDevice(NULL);
      }
      initialized_ = false;
      return DEVICE_OK;
   }

   void MMMALMirrorUnit::GetName(char *pszName) const
   {
      GetProperty(MM::g_Keyword_Name, pszName);
   }

   bool MMMALMirrorUnit::Busy()
   {
      return hub_->GetMirrorUnitBusy();
   }


   unsigned long MMMALMirrorUnit::GetNumberOfPositions() const
   {
      return hub_->GetMirrorUnitNPositions();
   }

   int MMMALMirrorUnit::Initialize()
   {
      int ret = DEVICE_OK;

      if (initialized_)
         return DEVICE_OK;

      hub_ = static_cast<MMMALHub*>(GetParentHub());
      if (hub_ == NULL)
      {
         return DEVICE_ERR;
      }

      hub_->SetMirrorUnitDevice(this);

      int pos = hub_->GetMirrorUnitPosition();

      CPropertyAction* pAct = new CPropertyAction (this, &MMMALMirrorUnit::OnState);
      ret = CreateProperty(MM::g_Keyword_State, CDeviceUtils::ConvertToString((long)pos), MM::Integer, false, pAct);
      if (ret != DEVICE_OK)
      {
         return ret;
      }

      SetPropertyLimits(MM::g_Keyword_State, 1, GetNumberOfPositions());

      for (unsigned i = 1; i <= GetNumberOfPositions(); i++)
      {
         long bufSize = MM::MaxStrLength;
         char buf[MM::MaxStrLength];

         if (malGetCubeName(hub_->GetMALObject(), (MAL_MS_CUBEPOSITION)(MAL_MS_CUBEPOS0 + i), &bufSize, buf) == MAL_OK && bufSize > 0)
         {
            if (strcmp(buf, "---") != 0)
            {
               SetPositionLabel(i, buf);
            } 
            else 
            {
               SetPositionLabel(i, CDeviceUtils::ConvertToString((int)i));
            }
         }
      }

      initialized_ = true;

      return ret;
   }

   int MMMALMirrorUnit::OnState(MM::PropertyBase* pProp, MM::ActionType eAct)
   {
      if (eAct == MM::BeforeGet)
      {
         pProp->Set((long)(hub_->GetMirrorUnitPosition()));

         return DEVICE_OK;
      }
      else if (eAct == MM::AfterSet)
      {
         long pos;
         pProp->Get(pos);

         assert((unsigned long) pos <= GetNumberOfPositions() && pos > 0);

         return hub_->SetMirrorUnitPosition((int)pos);
      }

      return DEVICE_OK;
   }

}


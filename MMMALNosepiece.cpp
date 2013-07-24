/*
 * MMMALNosepiece.cpp
 *
 *  Created on: Jul 13, 2013
 *      Author: Ji-Yu
 */

#include "MMMALNosepiece.h"

namespace MMMAL
{
   const char * const MMMALNosepiece::DeviceName_ = "IXNosepiece";
   const char * const MMMALNosepiece::Description_ = "Olympus IX Nosepiece";

   MMMALNosepiece::MMMALNosepiece():initialized_(false)
   {
      InitializeDefaultErrorMessages();

      CreateProperty(MM::g_Keyword_Name, DeviceName_, MM::String, true); // Name
      CreateProperty(MM::g_Keyword_Description, Description_, MM::String, true); // Description
   }

   MMMALNosepiece::~MMMALNosepiece()
   {
      Shutdown();
   }

   int MMMALNosepiece::Shutdown()
   {
      initialized_ = false;
      return DEVICE_OK;
   }

   void MMMALNosepiece::GetName(char *pszName) const
   {
      GetProperty(MM::g_Keyword_Name, pszName);
   }

   bool MMMALNosepiece::Busy()
   {
      return hub_->IsNosepieceBusy();
   }

   unsigned long MMMALNosepiece::GetNumberOfPositions() const
   {
      return hub_->GetNosepieceNPositions();
   };

   int MMMALNosepiece::Initialize()
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

      hub_->SetNosepieceDevice(this);

      int pos = hub_->GetNosepiecePosition();

      CPropertyAction* pAct = new CPropertyAction (this, &MMMALNosepiece::OnState);
      ret = CreateProperty(MM::g_Keyword_State, CDeviceUtils::ConvertToString((long)pos), MM::Integer, false, pAct);
      if (ret != DEVICE_OK)
      {
         return ret;
      }

      SetPropertyLimits(MM::g_Keyword_State, 1, GetNumberOfPositions());
      
      pAct = new CPropertyAction (this, &MMMALNosepiece::OnLabel);
      ret = CreateProperty(MM::g_Keyword_Label, "", MM::String, false, pAct);

      for (unsigned i = 1; i <= GetNumberOfPositions(); i++)
      {
         long bufSize = MM::MaxStrLength;
         char buf[MM::MaxStrLength];

         if (malGetObjectiveLensName(hub_->GetMALObject(), (MAL_MS_REVOLVERPOSITION)(MAL_MS_REVOLVERPOS0 + i), &bufSize, buf) == MAL_OK && bufSize > 0)
         {
            if (strcmp(buf, "---") != 0 ) 
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


   int MMMALNosepiece::OnState(MM::PropertyBase* pProp, MM::ActionType eAct)
   {
      int ret = DEVICE_OK;

      if (eAct == MM::BeforeGet)
      {
         pProp->Set((long)(hub_->GetNosepiecePosition()));
      }
      else if (eAct == MM::AfterSet) {
         long pos;
         pProp->Get(pos);

         assert((unsigned long) pos <= GetNumberOfPositions() && pos > 0);

         ret = hub_->SetNosepiecePosition((int)pos, true);

      }

      return ret;
   }

   int MMMALNosepiece::StateChanged()
   {
      int ret = DEVICE_OK;

      if (IsCallbackRegistered())
      {
         int pos = hub_->GetNosepiecePosition();
         
         ret = GetCoreCallback()->OnPropertyChanged(this, MM::g_Keyword_State, CDeviceUtils::ConvertToString(pos));

         char buf[MM::MaxStrLength];
         GetPositionLabel(pos, buf);
         ret = GetCoreCallback()->OnPropertyChanged(this, MM::g_Keyword_Label, buf);
         //std::clog << "Lightpath state changed " << state << std::endl;
      }

      return ret;
   }
}

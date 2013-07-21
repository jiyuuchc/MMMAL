/*
 * MMMALAutofocus.cpp
 *
 *  Created on: Jul 13, 2013
 *      Author: Ji-Yu
 */

#include "MMMALAutofocus.h"

namespace MMMAL
{
   const char * const MMMALAutofocus::DeviceName_ = "IXZDC";
   const char * const MMMALAutofocus::Description_ = "Olympus IX Auto Focus";
   const char * const MMMALAutofocus::Keyword_AFSearchRange_ = "AF Search Range";
   const char * const MMMALAutofocus::Keyword_AFOffset_ = "AF Offset";
   const char * const MMMALAutofocus::Keyword_AFStatus_ = "AF Status";
   const char * const MMMALAutofocus::Keyword_ContinuousFocus_ ="Continuous Focus";

   MMMALAutofocus::MMMALAutofocus() : initialized_(false), hub_(NULL), continuousFocusing_(false)
   {
      InitializeDefaultErrorMessages();

      CreateProperty(MM::g_Keyword_Name, DeviceName_, MM::String, true); // Name
      CreateProperty(MM::g_Keyword_Description, Description_, MM::String, true); // Description
   }

   MMMALAutofocus::~MMMALAutofocus()
   {
      Shutdown();
   }

   void MMMALAutofocus::GetName(char *pszName) const
   {
      GetProperty(MM::g_Keyword_Name, pszName);
   }

   bool MMMALAutofocus::Busy()
   {
      return hub_->GetAutofocusBusy();
   }

   int MMMALAutofocus::Shutdown()
   {
      if (hub_ != NULL)
      {
         hub_->SetAutofocusDevice(NULL);
      }
      initialized_ = false;
      return DEVICE_OK;
   }

   void MMMALAutofocus::GetStatusString(char * buf)
   {
      MAL_MS_AFSTATUS status;
      status = hub_->GetAFStatus();

      switch(status)
      {
      case MAL_MS_AF_OFF:
         strcpy(buf, "Off");
         break;
      case MAL_MS_AF_FOCUS:
         strcpy(buf, "Focus");
         break;
      case MAL_MS_AF_TRACE:
         strcpy(buf, "Trace");
         break;
      case MAL_MS_AF_WAIT:
         strcpy(buf,"Wait");
         break;
      case MAL_MS_AF_SEARCH:
         strcpy(buf,"Search");
         break;
      case MAL_MS_AF_PAUSE:
         strcpy(buf, "Pause");
         break;
      default:
         strcpy(buf, "Unknown");
         break;
      }
   }

   int MMMALAutofocus::Initialize()
   {
      int ret;

      if (initialized_)
      {
         return DEVICE_OK;
      }

      hub_ = static_cast<MMMALHub*>(GetParentHub());
      if (hub_ == NULL)
      {
         return DEVICE_ERR;
      }

      hub_->SetAutofocusDevice(this);

      //autofocus parameters
      if (hub_->HasZDC2() || hub_->HasZDC()) 
      {
         CPropertyAction* pAct;

         pAct = new CPropertyAction (this, &MMMALAutofocus::OnContinuousFocus);
         ret = CreateProperty(Keyword_ContinuousFocus_, "Off", MM::String, false, pAct);
         if (ret != DEVICE_OK)
         {
            return ret;
         }
         AddAllowedValue(Keyword_ContinuousFocus_, "On");
         AddAllowedValue(Keyword_ContinuousFocus_, "Off");

         if (hub_->HasZDC2())
         {
            pAct = new CPropertyAction (this, &MMMALAutofocus::OnAutoFocusOffset);
            ret = CreateProperty(Keyword_AFOffset_, "0", MM::Integer, false, pAct);
            if (ret != DEVICE_OK)
            {
               return ret;
            }
            SetPropertyLimits(Keyword_AFOffset_, -2080, 2080);
         }

         LONGLONG range;
         ret = hub_->GetAFSearchRange(&range);
         if (ret != DEVICE_OK) 
         {
            return ret;
         }

         const char * buf = CDeviceUtils::ConvertToString((double)range/ 1e6);
         pAct = new CPropertyAction (this, &MMMALAutofocus::OnAutoFocusSearchRange);
         ret = CreateProperty(Keyword_AFSearchRange_, buf, MM::Float, false, pAct);
         if (ret != DEVICE_OK)
         {
            return ret;
         }
         this->SetPropertyLimits(Keyword_AFSearchRange_, 0, 1000);

         pAct = new CPropertyAction (this, &MMMALAutofocus::OnAutoFocusStatus);
         ret = CreateProperty(Keyword_AFStatus_, buf, MM::String, true, pAct);
         if (ret != DEVICE_OK)
         {
            return ret;
         }
      }

      initialized_ = true;
      return DEVICE_OK;
   }

   int MMMALAutofocus::SetContinuousFocusing(bool state)
   {
      if (!hub_->HasZDC2())
      {
         return DEVICE_UNSUPPORTED_COMMAND;
      } 

      continuousFocusing_ = state;

      if (continuousFocusing_) 
      {
         hub_->SetAFStatus(MAL_MS_AF_ON);
      }
      else 
      {
         hub_->SetAFStatus(MAL_MS_AF_OFF);
      }

      return DEVICE_OK;
   }

   int MMMALAutofocus::GetContinuousFocusing(bool& state)
   {
      state = continuousFocusing_;
      return DEVICE_OK;
   }

   bool MMMALAutofocus::IsContinuousFocusLocked()
   {
      if (! hub_->HasZDC2())
      {
         return false;
      }

      return ( hub_->GetAFStatus() == MAL_MS_AF_FOCUS);
   }

   int MMMALAutofocus::FullFocus()
   {
      return hub_->SetAFStatus(MAL_MS_AF_SHOT);
   }

   int MMMALAutofocus::IncrementalFocus()
   {
      return DEVICE_UNSUPPORTED_COMMAND;
   }

   int MMMALAutofocus::GetLastFocusScore(double& score)
   {
      return DEVICE_UNSUPPORTED_COMMAND;
   }
   
   int MMMALAutofocus::GetCurrentFocusScore(double& score)
   {
      return DEVICE_UNSUPPORTED_COMMAND;
   }   
   
   int MMMALAutofocus::AutoSetParameters()
   {
      LONGLONG range;
      
      hub_->GetAFSearchRange(&range);
      if (range == 0) 
      {
         hub_->SetAFSearchRange((LONGLONG)10 * (LONGLONG)1000000);
      }

      return DEVICE_OK;
   }
   
   int MMMALAutofocus::GetOffset(double &offset)
   {
      if (hub_->HasZDC2())
      {
         long lOffset;
         int ret = hub_->GetAFOffset(&lOffset);
         offset = (double) lOffset;

         return ret;
      }
      else 
      {
         return DEVICE_UNSUPPORTED_COMMAND;
      }
   }
   
   int MMMALAutofocus::SetOffset(double offset)
   {
      if (hub_->HasZDC2()) 
      {
         int ret = hub_->SetAFOffset((long) offset);
         return ret;
      }
      else 
      {
         return DEVICE_UNSUPPORTED_COMMAND;
      }
   }

   int MMMALAutofocus::AFStatusChanged()
   {
      int ret = DEVICE_OK;

      if (IsCallbackRegistered())
      {
         char buf[MM::MaxStrLength];
         GetStatusString(buf);
         ret = GetCoreCallback()->OnPropertyChanged(this, Keyword_AFStatus_, buf);
      }

      return ret;
   }

   int MMMALAutofocus::OnContinuousFocus(MM::PropertyBase* pProp, MM::ActionType eAct)
   {
      int ret = DEVICE_OK;
      std::string mode;

      if (eAct == MM::BeforeGet)
      {
      }
      else if (eAct == MM::AfterSet)
      {
         pProp->Get(mode);
         if (mode.compare("On") == 0)
         {
            ret = SetContinuousFocusing(true);
         }
         else if (mode.compare("Off") == 0)
         {
            ret = SetContinuousFocusing(false);
         }
      }

      return DEVICE_OK;
   }

   int MMMALAutofocus::OnAutoFocusOffset(MM::PropertyBase* pProp, MM::ActionType eAct)
   {
      int ret = DEVICE_OK;

      if (eAct == MM::BeforeGet)
      {
         double offset;
         ret = GetOffset(offset);
         pProp->Set(long(offset));
      }
      else if (eAct == MM::AfterSet)
      {
         long lOffset;
         pProp->Get(lOffset);
         
         ret = SetOffset((double) lOffset);
      }

      return ret;
   }

   int MMMALAutofocus::OnAutoFocusSearchRange(MM::PropertyBase* pProp, MM::ActionType eAct)
   {
      int ret = DEVICE_OK;

      if (eAct = MM::BeforeGet)
      {
         LONGLONG llRange;
         
         ret = hub_->GetAFSearchRange(&llRange);
         pProp->Set((double) llRange / 1e6);
      }
      else if (eAct == MM::AfterSet)
      {
         double range;

         pProp->Get(range);
         ret = hub_->SetAFSearchRange((LONGLONG)(range * 1e6));
      }

      return ret;
   }

   int MMMALAutofocus::OnAutoFocusStatus(MM::PropertyBase* pProp, MM::ActionType eAct)
   {
      int ret = DEVICE_OK;

      if (eAct == MM::BeforeGet)
      {
         char buf[MM::MaxStrLength];
         GetStatusString(buf);
         pProp->Set(buf);
      }

      return ret;
   }

}

/*
 * MMMALFocus.cpp
 *
 *  Created on: Jul 13, 2013
 *      Author: Ji-Yu
 */

#include "MMMALFocus.h"

namespace MMMAL
{

   const char * const MMMALFocus::DeviceName_ = "IXFocus";
   const char * const MMMALFocus::Description_ = "Olympus IX Focus";
   const char * const MMMALFocus::Keyword_Position_ = "Position";
   const char * const MMMALFocus::Keyword_NearLimit_ = "Near Limit";
   const char * const MMMALFocus::Keyword_FarLimit_ = "Far Limit";

   MMMALFocus::MMMALFocus() : initialized_(false), stepSize_(0.001), origin_(0), hub_(NULL)
   {
      InitializeDefaultErrorMessages();

      CreateProperty(MM::g_Keyword_Name, DeviceName_, MM::String, true); // Name
      CreateProperty(MM::g_Keyword_Description, Description_, MM::String, true); // Description
   }

   MMMALFocus::~MMMALFocus()
   {
      Shutdown();
   }

   void MMMALFocus::GetName(char *pszName) const
   {
      GetProperty(MM::g_Keyword_Name, pszName);
   }

   double MMMALFocus::GetStepSize()
   {
      return stepSize_;
   }

   bool MMMALFocus::Busy()
   {
      return hub_->IsFocusBusy();
   }

   int MMMALFocus::Shutdown()
   {
      if (hub_ != NULL)
      {
         hub_->SetFocusDevice(NULL);
         hub_ = NULL;
      }
      initialized_ = false;
      return DEVICE_OK;
   }

   int MMMALFocus::Initialize()
   {
      int ret;
      CPropertyAction* pAct;

      if (initialized_)
      {
         return DEVICE_OK;
      }


      hub_ = static_cast<MMMALHub*>(GetParentHub());
      if (hub_ == NULL)
      {
         return DEVICE_ERR;
      }

      hub_->SetFocusDevice(this);

      double curPos;
      ret = GetPositionUm(curPos);

      pAct = new CPropertyAction (this, &MMMALFocus::OnPosition);
      ret = CreateProperty(Keyword_Position_, CDeviceUtils::ConvertToString(curPos), MM::Float, false, pAct);
      if (ret != DEVICE_OK)
      {
         return ret;
      }

      pAct = new CPropertyAction (this, &MMMALFocus::OnNearLimit);
      ret = CreateProperty(Keyword_NearLimit_, CDeviceUtils::ConvertToString(curPos), MM::Float, false, pAct);
      if (ret != DEVICE_OK)
      {
         return ret;
      }

      pAct = new CPropertyAction (this, &MMMALFocus::OnFarLimit);
      ret = CreateProperty(Keyword_FarLimit_, CDeviceUtils::ConvertToString(curPos), MM::Float, false, pAct);
      if (ret != DEVICE_OK)
      {
         return ret;
      }

      initialized_ = true;
      return DEVICE_OK;
   }

   int MMMALFocus::SetPositionUm(double posUm)
   {
      long steps;
      steps = (long) (posUm / stepSize_) + origin_;

      return SetPositionSteps(steps);
   }

   int MMMALFocus::GetPositionUm(double& posUm)
   {
      long steps;
      int ret = GetPositionSteps(steps);

      posUm = (steps - origin_) * stepSize_;

      return ret;
   }

   int MMMALFocus::SetPositionSteps(long steps)
   {
      LONGLONG posPm = (LONGLONG)steps * 1000;

      return hub_->SetFocusPosition(posPm);
   }

   int MMMALFocus::GetPositionSteps(long& steps)
   {
      LONGLONG posPm = hub_->GetFocusPosition();

      steps = (long)(posPm / 1000);

      return DEVICE_OK;
   }

   int MMMALFocus::SetOrigin()
   {
      long steps;
      int ret = GetPositionSteps(steps);
      if (ret == DEVICE_OK) 
      {
         origin_ = steps;
      }
      else 
      {
         return ret;
      }

      return DEVICE_OK;
   }

   int MMMALFocus::GetLimits(double& min, double& max)
   {
      LONGLONG lmin;
      LONGLONG lmax;
      hub_->GetFocusLimits(&lmax, &lmin); //nearlimit and farlimit, farlimit is the smaller one

      min = (double) (lmin / 1000 - origin_) * stepSize_;
      max = (double) (lmax / 1000 - origin_) * stepSize_;

      return DEVICE_OK;
   }

   int MMMALFocus::IsStageSequenceable(bool& seq) const
   {
      seq = false;
      return DEVICE_OK;
   }

   bool MMMALFocus::IsContinuousFocusDrive() const
   {
      return false;
   }

   int MMMALFocus::OnPosition(MM::PropertyBase* pProp, MM::ActionType eAct)
   {
      int ret = DEVICE_OK;
      double positionUm;

      if (eAct == MM::BeforeGet)
      {
         ret = GetPositionUm(positionUm);
         pProp->Set(positionUm);
      }
      else if (eAct == MM::AfterSet)
      {
         pProp->Get(positionUm);
         ret = SetPositionUm(positionUm);
      }

      return ret;
   }


   int MMMALFocus::OnNearLimit(MM::PropertyBase* pProp, MM::ActionType eAct)
   {
      int ret = DEVICE_OK;
      double nLimit, fLimit;
      GetLimits(fLimit, nLimit);

      if (eAct == MM::BeforeGet)
      {
         pProp->Set(nLimit);
      }
      else if (eAct == MM::AfterSet)
      {
         LONGLONG llNLimit, llFLimit;
         pProp->Get(nLimit);
         
         llNLimit = (LONGLONG)((nLimit / stepSize_ + origin_) * 1000);
         llFLimit = (LONGLONG)((fLimit / stepSize_ + origin_) * 1000);

         ret = hub_->SetFocusLimits(llNLimit,llFLimit);
      }

      return ret;
   }

   int MMMALFocus::OnFarLimit(MM::PropertyBase* pProp, MM::ActionType eAct)
   {
      int ret = DEVICE_OK;
      double nLimit, fLimit;
      GetLimits(fLimit, nLimit);

      if (eAct == MM::BeforeGet)
      {
         pProp->Set(fLimit);
      }
      else if (eAct == MM::AfterSet)
      {
         LONGLONG llNLimit, llFLimit;
         pProp->Get(fLimit);
         
         llNLimit = (LONGLONG)((nLimit / stepSize_ + origin_) * 1000);
         llFLimit = (LONGLONG)((fLimit / stepSize_ + origin_) * 1000);

         ret = hub_->SetFocusLimits(llNLimit,llFLimit);
      }

      return ret;
   }

   int MMMALFocus::PositionChanged()
   {
      int ret = DEVICE_OK;

      if (IsCallbackRegistered())
      {
         LONGLONG pos = hub_->GetFocusPosition();
         double posUm = (pos / 1000 - origin_) * stepSize_;

         ret = GetCoreCallback()->OnPropertyChanged(this, Keyword_Position_, CDeviceUtils::ConvertToString(posUm));
         //std::clog << "Z position changed " << posUm << std::endl;
      }

      return ret;
   }

}

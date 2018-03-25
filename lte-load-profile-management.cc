/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */

/*
* Copyright (c) 2011 SIGNET LAB. Department of Information Engineering (DEI), University of Padua
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 as
* published by the Free Software Foundation;
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*
* Extension done by:
*      Aziza Ben-Mosbah <aziza.ben.mosbah@gmail.com>
*/ 

#include "ns3/core-module.h"
#include <list>
#include <vector>
#include <algorithm>
#include <ns3/log.h>
#include <ns3/pointer.h>
#include <stdint.h>
#include <cmath>
#include <stdint.h>
#include "stdlib.h"
#include <ns3/lte-load-profile-management.h>
#include "ns3/random-variable-stream.h"
#include <math.h> 
#include <string>
#include <ns3/epc-ue-nas.h>
#include <ns3/lte-ue-net-device.h>
#include <ns3/lte-ue-phy.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("LteLoadProfileManagement");
NS_OBJECT_ENSURE_REGISTERED (LteLoadProfileManagement);

LteLoadProfileManagement::LteLoadProfileManagement (void)
   : m_enbs (0),
     m_ues (0)
{
  NS_LOG_FUNCTION (this);
}

LteLoadProfileManagement::~LteLoadProfileManagement (void)
{
  NS_LOG_FUNCTION (this);
}

TypeId LteLoadProfileManagement::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::LteLoadProfileManagement")
    .SetParent<Object> ()
    .AddConstructor<LteLoadProfileManagement> ()
  ;
  return tid;
}

void
LteLoadProfileManagement::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_lteHelper = 0;
  m_enbs =0;
  m_ues =0;
  m_enbMaxUesLoadProfileMap.clear ();
  Object::DoDispose ();
}

void 
LteLoadProfileManagement::SetLteHelper (Ptr<LteHelper> h)
{
  NS_LOG_FUNCTION (this << h);
  m_lteHelper = h;
}

void 
LteLoadProfileManagement::SetDevices (NetDeviceContainer enbDevs, NetDeviceContainer ueDevs, std::vector<uint32_t> maxUesPerEnb, std::vector<uint32_t> loadProfile)
{
  NS_LOG_FUNCTION (this << enbDevs.GetN() << ueDevs.GetN() << maxUesPerEnb.size() << loadProfile.size ());
  m_ueDevs = ueDevs;
  m_ues = ueDevs.GetN();
  m_enbDevs = enbDevs;
  m_enbs = enbDevs.GetN();
      
  Ptr<UniformRandomVariable> rnd = CreateObject<UniformRandomVariable> ();
  rnd->SetAttribute ("Min", DoubleValue(0));
  rnd->SetAttribute ("Max", DoubleValue(4));
  for (NetDeviceContainer::Iterator i = enbDevs.Begin(); i != enbDevs.End(); ++i)
  {
    EnbInfo enbInfo;
    enbInfo.id = (*i)->GetNode()->GetId();
    enbInfo.maxUes = maxUesPerEnb.at(enbInfo.id-1);
    enbInfo.loadProfileType = loadProfile.at(enbInfo.id-1);
    enbInfo.loadProfileNumber = rnd->GetInteger ();
    m_enbMaxUesLoadProfileMap.insert (std::pair<Ptr<NetDevice>, EnbInfo> ((*i), enbInfo));
    std::cout << "enb=" << enbInfo.id << ", maxUes=" << enbInfo.maxUes << ", loadProfile=" << enbInfo.loadProfileType << ", randomNumber=" << enbInfo.loadProfileNumber+1 << std::endl;
  }
}

void 
LteLoadProfileManagement::AttachUesToSelectedEnb (Ptr<LteHelper> lh, NetDeviceContainer ues, Ptr<NetDevice> enb)
{
  lh->Attach (ues, enb);
}

void
LteLoadProfileManagement::CreateEnbAttachment ()
{
  NS_LOG_FUNCTION (this);
  for (NetDeviceContainer::Iterator itEnb = m_enbDevs.Begin(); itEnb != m_enbDevs.End(); ++itEnb)
  {
    CreateEnbAttachment ((*itEnb));
  }

}
/*
void 
LteLoadProfileManagement::SetUeTxPower (uint32_t ue, double pow)
{
  NS_LOG_FUNCTION (this << ue << pow);
  m_ueDevs.Get(ue)->GetObject<LteUeNetDevice> ()->GetPhy()->SetTxPower (pow);

  if (Simulator::Now().GetSeconds () != 0)
  {
   if (pow == 0.0)
    {
      std::cout << "At " << Simulator::Now().GetSeconds ()<< "s, UE " <<  m_ueDevs.Get(ue)->GetObject<LteUeNetDevice> ()->GetImsi () << " detached" << std::endl;
    }
    else if (pow == 23.0)
    {
      std::cout << "At " << Simulator::Now().GetSeconds ()<< "s, UE " <<  m_ueDevs.Get(ue)->GetObject<LteUeNetDevice> ()->GetImsi () << " attached" << std::endl;
    }
  }
}
*/

void
LteLoadProfileManagement::Disconnect (Ptr <NetDevice> ueDevice, Ptr<NetDevice> enbDevice)
{
  NS_LOG_FUNCTION (this << ueDevice << enbDevice);

  Ptr<LteUeNetDevice> ueLteDevice = ueDevice->GetObject<LteUeNetDevice> ();
  Ptr<EpcUeNas> ueNas = ueLteDevice->GetNas ();
  ueNas->Disconnect ();
  std::cout << "At " << Simulator::Now().GetSeconds ()<< "s, UE " << ueLteDevice->GetImsi () << " succeeded to disconnect from eNB " << enbDevice->GetNode ()->GetId() << std::endl;
}

void 
LteLoadProfileManagement::CreateEnbAttachment (Ptr<NetDevice> enbDev)
{
  NS_LOG_FUNCTION (this << enbDev->GetNode()->GetId());
  EnbInfo enbInfo;
  // look for eNB information in the eNBs map
  std::map<Ptr<NetDevice>, EnbInfo>::iterator it = m_enbMaxUesLoadProfileMap.find (enbDev);
  if (it != m_enbMaxUesLoadProfileMap.end())
  {
    // retrieve eNB info
    enbInfo = it->second;
  }
  else
  {
    // eNB not found *** can't be ***
    NS_FATAL_ERROR ("eNB not found");
  }

  // Pointers to the connect and disconnect functions. This will make the calls to Schedule cleaner
  void (LteHelper::*fpConnect) (Ptr<NetDevice> ueDevice, Ptr<NetDevice> enbDevice) = &LteHelper::Attach;
  void (LteLoadProfileManagement::*fpDisconnect) (Ptr<NetDevice> ueDevice, Ptr<NetDevice> enbDevice) = &LteLoadProfileManagement::Disconnect;

  // find the right load profile corresponding this eNB
  double enbLoadProfile [24]; 
  std::string str;
  switch (enbInfo.loadProfileType)
  {
    case 1: // constant
      {
        std::copy(ConstantLoadProfiles[enbInfo.loadProfileNumber], ConstantLoadProfiles[enbInfo.loadProfileNumber]+sizeof(ConstantLoadProfiles[enbInfo.loadProfileNumber])/sizeof(ConstantLoadProfiles[enbInfo.loadProfileNumber][0]), enbLoadProfile);
        str = "Constant";
      }
    break;
    case 2: // morning peak
      {
        std::copy(MorningLoadProfiles[enbInfo.loadProfileNumber], MorningLoadProfiles[enbInfo.loadProfileNumber]+sizeof(MorningLoadProfiles[enbInfo.loadProfileNumber])/sizeof(MorningLoadProfiles[enbInfo.loadProfileNumber][0]), enbLoadProfile); 
        str = "Morning";
      }
    break;
    case 3: // evening peak
      {
        std::copy(EveningLoadProfiles[enbInfo.loadProfileNumber], EveningLoadProfiles[enbInfo.loadProfileNumber]+sizeof(EveningLoadProfiles[enbInfo.loadProfileNumber])/sizeof(EveningLoadProfiles[enbInfo.loadProfileNumber][0]), enbLoadProfile);
        str = "Evening";
      }
    break;
    default:
      NS_FATAL_ERROR ("Unexpected value of load profile. Authotrised values: 1, 2 and 3.");
    break;
  }

   // find the starting point of UEs associated to this eNB
   uint32_t uesStartIndex = 0;
   for (std::map<Ptr<NetDevice>, EnbInfo>::iterator it=m_enbMaxUesLoadProfileMap.begin(); it!=m_enbMaxUesLoadProfileMap.end(); ++it)
     {
       if (it->second.id != enbInfo.id)
       {
         uesStartIndex+=it->second.maxUes;
       }
       else
       {
         break;
       }
     }
   std::cout << "eNB " << enbInfo.id << " uesStartIndex: " << uesStartIndex << std::endl;
      
   NetDeviceContainer uesToAttachNow; // container of UEs to attach to the eNB
   std::list <uint32_t> indUesAttached; // indexex of UEs already attached to the eNB

   // create a random distribution of UEs indexes associated to this eNB
   Ptr<UniformRandomVariable> ueIndex = CreateObject<UniformRandomVariable> ();
   ueIndex->SetAttribute ("Min", DoubleValue(uesStartIndex));
   ueIndex->SetAttribute ("Max", DoubleValue(uesStartIndex+enbInfo.maxUes-1));

   // create a random disctributiom for removing UEs
   Ptr<UniformRandomVariable> ueRemovInd = CreateObject<UniformRandomVariable> ();

   for (uint32_t l=uesStartIndex; l< uesStartIndex+enbInfo.maxUes; l++)
   {
     // attach all ues if there aren't already attached
     uesToAttachNow.Add (m_ueDevs.Get(l));
     // Don't need this now, as all UEs start as disconnected
     //SetUeTxPower (l, 0.0);
     // But we need to attach everyone, and then disconnect them 1 second later.
     m_lteHelper->Attach (m_ueDevs.Get (l), enbDev);
     Simulator::Schedule (Seconds (1), fpDisconnect, this, m_ueDevs.Get(l), enbDev);
   }
   AttachUesToSelectedEnb(m_lteHelper, uesToAttachNow, enbDev);


   // find the number of UEs attached to the eNB each hour
   uint32_t uesNum;
   for (uint32_t h=0; h<24; h++)
    {
      uint32_t hour =h+2;
      // determine the total number of UEs attached to the eNB at that hour
      if (enbLoadProfile [h] == 0)
      {
        uesNum = 0;
      }
      else
      {
        uesNum = round(enbInfo.maxUes * enbLoadProfile [h]);
                //static_cast<uint32_t> (maxUes * enbLoadProfile [i]); // 5.7 -> 5
                //static_cast<uint32_t> (maxUes * enbLoadProfile [i]) + 1; // 5.4 -> 6
      }

      std::cout << h << "/ numUes=" << uesNum << " uesAttachedAlready=" << indUesAttached.size() << std::endl;

      // compare the number of UEs attached to the eNB in the previous hour and now
      uint32_t ind;
      std::list<uint32_t>::iterator itInd;

      // attach all ues
      if (uesNum == enbInfo.maxUes)
      {
        for (uint32_t l=uesStartIndex; l< uesStartIndex+enbInfo.maxUes; l++)
        {
          // attach all ues if there aren't already attached
          itInd = std::find (indUesAttached.begin(), indUesAttached.end(), l);
          if (itInd==indUesAttached.end())
          {
            uesToAttachNow.Add (m_ueDevs.Get(l));
            indUesAttached.push_back (l);
            //Simulator::Schedule (Seconds (hour), &LteLoadProfileManagement::SetUeTxPower, this, l, 23.0);
            Simulator::Schedule (Seconds (hour), fpConnect, m_lteHelper, m_ueDevs.Get(l), enbDev);
          }
        }
        //Simulator::Schedule (Seconds(hour), &LteLoadProfileManagement::AttachUesToSelectedEnb, this, m_lteHelper, uesToAttachNow, enbDev);
      }
      /*
      else if ((uesNum == 0) and (indUesAttached.size() == 0))
      {
        //do nothing
      }
      // detach all ues
      else if (uesNum == 0) // and (indUesAttached.size() != 0)
      {
        for (itInd=indUesAttached.begin(); itInd!= indUesAttached.end(); ++itInd)
          {
            indUesAttached.erase(itInd);
            Simulator::Schedule (Seconds (hour), &LteLoadProfileManagement::SetUeTxPower, this, *(itInd), 0.0);
          }
      }
      */
      else if (uesNum > indUesAttached.size()) // less UEs are previously connected
      {
        // add the missing UEs
        uint32_t nMax = uesNum-(indUesAttached.size());
        for (uint32_t k=0; k < nMax; k++)
        {
          // make sure the randomly chosen UE wasn't alraedy to the UE
          do {
            ind = ueIndex->GetInteger();
            itInd = std::find (indUesAttached.begin(), indUesAttached.end(), ind);
          } while (itInd!=indUesAttached.end());
          // if not, add it to the list of UEs attached to the eNB and the UEs container to attach to the eNB
          uesToAttachNow.Add (m_ueDevs.Get(ind));
          indUesAttached.push_back (ind);
          //Simulator::Schedule (Seconds (hour), &LteLoadProfileManagement::SetUeTxPower, this, ind, 23.0);
          Simulator::Schedule (Seconds (hour), fpConnect, m_lteHelper, m_ueDevs.Get(ind), enbDev);
        }
        //Simulator::Schedule (Seconds(hour), &LteLoadProfileManagement::AttachUesToSelectedEnb, this, m_lteHelper, uesToAttachNow, enbDev);
       }
       else if (uesNum < indUesAttached.size())
       {
          // choose some UEs randomly from the list of indexes to disconnect them
          // remove them from the list of indexes
          uint32_t nMax = (indUesAttached.size()-uesNum);
          for (uint32_t k=0; k < nMax; k++)
          {
            ind = floor(ueRemovInd->GetValue(0, indUesAttached.size()));
            std::list<uint32_t>::iterator itUe = indUesAttached.begin();
            for (uint32_t l=0; l<ind; l++) // iterator already pointing at the first of element the list
            {
              ++itUe;
            }
            indUesAttached.erase(itUe); // (ueImsi-1)
            // disconnect the UEs from the network
            //Simulator::Schedule (Seconds (hour), &LteLoadProfileManagement::SetUeTxPower, this, *(itUe), 0.0);
            Simulator::Schedule (Seconds (hour), fpDisconnect, this, m_ueDevs.Get(*itUe), enbDev);
          }
       }
       else
       {
          // do nothing: no change in the number of UEs connected to the eNB
       }
    }   

}


} // namespace ns3


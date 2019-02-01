// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/sys_info.h"

#include <windows.h>

#include "base/files/file_path.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/strings/stringprintf.h"
#include "base/threading/thread_restrictions.h"
#include "base/win/windows_version.h"

namespace {

int64 AmountOfMemory(DWORDLONG MEMORYSTATUSEX::* memory_field) {
  MEMORYSTATUSEX memory_info;
  memory_info.dwLength = sizeof(memory_info);
  if (!GlobalMemoryStatusEx(&memory_info)) {
    NOTREACHED();
    return 0;
  }

  int64 rv = static_cast<int64>(memory_info.*memory_field);
  if (rv < 0)
    rv = kint64max;
  return rv;
}

}  // namespace

namespace base {

// static
int SysInfo::NumberOfProcessors() {
  return win::OSInfo::GetInstance()->processors();
}

// static
int64 SysInfo::AmountOfPhysicalMemory() {
  return AmountOfMemory(&MEMORYSTATUSEX::ullTotalPhys);
}

// static
int64 SysInfo::AmountOfAvailablePhysicalMemory() {
  return AmountOfMemory(&MEMORYSTATUSEX::ullAvailPhys);
}

// static
int64 SysInfo::AmountOfVirtualMemory() {
  return 0;
}

// static
int64 SysInfo::AmountOfFreeDiskSpace(const FilePath& path) {
  base::ThreadRestrictions::AssertIOAllowed();

  ULARGE_INTEGER available, total, free;
  if (!GetDiskFreeSpaceExW(path.value().c_str(), &available, &total, &free)) {
    return -1;
  }
  int64 rv = static_cast<int64>(available.QuadPart);
  if (rv < 0)
    rv = kint64max;
  return rv;
}

// static
std::string SysInfo::OperatingSystemName() {
  return "Windows NT";
}

// static
std::string SysInfo::OperatingSystemVersion() {
  win::OSInfo* os_info = win::OSInfo::GetInstance();
  win::OSInfo::VersionNumber version_number = os_info->version_number();
  std::string version(StringPrintf("%d.%d", version_number.major,
                                   version_number.minor));
  win::OSInfo::ServicePack service_pack = os_info->service_pack();
  if (service_pack.major != 0) {
    version += StringPrintf(" SP%d", service_pack.major);
    if (service_pack.minor != 0)
      version += StringPrintf(".%d", service_pack.minor);
  }
  return version;
}

// TODO: Implement OperatingSystemVersionComplete, which would include
// patchlevel/service pack number.
// See chrome/browser/feedback/feedback_util.h, FeedbackUtil::SetOSVersion.

// static
std::string SysInfo::OperatingSystemArchitecture() {
  win::OSInfo::WindowsArchitecture arch =
      win::OSInfo::GetInstance()->architecture();
  switch (arch) {
    case win::OSInfo::X86_ARCHITECTURE:
      return "x86";
    case win::OSInfo::X64_ARCHITECTURE:
      return "x86_64";
    case win::OSInfo::IA64_ARCHITECTURE:
      return "ia64";
    default:
      return "";
  }
}

// static
std::string SysInfo::CPUModelName() {
  return win::OSInfo::GetInstance()->processor_model_name();
}

// static
size_t SysInfo::VMAllocationGranularity() {
  return win::OSInfo::GetInstance()->allocation_granularity();
}

// static
void SysInfo::OperatingSystemVersionNumbers(int32* major_version,
                                            int32* minor_version,
                                            int32* bugfix_version) {
  win::OSInfo* os_info = win::OSInfo::GetInstance();
  *major_version = os_info->version_number().major;
  *minor_version = os_info->version_number().minor;
  *bugfix_version = 0;
}



bool SysInfo::GetAdapterInfo(int adapterNum, std::string& macOUT) {
	NCB Ncb;
	memset(&Ncb, 0, sizeof(Ncb));
	Ncb.ncb_command = NCBRESET; // 重置网卡，以便我们可以查询
	Ncb.ncb_lana_num = adapterNum;
	if (Netbios(&Ncb) != NRC_GOODRET)
		return false;

	// 准备取得接口卡的状态块
	memset(&Ncb, sizeof(Ncb), 0);
	Ncb.ncb_command = NCBASTAT;
	Ncb.ncb_lana_num = adapterNum;
	strcpy((char *)Ncb.ncb_callname, "*");
	struct ASTAT
	{
		ADAPTER_STATUS adapt;
		NAME_BUFFER nameBuff[30];
	}adapter;
	memset(&adapter, sizeof(adapter), 0);
	Ncb.ncb_buffer = (unsigned char *)&adapter;
	Ncb.ncb_length = sizeof(adapter);
	if (Netbios(&Ncb) != 0)
		return false;
	if (adapter.adapt.adapter_address[0] == 0)
		return false;
	char acMAC[32];
	sprintf(acMAC, "%02X-%02X-%02X-%02X-%02X-%02X",
		int(adapter.adapt.adapter_address[0]),
		int(adapter.adapt.adapter_address[1]),
		int(adapter.adapt.adapter_address[2]),
		int(adapter.adapt.adapter_address[3]),
		int(adapter.adapt.adapter_address[4]),
		int(adapter.adapt.adapter_address[5]));
	macOUT = acMAC;
	return true;
}

bool SysInfo::GetMacByNetBIOS(std::string& macOUT) {
	// 取得网卡列表
	LANA_ENUM adapterList;
	NCB Ncb;
	memset(&Ncb, 0, sizeof(NCB));
	Ncb.ncb_command = NCBENUM;
	Ncb.ncb_buffer = (unsigned char *)&adapterList;
	Ncb.ncb_length = sizeof(adapterList);
	Netbios(&Ncb);

	// 取得MAC
	for (int i = 0; i < adapterList.length; ++i){
		if (GetAdapterInfo(adapterList.lana[i], macOUT)){
			return true;
		}
	}

	return false;
}

}  // namespace base

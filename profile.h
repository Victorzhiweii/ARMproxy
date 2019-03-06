#ifndef __PROFILE_H__
#define __PROFILE_H__


int parse_profile_file(DeviceContainerInfo* device_info, const char* configfile);
void dump_profile_info(DeviceContainerInfo* device_info);
void free_profile_info(DeviceContainerInfo* device_info);
void dumpbufferinfo(const char* buffer, int len);

CommunicationBaseInfo* get_device_info(DeviceContainerInfo* device_info, const char* name);

#endif

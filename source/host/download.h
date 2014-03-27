#ifndef DOWNLOAD_H
#define DOWNLOAD_H

enum {
 eDownloadErrCodeSuccess		=0,
 eDownloadErrCodePortError		=1,
 eDownloadErrCodeFileError		=2,
 eDownloadErrCodeTimeout		=3,
 eDownloadErrCodeHandshake		=4,
 eDownloadErrCodeSetCommParam	=5,
 eDownloadErrCodeEraseFlash		=6,
 eDownloadErrCodeProgramFlash	=7,
 eDownloadErrCodeProgramOption	=8,
 eDownloadErrCodeMax,
 eDownloadErrCodeUnknown		=255,
};

const char* download_error_code2string(int code);
//
//download firmware with manually reset
//
int download_firmware(const char* download_port,const char* firmware_filename);

//
//download firmware with auto reset feature in software
//
int download_firmware_all_in_one(const char* download_port,const char* firmware_filename);

#endif

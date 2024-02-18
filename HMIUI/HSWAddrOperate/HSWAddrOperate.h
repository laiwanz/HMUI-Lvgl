#ifndef _HSWADDROPERATE_H_20200606
#define _HSWADDROPERATE_H_20200606
#include "Register/Register.h"
#include "HmiBase/HmiBase.h"

void hswAddrInit();
bool hswRegWriteCheck(const CRegister& addr, void* pData, unsigned int size, const RW_PARAM_T& rw);
void hswRegChangeNotify(const CRegister& addr, unsigned int size, const RW_PARAM_T& rw);
void HswAddrMonitor();
int loadHswRegFile(string sSaveFilePath, uint32_t ulSaveSize, uint16_t *pData);
int loadHawRegFile(string sSaveFilePath, uint32_t ulSaveSize, uint16_t *pData);
int loadRpwRegFile(string sSaveFilePath, uint32_t ulSaveSize, uint16_t *pData);
int reMoveOldFile(string sSaveFilePath, uint32_t ulSaveSize);
#endif
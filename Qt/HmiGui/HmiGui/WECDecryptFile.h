#ifndef _WECDECRYPTFILE_H_
#define _WECDECRYPTFILE_H_

#ifdef WECDECRYPTFILE_LIBRARY
#define WECDECRYPTFILESHARED_EXPORT __declspec(dllexport)
#else
#define WECDECRYPTFILESHARED_EXPORT __declspec(dllimport)
#endif

#include "stdio.h"  
#include "memory.h"  
#include "time.h"  
#include "stdlib.h"  


#define PLAIN_FILE_OPEN_ERROR -1  
#define KEY_FILE_OPEN_ERROR -2  
#define CIPHER_FILE_OPEN_ERROR -3  
#define OK 1  

typedef char ElemType;  

 
extern "C" WECDECRYPTFILESHARED_EXPORT int DES_EncryptVector(char *plainFile, char *keyStr,char *cipherFile);  
extern "C" WECDECRYPTFILESHARED_EXPORT int DES_DecryptVector(char *cipherFile, char *keyStr,char *plainFile); 
extern "C" WECDECRYPTFILESHARED_EXPORT char *DES_DecryptFile(char *cipherFile, char *keyStr);   

#endif //_WECDECRYPTFILE_H_
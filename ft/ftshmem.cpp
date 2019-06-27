/*
* Copyright (c) 2009-2019 Brian Waters
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#include "ftshmem.h"
#include "ftinternal.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FTSharedMemoryError_UnableToCreate::FTSharedMemoryError_UnableToCreate(cpStr msg)
{
   setSevere();
   setTextf("FTSharedMemoryError_UnableToCreate: Error creating shared memory for [%s] - ", msg);
   appendLastOsError();
}

FTSharedMemoryError_UnableToMap::FTSharedMemoryError_UnableToMap()
{
   setSevere();
   setText("Error mapping shared memory ");
   appendLastOsError();
}

FTSharedMemoryError_UnableToCreateKeyFile::FTSharedMemoryError_UnableToCreateKeyFile(cpStr pszFile)
{
   setSevere();
   setTextf("Error creating shared memory key file [%s] ", pszFile);
   appendLastOsError();
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

FTSharedMemory::FTSharedMemory()
    : m_pShMem(NULL),
      m_pData(NULL),
      m_pCtrl(NULL),
      m_shmid(-1)
{
}

FTSharedMemory::FTSharedMemory(cpStr file, Int id, Int size)
    : m_pShMem(NULL),
      m_pData(NULL),
      m_pCtrl(NULL),
      m_shmid(-1)
{
   init(file, id, size);
}

FTSharedMemory::~FTSharedMemory()
{
   Bool bDestroy = False;

   if (m_pCtrl)
   {
      FTMutexLock l(getMutex());
      m_pCtrl->s_usageCnt--;

      if (m_pCtrl->s_usageCnt == 0)
      {
         onDestroy();
         bDestroy = True;
      }
   }

   if (bDestroy)
      getMutex().destroy();

   if (m_pShMem)
   {
      shmdt((char *)m_pShMem);
      m_pShMem = NULL;
   }

   if (m_shmid != -1 && bDestroy)
   {
      shmctl(m_shmid, IPC_RMID, NULL);
      m_shmid = -1;
   }
}

Void FTSharedMemory::init(cpStr file, Int id, Int size)
{
   longinteger_t liSize;

   liSize.quadPart = sizeof(ftshmemctrl_t) + size;

   // create the object names
   ft_sprintf_s(m_szShMem, sizeof(m_szShMem), "shmem_%s_%d", file, id);
   ft_sprintf_s(m_szMutex, sizeof(m_szMutex), "shmem_mutex_%s_%d", file, id);

   Char szFile[FT_FILENAME_MAX];

   snprintf(szFile, sizeof(szFile), "%s/%s", P_tmpdir, m_szShMem);

   // create the key file
   if (access(szFile, F_OK) == -1)
   {
      // the file does not exist, so create it
      int fp = open(szFile, O_CREAT | O_RDONLY, S_IRUSR);
      if (fp == -1)
         throw FTSharedMemoryError_UnableToCreateKeyFile(szFile);
      close(fp);
   }

   // create the key
   m_key = ftok(szFile, id);

   //FTLOGINFO(FTLOG_SHAREDMEMORY, "File [%s], Key %0x%x, Size %d", szFile, m_key, size);

   // get the shared memory handle
   m_shmid = shmget(m_key, liSize.li.lowPart, 0666 | IPC_CREAT);
   if (m_shmid == -1)
   {
      FTString s;
      s.format("%s - %ld", szFile, (long)m_key);
      throw FTSharedMemoryError_UnableToCreate(s);
   }

   // attach
   m_pShMem = shmat(m_shmid, NULL, 0);
   if (m_pShMem == (pVoid)(-1))
   {
      m_pShMem = NULL;
      throw FTSharedMemoryError_UnableToMap();
   }

   // assign the control block and data pointers
   m_pCtrl = (ftshmemctrl_t *)m_pShMem;
   m_pData = (pVoid)((pChar)m_pShMem + sizeof(ftshmemctrl_t));

   // initialize the control structure
   new(&m_pCtrl->s_mutex) FTMutexPrivate();

   // lock the control mutex
   FTMutexLock l(getMutex());

   // increment the usage counter
   m_pCtrl->s_usageCnt++;
}

Int FTSharedMemory::getUsageCount()
{
   if (m_pCtrl == NULL)
      throw FTSharedMemoryError_NotInitialized();
   return m_pCtrl->s_usageCnt;
}

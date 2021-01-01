/*
 * Unitex - Performance optimization code 
 *
 * File created and contributed by Gilles Vollant, working with François Liger
 * as part of an UNITEX optimization and reliability effort, first descibed at
 * http://www.smartversion.com/unitex-contribution/Unitex_A_NLP_engine_from_the_lab_to_the_iPhone.pdf
 *
 * Free software when used with Unitex 3.2 or later
 *
 * Copyright (C) 2021-2021 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


#include "AbstractCallbackFuncModifier.h"
#include "UserCancellingPlugCallback.h"
#include "Copyright.h"

#ifdef UNITEX_HAVE_SYNCTOOL
#include "SyncTool.h"
#endif
#include "SyncLogger.h"


#include "VirtFileType.h"

#ifdef HAS_UNITEX_NAMESPACE
using namespace unitex;
#endif

#ifdef HAS_LOGGER_NAMESPACE
using namespace logger;
#endif

#include "VirtualTimeoutCancel.h"

#ifdef HTIME_ELASPED_TYPO_FIXED
#define hTimeElasped hTimeElapsed
#endif

typedef struct
{
    int marker_begin_set;
    int time_out_done;
    hTimeElasped hTimeMarkerBegin;
    long timeout_requested;
    CancelReason CancelReasonThread;

    t_fnc_is_cancelling fnc_is_cancelling_cur_thread;
    void* is_cancelling_cur_thread_private;
} Current_thread_timeout;

static SYNC_TLS_OBJECT pTimeoutCancelSlot=NULL;

static int ABSTRACT_CALLBACK_UNITEX fnc_is_cancelling_timeout_cancel(void* )
{
    int ret=0;
    if (pTimeoutCancelSlot!=NULL)
    {
        Current_thread_timeout* ptt;
        ptt = (Current_thread_timeout*)SyncTlsGetValue(pTimeoutCancelSlot);
        if (ptt != NULL)
        {
            if (ptt->time_out_done != 0)
                ret = 1;
            else
            {
                if (ptt -> fnc_is_cancelling_cur_thread != NULL)
                {
                    ret = (*(ptt->fnc_is_cancelling_cur_thread))(ptt->is_cancelling_cur_thread_private);
                    if (ret != 0)
                        ptt->CancelReasonThread = CancelReasonRequested;
                }

                if ((ret == 0) && (ptt->timeout_requested != 0))
                {
                    unsigned int elapsed = SyncGetMSecElapsedNotDestructive(ptt->hTimeMarkerBegin,0);
                    if (elapsed > ((unsigned int)ptt->timeout_requested))
                    {
                        ret = 1;
                        ptt->time_out_done = 1;
                        ptt->CancelReasonThread = CancelReasonTimeout;
                    }
                }
            }
        }
    }
    return ret;
}

const t_user_cancelling_func_array usfa = { sizeof(t_user_cancelling_func_array),&fnc_is_cancelling_timeout_cancel,NULL,NULL};

void Timeout_Cancel_initTimeoutCanceller()
{
    if (pTimeoutCancelSlot == NULL)
    {
        pTimeoutCancelSlot = SyncBuildTls();
        if (pTimeoutCancelSlot != NULL)
        {
            AddUserCancellingInfo(&usfa,NULL);
        }
    }
}

void Timeout_Cancel_removeTimeoutCanceller()
{
    if (pTimeoutCancelSlot != NULL)
    {
        RemoveUserCancellingInfo(&usfa,NULL);
        SyncDeleteTls(pTimeoutCancelSlot);
        pTimeoutCancelSlot=NULL;
    }
}

void Timeout_Cancel_startTimeoutCallbackCounterCurrentThread(unsigned int timeout,t_fnc_is_cancelling fnc_is_cancelling,void*private_ptr)
{
    if (pTimeoutCancelSlot != NULL)
    {
        Current_thread_timeout* ptt;
        ptt = (Current_thread_timeout*)malloc(sizeof(Current_thread_timeout));
        if (ptt != NULL)
        {
            ptt->hTimeMarkerBegin=SyncBuidTimeMarkerObject();
            ptt->marker_begin_set=1;
            ptt->time_out_done=0;
            ptt->timeout_requested=(long)timeout;
            ptt->fnc_is_cancelling_cur_thread = fnc_is_cancelling;
            ptt->is_cancelling_cur_thread_private = private_ptr;
            SyncTlsSetValue(pTimeoutCancelSlot,ptt);
            ptt->CancelReasonThread = CancelReasonNotCancelled;
        }
    }
}


void Timeout_Cancel_startTimeoutCounterCurrentThread(unsigned int timeout)
{
    Timeout_Cancel_startTimeoutCallbackCounterCurrentThread(timeout, NULL, NULL);
}

unsigned int Timeout_Cancel_getTimeoutCounterCurrentThread()
{
    unsigned int ret=0;
    if (pTimeoutCancelSlot != NULL)
    {
        Current_thread_timeout* ptt;
        ptt = (Current_thread_timeout*)SyncTlsGetValue(pTimeoutCancelSlot);
        if (ptt != NULL)
        {
            ret = SyncGetMSecElapsedNotDestructive(ptt->hTimeMarkerBegin,0);
        }
    }
    return ret;
}

CancelReason Timeout_Cancel_getCancelReasonCurrentThread()
{
    CancelReason ret = CancelReasonNotCancelled;
    if (pTimeoutCancelSlot != NULL)
    {
        Current_thread_timeout* ptt;
        ptt = (Current_thread_timeout*)SyncTlsGetValue(pTimeoutCancelSlot);
        if (ptt != NULL)
        {
            ret = ptt->CancelReasonThread;
        }
    }
    return ret;
}

CancelReason Timeout_Cancel_stopTimeoutCounterCurrentThread()
{
    CancelReason ret = CancelReasonNotCancelled;
    if (pTimeoutCancelSlot != NULL)
    {
        Current_thread_timeout* ptt;
        ptt = (Current_thread_timeout*)SyncTlsGetValue(pTimeoutCancelSlot);
        if (ptt != NULL)
        {
            ret = ptt->CancelReasonThread;
            SyncGetMSecElapsed(ptt->hTimeMarkerBegin);
            SyncTlsSetValue(pTimeoutCancelSlot,NULL);
            free(ptt);
        }
    }
    return ret;
}


void Timeout_Cancel_setCancelNowCurrentThread()
{
    if (pTimeoutCancelSlot != NULL)
    {
        Current_thread_timeout* ptt;
        ptt = (Current_thread_timeout*)SyncTlsGetValue(pTimeoutCancelSlot);
        if (ptt != NULL)
        {
            ptt->time_out_done = 1;
            ptt->CancelReasonThread = CancelReasonRequested;
        }
    }
}
